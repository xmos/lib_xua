# Copyright 2018-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from pathlib import Path
import pytest
import Pyxsim
from Pyxsim import testers
import sys
import json


def do_test(options, capfd):

    build_options = []
    output = []

    testname = Path(__file__).stem
    binary = Path(__file__).parent / testname / "bin" / f"{testname}.xe"
    with capfd.disabled():
        print("***", binary)

    tester = testers.ComparisonTester(open(Path(__file__).parent / "test_audio_stop_start.expect"))

    loopback_args = (
        "-port tile[1] XS1_PORT_1M 1 0 -port tile[1] XS1_PORT_1D 1 0 " # mclk
    )


    max_cycles = 6000000  # enough to send all of the frames in the test and hit exit(0)

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
        build_options=build_options,
        tester=tester,
        simargs=simargs,
        capfd=capfd,
        instTracing=options.enabletracing,
        vcdTracing=options.enablevcdtracing,
    )

    return result


def test_audio_stop_start(options, capfd):

    result = do_test(options, capfd)

    assert result
