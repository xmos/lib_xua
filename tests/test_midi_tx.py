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

    def expect(self, commands):
        expected = ""
        for command in commands:
            while len(command) < 3:
                command.append(0)
            expected += "uart_tx_checker: " + " ".join([f"0x{byte:02x}" for byte in command]) + "\n"

        return expected


def create_midi_tx_file(commands):
    with open("midi_tx_cmds.txt", "wt") as mt:
        for command in commands:
            while len(command) < 3:
                command.append(0)
            text = " ".join([str(byte) for byte in command]) + "\n"
            mt.write(text)

#####
# This test builds the spdif transmitter app with a verity of presets and tests that the output matches those presets
#####
@pytest.mark.parametrize("config", CONFIGS)
def test_tx(capfd, config):
    xe = str(Path(__file__).parent / f"test_midi/bin/{config}/test_midi_{config}.xe")
    p_midi_out = "tile[1]:XS1_PORT_4C"

    midi_commands = [[0x90, 60, 81]]
    create_midi_tx_file(midi_commands)

    tester = testers.ComparisonTester(Midi_expect().expect(midi_commands),
                                        regexp = "uart_tx_checker:.+",
                                        ordered = True)

    
    tx_port = "tile[1]:XS1_PORT_4C"
    baud = MIDI_RATE
    bpb = 8
    parity = 0 
    stop = 1
    length_of_test = 3 # characters

    simthreads = [
        UARTTxChecker(tx_port, parity, baud, length_of_test, stop, bpb, debug=False)
    ]

    simargs = ["--max-cycles", str(MAX_CYCLES)]
    # simargs.extend(["--trace-to", "trace.txt", "--vcd-tracing", "-tile tile[1] -ports -o trace.vcd"]) #This is just for local debug so we can capture the run, pass as kwarg to run_with_pyxsim

    # result = Pyxsim.run_on_simulator(
    result = Pyxsim.run_on_simulator(
        xe,
        simthreads=simthreads,
        instTracing=True,
        # clean_before_build=True,
        clean_before_build=False,
        tester=tester,
        capfd=capfd,
        # capfd=None,
        timeout=120,
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