# Copyright 2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

class midi_expect_tx:
    def __init(self):
        pass

    def expect(self, commands):
        expected = ""
        for command in commands:
            while len(command) < 3:
                command.append(0)
            expected += "uart_tx_checker: " + " ".join([f"0x{byte:02x}" for byte in command]) + "\n"

        return expected

class midi_expect_rx:
    def __init(self):
        pass

    def expect(self, commands):
        expected = ""
        for command in commands:
            while len(command) < 3:
                command.append(0)
            expected += "dut_midi_rx: " + " ".join([f"{byte}" for byte in command]) + "\n"

        return expected

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
