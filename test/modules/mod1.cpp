#include <phynal/phynal.hpp>

template <class T>
T identity(T x) {
  return x;
}

int add(int x, int y) { return x + y; }

void nothing(int) {}

PHYNAL_MODULE(mod) {
  mod.def<identity<int>>("identity", "Returns the argument");
  mod.def<identity<long>>("identity_long",
                          "Returns the argument, which is a C long");
  mod.def<add>("add", "Adds two numbers");
  mod.def<nothing>("nothing", "Do nothing");
}
