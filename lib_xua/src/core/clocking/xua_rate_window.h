// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XUA_RATE_WINDOW_H_
#define XUA_RATE_WINDOW_H_

#include <stdint.h>

/*
 * Rate measurement of a sample stream against MCLK, for the SW PLL.
 *
 * The MCLK port timestamp and the reference timer are sampled at regular intervals and the
 * MCLK ticks elapsed are compared with the samples received over a window of several
 * intervals, which smooths out the quantisation of samples into packets or ticks. The
 * resulting error is passed to the SW PLL, which performs the actual frequency control.
 *
 * Used by:
 *  - Adaptive USB playback: sampled every 1 ms (every SOF at full speed, every 8th SOF at
 *    high speed), 16 intervals per window, counting received frames.
 *  - S/PDIF and ADAT receive: sampled every fixed number of received samples (~1.67 ms),
 *    6 intervals per window, counting two per sample period. No interval timing bounds
 *    (0 and UINT32_MAX) or reference timestamp: clockgen's clock validity check resets
 *    the window on gaps in the stream.
 *
 * Each interval's MCLK delta is taken from a 16-bit port timestamp, so the maximum interval
 * multiplied by the MCLK frequency must be below 65536 ticks.
 *
 * Implemented as static inline functions so the same code is used from XC and the C unit tests.
 */

/* Adaptive USB playback settings. Interval bounds are reference-timer (100 MHz) ticks,
 * nominally 1 ms. The upper bound keeps the MCLK delta unambiguous for MCLK up to
 * 65535 / 1.5 ms = 43.7 MHz; only 22.5792 and 24.576 MHz are supported. */
#define XUA_RATE_WINDOW_ADAPTIVE_INTERVALS (16)
#define XUA_RATE_WINDOW_ADAPTIVE_MIN_TICKS (50000)
#define XUA_RATE_WINDOW_ADAPTIVE_MAX_TICKS (150000)

typedef struct {
    uint32_t received_samples;          /* Samples received in this window, in the caller's units. */
    uint32_t mclk_ticks;                /* MCLK ticks elapsed in this window. */
    uint32_t interval_count;            /* Intervals accumulated in this window. */
    uint32_t last_reference_timestamp;  /* Reference timer at the last sample. */
    uint16_t last_mclk_timestamp;       /* MCLK port timestamp at the last sample. */
    uint32_t have_baseline;             /* Non-zero once a starting timestamp has been taken. */
    uint32_t completed_samples;         /* Totals of the last complete window, kept by */
    uint32_t completed_mclk_ticks;      /* xua_rate_window_next() for diagnostics. */
    uint32_t intervals;                 /* Intervals per window. */
    uint32_t min_interval_ticks;        /* Reference-timer bounds on an interval; samples outside */
    uint32_t max_interval_ticks;        /* them (e.g. missed or delayed) discard the window. */
} xua_rate_window_t;

/* Discard the window and its baseline. Does not affect the PLL. */
static inline void xua_rate_window_reset(xua_rate_window_t *window)
{
    window->received_samples = 0;
    window->mclk_ticks = 0;
    window->interval_count = 0;
    window->last_reference_timestamp = 0;
    window->last_mclk_timestamp = 0;
    window->have_baseline = 0;
    window->completed_samples = 0;
    window->completed_mclk_ticks = 0;
}

/* Set the window length and interval bounds (reference-timer ticks), and reset the window. */
static inline void xua_rate_window_init(xua_rate_window_t *window, uint32_t intervals,
                                        uint32_t min_interval_ticks, uint32_t max_interval_ticks)
{
    window->intervals = intervals;
    window->min_interval_ticks = min_interval_ticks;
    window->max_interval_ticks = max_interval_ticks;
    xua_rate_window_reset(window);
}

/* Start the next window after a complete one, using the end of the completed window as
 * its baseline so that no time is lost between windows. */
static inline void xua_rate_window_next(xua_rate_window_t *window)
{
    window->completed_samples = window->received_samples;
    window->completed_mclk_ticks = window->mclk_ticks;
    window->received_samples = 0;
    window->mclk_ticks = 0;
    window->interval_count = 0;
}

/*
 * Add a sample of the MCLK and reference timestamps. The caller adds to received_samples
 * as samples arrive. Returns non-zero when a complete window is ready; the caller should then
 * read the error and call xua_rate_window_next().
 *
 * No baseline is taken until samples have been received. The samples received before
 * the baseline are discarded so that sample counting and MCLK counting start together.
 * A window with no samples at all is discarded and the baseline dropped, so that a later stream
 * starts afresh.
 */
static inline unsigned xua_rate_window_sample(
    xua_rate_window_t *window, uint16_t mclk_timestamp,
    uint32_t reference_timestamp)
{
    /* Unsigned subtraction handles timer wrap */
    uint32_t elapsed = reference_timestamp - window->last_reference_timestamp;

    if (!window->have_baseline)
    {
        if (window->received_samples == 0)
        {
            return 0;
        }
        xua_rate_window_reset(window);
    }
    else if (elapsed < window->min_interval_ticks || elapsed > window->max_interval_ticks)
    {
        /* Timing broken: wait for fresh samples before taking a new baseline */
        xua_rate_window_reset(window);
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

    if (window->interval_count == window->intervals && window->received_samples == 0)
    {
        xua_rate_window_reset(window);
        return 0;
    }
    return window->interval_count == window->intervals;
}

/* Error for a complete window in MCLK ticks: positive means MCLK ran faster than samples
 * arrived. mclks_per_count is the nominal MCLK ticks per unit added to received_samples.
 * The SW PLL wrapper applies the sign convention it needs. */
static inline int32_t xua_rate_window_error(
    const xua_rate_window_t *window, uint32_t mclks_per_count)
{
    return (int32_t)window->mclk_ticks -
           (int32_t)(window->received_samples * mclks_per_count);
}

/*
 * Returns non-zero if an error from xua_rate_window_error() is plausible and may be
 * passed to the PLL.
 *
 * Genuine errors are small: the clock offset plus the quantisation of samples at each window
 * boundary. A window where the stream stopped or samples were lost part way through gives a
 * much larger error, which must not reach the PLL. The limit is one interval of MCLK ticks,
 * capped so that the error fits the int16_t the SW PLL takes.
 */
static inline unsigned xua_rate_window_error_valid(
    const xua_rate_window_t *window, int32_t error)
{
    int32_t limit = (int32_t)(window->mclk_ticks / window->intervals);
    if (limit > 32768)
    {
        limit = 32768;
    }
    return (error > -limit) && (error < limit);
}

#endif
