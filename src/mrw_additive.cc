#include "mrw.h"

#include <climits>
#include <cmath>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "data.h"
#include "heuristic.h"
#include "graphplan.h"
#include "trie.h"

using std::vector;

namespace mrw {

constexpr int kPure = 0;
constexpr int kMDA = 1;
constexpr double kAlpha = 0.9;
constexpr int kNumWalk = 2000;
constexpr int kLengthWalk[2] = {10, 1};
constexpr int kExtendingPeriod = 300;
constexpr double kExtendingRate[2] = {1.5, 2.0};
constexpr int kMaxSteps = 7;
constexpr double kTau[2] = {0.0, 0.5};

vector<double> q_mda;
vector<int> success;
vector<int> faild;

int total_walks = 0;
int faild_walks = 0;
bool is_initial_walk = true;
double p;
double ap;

std::vector< std::vector<int> > effect_map;
std::array<std::vector<int>, 2> additive_table;

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

void InitializeEffectMap(const Domain &domain,
                         std::vector< std::vector<int> > &effect_map) {
  size_t fact_size = static_cast<size_t>(domain.fact_offset.back());
  effect_map.resize(fact_size);
  size_t action_size = domain.preconditions.size();
  for (size_t i=0; i<action_size; ++i) {
    for (auto v : domain.effects[i]) {
      int var, value;
      DecodeVarValue(v, &var, &value);
      effect_map[domain.fact_offset[var]+value].push_back(i);
    }
  }
}

inline bool UpdatePAP(int h_min, int *h_min_old) {
  if (h_min == -1)
    p = 0.0;
  else
    p = std::max(0.0, static_cast<double>(*h_min_old - h_min));
  if (is_initial_walk) {
    ap = p;
    is_initial_walk = false;
  }
  bool result = p > ap;
  ap = (1.0-kAlpha)*ap + kAlpha*p;
  if (h_min != -1 && h_min < *h_min_old) *h_min_old = h_min;
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

int RandomWalk(int h_min_old, int mode, const Domain &domain,
               const TrieTable &table, vector<int> &s, vector<int> &sequence) {
  int length_walk = kLengthWalk[mode];
  PrintLengthWalk(length_walk);
  int h_min = INT_MAX;
  vector<int> s_min;
  vector<int> best_sequence;
  int counter = 0;
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  std::uniform_real_distribution<> dist(0.0, 1.0);
  for (int i=0; i<kNumWalk; ++i) {
    ++total_walks;
    auto s_prime = s;
    vector<int> footprints;
    if (counter > kExtendingPeriod) {
      length_walk = ExtendLengthWalk(kExtendingRate[mode], length_walk);
      PrintLengthWalk(length_walk);
      counter = 0;
    }
    for (int j=0; j<length_walk; ++j) {
      auto a_set = FindFromTable(table, s_prime, domain.fact_offset);
      if (a_set.empty()) {
        ++faild_walks;
        break;
      }
      int a;
      if (mode == kPure) {
        std::uniform_int_distribution<> int_dist(0, a_set.size()-1);
        a = a_set[int_dist(engine)];
      } else {
        a = GibbsSampling(a_set, q_mda, kTau[mode], dist(engine));
      }
      ApplyEffect(domain.effects[a], s_prime);
      footprints.push_back(a);
      ++generated;
      if (GoalCheck(domain.goal, s_prime)) {
        UpdateState(s_prime, footprints, s, sequence);
        return 0;
      }
    }
    vector<int> helpful_actions;
    int h = Additive(s_prime, domain, effect_map, additive_table);
    ++evaluated;
    UpdateMDA(h, footprints);
    if (h < h_min) {
      UpdateMinimum(h, s_prime, footprints, &h_min, s_min, best_sequence);
      counter = 0;
    } else {
      ++counter;
    }
    if (!UpdatePAP(h_min, &h_min_old)) continue;
    PrintStopWalk(i+1);
    UpdateState(s_min, best_sequence, s, sequence);
    return h_min;
  }
  PrintStopWalk(kNumWalk);
  if (h_min == INT_MAX) return h_min_old;
  UpdateState(s_min, best_sequence, s, sequence);
  return h_min;
}

vector<int> MRW(const vector<int> &initial, const Domain &domain,
                const TrieTable &table) {
  size_t n_actions = domain.names.size();
  q_mda.resize(n_actions);
  std::fill(q_mda.begin(), q_mda.end(), 0.0);
  success.resize(n_actions);
  std::fill(success.begin(), success.end(), 0.0);
  faild.resize(n_actions);
  std::fill(faild.begin(), faild.end(), 0.0);

  size_t fact_size = static_cast<size_t>(domain.fact_offset.back());
  InitializeEffectMap(domain, effect_map);
  additive_table[0].resize(fact_size);
  additive_table[1].resize(fact_size);

  vector<int> s = initial;
  ++generated;
  vector<int> helpful_actions;
  int initial_h_min = Additive(s, domain, effect_map, additive_table);
  ++evaluated;
  if (initial_h_min == INT_MAX) return vector<int>{-1};
  vector<int> sequence;
  int h_min = initial_h_min;
  PrintNewHeuristicValue(h_min, sequence.size());
  int counter = 0;
  double fail_rate = 0.0;
  while (!GoalCheck(domain.goal, s)) {
    if (counter > kMaxSteps) {
      s = initial;
      h_min = initial_h_min;
      sequence.clear();
      counter = 0;
      std::fill(q_mda.begin(), q_mda.end(), 0.0);
      std::fill(success.begin(), success.end(), 0.0);
      std::fill(faild.begin(), faild.end(), 0.0);
      is_initial_walk = true;
      std::cout << "Restart" << std::endl;
      PrintNewHeuristicValue(h_min, sequence.size());
    }
    int mode = kPure;
    if (fail_rate > 0.5) {
      std::cout << "Too many dead end. Use MDA" << std::endl;
      mode = kMDA;
    }
    int h = RandomWalk(h_min, mode, domain, table, s, sequence);
    fail_rate = static_cast<double>(faild_walks)
                / static_cast<double>(total_walks);
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
