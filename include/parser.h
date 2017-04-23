#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <vector>

#include "data.h"

namespace mrw {

void Parse(const std::string &filename, std::vector<int> &initial,
           std::vector<int> &fact_offset,
           std::vector< std::vector<var_value_t> > &mutex_groups,
           std::vector<var_value_t> &goal, Actions *actions);

} // namespace mrw

#endif // PARSER_H_
