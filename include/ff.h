#ifndef FF_H_
#define FF_H_

#include <vector>

#include "data.h"
#include "graphplan.h"

namespace mrw {

int FF(const std::vector<int> &variables, const std::vector<int> &fact_offset,
       const std::vector<var_value_t> &goal, const Actions &actions,
       const GraphSchema &schema, PlanningGraph *graph);

} // namespace mrw

#endif // FF_H_
