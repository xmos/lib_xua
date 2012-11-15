#include <xs1.h>
#include <print.h>
#include "flash_interface.h"
#include "flash_programmer.h"

unsigned int flash_programmer(unsigned int cmd, unsigned int request_data[16]) {
  unsigned int return_data_len = 0;

  switch (cmd) {
  case FLASH_CMD_WRITE_PAGE:
    return_data_len = flash_cmd_write_page((request_data, unsigned char[64]));
    break;

  case FLASH_CMD_READ_PAGE:
    return_data_len = flash_cmd_read_page((request_data, unsigned char[64]));
    break;

  case FLASH_CMD_WRITE_PAGE_DATA:
    return_data_len = flash_cmd_write_page_data((request_data, unsigned char[64]));
    break;

  case FLASH_CMD_READ_PAGE_DATA:
    return_data_len = flash_cmd_read_page_data((request_data, unsigned char[64]));
    break;

  case FLASH_CMD_ERASE_ALL:
    return_data_len = flash_cmd_erase_all();
    break;

  case FLASH_CMD_REBOOT:
    return_data_len = flash_cmd_reboot();
    break;

  case FLASH_CMD_INIT:
    return_data_len = flash_cmd_init();
    break;

  case FLASH_CMD_DEINIT:
    return_data_len = flash_cmd_deinit();
    break;

  default:
    break;
  }

  return return_data_len;
}

int HandleUserDeviceRequest(unsigned int cmd, unsigned int to_device, 
                            unsigned int request_size, unsigned int request_data[16])
{

  unsigned int return_data_len = flash_programmer(cmd, request_data);

  return return_data_len;
}

