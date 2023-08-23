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

  auto args = Py_BuildValue("ii", 5, 7);

  assert(args != nullptr);

  // Just for testing purposes, get the values back out in the same way as they
  // are done in the wrapper code:
  {
    PyObject *p1, *p2;
    auto test_unpack_result = PyArg_UnpackTuple(args, "idk", 2, 2, &p1, &p2);
    assert(test_unpack_result);
    assert(phynal::caster<int>::to_cpp(p1).template as<int>().value() == 5);
    assert(phynal::caster<int>::to_cpp(p2).template as<int>().value() == 7);
  }

  auto result = wrapped_func(nullptr, args);

  auto cast_result = PyLong_AsLong(result);

  if (PyErr_Occurred()) {
    std::cout << "Error!";
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    assert(type != nullptr);

    std::cout << value << '\n' << traceback << '\n';

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
  } else {
    std::cout << cast_result << '\n';
  }
}