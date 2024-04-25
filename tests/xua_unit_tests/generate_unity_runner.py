# Copyright 2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import glob
import os.path
import subprocess
import sys
import argparse

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-root", nargs='?', help="Project root directory")
    parser.add_argument("--source-file", nargs='?', help="source file.")
    parser.add_argument("--runner-file", nargs='?', help="runner file.")
    args = parser.parse_args()
    return args

def get_ruby():
    """
    Check ruby is avaliable and return the command to invoke it.
    """
    interpreter_name = 'ruby'
    try:
        dev_null = open(os.devnull, 'w')
        # Call the version command to check the interpreter can be run
        subprocess.check_call([interpreter_name, '--version'],
                              stdout=dev_null,
                              close_fds=True)
    except OSError as e:
        print("Failed to run Ruby interpreter: {}".format(e), file=sys.stderr)
        exit(1)  # TODO: Check this is the correct way to kill xwaf on error

    return interpreter_name

def get_unity_runner_generator(project_root_path):
    """
    Check the Unity generate_test_runner script is avaliable, and return the
    path to it.
    """
    unity_runner_generator = os.path.join(
        project_root_path, 'Unity', 'auto', 'generate_test_runner.rb')
    if not os.path.exists(unity_runner_generator):
        print("Unity repo not found in workspace", file=sys.stderr)
        exit(1)  # TODO: Check this is the correct way to kill xwaf on error
    return unity_runner_generator

if __name__ == "__main__":
    args = parse_arguments()
    print(f"in python: root {args.project_root}, source {args.source_file}, runner {args.runner_file}")

    try:
        subprocess.check_call([get_ruby(),
                               get_unity_runner_generator(args.project_root),
                               args.source_file,
                               args.runner_file])
    except OSError as e:
        print("Ruby generator failed for {}\n\t{}".format(unity_test_path, e),
              file=sys.stderr)
        exit(1)  # TODO: Check this is the correct way to kill xwaf on error
