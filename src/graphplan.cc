#include "graphplan.h"

#include <cassert>

#include <vector>

#include "data.h"

using std::vector;

namespace mrw {

void InitializeSchema(const vector<int> &fact_offset,
                      const vector<var_value_t> &goal, const Actions &actions,
                      GraphSchema *schema) {
  schema->goal_size = goal.size();
  int n_facts = fact_offset.back();
  schema->is_goal.resize(n_facts);
  std::fill(schema->is_goal.begin(), schema->is_goal.end(), 0);
  for (auto v : goal) {
    int var, value;
    DecodeVarValue(v, &var, &value);
    int f = fact_offset[var] + value;
    schema->is_goal[f] = 1;
    schema->goal_facts.push_back(f);
  }

  schema->precondition_map.resize(n_facts);
  schema->effect_map.resize(n_facts);
  int n_actions = actions.preconditions.size();
  schema->precondition_size.resize(n_actions);
  for (int i=0; i<n_actions; ++i) {
    schema->precondition_size[i] = actions.preconditions[i].size();
    for (auto v : actions.preconditions[i]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      schema->precondition_map[fact_offset[var]+value].push_back(i);
    }
    for (auto v : actions.effects[i]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      schema->effect_map[fact_offset[var]+value].push_back(i);
    }
  }
}

void InitializeGraph(const vector<int> &fact_offset,
                     const GraphSchema &schema, PlanningGraph *graph) {
  int n_facts = fact_offset.back();
  graph->fact_layer_membership.resize(n_facts);
  graph->closed.resize(n_facts);
  graph->marked[0].resize(n_facts);
  graph->marked[1].resize(n_facts);
  int n_actions = schema.precondition_size.size();
  graph->precondition_counter.resize(n_actions);
  graph->action_layer_membership.resize(n_actions);
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

void ActionLayer(const vector<int> &fact_offset,
                 const vector< vector<var_value_t> > &effects,
                 const GraphSchema &schema, PlanningGraph *graph)  {
  while (!graph->scheduled_actions.empty()) {
    int o = graph->scheduled_actions.back();
    graph->scheduled_actions.pop_back();
    graph->action_layer_membership[o] = graph->n_layers;
    for (auto v : effects[o]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      int f = fact_offset[var] + value;
      if (graph->closed[f] == 0) {
        graph->closed[f] = 1;
        graph->scheduled_facts.push_back(f);
      }
    }
  }
}

void ConstructGraph(const vector<int> &initial, const vector<int> &fact_offset,
                    const vector< vector<var_value_t> > &effects,
                    const GraphSchema &schema, PlanningGraph *graph) {
  ResetGraph(graph);
  for (int i=0, n=initial.size(); i<n; ++i) {
    int f = fact_offset[i] + initial[i];
    graph->closed[f] = 1;
    graph->scheduled_facts.push_back(f);
  }
  while (!graph->scheduled_facts.empty()) {
    int is_end = FactLayer(schema, graph);
    if (is_end == 1) {
      ++graph->n_layers;
      return;
    }
    ActionLayer(fact_offset, effects, schema, graph);
    ++graph->n_layers;
  }
  graph->n_layers = -1;
}

int ChooseAction(int index, int i, const vector<int> &fact_offset,
                 const vector< vector<var_value_t> > &preconditions,
                 const GraphSchema &schema, const PlanningGraph &graph) {
  int min = -1;
  int argmin = 0;
  for (auto o : schema.effect_map[index]) {
    if (graph.action_layer_membership[o] != i-1) continue;
    int difficulty = 0;
    for (auto p : preconditions[o]) {
      int var, value;
      DecodeVarValue(p, &var, &value);
      difficulty += graph.fact_layer_membership[fact_offset[var]+value];
    }
    if (difficulty < min || min == -1) {
      min = difficulty;
      argmin = o;
    }
  }
  assert(-1 != min);
  return argmin;
}

vector<int> ExtractPlan(const vector<int> &fact_offset,
                        const Actions &actions, const GraphSchema &schema,
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
      int o = ChooseAction(g, i, fact_offset, actions.preconditions, schema,
                           *graph);
      for (auto v : actions.preconditions[o]) {
        int var, value;
        DecodeVarValue(v, &var, &value);
        int f = fact_offset[var] + value;
        int j = graph->fact_layer_membership[f];
        if (j != 0 && graph->marked[(i+1)%2][f] == 0)
          graph->g_set[j].push_back(f);
      }
      for (auto v : actions.effects[o]) {
        int var, value;
        DecodeVarValue(v, &var, &value);
        int f = fact_offset[var] + value;
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
  return std::move(result);
}

vector<int> Search(const vector<int> &initial, const vector<int> &fact_offset,
                   const Actions &actions, const GraphSchema &schema,
                   PlanningGraph *graph, vector<int> &helpful_actions) {
  ConstructGraph(initial, fact_offset, actions.effects, schema, graph);
  if (graph->n_layers == -1)
    return std::vector<int>{-1};
  return ExtractPlan(fact_offset, actions, schema, graph, helpful_actions);
}

} // namespace mrw
