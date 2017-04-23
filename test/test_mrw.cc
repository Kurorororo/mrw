#include "mrw.h"

#include <iostream>
#include <string>
#include <vector>

#include "data.h"
#include "parser.h"
#include "trie.h"

using mrw::var_value_t;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: mrw <filename>" << std::endl;
    exit(1);
  }
  std::string filename = argv[1];
  std::vector<int> initial;
  std::vector<int> fact_offset;
  std::vector< std::vector<var_value_t> > mutex_groups;
  std::vector<var_value_t> goal;
  mrw::Actions actions;

  mrw::Parse(filename, initial, fact_offset, mutex_groups, goal, &actions);

  auto table = mrw::ConstructTable(actions.preconditions, fact_offset);
  auto result = mrw::MRW(initial, fact_offset, goal, actions, table);

  if (!result.empty() && result[0] == -1) {
    std::cout << "faild to solve problem" << std::endl;
    exit(0);
  }
  std::cout << "Plan" << std::endl;

  for (auto a : result)
    std::cout << actions.names[a] << std::endl;

  std::cout << "test" << std::endl;

  auto variables = initial;
  for (int i=0; i<result.size(); ++i) {
    int o = result[i];
    for (auto f : actions.preconditions[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      if (variables[var] != value) {
        std::cerr << "layer" << i << " precondition var" << var << "=" << value
                  << " is not satisfied for action " << actions.names[o]
                  << std::endl;
        exit(1);
      }
    }
    for (auto f : actions.effects[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      variables[var] = value;
    }
  }
  for (auto g : goal) {
    int var, value;
    mrw::DecodeVarValue(g, &var, &value);
    if (variables[var] != value) {
      std::cerr << "goal var" << var << "=" << value << " is not satisfied"
                << std::endl;
      exit(1);
    }
  }
  std::cout << "OK!" << std::endl;
}
