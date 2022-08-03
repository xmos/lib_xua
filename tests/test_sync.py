import pytest
import Pyxsim
from Pyxsim import testers
import os
import sys


@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


def do_test(test_file, options, capfd):

    testname, _ = os.path.splitext(os.path.basename(test_file))

    binary = f"{testname}/bin/{testname}.xe"

    tester = testers.ComparisonTester(open("pass.expect"))

    loopback_args = (
        "-port tile[0] XS1_PORT_1M 1 0 -port tile[0] XS1_PORT_1I 1 0 "
        + "-port tile[0] XS1_PORT_1N 1 0 -port tile[0] XS1_PORT_1J 1 0 "
        + "-port tile[0] XS1_PORT_1O 1 0 -port tile[0] XS1_PORT_1K 1 0 "
        + "-port tile[0] XS1_PORT_1P 1 0 -port tile[0] XS1_PORT_1L 1 0 "
        + "-port tile[0] XS1_PORT_1A 1 0 -port tile[0] XS1_PORT_1B 1 0 "
    )
    

    max_cycles = 15000000  # enough to reach the 10 skip + 100 test in sim at 48kHz

    simargs = [
        "--max-cycles",
        str(max_cycles),
        "--plugin",
        "LoopbackPort.dll",
        loopback_args,
    ]

    result = Pyxsim.run_on_simulator(binary, tester=tester, simargs=simargs, capfd=capfd, instTracing=options.enabletracing, vcdTracing=options.enablevcdtracing)

    return result


def test_sync(test_file, options, capfd):

    result = do_test(test_file, options, capfd)

    assert result
