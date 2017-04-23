#include "ff.h"

#include <vector>

#include "data.h"

namespace mrw {

int FF(const std::vector<int> &variables, const std::vector<int> &fact_offset,
       const std::vector<var_value_t> &goal, const Actions &actions,
       const GraphSchema &schema, PlanningGraph *graph,
       std::vector<int> &helpful_actions) {
  auto result = Search(variables, fact_offset, actions, schema, graph,
                       helpful_actions);
  int sum = 0;
  for (auto v : result) {
    if (v == -1) {
      return -1;
    }
    sum += actions.costs[v];
  }
  return sum;
}

} // mrw
