# Copyright 2018-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from pathlib import Path
import pytest
import Pyxsim
import json
from i2s_slave_checker import I2SSlaveChecker, Clock
from pathlib import Path
import re
from collections import Counter

# Enabling this prints to stdout and makes a VCD. Only used for dev since it doesn't run output through checker
DEBUG = 0

@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)

with open(Path(__file__).parent / "i2s_slave_sync_params.json") as f:
    params = json.load(f)


"""
This checks for syncerrors in the output but must see a syncerror cleared.
It checks the offsets in the transmitted (dut->tester) I2S frames due to missed rx frames
and throws an error if it is not an exact frame difference. The output word is:
0x00 00 FF CC where FF is frame count and CC is audio channel number.
We are looking for cases where we see bit shift which would cause content in upper two bytes (32b case)
or incorrect channel count (cycles through channels eg. 0-1-2-3-4-5-6-7-0-1...)
"""
class SyncTester:
    def __init__(self, channel_count):
        self.channel_count = channel_count
        pass

    def run(self, output):
        test_pass = True
        report = ""

        # ---- Tracking ----
        detect_stack = []
        sequence_errors = []
        adc_issues = []

        adc_re = re.compile(
            r"ADC unexpected frame (\d+) ch (\d+): (0x[0-9a-fA-F]+) \((0x[0-9a-fA-F]+)\)"
        )

        # ---- Parse Output ----
        for line in output:

            # ---- Sync ordering state machine ----
            if "syncError detected" in line:
                detect_stack.append(line)

            if "syncError cleared" in line:
                if not detect_stack:
                    # ERROR: cleared without detect
                    sequence_errors.append(f"Cleared without detect: {line}")
                else:
                    detect_stack.pop(0)  # FIFO match
            if "Tester Finished" in line and detect_stack:
                detect_stack.pop(0)

            # ---- ADC parsing ----
            m = adc_re.search(line)
            if m:
                frame = int(m.group(1))
                ch = int(m.group(2))
                v1 = int(m.group(3), 16)
                v2 = int(m.group(4), 16)
                adc_issues.append((frame, ch, v1, v2))

        # ---- End of log: check for unmatched detects ----
        if detect_stack:
            for d in detect_stack:
                sequence_errors.append(f"Detected but never cleared: {d}")

        # ---- Sync Report ----
        report += "=== Sync Error Sequence Check ===\n"

        if sequence_errors:
            for err in sequence_errors:
                report += f"ERROR: {err}\n"
            test_pass = False
        else:
            report += f"Sync detect->clear ordering: OK\n"

        # ---- ADC Offset Check ----
        report += f"\n=== ADC Offset Check ===\n"
        # These are for allowing a certain number of frames through until resynch
        max_resync_frames = 2 # How many seen frames can be non-pure offset. It is expected to see garbage for at least one frame
        max_resynch_samples = self.channel_count * max_resync_frames
        resync_counter = 0
        offset_list_counter = 0

        if not adc_issues:
            report += f"No ADC offset issues found.\n"
        else:
            offsets = [v1 - v2 for _, _, v1, v2 in adc_issues]
            
            offset_counts = Counter(offsets)
            offset_list = [0] + [value for value, count in offset_counts.items() if count >= 2] + [0] # Get a list of offsets (repeated at least twice)
            print(offset_list)

            report += f"Inferred ADC offsets: {[hex(offset) for offset in offset_list]}\n"

            # Allow some incorrect frames before re-synching 
            for frame, ch, v1, v2 in adc_issues:
                expected = v2 + offset_list[offset_list_counter]
                expected_next_offset = v2 + offset_list[offset_list_counter + 1]
                if v1 == expected_next_offset:
                    expected = expected_next_offset
                    offset_list_counter += 1
                    print(f"Using offset of: {hex(offset_list[offset_list_counter])}")

                ok = (v1 == expected)
                if not ok:
                    resync_counter += 1
                    if resync_counter > max_resynch_samples:
                        test_pass = False
                        report += f"Resync counter exceeded {max_resynch_samples} ({resync_counter})\n"
                else:
                    resync_counter = 0


                report += f"Frame {frame} Ch {ch}: {hex(v1)} vs {hex(v2)} -> expected {hex(expected)} {'OK' if ok else 'OFFSET ERROR'}\n"

        full_report = report + "\nRaw output:\n" + "\n".join(output) + "\n"
        print(full_report)

        assert test_pass, full_report

def do_test(pcm_format, channel_count, sample_rate, word_length, bit_slip, capfd, request, backpressure=False):
    # For parametisable tests
    build_options = []
    build_options += [f"pcm_format={pcm_format}"]
    build_options += [f"channel_count={channel_count}"]
    build_options += [f"sample_rate={sample_rate}"]
    build_options += [f"word_length={word_length}"]

    # Set configs for tests
    if backpressure:
        inject_bclks={}
        log_dut_to_harness = True
        desc = f"backpressure_{channel_count}in_{channel_count}out_{word_length}bit_{backpressure}"
    else:
        bclk_count_inject_start = 150
        # Inject bit_slip BCLKs after bclk_count_inject_start B clocks
        inject_bclks={bclk_count_inject_start:bit_slip}
        log_dut_to_harness = False
        desc = f"simulation_{pcm_format}_{channel_count}in_{channel_count}out_{sample_rate}_{word_length}bit"

    testname = Path(__file__).stem
    binary = Path(__file__).parent / testname / "bin" / desc / f"{testname}_{desc}.xe"
    assert Path(binary).exists(), f"Cannot find {binary}"

    tester = SyncTester(channel_count)
    max_cycles = 2000000
    clk = Clock("tile[0]:XS1_PORT_1A")

    checker = I2SSlaveChecker(
        "tile[0]:XS1_PORT_1B",
        "tile[0]:XS1_PORT_1C",
        ["tile[0]:XS1_PORT_1H","tile[0]:XS1_PORT_1I","tile[0]:XS1_PORT_1J", "tile[0]:XS1_PORT_1K"],
        ["tile[0]:XS1_PORT_1D","tile[0]:XS1_PORT_1E","tile[0]:XS1_PORT_1F", "tile[0]:XS1_PORT_1G"],
        "tile[0]:XS1_PORT_1L",
        "tile[0]:XS1_PORT_16A",
        "tile[0]:XS1_PORT_1M",
        clk,
        inject_bclks=inject_bclks,
        log_dut_to_harness = log_dut_to_harness
        )


    if DEBUG:
        with capfd.disabled():
            print(f"Running: {binary}")
            Pyxsim.run_on_simulator_(
                binary,
                tester=tester,
                simthreads=[clk, checker],
                do_xe_prebuild=False,
                simargs=[
                    f"--max-cycles", max_cycles,
                    "--vcd-tracing",
                    f"-o i2s_trace_{channel_count}_{channel_count}.vcd -tile tile[0] -cycles -ports -ports-detailed -cores -instructions",
                    "--trace-to",
                    f"i2s_trace_{channel_count}_{channel_count}.txt",
                ],
                
                capfd=None
            )
    else:
        Pyxsim.run_on_simulator_(
            binary,
            tester=tester,
            simthreads=[clk, checker],
            do_xe_prebuild=False,
            simargs=[f"--max-cycles", max_cycles],
            capfd=capfd
        )


@pytest.mark.parametrize("pcm_format", params["pcm_format"])
@pytest.mark.parametrize("channel_count", params["channel_count"])
@pytest.mark.parametrize("word_length", params["word_length"]) # I2S world length in bits
@pytest.mark.parametrize("sample_rate", params["sample_rate"])
@pytest.mark.parametrize("bit_slip", range(64))
def test_i2s_bitslip(pcm_format, channel_count, sample_rate, word_length, bit_slip, capfd, request, options):
    if options.level == "smoke":
        if (channel_count != 2 or 
            sample_rate != 44100 or
            word_length != 32):
            pytest.skip(f"Reduced test set for smoke")

    do_test(pcm_format, channel_count, sample_rate, word_length, bit_slip, capfd, request)


@pytest.mark.parametrize("pcm_format", params["pcm_format"])
@pytest.mark.parametrize("channel_count", params["channel_count"])
@pytest.mark.parametrize("word_length", params["word_length"]) # I2S world length in bits
@pytest.mark.parametrize("backpressure", [1, 2, 5, 10]) # Backpressure increase step
def test_i2s_backpressure(pcm_format, channel_count, word_length, backpressure, capfd, request, options):
    if options.level == "smoke":
        if (channel_count != 2 or 
            word_length != 32):
            pytest.skip("Reduced test set for smoke")

    do_test(pcm_format, channel_count, 48000, word_length, 0, capfd, request, backpressure=backpressure)
