# Copyright 2015-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
import Pyxsim as px
from functools import partial
from numbers import Number

# We need to disable output buffering for this test to work on MacOS; this has
# no effect on Linux systems. Let's redefine print once to avoid putting the
# same argument everywhere.
print = partial(print, flush=True)

class Clock(px.SimThread):

    def __init__(self, port: str) -> None:
        self._rate = 1000000000 # Hz
        self._driving = True
        self._half_period = float(500000000000000) / self._rate # 1/2 second in fs
        self._port = port

    def run(self) -> None:
        t = self.xsi.get_time()
        t += self._half_period
        while True:
            if self._driving:
                self.xsi.drive_port_pins(self._port, 0)
                t += self._half_period
                self.wait_until(t)
                self.xsi.drive_port_pins(self._port, 1)
                t += self._half_period
                self.wait_until(t)

    @property
    def rate(self) -> Number:
        return self._rate

    @rate.setter
    def rate(self, new_rate: Number) -> None:
        if new_rate == 0:
            self._driving = False
        else:
            self._driving = True
            self._half_period = float(500000000000000) / new_rate # 1/2 second in fs
        self._rate = new_rate


class I2SSlaveChecker(px.SimThread):
    """"
    This simulator thread will act as I2S master and check any transactions
    caused by the Slave.
    """

    def __init__(
        self,
        bclk: str,
        lrclk: str,
        din,
        dout,
        setup_strobe_port: str,
        setup_data_port: str,
        setup_resp_port: str,
        c: Clock,
        no_start_msg: bool = False,
        invert_bclk: bool = False,
        inject_bclks = {}, # This should be a dict, bit count where to slip and how many bclk slips
        log_dut_to_harness = False
    ):
        self._din = din
        self._dout = dout
        self._bclk = bclk
        self._lrclk = lrclk
        self._setup_strobe_port = setup_strobe_port
        self._setup_data_port = setup_data_port
        self._setup_resp_port = setup_resp_port
        self._clk = c
        self._no_start_msg = no_start_msg
        self._invert_bclk = invert_bclk
        self._inject_bclks = inject_bclks
        self._log_dut_to_harness = log_dut_to_harness

    # May be useful to have time the actual return value of SimThread.wait_until
    def wait_until_ret(self, t: int) -> int:
        self.wait_until(t)
        return t

    def get_setup_data(
        self, xsi: px.pyxsim.Xsi, setup_strobe_port: str, setup_data_port: str) -> int:
        self.wait_for_port_pins_change([setup_strobe_port])
        self.wait_for_port_pins_change([setup_strobe_port])
        return xsi.sample_port_pins(setup_data_port)

    def drive_bclk_period(self, time, bclock_half_period):
        self.xsi.drive_port_pins(self._bclk, self.bclk0)
        time = self.wait_until_ret(time + bclock_half_period)
        self.xsi.drive_port_pins(self._bclk, self.bclk1)
        time = self.wait_until_ret(time + bclock_half_period)
        return time
    
    def make_expected(self, num_frames):
        data = []
        for f in range(num_frames):
            data.append([((f << 8) + ch) for ch in range(0,8)])
        return data

    def run(self):
        xsi = self.xsi
        num_frames = 16
        self.bclk0 = 0
        self.bclk1 = 1
        din_sample_offset = 0
        first_iteration = True

        xsi.drive_port_pins(self._bclk, self.bclk1)
        if not self._no_start_msg:
            print("I2S Slave Checker Started")
        while True:
            xsi.drive_port_pins(self._setup_resp_port, 0)
            strobe_val = xsi.sample_port_pins(self._setup_strobe_port)
            if strobe_val == 1:
                self.wait_for_port_pins_change([self._setup_strobe_port])

            bclk_frequency_u = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            bclk_frequency_l = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            # Number of data lines not channels
            num_ins = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            # Number of data lines not channels
            num_outs = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            is_i2s_justified = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )
            data_bits = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port)
            
            num_frames = self.get_setup_data(
                xsi, self._setup_strobe_port, self._setup_data_port
            )

            # Start with BCLK and LRclk high
            xsi.drive_port_pins(self._bclk, self.bclk1)
            xsi.drive_port_pins(self._lrclk, 1)

            bclk_frequency = (bclk_frequency_u << 16) + bclk_frequency_l
            print(
                f"CONFIG: bclk:{bclk_frequency} in:{num_ins} out:{num_outs} i2s_justified:{is_i2s_justified} num_frames: {num_frames}"
            )
            bclock_half_period = float(1000000000000000) / float(2 * bclk_frequency) # 1fs
            data_bit_mask = int("1"*data_bits, base=2)

            if self._invert_bclk:
                self.bclk0 = 1
                self.bclk1 = 0
                din_sample_offset = bclock_half_period / 4
                if first_iteration:
                    print("Slave bit clock inverted")
                    print(
                        "Data-in sampling point offset to simulate real setup/hold timing"
                    )

            print(f"Settings: bclk {bclk_frequency}, num_wires {num_ins} {num_outs}, bits {data_bits}, is_i2s {is_i2s_justified}")

            rx_word = [0] * num_frames
            tx_word = [0] * num_frames
            tx_data = self.make_expected(num_frames) 
            rx_data = self.make_expected(num_frames) 
        
            time = float(xsi.get_time())

            time = self.wait_until_ret(time + (bclock_half_period * 2 * data_bits * 2))  # Add extra delay to ensure that the i2s_slave device sees the LRCLK transitions in the first for loop below

            lr_counter = data_bits + (is_i2s_justified)
            lr_count_max = (2 * data_bits) - 1

            # How many I2S cycles before target is sync'd and producing data
            num_startup_i2s_cycles = 3.5
            for i in range(0, int(data_bits * 2 * num_startup_i2s_cycles)):
                xsi.drive_port_pins(self._lrclk, lr_counter >= data_bits)
                lr_counter = lr_counter + 1 if lr_counter < lr_count_max else 0
                time = self.drive_bclk_period(time, bclock_half_period)

            error = False

            bit_counter = 0

            # ACK to dut ready
            xsi.drive_port_pins(self._setup_resp_port, 0)
            print("DATA start")
            for frame_count in range(0, num_frames):
                print(f"Checker frame: {frame_count} bclks: {bit_counter}")
                ##### FIRST HALF OF I2S FRAME #####
                for i in range(0, 4):
                    rx_word[i] = 0
                    tx_word[i] = tx_data[frame_count][i*2]

                # print(f"first tx - {hex(tx_word[0])}")
                for i in range(0, data_bits):
                    xsi.drive_port_pins(self._lrclk, lr_counter >= data_bits)
                    lr_counter = lr_counter + 1 if lr_counter < lr_count_max else 0
                    xsi.drive_port_pins(self._bclk, self.bclk0)

                    for p in range(0, num_ins):
                        xsi.drive_port_pins(self._dout[p], tx_word[p] >> (data_bits - 1))
                        tx_word[p] = tx_word[p] << 1

                    time = self.wait_until_ret(time + bclock_half_period)
                    xsi.drive_port_pins(self._bclk, self.bclk1)
                    time = self.wait_until_ret(time + din_sample_offset)

                    for p in range(0, num_outs):
                        val = xsi.sample_port_pins(self._din[p])
                        rx_word[p] = (rx_word[p] << 1) + val

                    time = self.wait_until_ret(time + bclock_half_period - din_sample_offset)
                    
                    # Inject clocks
                    if bit_counter in self._inject_bclks:
                        print(f"injecting {self._inject_bclks[bit_counter]} at {bit_counter}")
                        for i in range(self._inject_bclks[bit_counter]):
                            time = self.drive_bclk_period(time, bclock_half_period)
                            bit_counter += 1
                    bit_counter += 1

                for p in range(0, num_outs):
                    if (data_bit_mask & rx_data[frame_count][p*2]) != rx_word[p]:
                        if self._log_dut_to_harness:
                            print(f"ERROR dut->harness: first half frame {frame_count}: actual (0x{rx_word[p]:x}) expected (0x{(data_bit_mask & rx_data[frame_count][p*2]):x}) @ {time/1e9:4f}us")
                        error = True

                ##### SECOND HALF OF I2S FRAME #####
                for i in range(0, 4):
                    rx_word[i] = 0
                    tx_word[i] = tx_data[frame_count][i*2+1]

                # print(f"second tx - {hex(tx_word[0])}")
                for i in range(0, data_bits):
                    xsi.drive_port_pins(self._lrclk, lr_counter >= data_bits)
                    lr_counter = lr_counter + 1 if lr_counter < lr_count_max else 0

                    xsi.drive_port_pins(self._bclk, self.bclk0)

                    for p in range(0, num_ins):
                        xsi.drive_port_pins(self._dout[p], tx_word[p] >> (data_bits - 1))
                        tx_word[p] = tx_word[p] << 1

                    time = self.wait_until_ret(time + bclock_half_period)
                    xsi.drive_port_pins(self._bclk, self.bclk1)
                    time = self.wait_until_ret(time + din_sample_offset)

                    for p in range(0, num_outs):
                        val = xsi.sample_port_pins(self._din[p])
                        rx_word[p] = (rx_word[p] << 1) + val

                    time = self.wait_until_ret(time + bclock_half_period - din_sample_offset)
                    
                    # Inject clocks
                    if bit_counter in self._inject_bclks:
                        for i in range(self._inject_bclks[bit_counter]):
                            time = self.drive_bclk_period(time, bclock_half_period)
                            bit_counter += 1
                    bit_counter += 1


                for p in range(0, num_outs):
                    if (data_bit_mask & rx_data[frame_count][p*2 + 1]) != rx_word[p]:
                        if self._log_dut_to_harness:
                            print(f"ERROR dut->harness:  second half frame {frame_count}: actual (0x{rx_word[p]:x}) expected (0x{(data_bit_mask & rx_data[frame_count][p*2 + 1]):x}) @ {time/1e9:4f}us")
                        error = True

            print("Tester Finished")
            # Frames complete
            for i in range(0, data_bits):
                xsi.drive_port_pins(self._lrclk, lr_counter >= data_bits)
                lr_counter = lr_counter + 1 if lr_counter < lr_count_max else 0
                time = self.drive_bclk_period(time, bclock_half_period)

            # send the response
            xsi.drive_port_pins(self._setup_resp_port, 1)
            self.wait_for_port_pins_change([self._setup_strobe_port])
            xsi.drive_port_pins(self._setup_resp_port, error)
            # print error
            self.wait_for_port_pins_change([self._setup_strobe_port])

            first_iteration = False
