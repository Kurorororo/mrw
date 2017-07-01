#ifndef RANDOM_WALK_H_
#define RANDOM_WALK_H_

#include <queue>
#include <utility>
#include <vector>

#include "data.h"

namespace mrw {

class EpisodePool {
 public:
  EpisodePool(int n) {
    size_ = 0;
    sequences_.resize(n);
  }

  ~EpisodePool() {}

  void Insert(int h, const std::vector<int> &sequence);

  void Sample(const Domain &domain, const std::vector<int> &initial,
              std::vector<int> &state, std::vector<int> &sequence);

 private:
  int size_;
  std::priority_queue< std::pair<int,int> > q_;
  std::vector< std::vector<int> > sequences_;
};

int calc_g(const Domain &domain, std::vector<int> sequence);

} // namespace mrw

#endif // RANDOM_WALK_H_
