# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import Pyxsim
from Pyxsim import testers
import os
import sys


def do_test(options, capfd, test_file, test_seed):

    testname, _ = os.path.splitext(os.path.basename(test_file))

    binary = f"{testname}/bin/{testname}.xe"

    tester = testers.ComparisonTester(open("pass.expect"))

    max_cycles = 15000000

    simargs = [
        "--max-cycles",
        str(max_cycles),
    ]

    build_options = []
    build_options += ["TEST_SEED=" + str(test_seed)]

    result = Pyxsim.run_on_simulator(
        binary,
        tester=tester,
        build_options=build_options,
        simargs=simargs,
        capfd=capfd,
        instTracing=options.enabletracing,
        vcdTracing=options.enablevcdtracing,
    )

    return result


def test_mixer_routing_output(options, capfd, test_file, test_seed):

    result = do_test(options, capfd, test_file, test_seed)

    assert result
