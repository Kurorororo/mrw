#ifndef TRIE_H_
#define TRIE_H_

#include <utility>
#include <vector>

#include "data.h"

namespace mrw {

struct TrieTable {
  std::vector<int> to_child;
  std::vector<int> to_data;
  std::vector< std::vector<int> > data;
};

void InsertToTable(int query, std::vector<var_value_t> precondition,
                   const std::vector<int> &fact_offset, TrieTable *table);

std::vector<int> FindFromTable(const TrieTable &table,
                               const std::vector<int> &variables,
                               const std::vector<int> &fact_offset);

void PrintTable(const TrieTable &table);

TrieTable ConstructTable(
    const std::vector< std::vector<var_value_t> > &preconditions,
    const std::vector<int> &fact_offset);

} // namespace mrw

#endif // TRIE_H_
