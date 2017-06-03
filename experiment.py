import os
import platform

from lab.environments import LocalEnvironment
from lab.experiment import Experiment
from lab.reports import arithmetic_mean, Attribute

from downward import suites
from downward.reports.absolute import AbsoluteReport


DIR_NAME = os.path.dirname(os.path.abspath(__file__))
FILE_PATH = os.path.join(DIR_NAME, "mrw.py")
FILE_PATH1 = os.path.join(DIR_NAME, "additive.py")

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
ENV = LocalEnvironment(processes=1)
# SUITE = ['barman-sat11-strips', 'elevators-sat11-strips',
#          'floortile-sat11-strips', 'nomystery-sat11-strips',
#          'openstacks-sat11-strips', 'parcprinter-sat11-strips',
#          'parking-sat11-strips', 'pegsol-sat11-strips',
#          'scanalyzer-sat11-strips', 'sokoban-sat11-strips',
#          'tidybot-sat11-strips', 'transport-sat11-strips',
#          'visitall-sat11-strips', 'woodworking-sat11-strips']
# SUITE = ['airport', 'satellite', 'pipesworld-tankage', #
#          'pipesworld-notankage', 'psr-small']
SUITE = ['gripper']
nps = Attribute('node_per_second', absolute=True, min_wins=False,
                functions=[arithmetic_mean])
time = Attribute('wall_time', absolute=True, min_wins=True,
                 functions=[arithmetic_mean])
ATTRIBUTES = ['coverage', nps, time]

exp = Experiment(environment=ENV)
exp.add_resource('parser', 'parser.py')

for task in suites.build_suite(BENCHMARKS_DIR, SUITE):
    run = exp.add_run()
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)
    run.add_command(
        'run-planner',
        ['python', FILE_PATH, '{domain}', '{problem}'],
        time_limit=1800,
        memory_limit=None)
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', 'mrw')
    run.set_property('id', ['mrw', task.domain, task.problem])
    run.add_command('parse', ['{parser}'])

for task in suites.build_suite(BENCHMARKS_DIR, SUITE):
    run = exp.add_run()
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)
    run.add_command(
        'run-planner',
        ['python', FILE_PATH1, '{domain}', '{problem}'],
        time_limit=1800,
        memory_limit=None)
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', 'additive')
    run.set_property('id', ['additive', task.domain, task.problem])
    run.add_command('parse', ['{parser}'])

exp.add_report(AbsoluteReport(attributes=ATTRIBUTES), outfile='report.html')

exp.run_steps()
