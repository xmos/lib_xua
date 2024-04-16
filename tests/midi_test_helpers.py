# Copyright 2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

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
