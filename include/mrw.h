#ifndef MRW_H_
#define MRW_H_

#include <iostream>
#include <vector>

#include "data.h"
#include "trie.h"

namespace mrw {

extern int generated;
extern int evaluated;

inline void PrintNewHeuristicValue(int min_h, int g) {
  std::cout << "New best heuristic value: " << min_h << std::endl;
  std::cout << "[g=" << g << ", " << evaluated << " evaluated, "
            << generated << " generated]" << std::endl;
}

inline void PrintStopWalk(int i) {
  std::cout << "Exploration stopped " << i << " random walks" << std::endl;
}

std::vector<int> MRW(const std::vector<int> &initial, const Domain &domain,
                     const TrieTable &table);

} // namespace mrw

#endif // MRW_H_
