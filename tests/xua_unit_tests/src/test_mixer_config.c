// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/*
 * Mixer configuration tests. Built with the legacy (un-prefixed) mixer defines
 * set on the command line (see CMakeLists.txt), so checks that:
 *  - the legacy names are still honoured via the shims in xua_conf_default.h
 *  - an XUA_ prefixed define takes precedence over its legacy equivalent
 *  - defaults derived from XUA_MIXER_EN follow a legacy MIXER setting
 *  - default mixer weights are set up correctly when XUA_MAX_MIX_COUNT > XUA_MIX_INPUTS
 */
#include "xua_unit_tests.h"
#include "xua.h"

#define MIXER_WEIGHT_0DB     (0)
#define MIXER_WEIGHT_NEG_INF (0x8001)

extern short mixer1Weights[XUA_MIX_INPUTS * XUA_MAX_MIX_COUNT];
void InitLocalMixerState(void);

void test_mixer_legacy_defines_honoured(void)
{
    TEST_ASSERT_EQUAL_INT(1, XUA_MIXER_EN);
    TEST_ASSERT_EQUAL_INT(4, XUA_MAX_MIX_COUNT);
    TEST_ASSERT_EQUAL_INT(2, XUA_MIX_INPUTS);
    TEST_ASSERT_EQUAL_HEX(0x0100, XUA_MAX_MIXER_VOLUME);
    TEST_ASSERT_EQUAL_HEX(0x0200, XUA_VOLUME_RES_MIXER);
    TEST_ASSERT_EQUAL_INT(0, XUA_OUT_VOLUME_AFTER_MIX);
    TEST_ASSERT_EQUAL_INT(1, XUA_IN_VOLUME_IN_MIXER);
    TEST_ASSERT_EQUAL_INT(0, XUA_IN_VOLUME_AFTER_MIX);
}

void test_mixer_prefixed_define_takes_precedence(void)
{
    /* Both MIN_MIXER_VOLUME and XUA_MIN_MIXER_VOLUME are set, the XUA_ value should win */
    TEST_ASSERT_EQUAL_HEX(0x8300, XUA_MIN_MIXER_VOLUME);
}

void test_mixer_derived_default_follows_legacy_define(void)
{
    /* XUA_OUT_VOLUME_IN_MIXER is not set and defaults to enabled when the mixer is enabled */
    TEST_ASSERT_EQUAL_INT(1, XUA_OUT_VOLUME_IN_MIXER);
}

void test_mixer_default_weights_more_mixes_than_inputs(void)
{
    TEST_ASSERT_GREATER_THAN_INT(XUA_MIX_INPUTS, XUA_MAX_MIX_COUNT);

    InitLocalMixerState();

    /* Weights are indexed (input * XUA_MAX_MIX_COUNT) + mix. Mix i should take input i where
     * such an input exists, all other nodes should be muted */
    for (int input = 0; input < XUA_MIX_INPUTS; input++)
    {
        for (int mix = 0; mix < XUA_MAX_MIX_COUNT; mix++)
        {
            short expected = (mix == input) ? MIXER_WEIGHT_0DB : MIXER_WEIGHT_NEG_INF;
            TEST_ASSERT_EQUAL_INT16(expected, mixer1Weights[(input * XUA_MAX_MIX_COUNT) + mix]);
        }
    }
}
