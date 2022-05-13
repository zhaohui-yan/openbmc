#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include "provision.h"
#include "i2c_utils.h"

extern uint8_t debug_flag;

int extractQxQyFromPubkey(const char *file, uint8_t *qx, uint8_t *qy, int *len)
{
	EC_KEY *eckey = NULL;
	uint8_t *pub;
	int ret = 1;
	int publen;
	int i;
	int j;

	if (file == NULL) {
		printf("%s, File is NULL.\n", __func__);
		return ret;
	}

	BIO * in = BIO_new_file(file, "r");

	if (in == NULL) {
		printf("%s, Failed to read eckey %s.\n", __func__, file);
		goto bio;
	}

	eckey = PEM_read_bio_EC_PUBKEY(in, NULL, NULL, NULL);

	if (eckey == NULL) {
		printf("%s, Failed to read eckey %s.\n", __func__, file);
		goto bio;
	}

	publen = EC_KEY_key2buf(eckey, EC_KEY_get_conv_form(eckey), &pub, NULL);

	if (pub[0] != 0x04) {
		// key is compressed, we don't support this
		printf("%s, Key is in compressed format. This is currently not supported.\n", __func__);
		goto bio;
	}

	*len = (publen - 1) / 2;

	// set qx and qy
	for (i = 1, j = 0; j < *len; ++i, ++j)
		qx[j] = pub[i];

	for (i = *len + 1, j = 0; j < *len; ++i, ++j)
		qy[j] = pub[i];

	ret = 0;

bio:
	BIO_free(in);
	return ret;
}

int hashBuffer(const uint8_t *buffer, const int bufSize, const HashAlg hashAlg,
	uint8_t *hash, int *size)
{
	EVP_MD_CTX *mctx = NULL;
	const EVP_MD *md = NULL;
	int ret = 1;

	// check for any NULLs
	if (buffer == NULL || hash == NULL || size == NULL) {
		printf("%s, Buffer, hash or size is NULL.\n", __func__);
		return ret;
	}

	if (hashAlg == Sha256) {
		*size = SHA256_DIGEST_LENGTH;
		md = EVP_sha256();
	} else if (hashAlg == Sha384) {
		*size = SHA384_DIGEST_LENGTH;
		md = EVP_sha384();
	} else if (hashAlg == Sha512) {
		*size = SHA512_DIGEST_LENGTH;
		md = EVP_sha512();
	}

	mctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(mctx, md, NULL);
	ret = EVP_DigestUpdate(mctx, buffer, bufSize);

	// Final frees mctx memory
	if (ret != 0 && EVP_DigestFinal_ex(mctx, hash, (unsigned int *)size) <= 0) {
		printf("%s, Failed to generate hash.\n", __func__);
		ret = 1;
	}

	EVP_MD_CTX_free(mctx);
	return ret;
}

int getRootKeyHash(const char *file, uint8_t *hash, int *len)
{
	uint8_t pubkey_x[48];
	uint8_t pubkey_y[48];
	uint8_t buffer[96];
	int size = 0;
	int i;

	if (extractQxQyFromPubkey(file, pubkey_x, pubkey_y, &size)) {
		printf("%s, Failed to extract QX and QY.\n", __func__);
		return 1;
	}

	if ((size == 32) || (size == 48)) {
		// Collect root public key X and Y, in little endian byte order.
		for (i = 0; i < size; i++) {
			buffer[i] = pubkey_x[size - 1 - i];
			buffer[i + size] = pubkey_y[size - 1 - i];
		}

		if (size == 32)
			hashBuffer(buffer, size*2, Sha256, hash, len);
		else
			hashBuffer(buffer, size*2, Sha384, hash, len);
	} else {
		printf("%s, Unsuported key size.\n", __func__);
		return 1;
	}

	return 0;
}

void provision(int fd, char *cmd)
{
	uint8_t buffer[96];
	int hashLen = 0;

	if (strncmp(cmd, "show", strlen(cmd)) == 0)
		printf("do show command\n");
	else {
		if (getRootKeyHash(cmd, buffer, &hashLen) == 0) {
			if (debug_flag)
				print_raw_data(buffer, hashLen);
		} else
			printf("get rootkey hash failed\n");
	}
}
