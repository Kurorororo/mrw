#include "parser.h"

#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <sstream>
#include <vector>

#include "data.h"

namespace mrw {

const std::string kFileError = "invalid SAS file";
const std::string kAxiomError = "axiom is not supported";
const char kDelimiter = ' ';

int ParseVersion(std::queue<std::string> &lines) {
  if (lines.front() != "begin_version") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  int version = std::stoi(lines.front());
  lines.pop();
  if (lines.front() != "end_version") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  return version;
}

int ParseMetric(std::queue<std::string> &lines) {
  if (lines.front() != "begin_metric") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  int metric = std::stoi(lines.front());
  lines.pop();
  if (lines.front() != "end_metric") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  return metric;
}

int ParseVariable(std::queue<std::string> &lines) {
  if (lines.front() != "begin_variable") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  if (lines.front().substr(0, 3) != "var") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  if (std::stoi(lines.front()) != -1) {
    std::cerr << kAxiomError << std::endl;
    exit(1);
  };
  lines.pop();
  int n = std::stoi(lines.front());
  int sup = n;
  lines.pop();
  for (int i=0; i<n; ++i) {
    lines.pop();
  };
  if (lines.front() != "end_variable") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  return sup;
}

void ParseVariables(std::queue<std::string> &lines, std::vector<int> &initial,
                    std::vector<int> &fact_offset) {
  int n = std::stoi(lines.front());
  lines.pop();
  initial.resize(n);
  fact_offset.resize(n+1);
  int sum = 0;
  for (int i=0; i<n; ++i) {
    fact_offset[i] = sum;
    sum += ParseVariable(lines);
  }
  fact_offset[n] = sum;
}

void ParseVarValue(const std::string &line, var_value_t *var_value) {
  std::string buffer;
  std::istringstream line_separater(line);
  std::getline(line_separater, buffer, kDelimiter);
  int var = std::stoi(buffer);
  std::getline(line_separater, buffer, kDelimiter);
  int value = std::stoi(buffer);
  EncodeVarValue(var, value, var_value);
}

void ParseMutexGroup(std::queue<std::string> &lines,
                     std::vector<var_value_t> &mutex_group) {
  if (lines.front() != "begin_mutex_group") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  int n = std::stoi(lines.front());
  mutex_group.resize(n);
  lines.pop();
  for (int i=0; i<n; ++i) {
    ParseVarValue(lines.front(), &mutex_group[i]);
    lines.pop();
  }
  if (lines.front() != "end_mutex_group") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
}

void ParseMutexGroups(std::queue<std::string> &lines,
                      std::vector< std::vector<var_value_t> > &mutex_groups) {
  int n = std::stoi(lines.front());
  mutex_groups.resize(n);
  lines.pop();
  for (int i=0; i<n; ++i)
    ParseMutexGroup(lines, mutex_groups[i]);
}

void ParseState(std::queue<std::string> &lines, std::vector<int> &initial) {
  if (lines.front() != "begin_state") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  for (int i=0, n=initial.size(); i<n; ++i) {
    initial[i] = std::stoi(lines.front());
    lines.pop();
  }
  if (lines.front() != "end_state") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
}

void ParseGoal(std::queue<std::string> &lines,
               std::vector<var_value_t> &goal) {
  if (lines.front() != "begin_goal") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  int n = std::stoi(lines.front());
  goal.resize(n);
  lines.pop();
  for (int i=0; i<n; ++i) {
    ParseVarValue(lines.front(), &goal[i]);
    lines.pop();
  }
  if (lines.front() != "end_goal") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
}

int ParsePrecondition(std::queue<std::string> &lines,
                      std::vector<var_value_t> &precondition) {
  std::string buffer;
  int n = std::stoi(lines.front());
  precondition.resize(n);
  lines.pop();
  for (int i=0; i<n; ++i) {
   ParseVarValue(lines.front(), &precondition[i]);
   lines.pop();
  }
  return n;
}

void ParseEffect(std::queue<std::string> &lines,
                 std::vector<var_value_t> &precondition,
                 std::vector<var_value_t> &effect) {
  int n = std::stoi(lines.front());
  effect.resize(n);
  lines.pop();
  std::string buffer;
  for (int i=0; i<n; ++i) {
    std::istringstream line_separater(lines.front());
    std::getline(line_separater, buffer, kDelimiter);
    if (std::stoi(buffer) != 0)
      std::cerr << "conditional effect is not supported" << std::endl;
    std::getline(line_separater, buffer, kDelimiter);
    int var = std::stoi(buffer);
    std::getline(line_separater, buffer, kDelimiter);
    int value = std::stoi(buffer);
    if (value != -1) {
      int n = precondition.size();
      precondition.resize(n+1);
      EncodeVarValue(var, value, &precondition[n]);
    }
    std::getline(line_separater, buffer, kDelimiter);
    value = std::stoi(buffer);
    EncodeVarValue(var, value, &effect[i]);
    lines.pop();
  }
}

void ParseOperator(std::queue<std::string> &lines, int metric,
                   std::string *name, int *cost,
                   std::vector<var_value_t> &precondition,
                   std::vector<var_value_t> &effect) {
  if (lines.front() != "begin_operator") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
  *name = lines.front();
  lines.pop();
  ParsePrecondition(lines, precondition);
  ParseEffect(lines, precondition, effect);
  if (metric == 0)
    *cost = 1;
  else
    *cost = std::stoi(lines.front());
  lines.pop();
  if (lines.front() != "end_operator") {
    std::cerr << kFileError << std::endl;
    exit(1);
  }
  lines.pop();
}

void ParseOperators(std::queue<std::string> &lines, int metric,
                    Domain *domain) {
  int n = std::stoi(lines.front());
  domain->names.resize(n);
  domain->costs.resize(n);
  domain->preconditions.resize(n);
  domain->effects.resize(n);
  lines.pop();
  for (int i=0; i<n; ++i)
    ParseOperator(lines, metric, &domain->names[i], &domain->costs[i],
                  domain->preconditions[i], domain->effects[i]);
}

void Parse(const std::string &filename, std::vector<int> &initial,
           Domain *domain) {
  std::ifstream input;
  input.open(filename, std::ios::in);
  std::string buffer;
  std::queue<std::string> lines;
  while (std::getline(input, buffer)) {
    lines.push(buffer);
  }
  input.close();
  int version = ParseVersion(lines);
  if (version != 3) {
    std::cerr << "SAS version must be 3" << std::endl;
    exit(1);
  }
  std::cout << "Parsing metric" << std::endl;
  int metric = ParseMetric(lines);
  std::cout << "Parsing variables" << std::endl;
  ParseVariables(lines, initial, domain->fact_offset);
  std::cout << "Parsing mutex groups" << std::endl;
  ParseMutexGroups(lines, domain->mutex_groups);
  std::cout << "Parsing initial state" << std::endl;
  ParseState(lines, initial);
  std::cout << "Parsing goal" << std::endl;
  ParseGoal(lines, domain->goal);
  std::cout << "Parsing operators" << std::endl;
  ParseOperators(lines, metric, domain);
  if (std::stoi(lines.front()) != 0) {
    std::cerr << kAxiomError << std::endl;
    exit(1);
  }
  std::cout << "Parse succeed!" << std::endl;
}

} // namespace mrw
