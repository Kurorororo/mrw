#include "mrw.h"

#include <climits>
#include <cmath>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "additive.h"
#include "data.h"
#include "random_walk.h"
#include "trie.h"

using std::vector;

namespace mrw {

constexpr double kAlpha = 0.9;
constexpr int kNumWalk = 2000;
constexpr int kLengthWalk = 10;
constexpr int kExtendingPeriod = 300;
constexpr float kExtendingRate = 1.5;
constexpr int kMaxSteps = 7;
constexpr int kEpisodePoolSize = 50;

bool is_initial_walk = true;
double p;
double ap;

std::vector< std::vector<int> > effect_map;
std::vector<int> additive_table;

int generated = 0;
int evaluated = 0;

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

inline int ExtendLengthWalk(int length_walk) {
  return static_cast<int>(kExtendingRate * static_cast<float>(length_walk));
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

int RandomWalk(int h_min_old, const Domain &domain, const TrieTable &table,
               vector<int> &s, vector<int> &sequence) {
  int length_walk = kLengthWalk;
  PrintLengthWalk(length_walk);
  int h_min = INT_MAX;
  vector<int> s_min;
  vector<int> best_sequence;
  int counter = 0;
  for (int i=0; i<kNumWalk; ++i) {
    auto s_prime = s;
    vector<int> footprints;
    if (counter > kExtendingPeriod) {
      length_walk = ExtendLengthWalk(length_walk);
      PrintLengthWalk(length_walk);
      counter = 0;
    }
    for (int j=0; j<length_walk; ++j) {
      int a = SampleFromTable(table, domain, s_prime);
      if (a == -1) break;
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
  size_t fact_size = static_cast<size_t>(domain.fact_offset.back());
  InitializeEffectMap(domain, effect_map);
  additive_table.resize(fact_size);
  EpisodePool pool(kEpisodePoolSize);

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
  while (!GoalCheck(domain.goal, s)) {
    if (counter > kMaxSteps) {
      std::cout << "restart" << std::endl;
      pool.Insert(h_min, sequence);
      pool.Sample(domain, initial, s, sequence);
      h_min = Additive(s, domain, effect_map, additive_table);;
      ++evaluated;
      counter = 0;
      int g = calc_g(domain, sequence);
      PrintNewHeuristicValue(h_min, g);
    }
    int h = RandomWalk(h_min, domain, table, s, sequence);
    if (h < h_min) {
      h_min = h;
      counter = 0;
      int g = calc_g(domain, sequence);
      PrintNewHeuristicValue(h_min, g);
    } else {
      ++counter;
    }
  }
  return sequence;
}

} // namespace mrw
