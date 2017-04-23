#ifndef MRW_H_
#define MRW_H_

#include <vector>

#include "data.h"
#include "trie.h"

namespace mrw {

extern int generated;
extern int evaluated;

std::vector<int> MRW(const std::vector<int> &initial,
                     const std::vector<int> &fact_offset,
                     const std::vector<var_value_t> &goal,
                     const Actions &actions, const TrieTable &table);

} // namespace planning

#endif // MRW_H_
