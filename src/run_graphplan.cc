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
  mrw::Domain domain;
  mrw::Parse(filename, initial, &domain);
  mrw::GraphSchema schema;
  mrw::InitializeSchema(domain, &schema);
  mrw::PlanningGraph graph;
  mrw::InitializeGraph(domain, schema, &graph);
  std::vector<int> helpful_actions;
  auto result = Search(initial, domain, schema, &graph, helpful_actions);
  std::cout << "Plan" << std::endl;
  for (size_t i=result.size()-1; i>-1; --i) {
    if (result[i] == -1) {
      std::cout << "faild to solve problem." << std::endl;
      exit(0);
    }
    std::cout << domain.names[result[i]] << std::endl;
  }
  std::cout << "Helpful actinos" << std::endl;
  for (auto o : helpful_actions)
    std::cout << domain.names[o] << std::endl;
}
