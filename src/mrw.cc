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
vector<int> q_mda;
vector<int> q_mha;
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

int PureRandomWalk(int h_min_old, const vector<int> &fact_offset,
                   const vector<var_value_t> &goal, const Actions &actions,
                   const TrieTable &table, vector<int> &s,
                   vector<int> &sequence) {
  int length_walk = kLengthWalk;
  std::cout << "New length of random walk: " << length_walk << std::endl;
  int h_min = -1;
  vector<int> s_min;
  std::vector<int> best_sequence;
  int counter = 0;
  double p, ap;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  for (int i=0; i<kNumWalk; ++i) {
    ++total_walks;
    auto s_prime = s;
    vector<int> footprints;
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, fact_offset);
      if (a_set.empty()) {
        for (auto v : footprints)
          faild[v] += 1;
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
        s = std::move(s_prime);
        sequence.insert(sequence.end(), footprints.begin(), footprints.end());
        return 0;
      }
      if (j == length_walk-1) {
        for (auto v : footprints)
          success[v] += 1;
      }
    }
    for (int j=0, n=q_mda.size(); j<n; ++j) {
      q_mda[j] = success[j] + faild[j];
      if (q_mda[j] != 0) q_mda[j] = 0 - (faild[j] / q_mda[j]);
    }
    vector<int> helpful_actions;
    int h = FF(s_prime, fact_offset, goal, actions, schema, &graph,
               helpful_actions);
    ++evaluated;
    for (auto a : helpful_actions)
      ++q_mha[a];
    if ((h < h_min || h_min == -1) && h != -1) {
      s_min = std::move(s_prime);
      h_min = h;
      best_sequence = std::move(footprints);
      counter = 0;
    } else {
      ++counter;
      if (counter > kExtendingPeriod) {
        length_walk = static_cast<int>(
            kExtendingRate * static_cast<double>(length_walk));
        counter = 0;
        std::cout << "New length of random walk: " << length_walk << std::endl;
      }
    }
    p = std::max(0.0, static_cast<double>(h_min_old - h_min));
    if (i == 0) ap = p;
    if (p > ap) {
      std::cout << "Exploration stopped " << i+1 << " random walks"
                << std::endl;
      s = std::move(s_min);
      sequence.insert(sequence.end(), best_sequence.begin(),
                      best_sequence.end());
      return h_min;
    }
    ap = (1.0-kAlpha)*ap + kAlpha*p;
  }
  std::cout << "Exploration stopped " << kNumWalk << " random walks"
            << std::endl;
  if (h_min == -1) return h_min_old;
  s = std::move(s_min);
  sequence.insert(sequence.end(), best_sequence.begin(), best_sequence.end());
  return h_min;
}

int GibbsSampling(const vector<int> &v, const vector<int> &q, double tau,
                  double value) {
  int n = v.size();
  std::vector<double> p(n);
  double sum = 0.0;
  for (int i=0; i<n; ++i) {
    p[i] = exp(static_cast<double>(q[v[i]])/tau);
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
  std::cout << "New length of random walk: " << length_walk << std::endl;
  int h_min = -1;
  vector<int> s_min;
  std::vector<int> best_sequence;
  int counter = 0;
  double p, ap;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  std::uniform_real_distribution<> dist(0.0, 1.0);
  for (int i=0; i<kNumWalk; ++i) {
    auto s_prime = s;
    vector<int> footprints;
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, fact_offset);
      if (a_set.empty()) {
        for (auto v : footprints)
          faild[v] += 1;
        break;
      }
      int a = GibbsSampling(a_set, q_mda, kMDATau, dist(engine));
      ApplyEffect(actions.effects[a], s_prime);
      footprints.push_back(a);
      ++generated;
      if (GoalCheck(goal, s_prime)) {
        s = std::move(s_prime);
        sequence.insert(sequence.end(), footprints.begin(), footprints.end());
        return 0;
      }
      if (j == length_walk-1) {
        for (auto v : footprints)
          success[v] += 1;
      }
    }
    for (int j=0, n=q_mda.size(); j<n; ++j) {
      q_mda[j] = success[j] + faild[j];
      if (q_mda[j] != 0) q_mda[j] = 0 - (faild[j] / q_mda[j]);
    }
    vector<int> helpful_actions;
    int h = FF(s_prime, fact_offset, goal, actions, schema, &graph,
               helpful_actions);
    ++evaluated;
    if ((h < h_min || h_min == -1) && h != -1) {
      s_min = std::move(s_prime);
      h_min = h;
      best_sequence = std::move(footprints);
      counter = 0;
    } else {
      ++counter;
      if (counter > kExtendingPeriod) {
        length_walk = static_cast<int>(
            kMDAExtendingRate * static_cast<double>(length_walk));
        counter = 0;
        std::cout << "New length of random walk: " << length_walk << std::endl;
      }
    }
    p = std::max(0.0, static_cast<double>(h_min_old - h_min));
    if (i == 0) ap = p;
    if (p > ap) {
      std::cout << "Exploration stopped " << i+1 << " random walks"
                << std::endl;
      s = std::move(s_min);
      sequence.insert(sequence.end(), best_sequence.begin(),
                      best_sequence.end());
      return h_min;
    }
    ap = (1.0-kAlpha)*ap + kAlpha*p;
  }
  std::cout << "Exploration stopped " << kNumWalk << " random walks"
            << std::endl;
  if (h_min == -1) return h_min_old;
  s = std::move(s_min);
  sequence.insert(sequence.end(), best_sequence.begin(), best_sequence.end());
  return h_min;
}

int MHARandomWalk(int h_min_old, const vector<int> &fact_offset,
                  const vector<var_value_t> &goal, const Actions &actions,
                  const TrieTable &table, vector<int> &s,
                  vector<int> &sequence) {
  int length_walk = kLengthWalk;
  std::cout << "New length of random walk: " << length_walk << std::endl;
  int h_min = -1;
  vector<int> s_min;
  std::vector<int> best_sequence;
  int counter = 0;
  double p, ap;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  std::uniform_real_distribution<> dist(0.0, 1.0);
  for (int i=0; i<kNumWalk; ++i) {
    auto s_prime = s;
    vector<int> footprints;
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, fact_offset);
      if (a_set.empty()) break;
      int a = GibbsSampling(a_set, q_mha, kMHATau, dist(engine));
      ApplyEffect(actions.effects[a], s_prime);
      footprints.push_back(a);
      ++generated;
      if (GoalCheck(goal, s_prime)) {
        s = std::move(s_prime);
        sequence.insert(sequence.end(), footprints.begin(), footprints.end());
        return 0;
      }
    }
    vector<int> helpful_actions;
    int h = FF(s_prime, fact_offset, goal, actions, schema, &graph,
               helpful_actions);
    ++evaluated;
    for (auto a : helpful_actions)
      ++q_mha[a];
    if ((h < h_min || h_min == -1) && h != -1) {
      s_min = std::move(s_prime);
      h_min = h;
      best_sequence = std::move(footprints);
      counter = 0;
    } else {
      ++counter;
      if (counter > kExtendingPeriod) {
        length_walk = static_cast<int>(
            kExtendingRate * static_cast<double>(length_walk));
        counter = 0;
        std::cout << "New length of random walk: " << length_walk << std::endl;
      }
    }
    p = std::max(0.0, static_cast<double>(h_min_old - h_min));
    if (i == 0) ap = p;
    if (p > ap) {
      std::cout << "Exploration stopped " << i+1 << " random walks"
                << std::endl;
      s = std::move(s_min);
      sequence.insert(sequence.end(), best_sequence.begin(),
                      best_sequence.end());
      return h_min;
    }
    ap = (1.0-kAlpha)*ap + kAlpha*p;
  }
  std::cout << "Exploration stopped " << kNumWalk << " random walks"
            << std::endl;
  if (h_min == -1) return h_min_old;
  s = std::move(s_min);
  sequence.insert(sequence.end(), best_sequence.begin(), best_sequence.end());
  return h_min;
}

vector<int> MRW(const vector<int> &initial, const vector<int> &fact_offset,
                const vector<var_value_t> &goal, const Actions &actions,
                const TrieTable &table) {
  InitializeSchema(fact_offset, goal, actions, &schema);
  InitializeGraph(fact_offset, schema, &graph);
  q_mda.resize(actions.names.size());
  q_mha.resize(actions.names.size());
  success.resize(actions.names.size());
  faild.resize(actions.names.size());
  vector<int> sequence;

  vector<int> s = initial;
  ++generated;
  vector<int> helpful_actions;
  int initial_h_min = FF(s, fact_offset, goal, actions, schema, &graph,
                         helpful_actions);
  ++evaluated;
  if (initial_h_min == -1) return vector<int>{-1};
  int h_min = initial_h_min;
  int counter = 0;
  double fail_rate = 0.0;
  double branching_factor = 0.0;
  while (!GoalCheck(goal, s)) {
    if (counter > kMaxSteps || FindFromTable(table, s, fact_offset).empty()) {
      s = initial;
      h_min = initial_h_min;
      sequence.clear();
      counter = 0;
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
