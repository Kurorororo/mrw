import os
import platform

from lab.environments import LocalEnvironment
from lab.experiment import Experiment
from lab.reports import Attribute, geometric_mean

from downward import suites
from downward.reports.absolute import AbsoluteReport


DIR_NAME = os.path.dirname(os.path.abspath(__file__))
ADDITIVE = os.path.join(DIR_NAME, "additive.py")
MRW = os.path.join(DIR_NAME, "mrw.py")

# TIME_LIMIT = 60
TIME_LIMIT = 300
# TIME_LIMIT = 1800
MEMORY_LIMIT = 204800
# SUITE = ['barman-sat11-strips', 'elevators-sat11-strips',
#          'floortile-sat11-strips', 'nomystery-sat11-strips',
#          'openstacks-sat11-strips', 'parcprinter-sat11-strips',
#          'parking-sat11-strips', 'pegsol-sat11-strips',
#          'scanalyzer-sat11-strips', 'sokoban-sat11-strips',
#          'tidybot-sat11-strips', 'transport-sat11-strips',
#          'visitall-sat11-strips', 'woodworking-sat11-strips',
#          'barman-sat14-strips', 'childsnack-sat14-strips',
#          'citycar-sat14-adl',
#          'floortile-sat14-strips',
#          'ged-sat14-strips', 'hiking-sat14-strips',
#          'maintenance-sat14-adl', 'openstacks-sat14-strips',
#          'parking-sat14-strips', 'tetris-sat14-strips',
#          'thoughtful-sat14-strips', 'transport-sat14-strips',
#          'visitall-sat14-strips']
SUITE = ['gripper']

BENCHMARKS_DIR = os.environ["DOWNWARD_BENCHMARKS"]
REPO = os.environ["DOWNWARD_REPO"]
ENV = LocalEnvironment(processes=16)

nps = Attribute('node_per_second', min_wins=False, functions=[geometric_mean])
validate = Attribute('validate_success', functions=[sum])
ATTRIBUTES = ['coverage', 'evaluations', 'expansions', 'search_time', nps,
              validate]

exp = Experiment(environment=ENV)
exp.add_resource('parser', 'parser.py')

for task in suites.build_suite(BENCHMARKS_DIR, SUITE):
    run = exp.add_run()
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)
    run.add_command(
        'run-planner',
        ['python', ADDITIVE, '{domain}', '{problem}'],
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT)
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', 'cpu-mrw-additive')
    run.set_property('id', ['cpu-mrw-additive', task.domain, task.problem])
    run.add_command('validate', ['validate', '{domain}', '{problem}',
                                 'sas_plan'])
    run.add_command('parse', ['{parser}'])

for task in suites.build_suite(BENCHMARKS_DIR, SUITE):
    run = exp.add_run()
    run.add_resource('domain', task.domain_file, symlink=True)
    run.add_resource('problem', task.problem_file, symlink=True)
    run.add_command(
        'run-planner',
        ['python', MRW, '{domain}', '{problem}'],
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT)
    run.set_property('domain', task.domain)
    run.set_property('problem', task.problem)
    run.set_property('algorithm', 'cpu-mrw-ff')
    run.set_property('id', ['cpu-mrw-ff', task.domain, task.problem])
    run.add_command('validate', ['validate', '{domain}', '{problem}',
                                 'sas_plan'])
    run.add_command('parse', ['{parser}'])

exp.add_report(AbsoluteReport(attributes=ATTRIBUTES),
               outfile='cpu-mrw-additivee.html')

exp.run_steps()

