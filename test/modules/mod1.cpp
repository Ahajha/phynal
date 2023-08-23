#include <phynal/phynal.hpp>

#include <type_traits>

int add(int x, int y) { return x + y; }

PHYNAL_MODULE(mod) {
  mod.def<add>("add", "Adds two numbers");
  //
}
