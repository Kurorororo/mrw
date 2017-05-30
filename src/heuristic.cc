#include "heuristic.h"

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

int Cost(const Domain &domain, const std::vector<int> &table,
         const std::vector<var_value_t> &g) {
  int sum = 0;
  for (auto v : g) {
    int var, value;
    DecodeVarValue(v, &var, &value);
    int cost = table[domain.fact_offset[var]+value];
    if (cost == -1) return -1;
    sum += cost;
  }
  return sum;
}

int Next(const Domain &domain,
         const std::vector< std::vector<int> > &effect_map,
         const std::vector<int> &table, int g) {
  int min = table[g];
  for (auto v : effect_map[g]) {
    int cost = Cost(domain, table, domain.preconditions[v]);
    if (cost == -1) continue;
    int next = domain.costs[v] + cost;
    if (next < min || min == -1) min = next;
  }
  return min;
}

int Additive(const std::vector<int> &variables, const Domain &domain,
             const std::vector< std::vector<int> > &effect_map,
             std::array<std::vector<int>, 2> &table) {
  int fact_size = domain.fact_offset.back();
  std::fill(table[0].begin(), table[0].end(), -1);
  for (size_t i=0, n=variables.size(); i<n; ++i) {
    table[0][domain.fact_offset[i]+variables[i]] = 0;
  }
  int i = 0;
  while (true) {
    int j = (i+1) % 2;
    bool is_converged = true;
    for (int g=0; g<fact_size; ++g) {
      table[j][g] = Next(domain, effect_map, table[i], g);
      if (table[j][g] != table[i][g]) is_converged = false;
    }
    if (is_converged) break;
    i = (i+1) % 2;
  }
  int h = Cost(domain, table[0], domain.goal);
  if (h == -1) return INT_MAX;
  return h;
}

} // mrw
