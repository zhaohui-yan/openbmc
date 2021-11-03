/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/*
 * Copyright 2021 Aspeed Technology Inc.
 */
#ifndef _MCTP_SMBUS_TEST_H_
#define _MCTP_SMBUS_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libmctp.h>
#include <libmctp-log.h>
#include <libmctp-smbus.h>
#include <libmctp-cmds.h>
#include <libmctp-msgtypes.h>

#define MCTP_MESSAGE_TYPE_ASPEED_CTRL 0x85
#define MCTP_ASPEED_CTRL_CMD_ECHO 0x00
#define MCTP_ASPEED_CTRL_CMD_ECHO_LARGE 0x01
#define MAX_PAYLOAD_SIZE 255  // including Message Header and body
#define SMBUS_TEST_TX_BUFF_SIZE \
    ((MAX_PAYLOAD_SIZE) - (MCTP_HEADER_SIZE) - (SMBUS_HEADER_SIZE) - (SMBUS_PEC_BYTE_SIZE))
#define SMBUS_TEST_RX_BUFF_SIZE \
        ((MAX_PAYLOAD_SIZE) - (MCTP_HEADER_SIZE) - (SMBUS_HEADER_SIZE) - (SMBUS_PEC_BYTE_SIZE) - 1)

#define REQUESTER_EID 8
#define RESPONDER_EID 9

struct mctp_ctrl_req {
    struct mctp_ctrl_msg_hdr hdr;
    uint8_t data[SMBUS_TEST_TX_BUFF_SIZE];
};

struct mctp_ctrl_resp {
    struct mctp_ctrl_msg_hdr hdr;
    uint8_t completion_code;
    uint8_t data[SMBUS_TEST_RX_BUFF_SIZE];
};

/* Test MCTP ctx */
struct test_mctp_ctx {
    struct mctp *mctp;
    uint16_t len;
    void *rx_buf;
    void *prot;
};

void usage(FILE *fp, int argc, char **argv);
void print_raw_resp(uint8_t *rbuf, int rlen);
int compare_pattern(uint8_t *pattern0, uint8_t *pattern1, int size);
void test_pattern_prepare(uint8_t *pattern, int size);
void rx_response_handler(uint8_t eid, void *data, void *msg, size_t len,
                       bool tag_owner, uint8_t tag, void *prv);
void rx_request_handler(mctp_eid_t src, void *data, void *msg, size_t len,
               bool tag_owner, uint8_t tag, void *msg_binding_private);
void rx_request_control_handler(mctp_eid_t src, void *data, void *msg, size_t len,
               bool tag_owner, uint8_t tag, void *msg_binding_private);
void wait_for_request(struct test_mctp_ctx* ctx);
int test_mctp_smbus_recv_data_timeout_raw(struct test_mctp_ctx *ctx, uint8_t dst, int TOsec);
struct test_mctp_ctx* test_mctp_smbus_init(uint8_t bus, uint8_t src_addr, uint8_t dst_addr, uint8_t src_eid,
                                               int pkt_size);
int test_mctp_smbus_send_data(struct test_mctp_ctx* ctx, uint8_t dst, uint8_t flag_tag,
                         void *req, size_t size);
void test_mctp_smbus_free(struct test_mctp_ctx* ctx);
int test_send_mctp_cmd(uint8_t bus, uint8_t src_addr, uint8_t dst_addr, uint8_t src_eid, uint8_t dst_eid,
                       uint8_t *tbuf, int tlen, uint8_t *rbuf, int *rlen);
int test_mctp_fake_responder(uint8_t bus, uint8_t src_addr, uint8_t dst_addr, uint8_t src_eid, uint8_t dst_eid);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MCTP_SMBUS_TEST_H_ */
