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
  std::vector<int> fact_offset;
  std::vector< std::vector<var_value_t> > mutex_groups;
  std::vector<var_value_t> goal;
  mrw::Actions actions;
  mrw::Parse(filename, initial, fact_offset, mutex_groups, goal,
                  &actions);
  mrw::GraphSchema schema;
  mrw::InitializeSchema(fact_offset, goal, actions, &schema);
  mrw::PlanningGraph graph;
  mrw::InitializeGraph(fact_offset, schema, &graph);
  auto result = Search(initial, fact_offset, actions, schema, &graph);
  std::reverse(result.begin(), result.end());
  std::cout << "plan" << std::endl;
  for (auto o : result) {
    if (o == -1) {
      std::cerr << "faild to solve problem" << std::endl;
      exit(0);
    }
    std::cout << actions.names[o] << std::endl;
  }

  std::cout << "test" << std::endl;
  std::vector<int> facts(fact_offset.back(), 0);
  for (int i=0; i<initial.size(); ++i)
    facts[fact_offset[i]+initial[i]] = 1;
  for (int i=0; i<result.size(); ++i) {
    int o = result[i];
    for (auto f : actions.preconditions[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      if (facts[fact_offset[var]+value] == 0) {
        std::cerr << "layer" << i << " precondition var" << var << "=" << value
                  << " is not satisfied for action " << actions.names[o]
                  << std::endl;
        exit(1);
      }
    }
    for (auto f : actions.effects[o]) {
      int var, value;
      mrw::DecodeVarValue(f, &var, &value);
      facts[fact_offset[var]+value] = 1;
    }
  }
  for (auto g : goal) {
    int var, value;
    mrw::DecodeVarValue(g, &var, &value);
    if (facts[fact_offset[var]+value] == 0) {
      std::cerr << "goal var" << var << "=" << value << " is not satisfied"
                << std::endl;
      exit(1);
    }
  }
  std::cout << "OK!" << std::endl;
}
