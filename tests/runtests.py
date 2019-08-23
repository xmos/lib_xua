#!/usr/bin/env python2.7
# Copyright (c) 2018-2019, XMOS Ltd, All rights reserved
import xmostest
import os.path

if __name__ == "__main__":


    xmostest.init()

    xmostest.register_group("lib_xua",
                            "i2s_loopback_sim_tests",
                            "I2S loopback simulator tests",

    """
Tests are performed by running the audiohub code connected to a 
loopback plugin 
""")

    xmostest.runtests()

    xmostest.finish()
