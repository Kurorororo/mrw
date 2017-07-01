import os
import subprocess
import sys


def main():
    argv = sys.argv
    if (len(argv) != 3):
        sys.exit("usage: mrw.py domain.pddl problem.pddl")
    REPO = os.environ["DOWNWARD_REPO"]
    cmd = os.path.join(REPO, "fast-downward.py")
    args = [cmd, "--translate", argv[1], argv[2]]
    subprocess.check_call(args)
    DIR_NAME = os.path.dirname(os.path.abspath(__file__))
    search = os.path.join(DIR_NAME, "bin/ff")
    subprocess.check_call([search, "output.sas"])


if __name__ == "__main__":
    main()
