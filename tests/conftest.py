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
