# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import Pyxsim
from Pyxsim import testers
import os
import sys


@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


def do_test(test_file, options, capfd):
    testname, _ = os.path.splitext(os.path.basename(test_file))

    binary = f"{testname}/bin/{testname}.xe"

    tester = testers.ComparisonTester(open("pass.expect"))

    max_cycles = 15000000

    simargs = [
        "--max-cycles",
        str(max_cycles),
    ]

    result = Pyxsim.run_on_simulator(
        binary,
        tester=tester,
        simargs=simargs,
        capfd=capfd,
        instTracing=options.enabletracing,
        vcdTracing=options.enablevcdtracing,
    )

    return result


# TODO parameterise with:
# - sample rate
# - usb bus speed
def test_decouple_out_underflow(test_file, options, capfd):
    result = do_test(test_file, options, capfd)

    assert result
