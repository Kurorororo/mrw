#ifndef FF_H_
#define FF_H_

#include <vector>

#include "data.h"
#include "graphplan.h"

namespace mrw {

int FF(const std::vector<int> &variables, const Domain &domain,
       const GraphSchema &schema, PlanningGraph *graph,
       std::vector<int> &helpful_actions);

} // namespace mrw

#endif // FF_H_
