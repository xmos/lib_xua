// Copyright 2017-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "device_access_usb.h"

int main(int argc, char **argv) {
    printf("In control host main\n");

    int ret = control_init_usb(0x20b1, 0x0016, 3);
    printf("control_init_usb() ret = %d\n", ret);

    uint8_t payload[10];
    for(int i=0; i<8; i++)
    {
        payload[i] = i+1;
    }
    ret = control_write_command(1, 2, payload, 8);
    printf("control_write_command(), ret = %d\n", ret);

    ret = control_read_command(1, 2, payload, 8);
    printf("control_read_command(), ret = %d\n", ret);
}
