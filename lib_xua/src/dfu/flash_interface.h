// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _flash_interface_h_
#define _flash_interface_h_

int flash_cmd_init(void);
/**
 * Prepare to write a page of a new upgrade image.
 * The first word of data should be set to 0 if it is the first page,
 * 1 for all other pages and 2 to terminate the write (no further data is sent).
 */
int flash_cmd_write_page(unsigned char []);
/**
 * Provide upgrade image data. flash_cmd_write_page() must be called previously.
 * Once a page of data has been provided it is written to the device.
 */
int flash_cmd_write_page_data(unsigned char []);
/**
 * Read a page of data from the upgrade image.
 * If the first word of data is 0 the page is read from the start of the
 * upgrade image, otherwise the next page in the image will be read.
 * On return the first word of data is written with 1 if there is nothing to
 * read and 0 otherwise.
 */
int flash_cmd_read_page(unsigned char []);
/**
 * Get data previously read by flash_cmd_read_page().
 */
int flash_cmd_read_page_data(unsigned char []);
int flash_cmd_erase_all(void);
int flash_cmd_reboot(void);
int flash_cmd_init(void);
int flash_cmd_deinit(void);

#endif /*_flash_interface_h_*/
