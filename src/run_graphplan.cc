#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "data.h"
#include "graphplan.h"
#include "parser.h"

using mrw::var_value_t;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: graphplan <filename>" << std::endl;
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
  std::vector<int> helpful_actions;
  auto result = Search(initial, fact_offset, actions, schema, &graph,
                       helpful_actions);
  std::cout << "Plan" << std::endl;
  for (int i=result.size()-1; i>-1; --i) {
    if (result[i] == -1) {
      std::cout << "faild to solve problem." << std::endl;
      exit(0);
    }
    std::cout << actions.names[result[i]] << std::endl;
  }
  std::cout << "Helpful actinos" << std::endl;
  for (auto o : helpful_actions)
    std::cout << actions.names[o] << std::endl;
}
