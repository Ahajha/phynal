#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <tl/optional.hpp>

#include <concepts>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace phynal {

template <class T>
  requires std::same_as<std::remove_cvref_t<T>, T>
struct cast_result {
  static auto make_ref(T& ref) noexcept -> cast_result {
    return cast_result(std::in_place_type_t<T*>(), &ref);
  }

  static auto make_cref(const T& cref) noexcept -> cast_result {
    return cast_result(std::in_place_type_t<const T*>(), &cref);
  }

  static auto make_value(T value) noexcept -> cast_result {
    return cast_result(std::in_place_type_t<T>(), std::move(value));
  }

  static auto make_error() noexcept -> cast_result {
    return cast_result{std::in_place_type_t<std::monostate>()};
  }

  auto is_error() const noexcept -> bool {
    return std::holds_alternative<std::monostate>(m_state);
  }

  template <typename AsType>
    requires std::same_as<std::remove_cvref_t<AsType>, T>
  auto as() && noexcept -> tl::optional<AsType> { // TODO expected instead
    // At least for now, we only support 3 variations of the type.
    // We might want/need to support pointers later, as well as const T&&.
    static_assert(std::same_as<T, AsType> || std::same_as<T&, AsType> ||
                  std::same_as<const T&, AsType>);

    // Manual std::visit + overload
    switch (m_state.index()) {
    case 0: {
      // monostate -> Error
      return {}; // Error converting from Python to C++ (error already set)
    }
    case 1: {
      // T*
      if constexpr (std::is_reference_v<AsType>) {
        return *std::get<1>(m_state);
      } else { // Value requested
        if constexpr (std::copy_constructible<T>) {
          return *std::get<1>(m_state);
        } else {
          return {}; // Copy requested, but type is not copyable
          // Note: If the given Python value had a refcount of 1, this should be
          // stolen earlier as a T.
        }
      }
    }
    case 2: {
      // const T*
      // Same as the T* case, except that we don't allow conversion to T&
      if constexpr (std::is_reference_v<AsType>) {
        if constexpr (std::same_as<AsType, T&>) {
          return {}; // Cannot convert const reference to mutable reference
        } else {
          return *std::get<2>(m_state);
        }
      } else { // Value requested
        if constexpr (std::copy_constructible<T>) {
          return *std::get<2>(m_state);
        } else {
          return {}; // Copy requested, but type is not copyable
          // Note: If the given Python value had a refcount of 1, this should be
          // stolen earlier as a T.
        }
      }
    }
    case 3: {
      // T
      if constexpr (std::is_reference_v<AsType>) {
        return std::get<3>(m_state);
      } else { // Value requested
        return std::move(std::get<3>(m_state));
      }
    }
    default:
      std::unreachable();
    }
  }

private:
  std::variant<std::monostate, T*, const T*, T> m_state;

  cast_result(auto&&... args) noexcept
      : m_state{std::forward<decltype(args)>(args)...} {}
};

// Unspecialized base class
template <class>
struct caster;

// We want to do 2 things:
// 1. Take a C++ type and return it to Python
// 2. Take a Python type and return it to C++

// 1. This mostly easy, but a few questions pop up, mainly concerning l/rvalue
//    references. This can be solved with overloads within the type caster with
//    no impact on the API.
// 2. This is difficult. Some type casters will return new values, others will
//    not. Then, sometimes a reference is desired. This can often be provided,
//    but maybe there's an intermediate owned value? There are a few cases,
//    which we will indicate through a combination of categories:
//    - RC1: Python object, refcount == 1. This may enable move opportunities.
//    - RC2+: Python object, refcount > 1. This requires a copy.
//    - Ref: Reference requested.
//    - CRef: Const reference requested.
//    - Value: Value requested.
//    - NewObj: Type caster always constructs a new object.
//    - RefObj: Type caster can utilize the underlying object in some way
//              without constructing something new.
//
//    Then, the various cases:
//    - *, *, NewObj: Doesn't matter, so always return an owned thing.
//                    (All other cases assume RefObj)
//    - *, Ref: Type check to make sure the given value is mutable. Return a
//              reference.
//    - *, CRef: Return a reference.
//               (All remaining cases assume Value)
//    - RC1: Move the object out of the Python object
//    - RC2+: If copyable, copy, otherwise raise an error.

/**
 * @brief Custom type caster for integral values.
 *
 * Always constructs an owned value.
 */
template <std::integral Integral>
struct caster<Integral> {
  static auto to_cpp(PyObject* obj) -> cast_result<Integral> {
    assert(obj != nullptr);

    constexpr auto converter = [] {
      if constexpr (std::signed_integral<Integral>) {
        return PyLong_AsLong;
      } else {
        return PyLong_AsUnsignedLong;
      }
    }();

    auto cast_value = converter(obj);
    if (PyErr_Occurred()) {
      return cast_result<Integral>::make_error();
    }

    Py_DECREF(obj);

    // Error stays set
    if (PyErr_Occurred()) {
      return cast_result<Integral>::make_error();
    }

    if constexpr (std::signed_integral<Integral>) {
      if constexpr (sizeof(Integral) < sizeof(long)) {
        if (cast_value > std::numeric_limits<Integral>::max()) {
          PyErr_SetString(PyExc_RuntimeError, "too big");
          return cast_result<Integral>::make_error();
        }
        if (cast_value < std::numeric_limits<Integral>::min()) {
          PyErr_SetString(PyExc_RuntimeError, "too small");
          return cast_result<Integral>::make_error();
        }
      }
    } else {
      if constexpr (sizeof(Integral) < sizeof(unsigned long)) {
        if (cast_value > std::numeric_limits<Integral>::max()) {
          PyErr_SetString(PyExc_RuntimeError, "too big");
          return cast_result<Integral>::make_error();
        }
      }
    }
    return cast_result<Integral>::make_value(static_cast<Integral>(cast_value));
  }

  static auto to_py(Integral value) -> PyObject* {
    if constexpr (std::signed_integral<Integral>) {
      return PyLong_FromLong(value);
    } else {
      return PyLong_FromUnsignedLong(value);
    }
  }
};

template <std::floating_point Float>
struct caster<Float> {
  //
};

template <>
struct caster<bool> {
  //
};

template <>
struct caster<const char*> {
  //
};

template <>
struct caster<std::string> {
  //
};

template <>
struct caster<std::string_view> {
  //
};

} // namespace phynal