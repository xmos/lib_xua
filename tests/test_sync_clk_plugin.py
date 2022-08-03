import pytest
import Pyxsim
from Pyxsim import testers
import os
import sys

from test_sync_clk_basic import do_test

@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)

def test_sync_clk_plugin(test_file, options, capfd):

    #pytest.xfail("This is a known failure due to issue #275")

    result = do_test(test_file, options, capfd)

    assert result
