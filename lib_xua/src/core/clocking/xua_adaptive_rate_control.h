// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XUA_ADAPTIVE_RATE_CONTROL_H_
#define XUA_ADAPTIVE_RATE_CONTROL_H_

#include <stdint.h>

/*
 * Adaptive playback rate measurement.
 *
 * MCLK is sampled once per millisecond (every SOF at full speed, every 8th SOF at high speed)
 * and 16 intervals are combined into one measurement window, which smooths out the
 * quantisation of samples into USB packets. The resulting error is passed to the SW PLL,
 * which performs the actual frequency control.
 *
 * Implemented as static inline functions so the same code is used from XC and the C unit tests.
 */
#define XUA_ADAPTIVE_WINDOW_INTERVALS (16)

/* Bounds on the reference-timer time between samples (100 MHz ticks), nominally 1 ms.
 * Samples outside this range (e.g. missed or delayed SOFs) discard the window.
 * The upper bound keeps the 16-bit MCLK timestamp delta unambiguous for MCLK up to
 * 65535 / 1.5 ms = 43.7 MHz; only 22.5792 and 24.576 MHz are supported. */
#define XUA_ADAPTIVE_MIN_INTERVAL_TICKS (50000)
#define XUA_ADAPTIVE_MAX_INTERVAL_TICKS (150000)

typedef struct {
    uint32_t received_frames;           /* Complete OUT sample frames received in this window. */
    uint32_t mclk_ticks;                /* MCLK ticks elapsed in this window. */
    uint32_t interval_count;            /* Intervals accumulated in this window. */
    uint32_t last_reference_timestamp;  /* Reference timer at the last sample. */
    uint16_t last_mclk_timestamp;       /* MCLK port timestamp at the last sample. */
    uint32_t have_baseline;             /* Non-zero once a starting timestamp has been taken. */
} xua_adaptive_window_t;

/* Discard the window and its baseline. Does not affect the PLL. */
static inline void xua_adaptive_window_reset(xua_adaptive_window_t *window)
{
    window->received_frames = 0;
    window->mclk_ticks = 0;
    window->interval_count = 0;
    window->last_reference_timestamp = 0;
    window->last_mclk_timestamp = 0;
    window->have_baseline = 0;
}

/* Start the next window after a complete one, using the end of the completed window as
 * its baseline so that no time is lost between windows. */
static inline void xua_adaptive_window_next(xua_adaptive_window_t *window)
{
    window->received_frames = 0;
    window->mclk_ticks = 0;
    window->interval_count = 0;
}

/*
 * Add a sample of the MCLK and reference timestamps, taken at an SOF.
 * Returns non-zero when a complete window is ready; the caller should then read the error
 * and call xua_adaptive_window_next().
 *
 * No baseline is taken until playback frames have been received. The frames received before
 * the baseline are discarded so that frame counting and MCLK counting start together.
 * A window with no frames at all is discarded and the baseline dropped, so that a later stream
 * starts afresh.
 */
static inline unsigned xua_adaptive_window_sample(
    xua_adaptive_window_t *window, uint16_t mclk_timestamp,
    uint32_t reference_timestamp)
{
    /* Unsigned subtraction handles timer wrap */
    uint32_t elapsed = reference_timestamp - window->last_reference_timestamp;

    if (!window->have_baseline)
    {
        if (window->received_frames == 0)
        {
            return 0;
        }
        xua_adaptive_window_reset(window);
    }
    else if (elapsed < XUA_ADAPTIVE_MIN_INTERVAL_TICKS || elapsed > XUA_ADAPTIVE_MAX_INTERVAL_TICKS)
    {
        /* Timing broken: wait for fresh playback before taking a new baseline */
        xua_adaptive_window_reset(window);
        return 0;
    }
    else
    {
        window->mclk_ticks += (uint16_t)(mclk_timestamp - window->last_mclk_timestamp);
        window->interval_count++;
    }

    window->last_mclk_timestamp = mclk_timestamp;
    window->last_reference_timestamp = reference_timestamp;
    window->have_baseline = 1;

    if (window->interval_count == XUA_ADAPTIVE_WINDOW_INTERVALS && window->received_frames == 0)
    {
        xua_adaptive_window_reset(window);
        return 0;
    }
    return window->interval_count == XUA_ADAPTIVE_WINDOW_INTERVALS;
}

/* Error for a complete window in MCLK ticks: positive means MCLK ran faster than the host
 * delivered samples. The SW PLL wrapper applies the sign convention it needs. */
static inline int32_t xua_adaptive_window_error(
    const xua_adaptive_window_t *window, uint32_t mclks_per_sample)
{
    return (int32_t)window->mclk_ticks -
           (int32_t)(window->received_frames * mclks_per_sample);
}

/*
 * Returns non-zero if an error from xua_adaptive_window_error() is plausible and may be
 * passed to the PLL.
 *
 * Genuine errors are small: the clock offset plus at most one high-speed packet of samples
 * at each window boundary. A window where the stream stopped or packets were lost part way
 * through gives a much larger error, which must not reach the PLL. The SW PLL also takes the
 * error as an int16_t, so larger values would wrap. The limit is one interval (1 ms) of MCLK
 * ticks, which is below 32768 for the supported MCLK frequencies.
 */
static inline unsigned xua_adaptive_window_error_valid(
    const xua_adaptive_window_t *window, int32_t error)
{
    const int32_t limit = (int32_t)(window->mclk_ticks / XUA_ADAPTIVE_WINDOW_INTERVALS);
    return (error > -limit) && (error < limit);
}

#endif
