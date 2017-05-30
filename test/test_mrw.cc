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
  mrw::Domain domain;

  mrw::Parse(filename, initial, &domain);

  auto table = mrw::ConstructTable(domain.preconditions, domain.fact_offset);
  auto result = mrw::MRW(initial, domain, table);

  if (!result.empty() && result[0] == -1) {
    std::cout << "faild to solve problem" << std::endl;
    exit(0);
  }
  std::cout << "Plan" << std::endl;

  for (auto a : result)
    std::cout << domain.names[a] << std::endl;

  std::cout << "test" << std::endl;

  auto variables = initial;
  for (int i=0; i<result.size(); ++i) {
    int o = result[i];
    for (auto f : domain.preconditions[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      if (variables[var] != value) {
        std::cerr << "layer" << i << " precondition var" << var << "=" << value
                  << " is not satisfied for action " << domain.names[o]
                  << std::endl;
        exit(1);
      }
    }
    for (auto f : domain.effects[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      variables[var] = value;
    }
  }
  for (auto g : domain.goal) {
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
