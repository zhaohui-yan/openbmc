#pragma once
#include <stdint.h>

// Supported Hash Algorithms
typedef enum {
	Sha256,
	Sha384,
	Sha512
} HashAlg;

int extractQxQyFromPubkey(const char *file, uint8_t *qx,
	uint8_t *qy, int *len);
int hashBuffer(const uint8_t *buffer, const int bufSize,
	const HashAlg hashAlg, uint8_t *hash, int *size);
int getRootKeyHash(const char *file, uint8_t *hash, int *size);
void provision(int fd, char *cmd);

