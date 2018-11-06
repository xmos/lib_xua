#include <xs1.h>
#include "xua_ep0_wrapper.h"


[[combinable]]
unsafe void XUA_Buffer_lite(chanend c_ep0_out, chanend c_ep0_in, chanend c_aud_out, chanend ?c_feedback, chanend c_aud_in, chanend c_sof, in port p_for_mclk_count, streaming chanend c_audio_hub);
[[combinable]]
unsafe void XUA_Buffer_lite2(server ep0_control_if i_ep0_ctl, chanend c_aud_out, chanend ?c_feedback, chanend c_aud_in, chanend c_sof, in port p_for_mclk_count, streaming chanend c_audio_hub);