#include <phynal/invoke/cpp_from_py.hpp>
#include <phynal/invoke/py_from_cpp.hpp>

#include <concepts>

int func(int x, int y) { return x + y; }

int main() {
  auto wrapped_func = phynal::cpp_from_py<func>;
  static_assert(std::same_as<decltype(wrapped_func),
                             PyObject* (*)(PyObject*, PyObject*)>);

  auto result = wrapped_func(PyLong_FromLong(5), PyLong_FromLong(5));
}