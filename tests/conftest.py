import pytest

def pytest_addoption(parser):
    parser.addoption("--enabletracing", action="store_true", default=False, help="Enable xsim tracing")


@pytest.fixture
def options(request):
    yield request.config.option
