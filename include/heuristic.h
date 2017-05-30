#ifndef HEURISTIC_H_
#define HEURISTIC_H_

#include <vector>

#include "data.h"
#include "graphplan.h"

namespace mrw {

int FF(const std::vector<int> &variables, const Domain &domain,
       const GraphSchema &schema, PlanningGraph *graph,
       std::vector<int> &helpful_actions);

int Additive(const std::vector<int> &variables, const Domain &domain,
             const std::vector< std::vector<int> > &effect_map,
             std::array<std::vector<int>, 2> &table);

} // namespace mrw

#endif // FF_H_
