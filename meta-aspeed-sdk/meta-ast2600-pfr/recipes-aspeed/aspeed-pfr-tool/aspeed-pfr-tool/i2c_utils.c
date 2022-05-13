#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void print_raw_data(uint8_t *buf, int len)
{
	int i = 0;

	for (i = 0; i < len; ++i) {
		printf("%02x ", buf[i]);
		if ((i+1)%16 == 0)
			printf("\n");
	}

	printf("\n");
}

int open_i2c_dev(int bus, int slave_addr)
{
	char filename[20];
	int fd;

	snprintf(filename, 19, "/dev/i2c-%d", bus);
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("Unable to open i2c device.");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, I2C_SLAVE_FORCE, slave_addr) < 0) {
		printf("Unable to set i2c slave address, %x.\n", slave_addr);
		close(fd);
		exit(EXIT_FAILURE);
	}

	return fd;
}

int i2cWriteByteData(int fd, uint8_t offset, uint8_t value)
{
	int retries = 3;

	while (i2c_smbus_write_byte_data(fd, offset, value) < 0) {
		printf("i2c write byte failed, retrying....%d", retries);
		if (!retries--)	{
			printf("i2c_smbus_write_byte_data() failed");
			return 1;
		}
		usleep(10*1000);
	}

	return 0;
}

int i2cWriteBlockData(int fd, uint8_t offset, uint8_t length, uint8_t *value)
{
	int retries = 3;

	while (i2c_smbus_write_i2c_block_data(fd, offset, length, value) < 0) {
		printf("i2c write block failed, retrying....%d", retries);
		if (!retries--)	{
			printf("i2c_smbus_write_i2c_block_data() failed");
			return 1;
		}
		usleep(10*1000);
	}

	return 0;
}

uint8_t i2cReadByteData(int fd, uint8_t offset, uint8_t *value)
{
	int res = i2c_smbus_read_byte_data(fd, offset);

	if (res < 0) {
		printf("i2c_smbus_read_byte_data() failed");
		return 1;
	}

	*value = (uint8_t) res;
	return 0;
}

int i2cReadBlockData(int fd, uint8_t offset, uint8_t length, uint8_t *value)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(fd, offset, length, value);

	if (ret < 0) {
		printf("i2c_smbus_read_i2c_block_data() failed");
		return -1;
	}

	return ret;
}
