import pytest
from test_sync_clk_basic import do_test
from test_sync_clk_basic import test_file


@pytest.mark.parametrize("bus_speed", ["FS", "HS"])
def test_sync_clk_plugin(bus_speed, test_file, options, capfd):
    pytest.xfail("This is a known failure due to issue #275")
    result = do_test(bus_speed, test_file, options, capfd)
    assert result
