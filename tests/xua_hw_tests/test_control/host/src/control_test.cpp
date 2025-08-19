// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "device_access_usb.h"

int main(int argc, char **argv) {
    int ret = control_init_usb(0x20b1, 0x0016);
    if(ret)
    {
        printf("control_init_usb() returned error\n");
        exit(1);
    }

    uint8_t payload[10];
    for(int i=0; i<8; i++)
    {
        payload[i] = i+1;
    }
    // Write
    ret = control_write_command(1, 2, payload, 8);
    if(ret)
    {
        printf("control_write_command() returned error\n");
        exit(1);
    }

    // overwrite payload
    memset(payload, 0, 10*sizeof(uint8_t));

    // read back
    ret = control_read_command(1, 2, payload, 8);
    if(ret)
    {
        printf("control_read_command() returned error\n");
        exit(1);
    }

    for(int i=0; i<8; i++)
    {
        if(payload[i] != (i+1))
        {
            printf("Read result mismatch at index %d. Expected %d, read back %d\n", i, i+1, payload[i]);
            printf("read result = ");
            for(int j=0; j<8; j++)
            {
                printf("%d, ", payload[j]);
            }
            printf("\n");
            exit(1);
        }
        payload[i] = i+1;
    }

    return 0;
}
