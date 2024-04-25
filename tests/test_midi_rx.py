# Copyright 2014-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import Pyxsim
from Pyxsim import testers
from pathlib import Path
from uart_rx_checker import UARTRxChecker
from midi_test_helpers import midi_expect_rx, create_midi_rx_file, create_midi_tx_file, tempdir, MIDI_TEST_CONFIGS, MIDI_RATE
from distutils.dir_util import copy_tree # we're using python 3.7 and dirs_exist_ok=True isn't available until 3.8 :(

MAX_CYCLES = 15000000


#####
# This test takes the built binary, copies it to a tmp dir and runs the midi Rx test which sends some commands
# to using the UARTRX checker and the firmware receives them
#####
@pytest.mark.parametrize("config", MIDI_TEST_CONFIGS)
def test_rx(capfd, config, build_midi):
    # Need tempdir as we use the same config files and this causes issues when using xdist 
    with tempdir() as tmpdirname:
        copy_tree(build_midi, tmpdirname)
        xe = str(Path(tmpdirname) / f"{config}/test_midi_{config}.xe")

        midi_commands = [[0x90, 60, 81]]
        create_midi_rx_file(len(midi_commands))
        create_midi_tx_file()

        expected = midi_expect_rx().expect(midi_commands)
        tester = testers.ComparisonTester(expected, ordered = True)
        
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
        #This is just for local debug so we can capture the traces if needed. It slows xsim down so not good for Jenkins
        # simargs.extend(["--trace-to", "trace.txt", "--vcd-tracing", "-tile tile[1] -ports -o trace.vcd"]) 
        
        # with capfd.disabled(): # use to see xsim and tester output
        Pyxsim.run_with_pyxsim(
            xe,
            simthreads=simthreads,
            timeout=120,
            simargs=simargs,   
        )
        capture = capfd.readouterr().out
        result = tester.run(capture.split("\n"))

        assert result, f"expected: {expected}\n capture: {capture}"