# Copyright 2014-2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import pytest
import Pyxsim
from Pyxsim import testers
from pathlib import Path
from uart_rx_checker import UARTRxChecker
from midi_test_helpers import midi_expect_rx, create_midi_rx_file, create_midi_tx_file, tempdir, MIDI_RATE
from distutils.dir_util import copy_tree # we're using python 3.7 and dirs_exist_ok=True isn't available until 3.8 :(

MAX_CYCLES = 15000000


#####
# This test takes the built binary, copies it to a tmp dir and runs the midi loopback test which sends some commands
# the firmware receives them, prints and compares with the expected output
#####
def test_midi_loopback(capfd, build_midi):
    # Need tempdir as we use the same config files and this causes issues when using xdist 
    with tempdir() as tmpdirname:
        config = "LOOPBACK"
        copy_tree(build_midi, tmpdirname)
        xe = str(Path(tmpdirname) / f"{config}/test_midi_{config}.xe")

        midi_commands = [
                        [0x90, 60, 81], #note on
                        [0xc0, 15],     #instr select
                        [0xe0, 0, 96],  #pitch bend 
                        [0xff],         #MIDI reset
                        [0x80, 60, 81], #note off
                        [0xf0, 0x00, 0x21], [0x1D, 0x0b, 0x33], [0x3f, 0x1e, 0xf7], # Sysex, Ableton, 0b33f1e, terminator 
                        [0xc0, 17],     #instr select
                        ]

        create_midi_rx_file(len(midi_commands))
        create_midi_tx_file(midi_commands)

        expected = midi_expect_rx().expect(midi_commands)
        tester = testers.ComparisonTester(expected, ordered = True)

        simthreads = []

        simargs = ["--max-cycles", str(MAX_CYCLES)]
        #This is just for local debug so we can capture the traces if needed. It slows xsim down a lot
        # simargs.extend(["--trace-to", "trace.txt", "--vcd-tracing", "-tile tile[1] -ports -o trace.vcd"]) 

        Pyxsim.run_with_pyxsim(
            xe,
            simthreads=simthreads,
            timeout=180,
            simargs=simargs,   
        )
        capture = capfd.readouterr().out
        result = tester.run(capture.split("\n"))

        # Print to console
        with capfd.disabled():
            print("CAPTURE ++++", capture, "++++")
            print("EXPECTED ----", expected, "----")


        assert result