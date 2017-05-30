#ifndef MRW_H_
#define MRW_H_

#include <vector>

#include "data.h"
#include "trie.h"

namespace mrw {

extern int generated;
extern int evaluated;

std::vector<int> MRW(const std::vector<int> &initial, const Domain &domain,
                     const TrieTable &table);

} // namespace mrw

#endif // MRW_H_
