# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import time


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
