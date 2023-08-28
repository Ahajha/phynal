#include <phynal/invoke/cpp_from_py.hpp>

#include <concepts>
#include <iostream>

int func(int x, int y) { return x + y; }

std::ostream& operator<<(std::ostream& stream, PyObject* obj) {
  PyObject* repr = PyObject_Repr(obj);
  assert(repr != nullptr);
  PyObject* str = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
  const char* bytes = PyBytes_AS_STRING(str);

  stream << bytes;

  Py_XDECREF(repr);
  Py_XDECREF(str);

  return stream;
}

int main() {
  Py_Initialize();
  auto wrapped_func = phynal::cpp_from_py<func>();
  static_assert(std::same_as<decltype(wrapped_func),
                             PyObject* (*)(PyObject*, PyObject*)>);

  for (int iter = 0; iter < 100; ++iter) {
    auto args = Py_BuildValue("ii", 5, 7);

    assert(args != nullptr);
    assert(!PyErr_Occurred());

    PyObject_Print(args, stdout, 0);

    // Just for testing purposes, get the values back out in the same way as
    // they are done in the wrapper code:

    {
      PyObject *p1 = nullptr, *p2 = nullptr;
      auto test_unpack_result = PyArg_ParseTuple(args, "OO", &p1, &p2);
      PyObject_Print(p1, stdout, 0);
      PyObject_Print(p2, stdout, 0);
      assert(test_unpack_result);
      assert(!PyErr_Occurred());

      assert(p1);
      assert(p2);
      std::cerr << p1 << p2 << '\n';

      auto p1_cast = phynal::caster<int>::to_cpp(p1);
      assert(!p1_cast.is_error());
      auto p1_as = std::move(p1_cast).template as<int>();
      assert(p1_as.has_value());
      assert(p1_as.value() == 5);

      auto p2_cast = phynal::caster<int>::to_cpp(p2);
      assert(!p2_cast.is_error());
      auto p2_as = std::move(p2_cast).template as<int>();
      assert(p2_as.has_value());
      assert(p2_as.value() == 7);
    }

    // Also do it directly with integral values
    {
      int x, y;
      auto test_unpack_result = PyArg_ParseTuple(args, "ii", &x, &y);
      assert(test_unpack_result);
      assert(!PyErr_Occurred());

      assert(x == 5);
      assert(y == 7);
    }

    auto result = wrapped_func(nullptr, args);

    auto cast_result = PyLong_AsLong(result);

    if (PyErr_Occurred()) {
      std::cerr << "Error!";
      PyObject *type, *value, *traceback;
      PyErr_Fetch(&type, &value, &traceback);
      assert(type != nullptr);

      std::cout << value << '\n' << traceback << '\n';

      Py_XDECREF(type);
      Py_XDECREF(value);
      Py_XDECREF(traceback);

      PyErr_Clear();
    } else {
      std::cerr << cast_result << '\n';
    }

    for (int i = 0; i < 10; ++i) {
      auto result2 = PyLong_FromLong(1l << 34);
      assert(!PyErr_Occurred());

      auto cast_back = phynal::caster<long>::to_cpp(result2);
      assert(!PyErr_Occurred());

      // std::cout << std::move(cast_back).as<long>().value() << '\n';
    }

    Py_DECREF(args);
  }

  Py_Finalize();
}