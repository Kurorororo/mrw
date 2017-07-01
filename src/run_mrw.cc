#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "mrw.h"
#include "parser.h"
#include "trie.h"

using mrw::var_value_t;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: mrw output.sass" << std::endl;
    exit(1);
  }
  std::string filename = argv[1];
  std::vector<int> initial;
  mrw::Domain domain;

  auto chrono_start = std::chrono::system_clock::now();
  mrw::Parse(filename, initial, &domain);
  auto table = mrw::ConstructTable(domain.preconditions, domain.fact_offset);
  auto result = mrw::MRW(initial, domain, table);
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

  std::ofstream sas_plan;
  sas_plan.open("sas_plan", std::ios::out);
  int cost = 0;
  for (auto a : result) {
    std::cout << domain.names[a] << "(" << domain.costs[a] << ")"
              << std::endl;
    sas_plan << "(" << domain.names[a] << ")" << std::endl;
    cost += domain.costs[a];
  }
  double nps = static_cast<double>(mrw::generated) / search_time;

  std::cout << "Plan length: " << step << " step(s)" << std::endl;
  std::cout << "Plan cost: " << cost << std::endl;
  std::cout << "Evaluated " << mrw::evaluated << " state(s)" << std::endl;
  std::cout << "Generated " << mrw::generated << " state(s)" << std::endl;
  std::cout << "Search time: " << search_time << "s" << std::endl;
  std::cout << "Nodes per seconds: " << nps << std::endl;
  std::cout << "Solution found." << std::endl;
}
