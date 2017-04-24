#include "mrw.h"

#include <cmath>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "data.h"
#include "ff.h"
#include "graphplan.h"
#include "trie.h"

using std::vector;

namespace mrw {

constexpr double kAlpha = 0.9;
constexpr int kNumWalk = 2000;
constexpr int kLengthWalk = 10;
constexpr int kMDALengthWalk = 1;
constexpr int kExtendingPeriod = 300;
constexpr double kExtendingRate = 1.5;
constexpr double kMDAExtendingRate = 2.0;
constexpr int kMaxSteps = 7;
constexpr double kMDATau = 0.5;
constexpr double kMHATau = 10.0;

GraphSchema schema;
PlanningGraph graph;
vector<double> q_mda;
vector<double> q_mha;
vector<int> success;
vector<int> faild;

int total_walks = 0;
int faild_walks = 0;
int total_branches = 0;
int total_actions = 0;

int generated = 0;
int evaluated = 0;

inline void PrintNewHeuristicValue(int min_h, int g) {
  std::cout << "New best heuristic value: " << min_h << std::endl;
  std::cout << "[g=" << g << ", " << evaluated << " evaluated, "
            << generated << " generated]" << std::endl;
}

inline void PrintStopWalk(int i) {
  std::cout << "Exploration stopped " << i << " random walks" << std::endl;
}

inline void PrintLengthWalk(int length_walk) {
  std::cout << "New length of random walk: " << length_walk << std::endl;
}

inline bool UpdatePAP(int i, int h_min_old, int h_min, double *p, double *ap) {
  if (h_min == -1)
    *p = 0.0;
  else
    *p = std::max(0.0, static_cast<double>(h_min_old - h_min));
  if (i == 0) *ap = *p;
  bool result = *p > *ap;
  *ap = (1.0-kAlpha)*(*ap) + kAlpha*(*p);
  return result;
}

inline int ExtendLengthWalk(double extending_rate, int length_walk) {
  return static_cast<int>(extending_rate * static_cast<double>(length_walk));
}

inline void UpdateMinimum(int h, vector<int> &s_prime, vector<int> &footprints,
                          int *h_min, vector<int> &s_min,
                          vector<int> &best_sequence) {
  *h_min = h;
  s_min = std::move(s_prime);
  best_sequence = std::move(footprints);
}

inline void UpdateState(vector<int> &s_prime, vector<int> &footprints,
                        vector<int> &s, vector<int> &sequence) {
  s = std::move(s_prime);
  sequence.insert(sequence.end(), footprints.begin(), footprints.end());
}

void UpdateMDA(int h, const vector<int> &footprints) {
  if (h == -1) {
    for (auto v : footprints)
      faild[v] += 1;
  } else {
    for (auto v : footprints)
      success[v] += 1;
  }
  for (int j=0, n=q_mda.size(); j<n; ++j) {
    q_mda[j] = static_cast<double>(success[j] + faild[j]);
    if (success[j] + faild[j] != 0)
      q_mda[j] = 0.0 - (static_cast<double>(faild[j]) / q_mda[j]);
  }
}

int PureRandomWalk(int h_min_old, const vector<int> &fact_offset,
                   const vector<var_value_t> &goal, const Actions &actions,
                   const TrieTable &table, vector<int> &s,
                   vector<int> &sequence) {
  int length_walk = kLengthWalk;
  PrintLengthWalk(length_walk);
  int h_min = -1;
  vector<int> s_min;
  vector<int> best_sequence;
  int counter = 0;
  double p, ap;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  for (int i=0; i<kNumWalk; ++i) {
    ++total_walks;
    auto s_prime = s;
    vector<int> footprints;
    if (counter > kExtendingPeriod) {
      length_walk = ExtendLengthWalk(kExtendingRate, length_walk);
      PrintLengthWalk(length_walk);
      counter = 0;
    }
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, fact_offset);
      if (a_set.empty()) {
        ++faild_walks;
        break;
      }
      total_branches += a_set.size();
      ++total_actions;
      std::uniform_int_distribution<> dist(0, a_set.size()-1);
      int a = a_set[dist(engine)];
      ApplyEffect(actions.effects[a], s_prime);
      footprints.push_back(a);
      ++generated;
      if (GoalCheck(goal, s_prime)) {
        UpdateState(s_prime, footprints, s, sequence);
        return 0;
      }
    }
    vector<int> helpful_actions;
    int h = FF(s_prime, fact_offset, goal, actions, schema, &graph,
               helpful_actions);
    ++evaluated;
    for (auto a : helpful_actions)
      q_mha[a] += 1.0;
    UpdateMDA(h, footprints);
    if ((h < h_min || h_min == -1) && h != -1) {
      UpdateMinimum(h, s_prime, footprints, &h_min, s_min, best_sequence);
      counter = 0;
    } else {
      ++counter;
    }
    if (!UpdatePAP(i, h_min_old, h_min, &p, &ap)) continue;
    PrintStopWalk(i+1);
    UpdateState(s_min, best_sequence, s, sequence);
    return h_min;
  }
  PrintStopWalk(kNumWalk);
  if (h_min == -1) return h_min_old;
  UpdateState(s_min, best_sequence, s, sequence);
  return h_min;
}

int GibbsSampling(const vector<int> &v, const vector<double> &q, double tau,
                  double value) {
  int n = v.size();
  vector<double> p(n);
  double sum = 0.0;
  for (int i=0; i<n; ++i) {
    p[i] = exp(q[v[i]]/tau);
    sum += p[i];
  }
  for (int i=0; i<n; ++i) {
    value -= p[i] / sum;
    if (value <= 0.0) return v[i];
  }
  return v.back();
}

int MDARandomWalk(int h_min_old, const vector<int> &fact_offset,
                  const vector<var_value_t> &goal, const Actions &actions,
                  const TrieTable &table, vector<int> &s,
                  vector<int> &sequence) {
  int length_walk = kMDALengthWalk;
  PrintLengthWalk(length_walk);
  int h_min = -1;
  vector<int> s_min;
  vector<int> best_sequence;
  int counter = 0;
  double p, ap;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  std::uniform_real_distribution<> dist(0.0, 1.0);
  for (int i=0; i<kNumWalk; ++i) {
    auto s_prime = s;
    vector<int> footprints;
    if (counter > kExtendingPeriod) {
      length_walk = ExtendLengthWalk(kMDAExtendingRate, length_walk);
      PrintLengthWalk(length_walk);
      counter = 0;
    }
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, fact_offset);
      if (a_set.empty()) break;
      int a = GibbsSampling(a_set, q_mda, kMDATau, dist(engine));
      ApplyEffect(actions.effects[a], s_prime);
      footprints.push_back(a);
      ++generated;
      if (GoalCheck(goal, s_prime)) {
        UpdateState(s_prime, footprints, s, sequence);
        return 0;
      }
    }
    vector<int> helpful_actions;
    int h = FF(s_prime, fact_offset, goal, actions, schema, &graph,
               helpful_actions);
    ++evaluated;
    UpdateMDA(h, footprints);
    if ((h < h_min || h_min == -1) && h != -1) {
      UpdateMinimum(h, s_prime, footprints, &h_min, s_min, best_sequence);
      counter = 0;
    } else {
      ++counter;
    }
    if (!UpdatePAP(i, h_min_old, h_min, &p, &ap)) continue;
    PrintStopWalk(i+1);
    UpdateState(s_min, best_sequence, s, sequence);
    return h_min;
  }
  PrintStopWalk(kNumWalk);
  if (h_min == -1) return h_min_old;
  UpdateState(s_min, best_sequence, s, sequence);
  return h_min;
}

int MHARandomWalk(int h_min_old, const vector<int> &fact_offset,
                  const vector<var_value_t> &goal, const Actions &actions,
                  const TrieTable &table, vector<int> &s,
                  vector<int> &sequence) {
  int length_walk = kLengthWalk;
  PrintLengthWalk(length_walk);
  int h_min = -1;
  vector<int> s_min;
  vector<int> best_sequence;
  int counter = 0;
  double p, ap;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  std::uniform_real_distribution<> dist(0.0, 1.0);
  for (int i=0; i<kNumWalk; ++i) {
    auto s_prime = s;
    vector<int> footprints;
    if (counter > kExtendingPeriod) {
      length_walk = ExtendLengthWalk(kExtendingRate, length_walk);
      PrintLengthWalk(length_walk);
      counter = 0;
    }
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, fact_offset);
      if (a_set.empty()) break;
      int a = GibbsSampling(a_set, q_mha, kMHATau, dist(engine));
      ApplyEffect(actions.effects[a], s_prime);
      footprints.push_back(a);
      ++generated;
      if (GoalCheck(goal, s_prime)) {
        UpdateState(s_prime, footprints, s, sequence);
        return 0;
      }
    }
    vector<int> helpful_actions;
    int h = FF(s_prime, fact_offset, goal, actions, schema, &graph,
               helpful_actions);
    ++evaluated;
    for (auto a : helpful_actions)
      q_mha[a] += 1.0;
    if ((h < h_min || h_min == -1) && h != -1) {
      UpdateMinimum(h, s_prime, footprints, &h_min, s_min, best_sequence);
      counter = 0;
    } else {
      ++counter;
    }
    if (!UpdatePAP(i, h_min_old, h_min, &p, &ap)) continue;
    PrintStopWalk(i+1);
    UpdateState(s_min, best_sequence, s, sequence);
    return h_min;
  }
  PrintStopWalk(kNumWalk);
  if (h_min == -1) return h_min_old;
  UpdateState(s_min, best_sequence, s, sequence);
  return h_min;
}

vector<int> MRW(const vector<int> &initial, const vector<int> &fact_offset,
                const vector<var_value_t> &goal, const Actions &actions,
                const TrieTable &table) {
  InitializeSchema(fact_offset, goal, actions, &schema);
  InitializeGraph(fact_offset, schema, &graph);
  int n_actions = actions.names.size();
  q_mda.resize(n_actions);
  std::fill(q_mda.begin(), q_mda.end(), 0.0);
  q_mha.resize(n_actions);
  std::fill(q_mha.begin(), q_mha.end(), 0.0);
  success.resize(n_actions);
  std::fill(success.begin(), success.end(), 0.0);
  faild.resize(n_actions);
  std::fill(faild.begin(), faild.end(), 0.0);

  vector<int> s = initial;
  ++generated;
  vector<int> helpful_actions;
  int initial_h_min = FF(s, fact_offset, goal, actions, schema, &graph,
                         helpful_actions);
  ++evaluated;
  if (initial_h_min == -1) return vector<int>{-1};
  vector<int> sequence;
  int h_min = initial_h_min;
  PrintNewHeuristicValue(h_min, sequence.size());
  int counter = 0;
  double fail_rate = 0.0;
  double branching_factor = 0.0;
  while (!GoalCheck(goal, s)) {
    if (counter > kMaxSteps || FindFromTable(table, s, fact_offset).empty()) {
      s = initial;
      h_min = initial_h_min;
      sequence.clear();
      counter = 0;
      std::fill(q_mda.begin(), q_mda.end(), 0.0);
      std::fill(q_mha.begin(), q_mha.end(), 0.0);
      std::fill(success.begin(), success.end(), 0.0);
      std::fill(faild.begin(), faild.end(), 0.0);
      std::cout << "Restart" << std::endl;
      PrintNewHeuristicValue(h_min, sequence.size());
    }
    int h;
    if (branching_factor > 1000.0) {
      h = MHARandomWalk(h_min, fact_offset, goal, actions, table, s, sequence);
    } else if (fail_rate > 0.5) {
      h = MDARandomWalk(h_min, fact_offset, goal, actions, table, s, sequence);
    } else {
      h = PureRandomWalk(h_min, fact_offset, goal, actions, table, s,
                         sequence);
      fail_rate = static_cast<double>(faild_walks)
                  / static_cast<double>(total_walks);
      branching_factor = static_cast<double>(total_branches)
                         / static_cast<double>(total_actions);
      if (branching_factor > 1000.0)
        std::cout << "Too many brances. Use MHA" << std::endl;
      if (fail_rate > 0.5)
        std::cout << "Too many dead end. Use MDA" << std::endl;
    }
    if (h < h_min) {
      h_min = h;
      counter = 0;
      PrintNewHeuristicValue(h_min, sequence.size());
    } else {
      ++counter;
    }
  }
  return sequence;
}

} // namespace mrw
