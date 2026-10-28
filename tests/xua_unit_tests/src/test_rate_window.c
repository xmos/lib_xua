// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "unity.h"
#include "xua_rate_window.h"

static void init_adaptive(xua_rate_window_t *window)
{
    xua_rate_window_init(window, XUA_RATE_WINDOW_ADAPTIVE_INTERVALS,
                         XUA_RATE_WINDOW_ADAPTIVE_MIN_TICKS, XUA_RATE_WINDOW_ADAPTIVE_MAX_TICKS);
}

/* S/PDIF/ADAT receive: 6 intervals of ~1.67 ms (clockgen LOCAL_CLOCK_INCREMENT), two counts per
 * sample, no interval timing bounds and no reference timestamp */
#define DIG_RX_INTERVAL_TICKS (166667)

static void init_dig_rx(xua_rate_window_t *window)
{
    xua_rate_window_init(window, 6, 0, UINT32_MAX);
}

void test_adaptive_window_counter_wrap(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    window.received_samples = 48;
    TEST_ASSERT_FALSE(xua_rate_window_sample(&window, 65000, UINT32_MAX - 49999));
    window.received_samples = 48;
    TEST_ASSERT_FALSE(xua_rate_window_sample(&window, (uint16_t)(65000 + 24576), 50000));
    TEST_ASSERT_EQUAL_UINT32(24576, window.mclk_ticks);
    TEST_ASSERT_EQUAL_INT32(0, xua_rate_window_error(&window, 512));
}

void test_adaptive_window_uses_actual_fractional_sample_count(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    uint16_t mclk = 0;
    window.received_samples = 44;
    xua_rate_window_sample(&window, mclk, 0);
    unsigned ready = 0;
    for (unsigned i = 1; i <= 16; ++i) {
        /* 44.1 kHz: 44/45 frames per millisecond, not a rounded 44. */
        unsigned frames = i / 10 - (i - 1) / 10 + 44;
        window.received_samples += frames;
        mclk = (uint16_t)(mclk + frames * 512);
        ready = xua_rate_window_sample(&window, mclk, i * 100000);
        TEST_ASSERT_EQUAL_UINT(i == 16, ready);
    }
    TEST_ASSERT_EQUAL_UINT32(705, window.received_samples);
    TEST_ASSERT_EQUAL_INT32(0, xua_rate_window_error(&window, 512));
    window.mclk_ticks += 50;
    TEST_ASSERT_EQUAL_INT32(50, xua_rate_window_error(&window, 512));
    window.mclk_ticks -= 100;
    TEST_ASSERT_EQUAL_INT32(-50, xua_rate_window_error(&window, 512));
}

void test_adaptive_window_discards_prebaseline_and_invalid_gap(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    window.received_samples = 100;
    xua_rate_window_sample(&window, 100, 100000);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_samples);
    window.received_samples = 48;
    xua_rate_window_sample(&window, 200, 400000);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_samples);
    TEST_ASSERT_EQUAL_UINT32(0, window.mclk_ticks);
    TEST_ASSERT_EQUAL_UINT32(0, window.interval_count);
    TEST_ASSERT_FALSE(window.have_baseline);
}

void test_adaptive_window_waits_for_playback_before_baseline(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    uint16_t mclk = 0;
    /* SOFs and MCLK run for a full window before playback arrives. */
    for (unsigned i = 1; i <= 16; ++i) {
        mclk = (uint16_t)(mclk + 24576);
        TEST_ASSERT_FALSE(xua_rate_window_sample(&window, mclk, i * 100000));
        TEST_ASSERT_FALSE(window.have_baseline);
    }
    /* Partial startup packet delivery is acquisition-only. */
    window.received_samples = 696;
    mclk = (uint16_t)(mclk + 24576);
    TEST_ASSERT_FALSE(xua_rate_window_sample(&window, mclk, 1700000));
    TEST_ASSERT_TRUE(window.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_samples);
    for (unsigned i = 1; i <= 16; ++i) {
        window.received_samples += 192;
        mclk = (uint16_t)(mclk + 24576);
        TEST_ASSERT_EQUAL_UINT(i == 16,
            xua_rate_window_sample(&window, mclk, (17 + i) * 100000));
    }
    TEST_ASSERT_EQUAL_UINT32(3072, window.received_samples);
    TEST_ASSERT_EQUAL_INT32(0, xua_rate_window_error(&window, 128));
}

void test_adaptive_window_reacquires_after_empty_window(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    window.received_samples = 48;
    xua_rate_window_sample(&window, 0, 0);
    for (unsigned i = 1; i <= 16; ++i) {
        TEST_ASSERT_FALSE(xua_rate_window_sample(&window,
            (uint16_t)(i * 24576), i * 100000));
    }
    TEST_ASSERT_FALSE(window.have_baseline);
    window.received_samples = 12;
    TEST_ASSERT_FALSE(xua_rate_window_sample(&window, 0, 1700000));
    TEST_ASSERT_TRUE(window.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_samples);
    TEST_ASSERT_EQUAL_UINT32(0, window.mclk_ticks);
}

/* Run one 16 ms window of 48 kHz playback at 24.576 MHz MCLK, starting from the current baseline */
static unsigned run_window_48k(xua_rate_window_t *window, uint16_t *mclk, uint32_t *ref,
                               unsigned frames_per_ms, int mclk_offset)
{
    unsigned ready = 0;
    for (unsigned i = 0; i < XUA_RATE_WINDOW_ADAPTIVE_INTERVALS; ++i) {
        window->received_samples += frames_per_ms;
        *mclk = (uint16_t)(*mclk + 24576 + (i == 0 ? mclk_offset : 0));
        *ref += 100000;
        ready = xua_rate_window_sample(window, *mclk, *ref);
    }
    return ready;
}

void test_adaptive_window_next_keeps_baseline(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    uint16_t mclk = 1000;
    uint32_t ref = 0;
    window.received_samples = 48;
    xua_rate_window_sample(&window, mclk, ref);

    TEST_ASSERT_TRUE(run_window_48k(&window, &mclk, &ref, 48, 0));
    TEST_ASSERT_EQUAL_INT32(0, xua_rate_window_error(&window, 512));
    xua_rate_window_next(&window);
    TEST_ASSERT_EQUAL_UINT32(16 * 48, window.completed_samples);
    TEST_ASSERT_EQUAL_UINT32(16 * 24576, window.completed_mclk_ticks);
    TEST_ASSERT_TRUE(window.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(0, window.received_samples);
    TEST_ASSERT_EQUAL_UINT32(0, window.mclk_ticks);
    TEST_ASSERT_EQUAL_UINT32(0, window.interval_count);
    TEST_ASSERT_EQUAL_UINT16(mclk, window.last_mclk_timestamp);

    /* The next window continues from the previous end point: no new baseline is needed */
    TEST_ASSERT_TRUE(run_window_48k(&window, &mclk, &ref, 48, 10));
    TEST_ASSERT_EQUAL_UINT32(16 * 24576 + 10, window.mclk_ticks);
    TEST_ASSERT_EQUAL_INT32(10, xua_rate_window_error(&window, 512));
}

void test_adaptive_window_error_valid_accepts_genuine_errors(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    window.mclk_ticks = 16 * 24576;
    window.received_samples = 16 * 48;
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, 0));
    /* 1000 ppm clock offset */
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, 393));
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, -393));
    /* One high-speed packet (6 frames at 48 kHz) either side of a window boundary */
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, 6 * 512));
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, -6 * 512));
}

void test_adaptive_window_error_valid_rejects_partial_window(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    uint16_t mclk = 0;
    uint32_t ref = 0;
    window.received_samples = 48;
    xua_rate_window_sample(&window, mclk, ref);

    /* Stream stops half way through the window */
    unsigned ready = 0;
    for (unsigned i = 0; i < XUA_RATE_WINDOW_ADAPTIVE_INTERVALS; ++i) {
        if (i < 8)
            window.received_samples += 48;
        mclk = (uint16_t)(mclk + 24576);
        ref += 100000;
        ready = xua_rate_window_sample(&window, mclk, ref);
    }
    TEST_ASSERT_TRUE(ready);
    int32_t error = xua_rate_window_error(&window, 512);
    TEST_ASSERT_EQUAL_INT32(8 * 24576, error);
    TEST_ASSERT_FALSE(xua_rate_window_error_valid(&window, error));
}

void test_adaptive_window_error_valid_limit(void)
{
    xua_rate_window_t window;
    init_adaptive(&window);
    window.mclk_ticks = 16 * 24576;
    /* One lost full-speed packet (1 ms of samples) is rejected */
    TEST_ASSERT_FALSE(xua_rate_window_error_valid(&window, 24576));
    TEST_ASSERT_FALSE(xua_rate_window_error_valid(&window, -24576));
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, 24575));
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, -24575));
    /* Anything accepted fits in the SW PLL's int16_t error */
    TEST_ASSERT_TRUE(24575 <= INT16_MAX);
    /* An empty window has no valid error */
    window.mclk_ticks = 0;
    TEST_ASSERT_FALSE(xua_rate_window_error_valid(&window, 0));
}

/* Run one dig RX window: 6 calls of counts_per_tick counts, MCLK advancing by mclk_per_tick */
static unsigned run_window_dig_rx(xua_rate_window_t *window, uint16_t *mclk,
                                  unsigned counts_per_tick, unsigned mclk_per_tick, int mclk_offset)
{
    unsigned ready = 0;
    for (unsigned i = 0; i < 6; ++i) {
        window->received_samples += counts_per_tick;
        *mclk = (uint16_t)(*mclk + mclk_per_tick + (i == 0 ? mclk_offset : 0));
        ready = xua_rate_window_sample(window, *mclk, 0);
    }
    return ready;
}

void test_dig_rx_window_48k(void)
{
    xua_rate_window_t window;
    init_dig_rx(&window);
    uint16_t mclk = 0;
    window.received_samples = 160;
    xua_rate_window_sample(&window, mclk, 0);

    /* 160 counts (80 samples) per tick at 24.576 MHz: 40960 MCLK ticks, more than an int16 */
    TEST_ASSERT_TRUE(run_window_dig_rx(&window, &mclk, 160, 40960, 0));
    TEST_ASSERT_EQUAL_UINT32(6 * 40960, window.mclk_ticks);
    TEST_ASSERT_EQUAL_INT32(0, xua_rate_window_error(&window, 512 / 2));
    xua_rate_window_next(&window);

    TEST_ASSERT_TRUE(run_window_dig_rx(&window, &mclk, 160, 40960, 25));
    TEST_ASSERT_EQUAL_INT32(25, xua_rate_window_error(&window, 512 / 2));
    xua_rate_window_next(&window);

    TEST_ASSERT_TRUE(run_window_dig_rx(&window, &mclk, 160, 40960, -25));
    TEST_ASSERT_EQUAL_INT32(-25, xua_rate_window_error(&window, 512 / 2));
}

void test_dig_rx_window_44k1_odd_count(void)
{
    xua_rate_window_t window;
    init_dig_rx(&window);
    uint16_t mclk = 0;
    window.received_samples = 147;
    xua_rate_window_sample(&window, mclk, 0);

    /* 147 counts (73.5 samples) per tick at 22.5792 MHz: 37632 MCLK ticks */
    TEST_ASSERT_TRUE(run_window_dig_rx(&window, &mclk, 147, 37632, 0));
    TEST_ASSERT_EQUAL_UINT32(882, window.received_samples);
    TEST_ASSERT_EQUAL_INT32(0, xua_rate_window_error(&window, 512 / 2));
}

void test_rate_window_bounds_are_per_window(void)
{
    xua_rate_window_t adaptive;
    xua_rate_window_t dig_rx;
    init_adaptive(&adaptive);
    init_dig_rx(&dig_rx);
    adaptive.received_samples = 1;
    dig_rx.received_samples = 1;
    xua_rate_window_sample(&adaptive, 0, 0);
    xua_rate_window_sample(&dig_rx, 0, 0);

    /* A ~1.67 ms interval is too long for adaptive */
    xua_rate_window_sample(&adaptive, 40960, DIG_RX_INTERVAL_TICKS);
    TEST_ASSERT_FALSE(adaptive.have_baseline);

    /* Without bounds any interval is accepted, including a zero reference timestamp */
    xua_rate_window_sample(&dig_rx, 40960, 0);
    xua_rate_window_sample(&dig_rx, (uint16_t)(2 * 40960), UINT32_MAX / 2);
    TEST_ASSERT_TRUE(dig_rx.have_baseline);
    TEST_ASSERT_EQUAL_UINT32(2, dig_rx.interval_count);
}

void test_dig_rx_window_error_valid_fits_int16(void)
{
    xua_rate_window_t window;
    init_dig_rx(&window);
    /* One interval is 40960 MCLK ticks, so the limit is capped by the SW PLL's int16_t */
    window.mclk_ticks = 6 * 40960;
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, INT16_MAX));
    TEST_ASSERT_TRUE(xua_rate_window_error_valid(&window, INT16_MIN + 1));
    TEST_ASSERT_FALSE(xua_rate_window_error_valid(&window, INT16_MAX + 1));
    TEST_ASSERT_FALSE(xua_rate_window_error_valid(&window, INT16_MIN));
}
