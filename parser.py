#!/usr/bin/env python

import re

from lab.parser import Parser

#!/usr/bin/env python

import re

from lab.parser import Parser


time = re.compile("Search time: ([0-9.]+)s")
nps = re.compile("Nodes per seconds: (.+)")


def coverage(content, props):
    props['coverage'] = int(props['run-planner_returncode'] == 0)


def get_validate_success(content, props):
    if props['validate_returncode'] == 0:
        props['validate_success'] = 0
    else:
        props['validate_success'] = 1


def get_search_time(content, props):
    m = time.search(content)
    if m is None:
        props['search_time'] = None
        return
    props['search_time'] = float(m.group(1))


def get_node_per_second(content, props):
    m = nps.search(content)
    if m is None:
        props['node_per_second'] = None
        return
    props['node_per_second'] = float(m.group(1))


parser = Parser()
parser.add_function(coverage)
parser.add_function(get_validate_success)
parser.add_pattern("expansions", r"Generated (\d+) state")
parser.add_pattern("evaluations", r"Evaluated (\d+) state")
parser.add_function(get_search_time)
parser.add_function(get_node_per_second)
parser.parse()
