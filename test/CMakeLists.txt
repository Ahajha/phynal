add_executable(tests)
target_link_libraries(tests PRIVATE phynal)
target_sources(tests PRIVATE
    invoke/test_cpp_from_py.cpp
)

python3_add_library(module1)
target_link_libraries(module1 PRIVATE phynal)
target_sources(module1 PRIVATE
    modules/mod1.cpp
)
# This macro should be defined as the name of the resulting
# library name, not necessarily the target name.
target_compile_definitions(module1 PRIVATE
    PHYNAL_MODULE_NAME=module1
)