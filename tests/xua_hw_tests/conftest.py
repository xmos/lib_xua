# Copyright 2024-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

def pytest_addoption(parser):
    parser.addoption(
        "--xtag-id",
        action="store",
        default=None,
        help="XTAG ID to use for hardware tests",
    )
    parser.addoption(
        "--level",
        action="store",
        default="smoke",
        choices=["smoke", "nightly"],
        help="Test coverage level",
    )

def pytest_collection_modifyitems(config, items):
    selected = []
    deselected = []

    for item in items:
        m = item.get_closest_marker("uncollect_if")
        if m:
            func = m.kwargs["func"]
            if func(config, **item.callspec.params):
                deselected.append(item)
            else:
                selected.append(item)
        else: # If test doesn't define an uncollect function, default behaviour is to collect it
            selected.append(item)

    config.hook.pytest_deselected(items=deselected)
    items[:] = selected
