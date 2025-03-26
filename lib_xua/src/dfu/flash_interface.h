// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _flash_interface_h_
#define _flash_interface_h_

int flash_cmd_init(void);

/// Prepare to write a new image to the flash
int flash_cmd_start_write_image();

/// Reset the subpage index back to 0
void flash_cmd_reset_subpage_index();

/// Finish writing image to the flash
void flash_cmd_end_write_image();

/**
 * Provide upgrade image data. flash_cmd_start_write_image() must be called previously.
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
void flash_cmd_read_page(unsigned char []);
/**
 * Get data previously read by flash_cmd_read_page().
 */
int flash_cmd_read_page_data(unsigned char []);
int flash_cmd_erase_all(void);
int flash_cmd_reboot(void);
int flash_cmd_init(void);
int flash_cmd_deinit(void);

#endif /*_flash_interface_h_*/
