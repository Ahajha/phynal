#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "phynal/invoke/cpp_from_py.hpp"

#include <cstdint>
#include <vector>

// For now, we do not support stateful functions. This is due to us taking
// either a function pointer or member function pointer as a template parameter.

namespace phynal {
/*
struct module_;

template <class T>
struct class_ {
public:
  class_(const char* name, module_& host_mod, const char* doc)
      : m_name{name}
      , m_doc{doc}
      , m_host_module{host_mod} {}

  ~class_() {
    // TODO add class to module
  }
  class_(class_&&) = delete;
  class_(const class_&) = delete;
  class_& operator=(class_&&) = delete;
  class_& operator=(const class_&) = delete;

  template <auto Func>
  module_& def(const char* name, const char* doc) {
    m_methods.push_back(PyMethodDef{
        .ml_name = name,
        .ml_meth = cpp_from_py<Func>(),
        .ml_flags = METH_VARARGS,
        .ml_doc = doc,
    });
    return *this;
  }

private:
  const char* m_name;
  std::vector<PyMethodDef> m_methods;
  const char* m_doc;

  module_& m_host_module;
};
*/

// TODO eventually should derive from object or similar
struct module_ {
  module_(const char* name, const char* doc)
      : m_module_def({
            .m_base = PyModuleDef_HEAD_INIT,
            .m_name = name,
            .m_doc = doc,
            .m_size = -1,
            .m_methods = nullptr,
        }) {}

  ~module_() = default;

  PyObject* process() {
    m_methods.push_back({nullptr, nullptr, 0, nullptr});
    m_module_def.m_methods = m_methods.data();
    m_module = PyModule_Create(&m_module_def);

    return m_module;
  }
  module_(module_&&) = delete;
  module_(const module_&) = delete;
  module_& operator=(module_&&) = delete;
  module_& operator=(const module_&) = delete;

  template <auto Func>
  module_& def(const char* name, const char* doc) {
    m_methods.push_back(PyMethodDef{
        .ml_name = name,
        .ml_meth = cpp_from_py<Func>(),
        .ml_flags = METH_VARARGS,
        .ml_doc = doc,
    });
    return *this;
  }

private:
  // These need to be stored so that the module
  // "lives" as long as this object does.
  PyObject* m_module{nullptr};
  PyModuleDef m_module_def;
  std::vector<PyMethodDef> m_methods;
};

} // namespace phynal

// 3 parts to this:
// 1. Forward declare the helper function (which the user provides the
//    definition of)
// 2. Define the module initialization function, deferring to the helper
// 3. Provide the header for the helper, which will line up with the
//    user-provided definition.

#define PHYNAL_STR(s) #s
#define PHYNAL_PRIV_CONCAT_2(n1, n2) n1##n2
#define PHYNAL_CONCAT_2(n1, n2) PHYNAL_PRIV_CONCAT_2(n1, n2)

#ifdef PHYNAL_MODULE_NAME

#define PHYNAL_MODULE(var_name)                                                \
  void PHYNAL_CONCAT_2(PHYNAL_INIT_HELPER_,                                    \
                       PHYNAL_MODULE_NAME)(phynal::module_&);                  \
  PyMODINIT_FUNC PHYNAL_CONCAT_2(PyInit_, PHYNAL_MODULE_NAME)() {              \
    /* Module needs to remain after the function ends, maybe this should be    \
     * stored in per-interpreter state. */                                     \
    static phynal::module_ mod{PHYNAL_STR(PHYNAL_MODULE_NAME), ""};            \
    /* maybe clear the module, in case of re-init? Maybe revisit later*/       \
    PHYNAL_CONCAT_2(PHYNAL_INIT_HELPER_, PHYNAL_MODULE_NAME)(mod);             \
    return mod.process();                                                      \
  }                                                                            \
  void PHYNAL_CONCAT_2(PHYNAL_INIT_HELPER_,                                    \
                       PHYNAL_MODULE_NAME)(phynal::module_ & var_name)

#endif
