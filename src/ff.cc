#include "ff.h"

#include <climits>

#include <vector>

#include "data.h"

namespace mrw {

int FF(const std::vector<int> &variables, const Domain &domain,
       const GraphSchema &schema, PlanningGraph *graph,
       std::vector<int> &helpful_actions) {
  auto result = Search(variables, domain, schema, graph, helpful_actions);
  int sum = 0;
  for (auto v : result) {
    if (v == -1)
      return INT_MAX;
    sum += domain.costs[v];
  }
  return sum;
}

} // mrw
