#include "data.h"

#include <vector>

namespace mrw {

void ApplyEffect(const std::vector<var_value_t> &effect,
                std::vector<int> &variables) {
  for (auto v : effect) {
    int var, value;
    DecodeVarValue(v, &var, &value);
    variables[var] = value;
  }
}

bool GoalCheck(const std::vector<var_value_t> &goal,
               const std::vector<int> &variables) {
  for (auto v : goal) {
    int var, value;
    DecodeVarValue(v, &var, &value);
    if (variables[var] != value) return false;
  }
  return true;
}

} // namespace mrw
