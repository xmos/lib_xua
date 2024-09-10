# Copyright 2022-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import Pyxsim
from Pyxsim import testers
import sys
from pathlib import Path


@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


def do_test(bus_speed, test_file, options, capfd):

    testname = Path(__file__).stem
    binary = Path(__file__).parent / testname / "bin" / bus_speed / f"{testname}_{bus_speed}.xe"

    tester = testers.ComparisonTester(open(Path(__file__).parent / "pass.expect"))

    loopback_args = (
        "-port tile[0] XS1_PORT_1M 1 0 -port tile[0] XS1_PORT_1I 1 0 "
        + "-port tile[0] XS1_PORT_1N 1 0 -port tile[0] XS1_PORT_1J 1 0 "
        + "-port tile[0] XS1_PORT_1O 1 0 -port tile[0] XS1_PORT_1K 1 0 "
        + "-port tile[0] XS1_PORT_1P 1 0 -port tile[0] XS1_PORT_1L 1 0 "
        + "-port tile[0] XS1_PORT_1A 1 0 -port tile[0] XS1_PORT_1B 1 0 "
    )

    max_cycles = 15000000  # enough to reach the 10 skip + 100 test in sim at 48kHz

    simargs = [
        "--max-cycles",
        str(max_cycles),
        "--plugin",
        "LoopbackPort.dll",
        loopback_args,
    ]

    result = Pyxsim.run_on_simulator(
        binary,
        cmake=True,
        tester=tester,
        simargs=simargs,
        capfd=capfd,
        instTracing=options.enabletracing,
        vcdTracing=options.enablevcdtracing,
    )

    return result


@pytest.mark.parametrize("bus_speed", ["FS", "HS"])
def test_sync_clk_basic(bus_speed, test_file, options, capfd):

    result = do_test(bus_speed, test_file, options, capfd)

    assert result
