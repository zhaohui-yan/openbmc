#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <unistd.h>
#include "provision.h"
#include "i2c_utils.h"
#include "mailbox_enums.h"
#include "arguments.h"

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
	uint8_t pubkey_x[SHA384_LENGTH];
	uint8_t pubkey_y[SHA384_LENGTH];
	uint8_t buffer[SHA384_LENGTH*2];
	int size = 0;
	int i;

	if (extractQxQyFromPubkey(file, pubkey_x, pubkey_y, &size)) {
		printf("%s, Failed to extract QX and QY.\n", __func__);
		return 1;
	}

	if ((size == SHA256_LENGTH) || (size == SHA384_LENGTH)) {
		// Collect root public key X and Y, in little endian byte order.
		for (i = 0; i < size; i++) {
			buffer[i] = pubkey_x[size - 1 - i];
			buffer[i + size] = pubkey_y[size - 1 - i];
		}

		if (size == SHA256_LENGTH)
			hashBuffer(buffer, size*2, Sha256, hash, len);
		else
			hashBuffer(buffer, size*2, Sha384, hash, len);
	} else {
		printf("%s, Unsuported key size.\n", __func__);
		return 1;
	}

	return 0;
}

void writeUfmProvFifoCmd(ARGUMENTS args, MB_UFM_PROV_CMD_ENUM cmd, uint8_t *buf, int len)
{
	int i;

	// Flush Write FIFO
	i2cWriteByteData(args, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_FLUSH_WR_FIFO_MASK);
	usleep(60*1000);

	// Write FIFO
	for (i = 0; i < len; i++) {
		i2cWriteByteData(args, MB_UFM_WRITE_FIFO, buf[i]);
		usleep(60*1000);
	}

	// Trigger command
	i2cWriteByteData(args, MB_PROVISION_CMD, cmd);
	usleep(60*1000);
	i2cWriteByteData(args, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);
}

void readUfmProvFifoCmd(ARGUMENTS args, MB_UFM_PROV_CMD_ENUM cmd, uint8_t *buf, int len)
{
	int i;

	// Flush Read FIFO
	i2cWriteByteData(args, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_FLUSH_RD_FIFO_MASK);
	usleep(60*1000);

	// Trigger command
	i2cWriteByteData(args, MB_PROVISION_CMD, cmd);
	usleep(60*1000);
	i2cWriteByteData(args, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);

	// Read FIFO
	for (i = 0; i < len; i++) {
		buf[i] = i2cReadByteData(args, MB_UFM_READ_FIFO);
		usleep(20*1000);
	}
}

void writeUfmProvBmcPchRegionOffset(ARGUMENTS args)
{
	uint8_t bmc_offset[12];
	uint8_t pch_offset[12];

	memcpy(bmc_offset, &args.bmc_active_pfm_offset, 4);
	memcpy(bmc_offset + 4, &args.bmc_recovery_offset, 4);
	memcpy(bmc_offset + 8, &args.bmc_staging_offset, 4);
	memcpy(pch_offset, &args.pch_active_pfm_offset, 4);
	memcpy(pch_offset + 4, &args.pch_recovery_offset, 4);
	memcpy(pch_offset + 8, &args.pch_staging_offset, 4);

	if (args.debug_flag) {
		printf("BMC Offset\n");
		printRawData(bmc_offset, sizeof(bmc_offset));
		printf("PCH Offset\n");
		printRawData(pch_offset, sizeof(pch_offset));
	}

	// Write BMC offset
	writeUfmProvFifoCmd(args, MB_UFM_PROV_BMC_OFFSETS, bmc_offset, sizeof(bmc_offset));
	// Write PCH offset
	writeUfmProvFifoCmd(args, MB_UFM_PROV_PCH_OFFSETS, pch_offset, sizeof(pch_offset));
}

void provisionLock(ARGUMENTS args)
{
	i2cWriteByteData(args, MB_PROVISION_CMD, MB_UFM_PROV_END);
	usleep(60*1000);
	i2cWriteByteData(args, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);
}

void provision(ARGUMENTS args)
{
	uint8_t write_buffer[64];
	uint8_t read_buf[64];
	int hashLen = 0;

	if (strncmp(args.provision_cmd, "show", strlen("show")) == 0) {
		// Read BMC offset
		readUfmProvFifoCmd(args, MB_UFM_PROV_RD_BMC_OFFSETS, read_buf, 12);
		printf("BMC Active PFM Offset : 0x%08x\n", *(uint32_t *)&read_buf[0]);
		printf("BMC Recovery Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[4]);
		printf("BMC Staging Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[8]);
		// Read PCH Offset
		readUfmProvFifoCmd(args, MB_UFM_PROV_RD_PCH_OFFSETS, read_buf, 12);
		printf("PCH Active PFM Offset : 0x%08x\n", *(uint32_t *)&read_buf[0]);
		printf("PCH Recovery Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[4]);
		printf("PCH Staging Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[8]);
		// Read Root Key hash
		readUfmProvFifoCmd(args, MB_UFM_PROV_RD_ROOT_KEY, read_buf, SHA384_LENGTH);
		printf("Root Key Hash:\n");
		printRawData(read_buf, SHA384_LENGTH);
	} else if (strncmp(args.provision_cmd, "lock", strlen("lock")) == 0) {
		provisionLock(args);
	} else {
		if (getRootKeyHash(args.provision_cmd, write_buffer, &hashLen) == 0) {
			if (args.debug_flag)
				printRawData(write_buffer, hashLen);
			// Write BMC, PCH region offset
			writeUfmProvBmcPchRegionOffset(args);
			// Write Root Key hash
			writeUfmProvFifoCmd(args, MB_UFM_PROV_ROOT_KEY, write_buffer, hashLen);
		} else
			printf("get rootkey hash failed\n");
	}
}

void unprovision(ARGUMENTS args)
{
	i2cWriteByteData(args, MB_PROVISION_CMD, MB_UFM_PROV_ERASE);
	usleep(60*1000);
	i2cWriteByteData(args, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);
}

