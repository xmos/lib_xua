# Copyright 2024-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from pathlib import Path
import platform
import pytest
import re
import subprocess

from hardware_test_tools.UaDut import UaDut


def cfg_list():
    bin_dir = Path(__file__).parent / "test_control" / "device" / "bin"
    if not bin_dir.exists():
        return []
    all_cfgs = [dir.stem for dir in bin_dir.iterdir()]
    return all_cfgs


@pytest.mark.parametrize("cfg", cfg_list())
def test_control(pytestconfig, cfg):
    xtag_id = pytestconfig.getoption("xtag_id")
    assert xtag_id, "--xtag-id option must be provided on the command line"

    test_xe = Path(__file__).parent / "test_control" / "device" / "bin" / cfg / f"control_test_{cfg}.xe"
    host_app = Path(__file__).parent / "test_control" / "host" / "build" / "host_control_test"

    assert test_xe.exists(), f"DUT xe {test_xe} not found"

    pid = 0x0016
    # Set in_chans and out_chans = 0 so wait_for_enumeration() treats it as a non audio device when checking for enumeration
    in_chans = 0
    out_chans = 0

    with UaDut(xtag_id, test_xe, pid, in_chans, out_chans) as dut:
        cmd = [f"{host_app}"]
        ret = subprocess.run(cmd, text=True, capture_output=True, timeout=30) # Run host app
        print(f"ret.stdout = {ret.stdout}")
        print(f"ret.stderr = {ret.stderr}")
        print(f"ret.returncode = {ret.returncode}")
        assert ret.returncode == 0, f"{host_app} run failed.\nstdout:\n{ret.stdout}\nstderr:\n{ret.stderr}"

