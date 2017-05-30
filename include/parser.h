#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <vector>

#include "data.h"

namespace mrw {

void Parse(const std::string &filename, std::vector<int> &initial,
           Domain *domain);

} // namespace mrw

#endif // PARSER_H_
