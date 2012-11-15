#define FLASH_CMD_WRITE_PAGE 0xf1
#define FLASH_CMD_READ_PAGE 0xf2
#define FLASH_CMD_WRITE_PAGE_DATA 0xf3
#define FLASH_CMD_READ_PAGE_DATA 0xf4
#define FLASH_CMD_ERASE_ALL 0xf5
#define FLASH_CMD_REBOOT 0xf6
#define FLASH_CMD_INIT 0xf7
#define FLASH_CMD_DEINIT 0xf8

unsigned int flash_programmer(unsigned int cmd, unsigned int request_data[16]);
int HandleUserDeviceRequest(unsigned int cmd, unsigned int to_device, 
                            unsigned int request_size, unsigned int request_data[16]);

