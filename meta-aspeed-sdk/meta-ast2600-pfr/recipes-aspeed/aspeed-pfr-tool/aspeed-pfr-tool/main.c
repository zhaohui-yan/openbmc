#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "provision.h"
#include "i2c_utils.h"

uint8_t debug_flag;
static const char short_options[] = "hb:a:p:w:r:dum";

static const struct option
	long_options[] = {
	{ "help",		no_argument,		NULL,   'h' },
	{ "bus",		required_argument,	NULL,   'b' },
	{ "address",		required_argument,	NULL,   'a' },
	{ "provision",		required_argument,	NULL,   'p' },
	{ "unprovision",	no_argument,		NULL,   'u' },
	{ "write_reg",		required_argument,	NULL,   'w' },
	{ "read_reg",		required_argument,	NULL,	'r' },
	{ "aspeed_dcscm",	no_argument,		NULL,	'm' },
	{ "debug",		no_argument,		NULL,   'd' },
	{ 0, 0, 0, 0 }
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
		"Usage: %s [options]\n\n"
		"Options:\n"
		" -h | --help           Print this message\n"
		" -b | --bus            bus number [default : 14]\n"
		" -a | --address        slave address [default : 0x38]\n"
		" -p | --provision      provision\n"
		" -u | --unprovision    unprovision\n"
		" -w | --write_reg      write register\n"
		" -r | --read_reg       read register\n"
		" -m | --aspeed_dcscm   aspeed dcscm flash offset [default: aspeed_pfr]\n"
		" -d | --debug          debug mode\n"
		"example:\n"
		"--provision /usr/share/pfrconfig/rk_pub.pem\n"
		"--provision show\n"
		"--write_reg <rf_addr> <data> (byte mode)\n"
		"--write_reg <rf_addr> <data1> <data2>... (block mode)\n"
		"--read_reg <rf_addr> (byte mode)\n"
		"--read_reg <rf_addr> <length> (block mode)\n"
		"",
		argv[0]);
}

int main(int argc, char *argv[])
{
	char *provision_cmd = NULL;
	uint8_t slave_addr = 0x38;
	uint8_t tx_msg[64] = {0};
	uint8_t rx_msg[64] = {0};
	int unprovision_flag = 0;
	int provision_flag = 0;
	int write_reg_flag = 0;
	int read_reg_flag = 0;
	int is_dcscm_flag = 0;
	int tx_msg_len = 0;
	int rx_msg_len = 0;
	uint8_t bus = 14;
	char option = 0;
	int fd;
	int i;
	int j;

	debug_flag = 0;

	if (!argv[1]) {
		usage(stdout, argc, argv);
		exit(EXIT_FAILURE);
	}

	while ((option = getopt_long(argc, argv, short_options, long_options, NULL)) != (char) -1) {
		switch (option) {
		case 'h':
			usage(stdout, argc, argv);
			exit(EXIT_SUCCESS);
			break;
		case 'b':
			bus = strtoul(optarg, 0, 0);
			break;
		case 'a':
			slave_addr = strtoul(optarg, 0, 0);
			break;
		case 'p':
			provision_cmd = optarg;
			provision_flag = 1;
			break;
		case 'u':
			unprovision_flag = 1;
			break;
		case 'm':
			is_dcscm_flag = 1;
			break;
		case 'd':
			debug_flag = 1;
			break;
		case 'r':
			tx_msg[0] = strtoul(optarg, 0, 0);
			tx_msg_len += 1;
			read_reg_flag = 1;
			break;
		case 'w':
			tx_msg[0] = strtoul(optarg, 0, 0);
			tx_msg_len += 1;
			write_reg_flag = 1;
			break;
		default:
			usage(stdout, argc, argv);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (optind < argc) {
		tx_msg_len += argc - optind;
		for (j = 1, i = optind; i < argc; i++, j++)
			tx_msg[j] = strtoul(argv[i], 0, 0);
		if (debug_flag)
			print_raw_data(tx_msg, tx_msg_len);
	}

	if (provision_flag && unprovision_flag) {
		printf("provison and unprovision commands can not be set at the same time\n");
		exit(EXIT_FAILURE);
	}

	fd = open_i2c_dev(bus, slave_addr);
	if (read_reg_flag) {
		if (tx_msg_len > 2)
			printf("invalid read register command\n");
		else {
			if (tx_msg_len == 1) {
				rx_msg[0] = i2cReadByteData(fd, tx_msg[0]);
				if (rx_msg[0])
					print_raw_data(rx_msg, 1);
			} else {
				rx_msg_len = i2cReadBlockData(fd, tx_msg[0], tx_msg[1], rx_msg);
				if (rx_msg_len > 0)
					print_raw_data(rx_msg, rx_msg_len);
			}
		}
	}

	if (write_reg_flag) {
		if (tx_msg_len < 2)
			printf("invalid write register command\n");
		else
			if (tx_msg_len > 2)
				i2cWriteBlockData(fd, tx_msg[0], tx_msg_len - 1, &tx_msg[1]);
			else
				i2cWriteByteData(fd, tx_msg[0], tx_msg[1]);
	}

	if (provision_flag)
		provision(fd, provision_cmd, is_dcscm_flag);

	if (unprovision_flag)
		unprovision(fd);

	close(fd);
	return 0;
}
