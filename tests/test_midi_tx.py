# Copyright 2014-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import Pyxsim
from Pyxsim import testers
from pathlib import Path
from uart_tx_checker import UARTTxChecker
# from spdif_test_utils import (
#     Clock,
#     Spdif_rx,
#     Frames,
#     freq_for_sample_rate,
# )

MAX_CYCLES = 15000000
MIDI_RATE = 31250
CONFIGS = ["xs2", "xs3"]
CONFIGS = ["xs3"]


class Midi_expect:
    def __init(self):
        pass

    def expect(self):
        expected = "Hello"
        return expected



#####
# This test builds the spdif transmitter app with a verity of presets and tests that the output matches those presets
#####
@pytest.mark.parametrize("config", CONFIGS)
def test_tx(capfd, config):
    xe = str(Path(__file__).parent / f"test_midi/bin/{config}/test_midi_{config}.xe")
    p_midi_out = "tile[1]:XS1_PORT_4C"
    

    # tester = testers.ComparisonTester(
    #     Frames(channels=audio, no_of_blocks=no_of_blocks, sam_freq=sam_freq).expect()[
    #         : no_of_samples * len(audio)
    #     ]
    # )
    tester = testers.ComparisonTester(Midi_expect().expect())
    
    tx_port = "tile[1]:XS1_PORT_4C"
    rx_port = None
    baud = MIDI_RATE
    bpb = 8
    parity = 0 
    stop = 1
    length_of_test = 3 # characters

    simthreads = [
        # UARTTxChecker(rx_port, tx_port, parity, baud, length_of_test, stop, bpb)
    ]

    simargs = ["--max-cycles", str(MAX_CYCLES)]
    simargs.extend(["--trace-to", "trace.txt", "--vcd-tracing", "-tile tile[1] -ports -o trace.vcd"]) #This is just for local debug so we can capture the run, pass as kwarg to run_with_pyxsim

    # result = Pyxsim.run_on_simulator(
    result = Pyxsim.run_on_simulator(
        xe,
        simthreads=simthreads,
        instTracing=True,
        # clean_before_build=True,
        clean_before_build=False,
        tester=tester,
        # capfd=capfd,
        capfd=None,
        timeout=1500,
        simargs=simargs,
        build_options=[
            "-j",
            f"CONFIG={config}",
            "EXTRA_BUILD_FLAGS="
            + f" -DMIDI_RATE_HZ={MIDI_RATE}"
 ,
        ],
    )
    assert result