// Copyright 2018-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef VTB_UNIT_TESTS_H_
#define VTB_UNIT_TESTS_H_

#include "unity.h"

#ifdef __XC__

#include <xs1.h>
#include <string.h>
#include <math.h>

#include <xclib.h>

#include "audio_test_tools.h"
#include "voice_toolbox.h"
#include "voice_toolbox_fp.h"
#include "vtb_references.h"

#define TEST_ASM 1

#endif // __XC__

#endif /* VTB_UNIT_TESTS_H_ */
