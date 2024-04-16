# Copyright 2014-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import Pyxsim
from Pyxsim import testers
from pathlib import Path
from uart_rx_checker import UARTRxChecker
from midi_test_helpers import midi_expect_rx, create_midi_rx_file, create_midi_tx_file

MAX_CYCLES = 15000000
MIDI_RATE = 31250
CONFIGS = ["xs2", "xs3"]
CONFIGS = ["xs3"]


#####
# This test builds the spdif transmitter app with a verity of presets and tests that the output matches those presets
#####
@pytest.mark.parametrize("config", CONFIGS)
def test_rx(capfd, config):
    xe = str(Path(__file__).parent / f"test_midi/bin/{config}/test_midi_{config}.xe")

    midi_commands = [[0x90, 60, 81]]
    create_midi_rx_file(1)
    create_midi_tx_file()


    tester = testers.ComparisonTester(midi_expect_rx().expect(midi_commands),
                                        regexp = "uart_tx_checker:.+",
                                        ordered = True)
    
    rx_port = "tile[1]:XS1_PORT_1F"
    tx_port = "tile[1]:XS1_PORT_4C" # Needed so that UARTRxChecker (a transmitter) knows when to start
    baud = MIDI_RATE
    bpb = 8
    parity = 0 
    stop = 1

    midi_commands_flattened = [item for row in midi_commands for item in row]
    # midi_commands_flattened.append(0x00) # send a null afterwards to give RXChecker to complete

    simthreads = [
        UARTRxChecker(tx_port, rx_port, parity, baud, stop, bpb, midi_commands_flattened, debug=False)
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