# Copyright 2014-2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import contextlib
import os
import pytest
import tempfile

import Pyxsim
from Pyxsim import testers
from pathlib import Path
from uart_tx_checker import UARTTxChecker
from uart_rx_checker import UARTRxChecker


MAX_CYCLES = 15000000
MIDI_TEST_CONFIGS = ["xs2", "xs3"]
MIDI_RATE = 31250


# MIDI test applications are used in multiple test cases, so the XE files can't be built by
# Pyxsim as this could break when tests are run in parallel. So the applications must be
# built before running these tests.
def midi_xe_path(config):
    xe_path = Path(__file__).parent / "test_midi" / "bin" / f"test_midi_{config}.xe"
    assert xe_path.exists()
    return xe_path


class midi_expect_tx:
    def expect(self, commands):
        expected = ""
        for command in commands:
            while len(command) < 3:
                command.append(0)
            expected += "uart_tx_checker: " + " ".join([f"0x{byte:02x}" for byte in command]) + "\n"

        return expected + "\n"

class midi_expect_rx:
    def expect(self, commands):
        expected = ""
        for command in commands:
            while len(command) < 3:
                command.append(0)
            expected += "dut_midi_rx: " + " ".join([f"{byte}" for byte in command]) + "\n"

        return expected + "\n"

midi_tx_file = "midi_tx_cmds.txt"
midi_rx_file = "midi_rx_cmds.txt"

def create_midi_tx_file(commands=None):
    with open(midi_tx_file, "wt") as mt:
        if commands is None:
            return
        for command in commands:
            while len(command) < 3:
                command.append(0)
            text = " ".join([str(byte) for byte in command]) + "\n"
            mt.write(text)

def create_midi_rx_file(num_commands=0):
    with open(midi_rx_file, "wt") as mr:
        text = f"{num_commands}\n"
        mr.write(text)


# The test applications access some filenames relative to the current working directory, so
# tests are run inside temporary directories to allow parallel test execution.
@contextlib.contextmanager
def cd(new_dir):
    prev_dir = Path.cwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(prev_dir)


#####
# This test takes the built binary, copies it to a tmp dir and runs the midi Tx test which sends some commands
# to the firmware and then receives them using the UARTTX checker
#####
@pytest.mark.parametrize("config", MIDI_TEST_CONFIGS)
def test_tx(capfd, config):
    xe = midi_xe_path(config)

    midi_commands = [[0x90, 0x91, 0x90],# Invalid and should be discarded
                    [0x90, 60, 81],     # Note on
                    [0x80, 60, 81]]     # Note off

    # Make a 1D list from the 2D list [1:] because first is invalid and we expect to skip it
    midi_command_expected = [[item for row in midi_commands[1:] for item in row]]

    with (
        tempfile.TemporaryDirectory() as tmpdir,
        cd(tmpdir),
    ):
        create_midi_tx_file(midi_commands)
        create_midi_rx_file()

        expected = midi_expect_tx().expect(midi_command_expected)
        tester = testers.ComparisonTester(expected, ordered = True)

        tx_port = "tile[1]:XS1_PORT_4C"
        baud = MIDI_RATE
        bpb = 8
        parity = 0 
        stop = 1
        length_of_test = sum(len(cmd) for cmd in midi_command_expected)

        simthreads = [
            UARTTxChecker(tx_port, parity, baud, length_of_test, stop, bpb, debug=False)
        ]

        simargs = ["--max-cycles", str(MAX_CYCLES), "--trace-to", "trace.txt"]
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


#####
# This test takes the built binary, copies it to a tmp dir and runs the midi Rx test which sends some commands
# to using the UARTRX checker and the firmware receives them
#####
@pytest.mark.parametrize("config", MIDI_TEST_CONFIGS)
def test_rx(capfd, config):
    xe = midi_xe_path(config)

    midi_commands = [[0x90, 0x91, 0x90],# Invalid and should be discarded
                    [0x90, 60, 81],     # Note on
                    [0x80, 60, 81]]     # Note off

    midi_command_expected = midi_commands[1:] # should skip invalid first message


    with (
        tempfile.TemporaryDirectory() as tmpdir,
        cd(tmpdir),
    ):
        create_midi_rx_file(len(midi_command_expected))
        create_midi_tx_file()

        expected = midi_expect_rx().expect(midi_command_expected)
        tester = testers.ComparisonTester(expected, ordered = True)
        
        rx_port = "tile[1]:XS1_PORT_1F"
        tx_port = "tile[1]:XS1_PORT_4C" # Needed so that UARTRxChecker (a transmitter) knows when to start
        baud = MIDI_RATE
        bpb = 8
        parity = 0 
        stop = 1

        midi_commands_flattened = [item for row in midi_commands for item in row]

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

        # Print to console
        with capfd.disabled():
            print("CAPTURE:", capture)
            print("EXPECTED:", expected)

        assert result, f"expected: {expected}\n capture: {capture}"


#####
# sends some commands, the firmware receives them, prints and compares with the expected output
#####
def test_midi_loopback(capfd):
    xe = midi_xe_path("loopback")

    midi_commands = [
                    [0x90, 60, 81], #note on
                    [0xc0, 15],     #instr select
                    [0xe0, 0, 96],  #pitch bend 
                    [0xff],         #MIDI reset
                    [0x80, 60, 81], #note off
                    [0xf0, 0x00, 0x21], [0x1D, 0x0b, 0x33], [0x3f, 0x1e, 0xf7], # Sysex, Ableton, 0b33f1e, terminator 
                    [0xc0, 17],     #instr select
                    ]

    with (
        tempfile.TemporaryDirectory() as tmpdir,
        cd(tmpdir),
    ):
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
            print("CAPTURE:", capture)
            print("EXPECTED:", expected)

        assert result
