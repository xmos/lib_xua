// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>

#include "xua_unit_tests.h"
#include "xua_hid_report_descriptor.h"

void test_uninitialised_hidGetReportDescriptor()
{
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NULL( reportDescPtr );
}

void test_initialised_hidGetReportDescriptor()
{
    hidInitReportDescriptor();
    unsigned char* reportDescPtr = hidGetReportDescriptor();
    TEST_ASSERT_NOT_NULL( reportDescPtr );
}
