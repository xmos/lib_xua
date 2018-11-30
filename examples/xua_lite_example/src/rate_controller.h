#include <stdint.h>
#include <limits.h>

typedef int32_t xua_lite_fixed_point_t;

typedef struct pid_state_t{
  xua_lite_fixed_point_t fifo_level_filtered_old;
  xua_lite_fixed_point_t fifo_level_accum;
} pid_state_t;


//USB Adaptive mode helper
xua_lite_fixed_point_t do_rate_control(int fill_level, pid_state_t *pid_state);

//PDM modulator for clock control
void do_clock_nudge_pdm(xua_lite_fixed_point_t controller_out, int *clock_nudge);


//USB Asynch mode helper
void do_feedback_calculation(unsigned &sof_count, const unsigned mclk_hz, unsigned mclk_port_counter,unsigned &mclk_port_counter_old
                                ,long long &feedback_value, unsigned &mod_from_last_time, unsigned fb_clocks[1]);