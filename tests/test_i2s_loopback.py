#!/usr/bin/env python
# Copyright (c) 2018-2019, XMOS Ltd, All rights reserved
import xmostest

def runtest_one_config(env, format, i2s_role, num_chans_in, num_chans_out, sample_rate):
    testlevel = 'smoke'
    resources = xmostest.request_resource('xsim')

    binary = 'i2s_loopback/bin/{env}_{format}_{i2s_role}_{num_chans_in}in_{num_chans_out}out_{sample_rate}/i2s_loopback_{env}_{format}_{i2s_role}_{num_chans_in}in_{num_chans_out}out_{sample_rate}.xe'.format(env=env, format=format, i2s_role=i2s_role, num_chans_in=num_chans_in, num_chans_out=num_chans_out, sample_rate=sample_rate)
    tester = xmostest.ComparisonTester(open('pass.expect'),
                                       'lib_xua',
                                       'i2s_loopback_sim_tests',
                                       'i2s_loopback',
                                       {'env':env,
                                        'format':format,
                                        'i2s_role':i2s_role,
                                        'num_chans_in':num_chans_in,
                                        'num_chans_out':num_chans_out,
                                        'sample_rate':sample_rate})
    tester.set_min_testlevel(testlevel)
    loopback_args=              '-port tile[0] XS1_PORT_1M 1 0 -port tile[0] XS1_PORT_1I 1 0 ' + \
                                '-port tile[0] XS1_PORT_1N 1 0 -port tile[0] XS1_PORT_1J 1 0 ' + \
                                '-port tile[0] XS1_PORT_1O 1 0 -port tile[0] XS1_PORT_1K 1 0 ' + \
                                '-port tile[0] XS1_PORT_1P 1 0 -port tile[0] XS1_PORT_1L 1 0 ' + \
                                '-port tile[0] XS1_PORT_1A 1 0 -port tile[0] XS1_PORT_1F 1 0 '
    if i2s_role == 'slave':
        loopback_args += '-port tile[0] XS1_PORT_1B 1 0 -port tile[0] XS1_PORT_1H 1 0 '   #bclk
        loopback_args += '-port tile[0] XS1_PORT_1C 1 0 -port tile[0] XS1_PORT_1G 1 0 '   #lrclk

    max_cycles = 1500000 #enough to reach the 10 skip + 100 test in sim at 48kHz
    xmostest.run_on_simulator(resources['xsim'], binary, tester=tester, simargs=['--max-cycles', str(max_cycles), '--plugin', 'LoopbackPort.dll', loopback_args])

def runtest():
    #runtest_one_config('simulation', 'i2s', 'master', 2, 2, '48khz')
    #runtest_one_config('simulation', 'i2s', 'slave', 2, 2, '48khz')

    #runtest_one_config('simulation', 'i2s', 'master', 2, 2, '192khz')
    #runtest_one_config('simulation', 'i2s', 'slave', 2, 2, '192khz')

    #runtest_one_config('simulation', 'i2s', 'master', 8, 8, '48khz')
    #runtest_one_config('simulation', 'i2s', 'slave', 8, 8, '48khz')

    #runtest_one_config('simulation', 'i2s', 'master', 8, 8, '192khz')
    #runtest_one_config('simulation', 'i2s', 'slave', 8, 8, '192khz')

    #runtest_one_config('simulation', 'tdm', 'master', 8, 8, '48khz')
    #runtest_one_config('simulation', 'tdm', 'slave', 8, 8, '48khz')

    #runtest_one_config('simulation', 'tdm', 'master', 16, 16, '48khz')
    #runtest_one_config('simulation', 'tdm', 'slave', 16, 16, '48khz')
