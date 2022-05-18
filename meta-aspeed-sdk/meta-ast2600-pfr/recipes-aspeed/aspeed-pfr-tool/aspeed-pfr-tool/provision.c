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

void write_ufm_prov_fifo_cmd(int fd, MB_UFM_PROV_CMD_ENUM cmd, uint8_t *buf, int len)
{
	int i;

	// Flush Write FIFO
	i2cWriteByteData(fd, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_FLUSH_WR_FIFO_MASK);
	usleep(60*1000);

	// Write FIFO
	for (i = 0; i < len; i++) {
		i2cWriteByteData(fd, MB_UFM_WRITE_FIFO, buf[i]);
		usleep(60*1000);
	}

	// Trigger command
	i2cWriteByteData(fd, MB_PROVISION_CMD, cmd);
	usleep(60*1000);
	i2cWriteByteData(fd, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);
}

void read_ufm_prov_fifo_cmd(int fd, MB_UFM_PROV_CMD_ENUM cmd, uint8_t *buf, int len)
{
	int i;

	// Flush Read FIFO
	i2cWriteByteData(fd, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_FLUSH_RD_FIFO_MASK);
	usleep(60*1000);

	// Trigger command
	i2cWriteByteData(fd, MB_PROVISION_CMD, cmd);
	usleep(60*1000);
	i2cWriteByteData(fd, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);

	// Read FIFO
	for (i = 0; i < len; i++) {
		buf[i] = i2cReadByteData(fd, MB_UFM_READ_FIFO);
		usleep(20*1000);
	}
}

void write_ufm_prov_bmcpch_default_offset(int fd, int is_dcscm_flag)
{
	uint8_t bmc_offset[12];
	uint8_t pch_offset[12];
	uint32_t offset = 0;

	if (is_dcscm_flag) {
		offset = DCSCM_BMC_ACTIVE_PFM_OFFSET;
		memcpy(bmc_offset, &offset, 4);
		offset = DCSCM_BMC_RECOVERY_REGION_OFFSET;
		memcpy(bmc_offset + 4, &offset, 4);
		offset = DCSCM_BMC_STAGING_REGION_OFFSET;
		memcpy(bmc_offset + 8, &offset, 4);
		offset = DCSCM_PCH_ACTIVE_PFM_OFFSET;
		memcpy(pch_offset, &offset, 4);
		offset = DCSCM_PCH_RECOVERY_REGION_OFFSET;
		memcpy(pch_offset + 4, &offset, 4);
		offset = DCSCM_PCH_STAGING_REGION_OFFSET;
		memcpy(pch_offset + 8, &offset, 4);
	} else {
		offset = PFR_BMC_ACTIVE_PFM_OFFSET;
		memcpy(bmc_offset, &offset, 4);
		offset = PFR_BMC_RECOVERY_REGION_OFFSET;
		memcpy(bmc_offset + 4, &offset, 4);
		offset = PFR_BMC_STAGING_REGION_OFFSET;
		memcpy(bmc_offset + 8, &offset, 4);
		offset = PFR_PCH_ACTIVE_PFM_OFFSET;
		memcpy(pch_offset, &offset, 4);
		offset = PFR_PCH_RECOVERY_REGION_OFFSET;
		memcpy(pch_offset + 4, &offset, 4);
		offset = PFR_PCH_STAGING_REGION_OFFSET;
		memcpy(pch_offset + 8, &offset, 4);
	}

	if (debug_flag) {
		printf("BMC Offset\n");
		print_raw_data(bmc_offset, sizeof(bmc_offset));
		printf("PCH Offset\n");
		print_raw_data(pch_offset, sizeof(pch_offset));
	}

	// Write BMC offset
	write_ufm_prov_fifo_cmd(fd, MB_UFM_PROV_BMC_OFFSETS, bmc_offset, sizeof(bmc_offset));
	// Write PCH offset
	write_ufm_prov_fifo_cmd(fd, MB_UFM_PROV_PCH_OFFSETS, pch_offset, sizeof(pch_offset));
}

void provision(int fd, char *cmd, int is_dcscm_flag)
{
	uint8_t write_buffer[64];
	uint8_t read_buf[64];
	int hashLen = 0;

	if (strncmp(cmd, "show", strlen(cmd)) == 0) {
		// Read BMC offset
		read_ufm_prov_fifo_cmd(fd, MB_UFM_PROV_RD_BMC_OFFSETS, read_buf, 12);
		printf("BMC Active PFM Offset : 0x%08x\n", *(uint32_t *)&read_buf[0]);
		printf("BMC Recovery Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[4]);
		printf("BMC Staging Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[8]);
		// Read PCH Offset
		read_ufm_prov_fifo_cmd(fd, MB_UFM_PROV_RD_PCH_OFFSETS, read_buf, 12);
		printf("PCH Active PFM Offset : 0x%08x\n", *(uint32_t *)&read_buf[0]);
		printf("PCH Recovery Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[4]);
		printf("PCH Staging Region Offset : 0x%08x\n", *(uint32_t *)&read_buf[8]);
		// Read Root Key hash
		read_ufm_prov_fifo_cmd(fd, MB_UFM_PROV_RD_ROOT_KEY, read_buf, SHA384_LENGTH);
		printf("Root Key Hash:\n");
		print_raw_data(read_buf, SHA384_LENGTH);

	} else {
		if (getRootKeyHash(cmd, write_buffer, &hashLen) == 0) {
			if (debug_flag)
				print_raw_data(write_buffer, hashLen);
			// Write Root Key hash
			write_ufm_prov_fifo_cmd(fd, MB_UFM_PROV_ROOT_KEY, write_buffer, hashLen);
			write_ufm_prov_bmcpch_default_offset(fd, is_dcscm_flag);
		} else
			printf("get rootkey hash failed\n");
	}
}

void unprovision(int fd)
{
	i2cWriteByteData(fd, MB_PROVISION_CMD, MB_UFM_PROV_ERASE);
	usleep(60*1000);
	i2cWriteByteData(fd, MB_UFM_CMD_TRIGGER, MB_UFM_CMD_EXECUTE_MASK);
	usleep(60*1000);
}

