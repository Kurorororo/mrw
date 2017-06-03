#!/usr/bin/env python

import re

from lab.parser import Parser


nps = re.compile("Nodes per seconds: ([0-9.]+)")
time = re.compile("Search time: ([0-9.]+)s")


def coverage(content, props):
    props['coverage'] = int(props['run-planner_returncode'] == 0)


def node_per_second(content, props):
    m = nps.search(content)
    if m is None:
        props['node_per_second'] = None
        return
    props['node_per_second'] = float(m.group(1))


def wall_time(content, props):
    m = time.search(content)
    if m is None:
        props['wall_time'] = None
        return
    props['wall_time'] = float(m.group(1))


parser = Parser()
parser.add_function(coverage)
parser.add_function(node_per_second)
parser.add_function(wall_time)
parser.parse()
