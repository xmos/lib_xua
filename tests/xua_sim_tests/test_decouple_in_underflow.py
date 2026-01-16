# Copyright 2022-2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import pytest
import Pyxsim
from Pyxsim import testers
from pathlib import Path


@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


def do_test(test_file, options, capfd, test_seed, sample_rate):
    testname = Path(__file__).stem
    binary = Path(__file__).parent / testname / "bin" / f"{sample_rate}" / f"{testname}_{sample_rate}.xe"

    tester = testers.ComparisonTester(open("pass.expect"))

    max_cycles = 15000000

    simargs = [
        "--max-cycles",
        str(max_cycles),
    ]

    seed_hdr = Path(__file__).parent / testname / "src" / "test_seed.h"
    with open(seed_hdr, "w") as f:
        f.write(f"#define TEST_SEED ({test_seed})")

    result = Pyxsim.run_on_simulator(
        binary,
        cmake=True,
        tester=tester,
        simargs=simargs,
        capfd=capfd,
        instTracing=options.enabletracing,
        vcdTracing=options.enablevcdtracing,
        clean_before_build=False
        )

    return result


# TODO parameterise with:
# - usb bus speed
@pytest.mark.parametrize("sample_rate", [48000, 96000, 192000])
def test_decouple_in_underflow(test_file, options, capfd, test_seed, sample_rate):
    result = do_test(test_file, options, capfd, test_seed, sample_rate)

    assert result
