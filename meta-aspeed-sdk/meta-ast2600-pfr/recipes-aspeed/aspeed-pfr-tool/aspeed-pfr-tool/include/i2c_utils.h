#pragma once
#include "arguments.h"

void printRawData(uint8_t *buf, int len);
int i2cOpenDev(int bus, int slave_addr);
int i2cWriteByteData(ARGUMENTS args, uint8_t offset, uint8_t value);
int i2cWriteBlockData(ARGUMENTS args, uint8_t offset, uint8_t length, uint8_t *value);
int i2cReadByteData(ARGUMENTS args, uint8_t offset);
int i2cReadBlockData(ARGUMENTS args, uint8_t offset, uint8_t length, uint8_t *value);

