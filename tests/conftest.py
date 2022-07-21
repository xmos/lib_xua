import pytest


def pytest_addoption(parser):
    pass


@pytest.fixture
def options(request):
    yield request.config.option
