#include "data.h"

#include <cassert>

#include <iostream>

int main() {
  using mrw::var_value_t;
  using mrw::EncodeVarValue;
  using mrw::DecodeVarValue;

  int var = 5;
  int value = 12;
  var_value_t var_value;
  EncodeVarValue(var, value, &var_value);
  DecodeVarValue(var_value, &var, &value);
  assert(5 == var);
  assert(12 == value);
  std::cout << "passed" << std::endl;
}
