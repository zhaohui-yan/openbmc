#pragma once

void print_raw_data(uint8_t *buf, int len);
int open_i2c_dev(int bus, int slave_addr);
int i2cWriteByteData(int fd, uint8_t offset, uint8_t value);
int i2cWriteBlockData(int fd, uint8_t offset, uint8_t length, uint8_t *value);
int i2cReadByteData(int fd, uint8_t offset);
int i2cReadBlockData(int fd, uint8_t offset, uint8_t length, uint8_t *value);

