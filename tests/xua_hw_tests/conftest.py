# Copyright 2024 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

def pytest_addoption(parser):
    parser.addoption(
        "--xtag-id",
        action="store",
        default=None,
        help="XTAG ID to use for hardware tests",
    )
