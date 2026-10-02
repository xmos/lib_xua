// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "unity.h"
#include "xua_adaptive_rate_control.h"

void test_adaptive_window_counter_wrap(void)
{
    xua_adaptive_window_t window = {0};
    xua_adaptive_window_reset(&window);
    window.received_frames = 48;
    TEST_ASSERT_FALSE(xua_adaptive_window_sample(&window, 65000, UINT32_MAX - 49999));
    window.received_frames = 48;
    TEST_ASSERT_FALSE(xua_adaptive_window_sample(&window, (uint16_t)(65000 + 24576), 50000));
    TEST_ASSERT_EQUAL_UINT32(24576, window.mclk_ticks);
    TEST_ASSERT_EQUAL_INT32(0, xua_adaptive_window_error(&window, 512));
}

void test_adaptive_window_uses_actual_fractional_sample_count(void)
{
    xua_adaptive_window_t window = {0};
    uint16_t mclk = 0;
    window.received_frames = 44;
    xua_adaptive_window_sample(&window, mclk, 0);
    unsigned ready = 0;
    for (unsigned i = 1; i <= 16; ++i) {
        /* 44.1 kHz: 44/45 frames per millisecond, not a rounded 44. */
        unsigned frames = i / 10 - (i - 1) / 10 + 44;
        window.received_frames += frames;
        mclk = (uint16_t)(mclk + frames * 512);
        ready = xua_adaptive_window_sample(&window, mclk, i * 100000);
        TEST_ASSERT_EQUAL_UINT(i == 16, ready);
    }
    TEST_ASSERT_EQUAL_UINT32(705, window.received_frames);
    TEST_ASSERT_EQUAL_INT32(0, xua_adaptive_window_error(&window, 512));
    window.mclk_ticks += 50;
    TEST_ASSERT_EQUAL_INT32(50, xua_adaptive_window_error(&window, 512));
    window.mclk_ticks -= 100;
    TEST_ASSERT_EQUAL_INT32(-50, xua_adaptive_window_error(&window, 512));
}

void test_adaptive_window_discards_prebaseline_and_invalid_gap(void)
{
    xua_adaptive_window_t window = {0};
    window.received_frames = 100;
    xua_adaptive_window_sample(&window, 100, 100000);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_frames);
    window.received_frames = 48;
    xua_adaptive_window_sample(&window, 200, 400000);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_frames);
    TEST_ASSERT_EQUAL_UINT32(0, window.mclk_ticks);
    TEST_ASSERT_EQUAL_UINT32(0, window.interval_count);
    TEST_ASSERT_FALSE(window.have_baseline);
}

void test_adaptive_window_waits_for_playback_before_baseline(void)
{
    xua_adaptive_window_t window = {0};
    uint16_t mclk = 0;
    /* SOFs and MCLK run for a full window before playback arrives. */
    for (unsigned i = 1; i <= 16; ++i) {
        mclk = (uint16_t)(mclk + 24576);
        TEST_ASSERT_FALSE(xua_adaptive_window_sample(&window, mclk, i * 100000));
        TEST_ASSERT_FALSE(window.have_baseline);
    }
    /* Partial startup packet delivery is acquisition-only. */
    window.received_frames = 696;
    mclk = (uint16_t)(mclk + 24576);
    TEST_ASSERT_FALSE(xua_adaptive_window_sample(&window, mclk, 1700000));
    TEST_ASSERT_TRUE(window.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_frames);
    for (unsigned i = 1; i <= 16; ++i) {
        window.received_frames += 192;
        mclk = (uint16_t)(mclk + 24576);
        TEST_ASSERT_EQUAL_UINT(i == 16,
            xua_adaptive_window_sample(&window, mclk, (17 + i) * 100000));
    }
    TEST_ASSERT_EQUAL_UINT32(3072, window.received_frames);
    TEST_ASSERT_EQUAL_INT32(0, xua_adaptive_window_error(&window, 128));
}

void test_adaptive_window_reacquires_after_empty_window(void)
{
    xua_adaptive_window_t window = {0};
    window.received_frames = 48;
    xua_adaptive_window_sample(&window, 0, 0);
    for (unsigned i = 1; i <= 16; ++i) {
        TEST_ASSERT_FALSE(xua_adaptive_window_sample(&window,
            (uint16_t)(i * 24576), i * 100000));
    }
    TEST_ASSERT_FALSE(window.have_baseline);
    window.received_frames = 12;
    TEST_ASSERT_FALSE(xua_adaptive_window_sample(&window, 0, 1700000));
    TEST_ASSERT_TRUE(window.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_frames);
    TEST_ASSERT_EQUAL_UINT32(0, window.mclk_ticks);
}

/* Run one 16 ms window of 48 kHz playback at 24.576 MHz MCLK, starting from the current baseline */
static unsigned run_window_48k(xua_adaptive_window_t *window, uint16_t *mclk, uint32_t *ref,
                               unsigned frames_per_ms, int mclk_offset)
{
    unsigned ready = 0;
    for (unsigned i = 0; i < XUA_ADAPTIVE_WINDOW_INTERVALS; ++i) {
        window->received_frames += frames_per_ms;
        *mclk = (uint16_t)(*mclk + 24576 + (i == 0 ? mclk_offset : 0));
        *ref += 100000;
        ready = xua_adaptive_window_sample(window, *mclk, *ref);
    }
    return ready;
}

void test_adaptive_window_next_keeps_baseline(void)
{
    xua_adaptive_window_t window = {0};
    uint16_t mclk = 1000;
    uint32_t ref = 0;
    window.received_frames = 48;
    xua_adaptive_window_sample(&window, mclk, ref);

    TEST_ASSERT_TRUE(run_window_48k(&window, &mclk, &ref, 48, 0));
    TEST_ASSERT_EQUAL_INT32(0, xua_adaptive_window_error(&window, 512));
    xua_adaptive_window_next(&window);
    TEST_ASSERT_TRUE(window.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_frames);
    TEST_ASSERT_EQUAL_UINT32(0, window.mclk_ticks);
    TEST_ASSERT_EQUAL_UINT32(0, window.interval_count);
    TEST_ASSERT_EQUAL_UINT16(mclk, window.last_mclk_timestamp);

    /* The next window continues from the previous end point: no new baseline is needed */
    TEST_ASSERT_TRUE(run_window_48k(&window, &mclk, &ref, 48, 10));
    TEST_ASSERT_EQUAL_UINT32(16 * 24576 + 10, window.mclk_ticks);
    TEST_ASSERT_EQUAL_INT32(10, xua_adaptive_window_error(&window, 512));
}

void test_adaptive_window_error_valid_accepts_genuine_errors(void)
{
    xua_adaptive_window_t window = {0};
    window.mclk_ticks = 16 * 24576;
    window.received_frames = 16 * 48;
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, 0));
    /* 1000 ppm clock offset */
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, 393));
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, -393));
    /* One high-speed packet (6 frames at 48 kHz) either side of a window boundary */
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, 6 * 512));
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, -6 * 512));
}

void test_adaptive_window_error_valid_rejects_partial_window(void)
{
    xua_adaptive_window_t window = {0};
    uint16_t mclk = 0;
    uint32_t ref = 0;
    window.received_frames = 48;
    xua_adaptive_window_sample(&window, mclk, ref);

    /* Stream stops half way through the window */
    unsigned ready = 0;
    for (unsigned i = 0; i < XUA_ADAPTIVE_WINDOW_INTERVALS; ++i) {
        if (i < 8)
            window.received_frames += 48;
        mclk = (uint16_t)(mclk + 24576);
        ref += 100000;
        ready = xua_adaptive_window_sample(&window, mclk, ref);
    }
    TEST_ASSERT_TRUE(ready);
    int32_t error = xua_adaptive_window_error(&window, 512);
    TEST_ASSERT_EQUAL_INT32(8 * 24576, error);
    TEST_ASSERT_FALSE(xua_adaptive_window_error_valid(&window, error));
}

void test_adaptive_window_error_valid_limit(void)
{
    xua_adaptive_window_t window = {0};
    window.mclk_ticks = 16 * 24576;
    /* One lost full-speed packet (1 ms of samples) is rejected */
    TEST_ASSERT_FALSE(xua_adaptive_window_error_valid(&window, 24576));
    TEST_ASSERT_FALSE(xua_adaptive_window_error_valid(&window, -24576));
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, 24575));
    TEST_ASSERT_TRUE(xua_adaptive_window_error_valid(&window, -24575));
    /* Anything accepted fits in the SW PLL's int16_t error */
    TEST_ASSERT_TRUE(24575 <= INT16_MAX);
    /* An empty window has no valid error */
    window.mclk_ticks = 0;
    TEST_ASSERT_FALSE(xua_adaptive_window_error_valid(&window, 0));
}
