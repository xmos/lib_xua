# Copyright 2022-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import time
import Pyxsim
from pathlib import Path
from midi_test_helpers import MIDI_TEST_CONFIGS
import subprocess

@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


@pytest.fixture(scope="session")  # Use same seed for whole run
def test_seed(request):

    seed = str(int(time.time()))
    # We dont need the following since pytest will print the values of our fixtures on a failure
    # capmanager = request.config.pluginmanager.getplugin("capturemanager")
    # with capmanager.global_and_fixture_disabled():
    #    print("Using seed: "+ seed)
    return seed


def pytest_addoption(parser):
    parser.addoption(
        "--enabletracing",
        action="store_true",
        default=False,
        help="Run tests with instruction tracing",
    )

    parser.addoption(
        "--enablevcdtracing",
        action="store_true",
        default=False,
        help="Run tests with vcd tracing",
    )


@pytest.fixture
def options(request):
    yield request.config.option

# We use the same binary multiple times so just build once for all MIDI tests
@pytest.fixture(scope="session")
def build_midi():
    cmd = "xmake -C test_midi -j"
    result = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    all_build_success = result.returncode

    assert all_build_success == 0, f"{result.stderr}\n{result.stdout}"

    return str(Path(__file__).parent / f"test_midi/bin/") 

