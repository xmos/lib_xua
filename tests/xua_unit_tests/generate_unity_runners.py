# Copyright 2021-2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import glob
import os.path
import subprocess
import sys

UNITY_TEST_DIR = "src"
UNITY_TEST_PREFIX = "test_"
UNITY_RUNNER_DIR = "runners"
UNITY_RUNNER_SUFFIX = "_Runner"
project_root = os.path.join("..", "..", "..")


def get_ruby():
    """
    Check ruby is avaliable and return the command to invoke it.
    """
    interpreter_name = "ruby"
    try:
        dev_null = open(os.devnull, "w")
        # Call the version command to check the interpreter can be run
        subprocess.check_call(
            [interpreter_name, "--version"], stdout=dev_null, close_fds=True
        )
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
        project_root_path, "Unity", "auto", "generate_test_runner.rb"
    )
    if not os.path.exists(unity_runner_generator):
        print("Unity repo not found in workspace", file=sys.stderr)
        exit(1)  # TODO: Check this is the correct way to kill xwaf on error
    return unity_runner_generator


def get_test_name(test_path):
    """
    Return the test name by removing the extension from the filename.
    """
    return os.path.splitext(os.path.basename(test_path))[0]


def get_file_type(filename):
    """
    Return the extension from the filename.
    """
    return filename.rsplit(".")[-1:][0]


def generate_unity_runner(
    project_root_path, unity_test_path, unity_runner_dir, unity_runner_suffix
):
    """
    Invoke the Unity runner generation script for the given test file, and
    return the path to the generated file. The output directory will be created
    if it does not already exist.
    """
    runner_path = os.path.join(
        os.path.join(unity_runner_dir, get_test_name(unity_test_path))
    )
    if not os.path.exists(runner_path):
        os.makedirs(runner_path)

    unity_runner_path = os.path.join(
        runner_path, get_test_name(unity_test_path) + unity_runner_suffix + "." + "c"
    )

    try:
        subprocess.check_call(
            [
                get_ruby(),
                get_unity_runner_generator(project_root_path),
                unity_test_path,
                unity_runner_path,
            ]
        )
    except OSError as e:
        print(
            "Ruby generator failed for {}\n\t{}".format(unity_test_path, e),
            file=sys.stderr,
        )
        exit(1)  # TODO: Check this is the correct way to kill xwaf on error


def find_unity_test_paths(unity_test_dir, unity_test_prefix):
    """
    Return a list of all file paths with the unity_test_prefix found in the
    unity_test_dir.
    """
    return glob.glob(os.path.join(unity_test_dir, unity_test_prefix + "*"))


def find_unity_tests(unity_test_dir, unity_test_prefix):
    """
    Return a dictionary of all {test names, test language} pairs with the
    unity_test_prefix found in the unity_test_dir.
    """
    unity_test_paths = find_unity_test_paths(unity_test_dir, unity_test_prefix)
    print("unity_test_paths = ", unity_test_paths)
    return {get_test_name(path): get_file_type(path) for path in unity_test_paths}


def find_unity_test_paths(unity_test_dir, unity_test_prefix):
    """
    Return a list of all file paths with the unity_test_prefix found in the
    unity_test_dir.
    """
    return glob.glob(os.path.join(unity_test_dir, unity_test_prefix + "*"))


def generate_runners():
    UNITY_TESTS = find_unity_tests(UNITY_TEST_DIR, UNITY_TEST_PREFIX)
    print("UNITY_TESTS = ", UNITY_TESTS)
    unity_test_paths = find_unity_test_paths(UNITY_TEST_DIR, UNITY_TEST_PREFIX)
    print("unity_test_paths = ", unity_test_paths)
    for unity_test_path in unity_test_paths:
        generate_unity_runner(
            project_root, unity_test_path, UNITY_RUNNER_DIR, UNITY_RUNNER_SUFFIX
        )


if __name__ == "__main__":
    generate_runners()
