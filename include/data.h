#ifndef DATA_H_
#define DATA_H_

#include <cstdint>

#include <string>
#include <vector>

namespace mrw {

using var_value_t = uint64_t;

inline void EncodeVarValue(int var, int value, var_value_t *var_value) {
  *var_value = (static_cast<uint64_t>(var) << 32)
               ^ static_cast<uint64_t>(value);
}

inline void DecodeVarValue(var_value_t var_value, int *var, int *value) {
  *var = static_cast<int>(var_value >> 32);
  *value = static_cast<int>(var_value & 0xFFFFFFFF);
}

struct Actions {
  std::vector<std::string> names;
  std::vector<int> costs;
  std::vector< std::vector<var_value_t> > preconditions;
  std::vector< std::vector<var_value_t> > effects;
};

void ApplyEffect(const std::vector<var_value_t> &effect,
                std::vector<int> &variables);

bool GoalCheck(const std::vector<var_value_t> &goal,
               const std::vector<int> &variables);

} // namespace mrw

#endif // DATA_H_
