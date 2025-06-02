# Copyright 2018-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from pathlib import Path
import pytest
import Pyxsim
from Pyxsim import testers
import sys
import json


@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


with open(Path(__file__).parent / "i2s_loopback_params.json") as f:
    params = json.load(f)


def do_test(
    pcm_format, i2s_role, channel_count, sample_rate, word_length, tile, test_file, options, capfd
):

    build_options = []
    output = []

    build_options += [f"pcm_format={pcm_format}"]
    build_options += [f"i2s_role={i2s_role}"]
    build_options += [f"channel_count={channel_count}"]
    build_options += [f"sample_rate={sample_rate}"]
    build_options += [f"word_length={word_length}"]
    build_options += [f"tile={tile}"]

    desc = f"simulation_{pcm_format}_{i2s_role}_{channel_count}in_{channel_count}out_{sample_rate}_{word_length}bit_{tile}_xud_tile"
    testname = Path(__file__).stem
    binary = Path(__file__).parent / testname / "bin" / desc / f"{testname}_{desc}.xe"

    tester = testers.ComparisonTester(open(Path(__file__).parent / "pass.expect"))

    loopback_args = (
        "-port tile[0] XS1_PORT_1M 1 0 -port tile[0] XS1_PORT_1I 1 0 "
        + "-port tile[0] XS1_PORT_1N 1 0 -port tile[0] XS1_PORT_1J 1 0 "
        + "-port tile[0] XS1_PORT_1O 1 0 -port tile[0] XS1_PORT_1K 1 0 "
        + "-port tile[0] XS1_PORT_1P 1 0 -port tile[0] XS1_PORT_1L 1 0 "
        + "-port tile[0] XS1_PORT_1A 1 0 -port tile[0] XS1_PORT_1F 1 0 "
    )
    if i2s_role == "slave":
        loopback_args += (
            "-port tile[0] XS1_PORT_1B 1 0 -port tile[0] XS1_PORT_1H 1 0 "  # bclk
        )
        loopback_args += (
            "-port tile[0] XS1_PORT_1C 1 0 -port tile[0] XS1_PORT_1G 1 0 "  # lrclk
        )

    max_cycles = 1500000  # enough to reach the 10 skip + 100 test in sim at 48kHz

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

    with capfd.disabled():
        print(build_options)
        print(binary)
        print(binary.exists())

    return result


@pytest.mark.parametrize("i2s_role", params["i2s_role"])
@pytest.mark.parametrize("pcm_format", params["pcm_format"])
@pytest.mark.parametrize("channel_count", params["channel_count"])
@pytest.mark.parametrize("word_length", params["word_length"]) # I2S world length in bits
@pytest.mark.parametrize("sample_rate", params["sample_rate"])
@pytest.mark.parametrize("tile", params["tile"])
def test_i2s_loopback(
    i2s_role, pcm_format, channel_count, sample_rate, word_length, tile, test_file, options, capfd
):

    if pcm_format == "i2s" and channel_count == 16:
        pytest.skip("Invalid parameter combination")

    if pcm_format == "i2s" and sample_rate not in [48000, 192000]:
        pytest.skip("Invalid parameter combination")

    if pcm_format == "tdm" and channel_count == 2:
        pytest.skip("Invalid parameter combination")

    if pcm_format == "tdm" and sample_rate == 192000:
        pytest.skip("Invalid parameter combination")

    # We only want to test a handful of cases for when on the same tile since we are just testing that the clock works
    # So don't bother sweeping word_length, sample_rate, role and channel count
    if tile == "same" and (word_length != 32 or sample_rate != 192 or i2s_role != "master" or channel_count != 8):
        pytest.skip("Tile placement test doesn't need full sweep")

    result = do_test(
        pcm_format, i2s_role, channel_count, sample_rate, word_length, tile, test_file, options, capfd
    )

    assert result
