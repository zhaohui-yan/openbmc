#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <unistd.h>
#include "i2c_utils.h"
#include "mailbox_enums.h"
#include "arguments.h"

void checkpointStart(ARGUMENTS args)
{
	i2cWriteByteData(args, MB_BMC_CHECKPOINT, MB_CHKPT_START);
	usleep(60*1000);
}

void checkpointPause(ARGUMENTS args)
{
	i2cWriteByteData(args, MB_BMC_CHECKPOINT, MB_CHKPT_PAUSE);
	usleep(60*1000);
}

void checkpointResume(ARGUMENTS args)
{
	i2cWriteByteData(args, MB_BMC_CHECKPOINT, MB_CHKPT_RESUME);
	usleep(60*1000);
}

void checkpointComplete(ARGUMENTS args)
{
	i2cWriteByteData(args, MB_PROVISION_CMD, MB_CHKPT_COMPLETE);
	usleep(60*1000);
}

void Checkpoint(ARGUMENTS args)
{
	if (strncmp(args.checkpoint_cmd, "start", strlen(args.checkpoint_cmd)) == 0)
		checkpointStart(args);
	else if (strncmp(args.checkpoint_cmd, "pause", strlen(args.checkpoint_cmd)) == 0)
		checkpointPause(args);
	else if (strncmp(args.checkpoint_cmd, "resume", strlen(args.checkpoint_cmd)) == 0)
		checkpointResume(args);
	else if (strncmp(args.checkpoint_cmd, "complete", strlen(args.checkpoint_cmd)) == 0)
		checkpointComplete(args);
	else
		printf("unsupported checkpoint command, %s\n", args.checkpoint_cmd);
}

