#include "graphplan.h"

#include <cassert>

#include <vector>

#include "data.h"

using std::vector;

namespace mrw {

void InitializeSchema(const Domain &domain, GraphSchema *schema) {
  schema->goal_size = domain.goal.size();
  size_t fact_size = static_cast<size_t>(domain.fact_offset.back());
  schema->is_goal.resize(fact_size);
  std::fill(schema->is_goal.begin(), schema->is_goal.end(), 0);
  for (auto v : domain.goal) {
    int var, value;
    DecodeVarValue(v, &var, &value);
    int f = domain.fact_offset[var] + value;
    schema->is_goal[f] = 1;
    schema->goal_facts.push_back(f);
  }

  schema->precondition_map.resize(fact_size);
  schema->effect_map.resize(fact_size);
  size_t action_size = domain.preconditions.size();
  schema->precondition_size.resize(action_size);
  for (int i=0; i<action_size; ++i) {
    schema->precondition_size[i] = domain.preconditions[i].size();
    for (auto v : domain.preconditions[i]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      schema->precondition_map[domain.fact_offset[var]+value].push_back(i);
    }
    for (auto v : domain.effects[i]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      schema->effect_map[domain.fact_offset[var]+value].push_back(i);
    }
  }
}

void InitializeGraph(const Domain &domain, const GraphSchema &schema,
                     PlanningGraph *graph) {
  size_t fact_size = static_cast<size_t>(domain.fact_offset.back());
  graph->fact_layer_membership.resize(fact_size);
  graph->closed.resize(fact_size);
  graph->marked[0].resize(fact_size);
  graph->marked[1].resize(fact_size);
  size_t action_size = schema.precondition_size.size();
  graph->precondition_counter.resize(action_size);
  graph->action_layer_membership.resize(action_size);
}

void ResetGraph(PlanningGraph *graph) {
  graph->n_layers = 0;
  graph->goal_counter = 0;
  std::fill(graph->fact_layer_membership.begin(),
            graph->fact_layer_membership.end(), -1);
  std::fill(graph->action_layer_membership.begin(),
            graph->action_layer_membership.end(), -1);
  std::fill(graph->closed.begin(), graph->closed.end(), 0);
  std::fill(graph->precondition_counter.begin(),
            graph->precondition_counter.end(), 0);
  graph->scheduled_facts.clear();
  graph->scheduled_actions.clear();
  for (int i=0, n=graph->g_set.size(); i<n; ++i)
    graph->g_set.clear();
}

int FactLayer(const GraphSchema &schema, PlanningGraph *graph) {
  while (!graph->scheduled_facts.empty()) {
    int f = graph->scheduled_facts.back();
    graph->scheduled_facts.pop_back();
    graph->fact_layer_membership[f] = graph->n_layers;
    if (schema.is_goal[f] == 1 && ++graph->goal_counter == schema.goal_size)
      return 1;
    for (auto o : schema.precondition_map[f]) {
      if (++graph->precondition_counter[o] == schema.precondition_size[o])
        graph->scheduled_actions.push_back(o);
    }
  }
  return 0;
}

void ActionLayer(const Domain &domain, const GraphSchema &schema,
                 PlanningGraph *graph)  {
  while (!graph->scheduled_actions.empty()) {
    int o = graph->scheduled_actions.back();
    graph->scheduled_actions.pop_back();
    graph->action_layer_membership[o] = graph->n_layers;
    for (auto v : domain.effects[o]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      int f = domain.fact_offset[var] + value;
      if (graph->closed[f] == 0) {
        graph->closed[f] = 1;
        graph->scheduled_facts.push_back(f);
      }
    }
  }
}

void ConstructGraph(const vector<int> &initial, const Domain &domain,
                    const GraphSchema &schema, PlanningGraph *graph) {
  ResetGraph(graph);
  for (size_t i=0, n=initial.size(); i<n; ++i) {
    int f = domain.fact_offset[i] + initial[i];
    graph->closed[f] = 1;
    graph->scheduled_facts.push_back(f);
  }
  while (!graph->scheduled_facts.empty()) {
    int is_end = FactLayer(schema, graph);
    if (is_end == 1) {
      ++graph->n_layers;
      return;
    }
    ActionLayer(domain, schema, graph);
    ++graph->n_layers;
  }
  graph->n_layers = -1;
}

int ChooseAction(int index, int i, const Domain &domain,
                 const GraphSchema &schema, const PlanningGraph &graph) {
  int min = -1;
  int argmin = 0;
  for (auto o : schema.effect_map[index]) {
    if (graph.action_layer_membership[o] != i-1) continue;
    int difficulty = 0;
    for (auto p : domain.preconditions[o]) {
      int var, value;
      DecodeVarValue(p, &var, &value);
      difficulty += graph.fact_layer_membership[domain.fact_offset[var]+value];
    }
    if (difficulty < min || min == -1) {
      min = difficulty;
      argmin = o;
    }
  }
  assert(-1 != min);
  return argmin;
}

vector<int> ExtractPlan(const Domain &domain, const GraphSchema &schema,
                        PlanningGraph *graph, vector<int> &helpful_actions) {
  vector<int> result;
  graph->g_set.resize(graph->n_layers);
  for (auto g : schema.goal_facts)
    graph->g_set[graph->fact_layer_membership[g]].push_back(g);
  int m = graph->n_layers - 1;
  std::fill(graph->marked[m%2].begin(), graph->marked[m%2].end(), 0);
  for (int i=m; i>0; --i) {
    std::fill(graph->marked[(i+1)%2].begin(), graph->marked[(i+1)%2].end(), 0);
    for (auto g : graph->g_set[i]) {
      if (graph->marked[i%2][g] == 1) continue;
      int o = ChooseAction(g, i, domain, schema, *graph);
      for (auto v : domain.preconditions[o]) {
        int var, value;
        DecodeVarValue(v, &var, &value);
        int f = domain.fact_offset[var] + value;
        int j = graph->fact_layer_membership[f];
        if (j != 0 && graph->marked[(i+1)%2][f] == 0)
          graph->g_set[j].push_back(f);
      }
      for (auto v : domain.effects[o]) {
        int var, value;
        DecodeVarValue(v, &var, &value);
        int f = domain.fact_offset[var] + value;
        graph->marked[i%2][f] = 1;
        graph->marked[(i+1)%2][f] = 1;
      }
      result.push_back(o);
    }
  }
  if (graph->g_set.size() > 1) {
    for (auto g : graph->g_set[1]) {
      for (auto o : schema.effect_map[g]) {
        if (graph->action_layer_membership[o] == 0)
          helpful_actions.push_back(o);
      }
    }
  }
  return result;
}

vector<int> Search(const vector<int> &initial, const Domain &domain,
                   const GraphSchema &schema, PlanningGraph *graph,
                   vector<int> &helpful_actions) {
  ConstructGraph(initial, domain, schema, graph);
  if (graph->n_layers == -1)
    return std::vector<int>{-1};
  return ExtractPlan(domain, schema, graph, helpful_actions);
}

} // namespace mrw
