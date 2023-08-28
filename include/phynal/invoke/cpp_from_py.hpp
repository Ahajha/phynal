/**
 * @brief Wraps a C++ function in a CPython callable function
 */

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "phynal/casters/primitives.hpp"

#include <tl/optional.hpp>

#include <concepts>
#include <tuple>
#include <utility>

namespace phynal {

namespace detail {

// template <auto Func, class Ret, class... Args>
// PyObject* call_impl(PyObject* self, PyObject* args) {
//   static_assert(std::invocable<decltype(Func), Args...>);
//
//   return nullptr;
// }

// Unspecialized
template <auto Func, class FuncType>
struct cpp_from_py_impl;

// Specialization for function pointers
// `self` is unused, unlike member function pointers.
template <auto Func, class Ret, class... Args>
struct cpp_from_py_impl<Func, Ret (*)(Args...)> {
  static PyObject* call([[maybe_unused]] PyObject* self, PyObject* args) {
    return call_impl(args, std::make_index_sequence<sizeof...(Args)>());
  }

private:
  template <std::size_t... Indexes>
  static PyObject* call_impl(PyObject* args, std::index_sequence<Indexes...>) {
    std::array<PyObject*, sizeof...(Indexes)> collected_args{nullptr};

    // Not sure about this "name" argument, the docs don't mention what it's
    // for. Best guess is that it's the function name.
    // There seems to be a lot of choice in arg parsing functions, so we can
    // probably try out some other ones.

    auto unpack_result =
        PyArg_UnpackTuple(args, "a_string", sizeof...(Args), sizeof...(Args),
                          &collected_args[Indexes]...);

    assert(unpack_result);

    // Lazy initialize them.
    // We could potentially use single-member unions, except maybe for
    // references. Would make destruction difficult.
    std::tuple<tl::optional<Args>...> cast_args;

    auto parse_arg = []<class Arg>(PyObject* in, tl::optional<Arg>& out) {
      auto cast = caster<Arg>::to_cpp(in).template as<Arg>();

      assert((bool)PyErr_Occurred() == !(bool)cast);

      if (cast) {
        out.emplace(*std::move(cast));
        return true;
      } else {
        return false;
      }
    };

    if (!(... &&
          parse_arg(collected_args[Indexes], std::get<Indexes>(cast_args)))) {
      // Error already set, just return nullptr.
      assert(PyErr_Occurred());
      return nullptr;
    }

    // Actually call the function

    if constexpr (std::is_void_v<Ret>) {
      std::invoke(Func, std::move(std::get<Indexes>(cast_args)).value()...);

      Py_INCREF(Py_None);
      return Py_None;
    } else {
      auto result =
          std::invoke(Func, std::move(std::get<Indexes>(cast_args)).value()...);

      // and then we need to cast the result to Python

      return caster<Ret>::to_py(std::move(result));
    }
  }
};

// Specialization for member function pointers
// `self` ~= `this`
template <auto Func, class Ret, class This, class... Args>
struct cpp_from_py_impl<Func, Ret (This::*)(Args...)> {
  static PyObject* call([[maybe_unused]] PyObject* self,
                        [[maybe_unused]] PyObject* args) {
    //
    return nullptr;
  }
};

} // namespace detail

template <auto Func>
consteval auto cpp_from_py() -> PyObject* (*)(PyObject*, PyObject*) {
  return detail::cpp_from_py_impl<Func, decltype(Func)>::call;
}

} // namespace phynal
