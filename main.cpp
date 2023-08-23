#include <Python.h>

namespace py {

struct handle {
public:
private:
  PyObject *m_object;
};

struct object : handle {
  //
};

struct maybe_handle {
public:
private:
  handle m_handle;
};

struct maybe_object {
public:
private:
  object m_object;
};

} // namespace py

#include <iostream>

int main() { std::cout << "Hello, World\n"; }