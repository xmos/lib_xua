# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
"""
Check that xua_conf_default.h rejects unsupported configurations with the intended #error, and
accepts the nearest supported ones. Only the preprocessor is run, so no build or simulation.
"""
import subprocess
from pathlib import Path

import pytest

XUA_API_DIR = Path(__file__).resolve().parents[2] / "lib_xua" / "api"

# Minimum required defines, with MCLKs supported by the SW PLL clock recovery profiles
BASE = {
    "NUM_USB_CHAN_OUT": "2",
    "NUM_USB_CHAN_IN": "2",
    "I2S_CHANS_DAC": "2",
    "I2S_CHANS_ADC": "2",
    "MCLK_441": "(512*44100)",
    "MCLK_48": "(512*48000)",
}

ASYNC = {}
SYNC = {"XUA_SYNCMODE": "XUA_SYNCMODE_SYNC"}
ADAPT = {"XUA_SYNCMODE": "XUA_SYNCMODE_ADAPT"}
MCLK_256 = {"MCLK_441": "(256*44100)", "MCLK_48": "(256*48000)"}
SPDIF_RX = {"XUA_SPDIF_RX_EN": "1", "SPDIF_RX_INDEX": "2", "NUM_USB_CHAN_IN": "4"}
ADAT_RX = {"XUA_ADAT_RX_EN": "1", "ADAT_RX_INDEX": "2", "NUM_USB_CHAN_IN": "10"}

# id: (defines, expected #error text or None if the configuration is valid)
CONFIGS = {
    "async": (ASYNC, None),
    "async_no_usb": ({**ASYNC, "XUA_USB_EN": "0"}, None),
    "async_spdif_rx": ({**ASYNC, **SPDIF_RX}, None),
    "async_spdif_rx_mclk_256": (
        {**ASYNC, **SPDIF_RX, **MCLK_256},
        "S/PDIF/ADAT RX with XUA_USE_SW_PLL requires MCLK_441 = 512 * 44100 and MCLK_48 = 512 * 48000",
    ),
    "sync": (SYNC, None),
    "sync_record_only": ({**SYNC, "NUM_USB_CHAN_OUT": "0"}, None),
    "sync_no_usb": ({**SYNC, "XUA_USB_EN": "0"}, "Sync/Adaptive mode requires XUA_USB_EN"),
    "sync_spdif_rx": (
        {**SYNC, **SPDIF_RX},
        "Digital input streams not supported in Sync or Adaptive mode",
    ),
    "sync_mclk_256": (
        {**SYNC, **MCLK_256},
        "Sync/Adaptive mode with XUA_USE_SW_PLL requires MCLK_441 = 512 * 44100 and MCLK_48 = 512 * 48000",
    ),
    "sync_no_sw_pll_mclk_256": ({**SYNC, "XUA_USE_SW_PLL": "0", **MCLK_256}, None),
    "adapt": (ADAPT, None),
    "adapt_playback_only": ({**ADAPT, "NUM_USB_CHAN_IN": "0"}, None),
    "adapt_record_only": (
        {**ADAPT, "NUM_USB_CHAN_OUT": "0"},
        "Adaptive mode requires NUM_USB_CHAN_OUT > 0",
    ),
    "adapt_no_usb": ({**ADAPT, "XUA_USB_EN": "0"}, "Sync/Adaptive mode requires XUA_USB_EN"),
    "adapt_no_sw_pll": ({**ADAPT, "XUA_USE_SW_PLL": "0"}, "Adaptive mode requires XUA_USE_SW_PLL"),
    "adapt_adat_rx": (
        {**ADAPT, **ADAT_RX},
        "Digital input streams not supported in Sync or Adaptive mode",
    ),
}


@pytest.mark.parametrize("config", CONFIGS.keys())
def test_conf_checks(config, tmp_path):
    defines, expected_error = CONFIGS[config]

    src = tmp_path / "conf_check.c"
    src.write_text('#include "xua_conf_default.h"\n')

    cmd = ["xcc", "-E", "-target=XK-EVK-XU316", f"-I{XUA_API_DIR}", str(src), "-o", "/dev/null"]
    cmd += [f"-D{k}={v}" for k, v in {**BASE, **defines}.items()]
    result = subprocess.run(cmd, capture_output=True, text=True)
    errors = [line for line in result.stderr.splitlines() if "error:" in line]

    if expected_error is None:
        assert result.returncode == 0, "\n".join(errors)
    else:
        assert result.returncode != 0, f"Expected #error: {expected_error}"
        assert len(errors) == 1 and expected_error in errors[0], "\n".join(errors)
