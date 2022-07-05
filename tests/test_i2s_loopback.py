import pytest
import Pyxsim
from Pyxsim import testers
import os
import sys


@pytest.fixture()
def test_file(request):
    return str(request.node.fspath)


def create_if_needed(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)
    return folder


def get_sim_args(testname, desc, options):
    sim_args = []

    if options.enabletracing:
        log_folder = create_if_needed("logs")

        filename = "{log}/xsim_trace_{test}_{desc}".format(
            log=log_folder,
            test=testname,
            desc=desc,
        )

        sim_args += [
            "--trace-to",
            "{0}.txt".format(filename),
            "--enable-fnop-tracing",
        ]

        vcd_args = "-o {0}.vcd".format(filename)
        vcd_args += (
            " -tile tile[0] -ports -ports-detailed -instructions"
            " -functions -cycles -clock-blocks -pads -cores -usb"
        )

        sim_args += ["--vcd-tracing", vcd_args]

    return sim_args


def run_on_simulator(xe, simthreads, **kwargs):
    for k in ["do_xe_prebuild", "build_env", "clean_before_build"]:
        if k in kwargs:
            kwargs.pop(k)

    Pyxsim.run_with_pyxsim(xe, simthreads, **kwargs)


def do_test(pcm_format, i2s_role, channel_count, sample_rate, test_file, capfd, options):

    build_options = []
    output = []
    testname, _ = os.path.splitext(os.path.basename(test_file))

    desc = f"simulation_{pcm_format}_{i2s_role}_{channel_count}in_{channel_count}out_{sample_rate}"
    binary = f"{testname}/bin/{desc}/{testname}_{desc}.xe"

    build_success, _ = Pyxsim._build(
        binary, do_clean=False, build_options=build_options
    )

    if build_success:

        tester = testers.ComparisonTester(
            open("pass.expect"),
            "lib_xua",
            "xua_sim_tests",
            testname,
            {
                "speed": "500MHz",
                "arch": "XS2",
            },  # TODO run tests on XS3 and other core freqs
        )

        loopback_args = (
            "-port tile[0] XS1_PORT_1M 1 0 -port tile[0] XS1_PORT_1I 1 0 "
            + "-port tile[0] XS1_PORT_1N 1 0 -port tile[0] XS1_PORT_1J 1 0 "
            + "-port tile[0] XS1_PORT_1O 1 0 -port tile[0] XS1_PORT_1K 1 0 "
            + "-port tile[0] XS1_PORT_1P 1 0 -port tile[0] XS1_PORT_1L 1 0 "
            + "-port tile[0] XS1_PORT_1A 1 0 -port tile[0] XS1_PORT_1F 1 0 "
        )
        if i2s_role == "slave":
            loopback_args += (
                "-port tile[0] XS1_PORT_1B 1 0 -port tile[0] XS1_PORT_1H 1 0 "  # bclk
            )
            loopback_args += (
                "-port tile[0] XS1_PORT_1C 1 0 -port tile[0] XS1_PORT_1G 1 0 "  # lrclk
            )

        max_cycles = 1500000  # enough to reach the 10 skip + 100 test in sim at 48kHz

        simargs = get_sim_args(testname, desc, options)
        simargs = simargs + [
            "--max-cycles",
            str(max_cycles),
            "--plugin",
            "LoopbackPort.dll",
            loopback_args,
        ]

        simthreads = []
        run_on_simulator(binary, simthreads, simargs=simargs)

        cap_output, err = capfd.readouterr()
        output.append(cap_output.split("\n"))

        sys.stdout.write("\n")

        results = Pyxsim.run_tester(output, [tester])

        return results

    else:
        print("Build Failed")

    return [False]


@pytest.mark.parametrize("i2s_role", ["master", "slave"])
@pytest.mark.parametrize("pcm_format", ["i2s", "tdm"])
@pytest.mark.parametrize("channel_count", [2, 8, 16])
@pytest.mark.parametrize("sample_rate", ["48khz", "192khz"])
def test_i2s_loopback(
    i2s_role, pcm_format, channel_count, sample_rate, test_file, capfd, options
):

    if pcm_format == "i2s" and channel_count == 16:
        pytest.skip("Invalid parameter combination")

    if pcm_format == "tdm" and channel_count == 2:
        pytest.skip("Invalid parameter combination")

    if pcm_format == "tdm" and sample_rate == "192khz":
        pytest.skip("Invalid parameter combination")

    results = do_test(
        pcm_format, i2s_role, channel_count, sample_rate, test_file, capfd, options
    )

    assert results[0]
