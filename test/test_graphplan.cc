#include "graphplan.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "data.h"
#include "parser.h"

using mrw::var_value_t;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: test_graphplan <filename>" << std::endl;
    exit(1);
  }
  std::string filename = argv[1];
  std::vector<int> initial;
  mrw::Domain domain;
  mrw::Parse(filename, initial, &domain);
  mrw::GraphSchema schema;
  mrw::InitializeSchema(domain, &schema);
  mrw::PlanningGraph graph;
  mrw::InitializeGraph(domain, schema, &graph);
  std::vector<int> helpful_actions;
  auto result = Search(initial, domain, schema, &graph, helpful_actions);
  std::reverse(result.begin(), result.end());
  std::cout << "Plan" << std::endl;
  for (auto o : result) {
    if (o == -1) {
      std::cerr << "faild to solve problem" << std::endl;
      exit(0);
    }
    std::cout << domain.names[o] << std::endl;
  }

  std::cout << "Test" << std::endl;
  std::vector<int> facts(domain.fact_offset.back(), 0);
  for (int i=0; i<initial.size(); ++i)
    facts[domain.fact_offset[i]+initial[i]] = 1;
  for (int i=0; i<result.size(); ++i) {
    int o = result[i];
    for (auto f : domain.preconditions[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      if (facts[domain.fact_offset[var]+value] == 0) {
        std::cerr << "layer" << i << " precondition var" << var << "=" << value
                  << " is not satisfied for action " << domain.names[o]
                  << std::endl;
        exit(1);
      }
    }
    for (auto f : domain.effects[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      facts[domain.fact_offset[var]+value] = 1;
    }
  }
  for (auto g : domain.goal) {
    int var, value;
    mrw::DecodeVarValue(g, &var, &value);
    if (facts[domain.fact_offset[var]+value] == 0) {
      std::cerr << "goal var" << var << "=" << value << " is not satisfied"
                << std::endl;
      exit(1);
    }
  }
  std::cout << "OK!" << std::endl;
}
