#ifndef ADDITIVE_H_
#define ADDITIVE_H_

#include <vector>

#include "data.h"

namespace mrw {

int Additive(const std::vector<int> &variables, const Domain &domain,
             const std::vector< std::vector<int> > &effect_map,
             std::vector<int> &table);

} // namespace mrw

#endif // ADDITIVE_H_
