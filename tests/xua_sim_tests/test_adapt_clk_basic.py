# Copyright 2026 XMOS LIMITED.
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

    max_cycles = 40000000  # ~136 ms of streaming at 200 MHz, with margin

    simargs = [
        "--max-cycles",
        str(max_cycles),
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
def test_adapt_clk_basic(bus_speed, test_file, options, capfd):

    result = do_test(bus_speed, test_file, options, capfd)

    assert result
