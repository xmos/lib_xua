# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest


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
