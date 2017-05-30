#ifndef GRAPHPLAN_H_
#define GRAPHPLAN_H_

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "data.h"

namespace mrw {

struct GraphSchema {
  int goal_size;
  std::vector<int> is_goal;
  std::vector<int> goal_facts;
  std::vector<int> precondition_size;
  std::vector< std::vector<int> > precondition_map;
  std::vector< std::vector<int> > effect_map;
};

struct PlanningGraph {
  int n_layers;
  int goal_counter;
  std::vector<int> fact_layer_membership;
  std::vector<int> action_layer_membership;
  std::vector<int> precondition_counter;
  std::vector<int> closed;
  std::vector<int> scheduled_facts;
  std::vector<int> scheduled_actions;
  std::vector< std::vector<int> > g_set;
  std::array<std::vector<int>, 2> marked;
};

void InitializeSchema(const Domain &domain, GraphSchema *schema);

void InitializeGraph(const Domain &domain, const GraphSchema &schema,
                     PlanningGraph *graph);

std::vector<int> Search(const std::vector<int> &initial,
                        const Domain &domain, const GraphSchema &schema,
                        PlanningGraph *graph,
                        std::vector<int> &helpful_actions);

} // namespace mrw

#endif // GRAPHPLAN_H_
