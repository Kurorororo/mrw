#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "mrw.h"
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

  auto chrono_start = std::chrono::system_clock::now();
  mrw::Parse(filename, initial, fact_offset, mutex_groups, goal, &actions);
  auto table = mrw::ConstructTable(actions.preconditions, fact_offset);
  auto result = mrw::MRW(initial, fact_offset, goal, actions, table);
  auto chrono_end = std::chrono::system_clock::now();

  if (!result.empty() && result[0] == -1) {
    std::cout << "faild to solve problem" << std::endl;
    exit(0);
  }
  std::cout << "Solution found!" << std::endl;

  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
     chrono_end - chrono_start).count();
  double search_time = static_cast<double>(ns) / 1e9;
  std::cout << "Acutual search time: " << search_time << "s" << std::endl;

  int step = result.size();
  int cost = 0;
  for (auto a : result) {
    std::cout << actions.names[a] << "(" << actions.costs[a] << ")"
              << std::endl;
    cost += actions.costs[a];
  }

  std::cout << "Plan length: " << step << " step(s)" << std::endl;
  std::cout << "Plan cost: " << cost << std::endl;
  std::cout << "Evaluated " << mrw::evaluated << " state(s)" << std::endl;
  std::cout << "Generated " << mrw::generated << " state(s)" << std::endl;
  std::cout << "Search time: " << search_time << "s" << std::endl;
  std::cout << "Solution found." << std::endl;
}
