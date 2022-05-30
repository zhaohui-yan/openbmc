#pragma once
#include <stdint.h>
#include "arguments.h"

#define SHA256_LENGTH 32
#define SHA384_LENGTH 48
#define SHA512_LENGTH 64

// Supported Hash Algorithms
typedef enum {
	Sha256,
	Sha384,
	Sha512
} HashAlg;

void provision(ARGUMENTS args);
void unprovision(ARGUMENTS args);

