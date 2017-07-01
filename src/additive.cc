#include "additive.h"

#include <iostream>
#include <vector>

#include "data.h"

namespace mrw {

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

void BellmanFord(const std::vector<int> &variables, const Domain &domain,
                 const std::vector< std::vector<int> > &effect_map,
                 std::vector<int> &table) {
  int fact_size = domain.fact_offset.back();
  std::fill(table.begin(), table.end(), -1);
  for (size_t i=0, n=variables.size(); i<n; ++i)
    table[domain.fact_offset[i]+variables[i]] = 0;
  for (int i=0; i<fact_size; ++i) {
    int converge = 0;
    for (int g=0; g<fact_size; ++g) {
      int min = Next(domain, effect_map, table, g);
      if (table[g] == min) ++converge;
      table[g] = min;
    }
    if (converge == fact_size) break;
  }
}

int Additive(const std::vector<int> &variables, const Domain &domain,
             const std::vector< std::vector<int> > &effect_map,
             std::vector<int> &table) {
  BellmanFord(variables, domain, effect_map, table);
  int h = Cost(domain, table, domain.goal);
  if (h == -1) return std::numeric_limits<int>::max();
  return h;
}

} // namespace mrw
