#include "trie.h"

#include <algorithm>
#include <numeric>
#include <iostream>
#include <utility>
#include <vector>

#include "data.h"

namespace mrw {

void InsertToTable(int query, std::vector<var_value_t> precondition,
                   const std::vector<int> &fact_offset, TrieTable *table) {
  int i = 0;
  int j = 0;
  int max = precondition.size() - 1;
  int parent_prefix = 0;
  size_t max_children = static_cast<size_t>(fact_offset.back());
  if (table->to_child.size() < max_children) {
    table->to_child.resize(max_children);
    table->to_data.resize(max_children);
    std::fill(table->to_child.begin(), table->to_child.end(), -1);
    std::fill(table->to_data.begin(), table->to_data.end(), -1);
  }
  std::sort(precondition.begin(), precondition.end());
  for (auto v : precondition) {
    int var, value;
    DecodeVarValue(v, &var, &value);
    int index = j + fact_offset[var] - parent_prefix + value;
    if (i == max) {
      if (table->to_data[index] == -1) {
        table->to_data[index] = table->data.size();
        table->data.resize(table->data.size()+1);
      }
      table->data[table->to_data[index]].push_back(query);
      return;
    }
    parent_prefix = fact_offset[var+1];
    if (table->to_child[index] == -1) {
      table->to_child[index] = table->to_child.size();
      int new_size = table->to_child.size()+max_children-parent_prefix;
      table->to_child.resize(new_size);
      table->to_data.resize(new_size);
      std::fill_n(&table->to_child[table->to_child[index]],
                  max_children-parent_prefix, -1);
      std::fill_n(&table->to_data[table->to_child[index]],
                  max_children-parent_prefix, -1);
    }
    j = table->to_child[index];
    ++i;
  }
}

void RecursiveFind(const TrieTable &table, const std::vector<int> &variables,
                   const std::vector<int> &fact_offset, int index,
                   int current, std::vector<int> &result) {
  int prefix = index - fact_offset[current];
  for (int i=current, n=variables.size(); i<n; ++i) {
    int next = fact_offset[i] + variables[i] + prefix;
    int offset = table.to_data[next];
    if (offset != -1)
      result.insert(result.end(), table.data[offset].begin(),
                    table.data[offset].end());
    if (table.to_child[next] == -1) continue;
    RecursiveFind(table, variables, fact_offset, table.to_child[next], i+1,
                  result);
  }
}

std::vector<int> FindFromTable(const TrieTable &table,
                               const std::vector<int> &variables,
                               const std::vector<int> &fact_offset) {
  std::vector<int> result;
  RecursiveFind(table, variables, fact_offset, 0, 0, result);
  return result;
}

TrieTable ConstructTable(
    const std::vector< std::vector<var_value_t> > &preconditions,
    const std::vector<int> &fact_offset) {
  TrieTable table;
  int n = preconditions.size();
  for (int i=0; i<n; ++i)
    InsertToTable(i, preconditions[i], fact_offset, &table);
  return table;
}

void PrintTable(const TrieTable &table) {
  for (auto v : table.to_child) {
    std::cout << v << " ";
  }
  std::cout << std::endl;
  for (auto v : table.to_data) {
    std::cout << v << " ";
  }
  std::cout << std::endl;
  for (auto v : table.data) {
    for (auto u : v) {
      std::cout << u << " ";
    }
    std::cout << std::endl;
  }
}

} // namespace mrw
