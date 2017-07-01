#include "random_walk.h"

#include <iostream>
#include <random>
#include <utility>
#include <vector>

using std::vector;

namespace mrw {

void EpisodePool::Insert(int h, const vector<int> &sequence) {
  auto p = q_.top();
  int max = p.first;
  int index;
  if (size_ < sequences_.size()) {
    index = size_;
    ++size_;
  } else if (h < max) {
    index = p.second;
    q_.pop();
  } else {
    return;
  }
  sequences_[index] = sequence;
  q_.push(std::make_pair(h, index));
}

void EpisodePool::Sample(const Domain &domain, const vector<int> &initial,
                         vector<int> &state, vector<int> &sequence) {
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());
  std::uniform_int_distribution<> episode_dist(0, size_-1);
  int index = episode_dist(engine);
  std::cout << "smart restart episode: " << index << std::endl;
  sequence = sequences_[index];
  int max = static_cast<int>(sequence.size());
  std::uniform_int_distribution<size_t> state_dist(0, max-1);
  int length = state_dist(engine);
  std::cout << "smart restart step: " << length << std::endl;
  sequence.resize(length);
  state = initial;
  for (int i=0; i<length; ++i)
    ApplyEffect(domain.effects[sequence[i]], state);
}

int calc_g(const Domain &domain, vector<int> sequence) {
  int sum = 0;
  for (auto v : sequence) {
    sum += domain.costs[v];
  }
  return sum;
}

} // namespace mrw
