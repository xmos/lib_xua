#include <stdio.h>
#include <assert.h>
#if !defined(_MSC_VER) || (_MSC_VER >= 1800) // !MSVC or MSVC >=VS2013
#include <stdbool.h>
#else
typedef enum { false = 0, true = 1} bool;
#endif // MSC
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <libusb.h>

#define DBG(x) x
#define PRINT_ERROR(...)   fprintf(stderr, "Error  : " __VA_ARGS__)

static unsigned num_commands = 0;

static libusb_device_handle *devh = NULL;

static const int sync_timeout_ms = 500;

/* Control query transfers require smaller buffers */
#define VERSION_MAX_PAYLOAD_SIZE 64

void debug_libusb_error(int err_code)
{
#if defined _WIN32
  PRINT_ERROR("libusb_control_transfer returned %s\n", libusb_error_name(errno));
#elif defined __APPLE__
  PRINT_ERROR("libusb_control_transfer returned %s\n", libusb_error_name(err_code));
#elif defined __linux
  PRINT_ERROR("libusb_control_transfer returned %d\n", err_code);
#endif
}

int control_init_usb(int vendor_id, int product_id, int interface_num)
{
  int ret = libusb_init(NULL);
  if (ret < 0) {
    PRINT_ERROR("Failed to initialise libusb\n");
    return -1;
  }

  libusb_device **devs = NULL;
  int num_dev = libusb_get_device_list(NULL, &devs);

  libusb_device *dev = NULL;
  for (int i = 0; i < num_dev; i++) {
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(devs[i], &desc);
    if (desc.idVendor == vendor_id && desc.idProduct == product_id) {
      dev = devs[i];
      break;
    }
  }

  if (dev == NULL) {
    // Do not add any error printout here.
    // This case  will be called multiple times when searching for a list of devices.
    return -1;
  }

  if (libusb_open(dev, &devh) < 0) {
    PRINT_ERROR("Failed to open device. Ensure adequate permissions if using Linux,\nor remove any pre-installed drivers with Device Manager on Windows.\n");
    return -1;
  }

  libusb_free_device_list(devs, 1);

  return 0;
}

int control_cleanup_usb(void)
{
  libusb_close(devh);
  libusb_exit(NULL);

  return -1;
}


/**
 * Sets the read bit on a command code
 *
 * \param[in,out] c The command code to set the read bit on.
 */
#define CONTROL_CMD_SET_READ(c) ((c) | 0x80)

/**
 * Clears the read bit on a command code
 *
 * \param[in,out] c The command code to clear the read bit on.
 */
#define CONTROL_CMD_SET_WRITE(c) ((c) & ~0x80)

void print_bytes(const unsigned char data[], int num_bytes)
{
  int i;
  for (i = 0; i < num_bytes; i++) {
    printf("%02x ", data[i]);
  }
  printf("\n");
}

#define USB_TRANSACTION_MAX_BYTES (64)
static bool payload_len_exceeds_control_packet_size(size_t payload_len)
{
  if (payload_len > USB_TRANSACTION_MAX_BYTES) {
    printf("control transfer of %zd bytes requested\n", payload_len);
    printf("maximum control packet size is %d\n", USB_TRANSACTION_MAX_BYTES);
    return true;
  }
  else {
    return false;
  }
}

void control_usb_fill_header(uint16_t *windex, uint16_t *wvalue, uint16_t *wlength,
                        uint8_t resid, uint8_t cmd, unsigned payload_len)
{
  *windex = resid;
  *wvalue = cmd;

  assert(payload_len < (1<<16) && "payload length can't be represented as a uint16_t");
  *wlength = (uint16_t)payload_len;
}

int control_read_command(uint8_t resid, uint8_t cmd,
                     uint8_t payload[], size_t payload_len)
{
  uint16_t windex, wvalue, wlength;

  if (payload_len_exceeds_control_packet_size(payload_len))
  {
    PRINT_ERROR("Payload len %zu exceeds max packet size\n", payload_len);
    return -1;
  }

  control_usb_fill_header(&windex, &wvalue, &wlength,
    resid, CONTROL_CMD_SET_READ(cmd), (unsigned int)payload_len);

  DBG(printf("%u: send read command: 0x%04x 0x%04x 0x%04x\n",
    num_commands, windex, wvalue, wlength));

  int ret = libusb_control_transfer(devh,
    LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
    0, wvalue, windex, payload, wlength, sync_timeout_ms);

  num_commands++;

  if (ret != (int)payload_len) {
    debug_libusb_error(ret);
    return -1;
  }

  DBG(printf("read data returned: "));
  DBG(print_bytes(payload, payload_len));

  return 0;
}

int control_write_command(uint8_t resid, uint8_t cmd,
                      const uint8_t payload[], size_t payload_len)
{
  uint16_t windex, wvalue, wlength;

  if (payload_len_exceeds_control_packet_size(payload_len))
  {
    PRINT_ERROR("Payload len %zu exceeds max packet size\n", payload_len);
    return -1;
  }

  control_usb_fill_header(&windex, &wvalue, &wlength,
    resid, CONTROL_CMD_SET_WRITE(cmd), (unsigned int)payload_len);

  DBG(printf("%u: send write command: 0x%04x 0x%04x 0x%04x ",
    num_commands, windex, wvalue, wlength));
  DBG(print_bytes(payload, payload_len));

  int ret = libusb_control_transfer(devh,
    LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
    0, wvalue, windex, (unsigned char*)payload, wlength, sync_timeout_ms);

  num_commands++;

  if (ret != (int)payload_len) {
    debug_libusb_error(ret);
    return -1;
  }
  return 0;
}
