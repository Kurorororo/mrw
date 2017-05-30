#include "parser.h"

#include <iostream>
#include <string>
#include <vector>

#include "data.h"

int main(int argc, char *argv[]) {
  using mrw::var_value_t;
  if (argc != 2) {
    std::cerr << "Usage: test_parser <filename>" << std::endl;
    exit(1);
  }
  std::string filename = argv[1];
  std::vector<int> initial;
  mrw::Domain domain;
  mrw::Parse(filename, initial, &domain);

  std::cout << "initial" << std::endl;
  for (size_t i=0; i<initial.size(); ++i) {
    std::cout << "var" << i << "=" << initial[i] << std::endl;
  }
  std::cout << "fact offset" << std::endl;
  for (size_t i=0; i<domain.fact_offset.size(); ++i) {
    std::cout << "var" << i << "=0: " << domain.fact_offset[i] << std::endl;
  }
  std::cout << "mutex_groups" << std::endl;
  for (int i=0; i<domain.mutex_groups.size(); ++i) {
    for (auto v : domain.mutex_groups[i]) {
      int var, value;
      mrw::DecodeVarValue(v, &var, &value);
      std::cout << "var" << var << "=" << value << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << "goal" << std::endl;
  for (auto v : domain.goal) {
    int var, value;
    mrw::DecodeVarValue(v, &var, &value);
    std::cout << "var" << var << " = " << value << std::endl;
  }
  std::cout << "operators" << std::endl;
  for (int i=0; i<domain.names.size(); ++i) {
    std::cout << domain.names[i] << std::endl;
    std::cout << "cost = " << domain.costs[i] << std::endl;
    std::cout << "precondition" << std::endl;
    for (auto v : domain.preconditions[i]) {
      int var, value;
      mrw::DecodeVarValue(v, &var, &value);
      std::cout << "var" << var << "=" << value << ", ";
    }
    std::cout << std::endl;
    std::cout << "effect" << std::endl;
    for (auto v : domain.effects[i]) {
      int var, value;
      mrw::DecodeVarValue(v, &var, &value);
      std::cout << "var" << var <<  "=" << value << ", ";
    }
    std::cout << std::endl;
  }
}
