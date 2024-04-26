# Copyright 2014-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import Pyxsim
from Pyxsim import testers
from pathlib import Path
from uart_tx_checker import UARTTxChecker
from midi_test_helpers import midi_expect_tx, create_midi_tx_file, create_midi_rx_file, tempdir, MIDI_TEST_CONFIGS, MIDI_RATE
from distutils.dir_util import copy_tree # we're using python 3.7 and dirs_exist_ok=True isn't available until 3.8 :(

MAX_CYCLES = 15000000

#####
# This test takes the built binary, copies it to a tmp dir and runs the midi Tx test which sends some commands
# to the firmware and then receives them using the UARTTX checker
#####
@pytest.mark.parametrize("config", MIDI_TEST_CONFIGS)
def test_tx(capfd, config, build_midi):

    # Need tempdir as we use the same config files and this causes issues when using xdist 
    with tempdir() as tmpdirname:
        copy_tree(build_midi, tmpdirname)
        xe = str(Path(tmpdirname) / f"{config}/test_midi_{config}.xe")

        # midi_commands = [[0x90, 0x91, 0x90],# Invalid and should be discarded
        #                 [0x90, 60, 81],     # Note on
        #                 [0x80, 60, 81]]     # Note off

        midi_commands = [
                        [0x90, 60, 81],     # Note on
                        [0x80, 60, 81]]     # Note off


        # midi_command_expected = midi_commands[1:] # should skip invalid first message
        # Make a 1D list from the 2D list
        midi_command_expected = [[item for row in midi_commands for item in row]]

        create_midi_tx_file(midi_commands)
        create_midi_rx_file()

        expected = midi_expect_tx().expect(midi_command_expected)
        tester = testers.ComparisonTester(expected, ordered = True)

        tx_port = "tile[1]:XS1_PORT_4C"
        baud = MIDI_RATE
        bpb = 8
        parity = 0 
        stop = 1
        length_of_test = sum(len(cmd) for cmd in midi_commands)

        simthreads = [
            UARTTxChecker(tx_port, parity, baud, length_of_test, stop, bpb, debug=False)
        ]


        simargs = ["--max-cycles", str(MAX_CYCLES), "-o", "trace.txt"]
        #This is just for local debug so we can capture the traces if needed. It slows xsim down so not needed
        # simargs.extend(["--vcd-tracing", "-tile tile[1] -ports -o trace.vcd"]) 

        # with capfd.disabled(): # use to see xsim and tester output
        Pyxsim.run_with_pyxsim(
            xe,
            simthreads=simthreads,
            timeout=120,
            simargs=simargs,   
        )
        capture = capfd.readouterr().out
        result = tester.run(capture.split("\n"))

        # Print to console
        with capfd.disabled():
            print("CAPTURE:", capture)
            print("EXPECTED:", expected)

        # Show tail of trace if there is an error
        if not result:
            with capfd.disabled():
                print("Simulator trace tail:")
                with open("trace.txt") as trace:
                    output = trace.readlines()
                    print("".join(output[-25:]))


        assert result, f"expected: {expected}\n capture: {capture}"
