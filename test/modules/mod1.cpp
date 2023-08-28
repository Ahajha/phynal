#include <phynal/phynal.hpp>

int identity(int x) { return x; }

long widen(int x) { return x; }

int narrow(long x) { return static_cast<int>(x); }

int add(int x, int y) { return x + y; }

void nothing(int) {}

PHYNAL_MODULE(mod) {
  mod.def<identity>("identity", "Returns the argument");
  mod.def<widen>("widen", "Widens the argument from int to long");
  mod.def<narrow>("narrow", "Narrows the argument from long to int");
  mod.def<add>("add", "Adds two numbers");
  mod.def<nothing>("nothing", "Do nothing");
}
