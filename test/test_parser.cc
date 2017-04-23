#include "parser.h"

#include <iostream>
#include <string>
#include <vector>

#include "data.h"

int main(int argc, char *argv[]) {
  using planning::var_value_t;
  if (argc != 2) {
    std::cerr << "Usage: test_parser <filename>" << std::endl;
    exit(1);
  }
  std::string filename = argv[1];
  std::vector<int> initial;
  std::vector<int> fact_offset;
  std::vector< std::vector<var_value_t> > mutex_groups;
  std::vector<var_value_t> goal;
  planning::Actions actions;
  planning::Parse(filename, initial, fact_offset, mutex_groups, goal,
                  &actions);

  std::cout << "initial" << std::endl;
  for (int i=0; i<initial.size(); ++i) {
    std::cout << "var" << i << "=" << initial[i] << std::endl;
  }
  std::cout << "fact offset" << std::endl;
  for (int i=0; i<fact_offset.size(); ++i) {
    std::cout << "var" << i << "=0: " << fact_offset[i] << std::endl;
  }
  std::cout << "mutex_groups" << std::endl;
  for (int i=0; i<mutex_groups.size(); ++i) {
    for (auto v : mutex_groups[i]) {
      int var, value;
      planning::DecodeVarValue(v, &var, &value);
      std::cout << "var" << var << "=" << value << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << "goal" << std::endl;
  for (auto v : goal) {
    int var, value;
    planning::DecodeVarValue(v, &var, &value);
    std::cout << "var" << var << " = " << value << std::endl;
  }
  std::cout << "operators" << std::endl;
  for (int i=0; i<actions.names.size(); ++i) {
    std::cout << actions.names[i] << std::endl;
    std::cout << "cost = " << actions.costs[i] << std::endl;
    std::cout << "precondition" << std::endl;
    for (auto v : actions.preconditions[i]) {
      int var, value;
      planning::DecodeVarValue(v, &var, &value);
      std::cout << "var" << var << "=" << value << ", ";
    }
    std::cout << std::endl;
    std::cout << "effect" << std::endl;
    for (auto v : actions.effects[i]) {
      int var, value;
      planning::DecodeVarValue(v, &var, &value);
      std::cout << "var" << var <<  "=" << value << ", ";
    }
    std::cout << std::endl;
  }
}
