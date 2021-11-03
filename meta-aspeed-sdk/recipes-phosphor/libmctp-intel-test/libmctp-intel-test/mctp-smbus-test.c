/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/*
 * Copyright 2021 Aspeed Technology Inc.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include "mctp-smbus-test.h"

#define SYSFS_SLAVE_QUEUE "/sys/bus/i2c/devices/%d-00%02x/slave-mqueue"

struct mctp_smbus_pkt_private *smbus_extra_params = NULL;
const char short_options [] = "htrdl:c:";
const struct option
    long_options [] = {
    { "help",   no_argument,        NULL,   'h' },
    { "req",    no_argument,        NULL,   't' },
    { "resp",   no_argument,        NULL,   'r' },
    { "deb",    no_argument,        NULL,   'd' },
    { "len",    required_argument,  NULL,   'l' },
    { "count",  required_argument,  NULL,   'c' },
    { 0, 0, 0, 0 }
};

void usage(FILE *fp, int argc, char **argv)
{
    fprintf(fp,
        "Usage: %s [options] <bus_num> <dst_addr> <src_addr> <dst_eid> <src_eid> <message_type> <cmd payload>\n\n"
        "Sends MCTP data over SMbus\n"
        "Options:\n"
        " -h | --help       print this message\n"
        " -t | --req        requester\n"
        " -r | --resp       responder\n"
        " -d | --deb        debug\n"
        " -l | --len        data length\n"
        " -c | --count      test times\n"
        "Command fields\n"
        " <bus_num>         I2C bus number\n"
        " <dst_addr>        destination slave address\n"
        " <src_addr>        source slave address\n"
        " <dst_eid>         destination EID\n"
        " <src_eid>         source EID\n"
        " <type>            MCTP message type\n"
        "   0x00                - MCTP Control Message\n"
        "   0x85                - ASPEED Control Message\n"
        " example: rx : mctp-smbus-test -r 8 0x24 0x28 8 9\n"
        " example: tx :\n"
        "   MCTP Control Message\n"
        "       GET MESSAGE TYPE SUPPORT : mctp-smbus-test -t 8 0x28 0x24 9 8 0x00 0x80 0x05\n"
        "   MCTP ASPEED Control Message\n"
        "       ECHO : mctp-smbus-test -t 8 0x28 0x24 9 8 0x85 0x80 0x00 0x01 0x02 0x03 0x04 0x05\n"
        "       ECHO LARGE: mctp-smbus-test -t -l 32 8 0x28 0x24 9 8 0x85 0x80 0x01\n"
        "",
        argv[0]);
}

void print_raw_resp(uint8_t *rbuf, int rlen)
{
    int i = 0;

    for (i=0; i<rlen; ++i) {
        printf("%02x ", rbuf[i]);
    }

    printf("\n");
}

int compare_pattern(uint8_t *pattern0, uint8_t *pattern1, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (pattern0[i] != pattern1[i]) {
            return 0;
        }
    }

    return 1;
}

void test_pattern_prepare(uint8_t *pattern, int size)
{
    int i;
    int j;

    for (i = 0; i < size; i++) {
        j = i % 0xff;
        pattern[i] = j;
    }
}

/*
 * This rx_response_handler is the callback function which is registered by mctp_set_rx_all()
 * if we received a response by calling mctp_smbus_read().
 * It should be useful to reply a basic request if we are not the bus owner.
 */
void rx_response_handler(uint8_t eid, void *data, void *msg, size_t len,
                       bool tag_owner, uint8_t tag, void *prv)
{
    // TODO:
    //    We should send device the response according to the EID
    struct test_mctp_ctx *p = (struct test_mctp_ctx *)data;

    mctp_prinfo("%s: Received a response", __func__);

    //notify test_mctp_smbus_recv_data_timeout_raw
    p->len = len;
    memcpy(p->rx_buf, msg, len);
}

/*
 * This rx_request_handler is the callback function which is registered by mctp_set_rx_all()
 * if we received a request for none control message by calling mctp_smbus_read().
 * It should be useful to reply a basic request if we are not the bus owner.
 */
void rx_request_handler(mctp_eid_t src, void *data, void *msg, size_t len,
               bool tag_owner, uint8_t tag, void *msg_binding_private)
{
    struct mctp_smbus_pkt_private *pkt_prv =
        (struct mctp_smbus_pkt_private *)msg_binding_private;
    struct test_mctp_ctx *ctx = (struct test_mctp_ctx *)data;
    struct mctp_ctrl_req *req = (struct mctp_ctrl_req *)msg;
    uint8_t msg_hdr_len = sizeof(struct mctp_ctrl_msg_hdr);
    struct mctp_ctrl_resp resp = {0};
    uint16_t resp_len = 0;
    uint8_t mctp_type;
    uint8_t cmd;
    int rc;

    if (ctx == NULL) {
        mctp_prerr("%s: ctx not found", __func__);
        return;
    }

    if (req == NULL) {
        mctp_prerr("%s: request message not found", __func__);
        return;
    }

    if (pkt_prv == NULL) {
        mctp_prerr("%s: private smbus message not found", __func__);
        return;
    }

    mctp_type = req->hdr.ic_msg_type;
    cmd = req->hdr.command_code;
    mctp_prinfo("Received Message Type: %d", mctp_type);
    mctp_prinfo("Received Command: %d", cmd);


    if(mctp_type != MCTP_MESSAGE_TYPE_ASPEED_CTRL) {
        mctp_prwarn("%s: Not support message type 0x%X\n", __func__, mctp_type);
        return;
    }

    memcpy(&resp.hdr, &req->hdr, msg_hdr_len);
    resp.hdr.rq_dgram_inst &= ~(MCTP_CTRL_HDR_FLAG_REQUEST);
    ctx->len = len;  // notify wait_for_request

    switch (cmd) {
    case MCTP_ASPEED_CTRL_CMD_ECHO_LARGE:
    case MCTP_ASPEED_CTRL_CMD_ECHO:
        resp.completion_code = MCTP_CTRL_CC_SUCCESS;
        memcpy(&resp.data, &req->data, len-msg_hdr_len);
        resp_len = len + 1;
        break;
    default:
        resp.completion_code = MCTP_CTRL_CC_ERROR_UNSUPPORTED_CMD;
        resp_len = msg_hdr_len + 1;
        mctp_prwarn("Not handled: %02x", cmd);
        break;
    }

    rc = mctp_message_tx(ctx->mctp, src, &resp, resp_len, false, tag,
                 (void *)pkt_prv);

    if (rc < 0) {
        mctp_prerr("%s: send response failed", __func__);
    }
}

/*
 * This rx_request_control_handler is the callback function which is registered by mctp_set_rx_all()
 * if we received a request for control message by calling mctp_smbus_read().
 * It should be useful to reply a basic request if we are not the bus owner.
 */
void rx_request_control_handler(mctp_eid_t src, void *data, void *msg, size_t len,
               bool tag_owner, uint8_t tag, void *msg_binding_private)
{
    struct mctp_smbus_pkt_private *pkt_prv =
        (struct mctp_smbus_pkt_private *)msg_binding_private;
    struct test_mctp_ctx *ctx = (struct test_mctp_ctx *)data;
    struct mctp_ctrl_req *req = (struct mctp_ctrl_req *)msg;
    uint16_t resp_len = sizeof(struct mctp_ctrl_msg_hdr);
    struct mctp_ctrl_resp resp = {0};
    uint8_t cmd;
    int rc;

    if (ctx == NULL) {
        mctp_prerr("%s: ctx not found", __func__);
        return;
    }

    if (req == NULL) {
        mctp_prerr("%s: request message not found", __func__);
        return;
    }

    if (pkt_prv == NULL) {
        mctp_prerr("%s: private smbus message not found", __func__);
        return;
    }

    cmd = req->hdr.command_code;
    mctp_prinfo("Received Control Command: %d", cmd);

    memcpy(&resp.hdr, &req->hdr, sizeof(struct mctp_ctrl_msg_hdr));
    resp.hdr.rq_dgram_inst &= ~(MCTP_CTRL_HDR_FLAG_REQUEST);
    ctx->len = len;  // notify wait_for_request

    switch (cmd) {
    case MCTP_CTRL_CMD_GET_MESSAGE_TYPE_SUPPORT:
        resp.completion_code = MCTP_CTRL_CC_SUCCESS;
        resp.data[0] = 0x05;
        resp_len += 2;
        break;
    default:
        resp.completion_code = MCTP_CTRL_CC_ERROR_UNSUPPORTED_CMD;
        resp_len += 1;
        mctp_prwarn("Not handled: %02x", cmd);
        break;
    }

    rc = mctp_message_tx(ctx->mctp, src, &resp, resp_len, false, tag,
                 (void *)pkt_prv);

    if (rc < 0) {
        mctp_prerr("%s: send response failed", __func__);
    }
}

/*
 * Fake responder and wait for request
 */
void wait_for_request(struct test_mctp_ctx *ctx)
{
    struct mctp_binding_smbus *smbus = (struct mctp_binding_smbus *)ctx->prot;
    struct mctp *mctp = ctx->mctp;
    struct pollfd pfd;
    int count = 0;
    int r;

    pfd.fd = smbus->in_fd;
    pfd.events = POLLPRI;

    mctp_set_rx_all(mctp, rx_request_handler, ctx);
    mctp_set_rx_ctrl(mctp, rx_request_control_handler, ctx);

    while (count <= 10000) {
        r = poll(&pfd, 1, 5000);

        if (r < 0)
        {
            mctp_prwarn("Poll returned error status (errno=%d)", errno);
            break;
        }

        if (r == 0 || !(pfd.revents & POLLPRI))
            continue;

        if (mctp_smbus_read(smbus) < 0) {
            mctp_prerr("%s: MCTP RX error", __func__);
            break;
        }

        if(ctx->len > 0) {
            count++;
            ctx->len = 0;
            mctp_prinfo("%s: MCTP RX count = %d", __func__, count);
        }
    }
}

// reads MCTP response off smbus
// return byte count, do not interpret data (e.g. MCTP msg type)
int test_mctp_smbus_recv_data_timeout_raw(struct test_mctp_ctx *ctx, uint8_t dst, int TOsec)
{
    struct mctp_binding_smbus *smbus = (struct mctp_binding_smbus *)ctx->prot;
    struct test_mctp_ctx *p = (struct test_mctp_ctx *)ctx;
    struct mctp *mctp = ctx->mctp;
    int retry = 6*10; // Default 6 secs

    mctp_set_rx_all(mctp, rx_response_handler, ctx);

    if (TOsec > 0) {
        retry = TOsec*10;
    }

    while (retry--) {
        usleep(100*1000);

        if (mctp_smbus_read(smbus) < 0) {
            mctp_prerr("%s: MCTP RX error", __func__);
            return -1;
        }

        // Total size of raw message
        if (p->len > 0) {
            return p->len;
        }

        mctp_prdebug("%s: MCTP retry %d", __func__, retry);
    }

    mctp_prerr("%s: MCTP timeout", __func__);
    return -1;
}

struct test_mctp_ctx* test_mctp_smbus_init(uint8_t bus, uint8_t src_addr, uint8_t dst_addr, uint8_t src_eid,
                                               int pkt_size)
{
    uint8_t src_addr_7bits = src_addr >> 1;
    struct mctp_binding_smbus *smbus;
    struct test_mctp_ctx *mctp_ctx;
    char slave_queue[64] = {0};
    char dev[64] = {0};
    struct mctp *mctp;
    int fd;

    mctp_ctx = (struct test_mctp_ctx *)malloc(sizeof(struct test_mctp_ctx));

    if (mctp_ctx == NULL) {
        mctp_prerr("%s: out of memory(test_mctp_ctx)", __func__);
        return NULL;
    }

    if (pkt_size < MCTP_PAYLOAD_SIZE + MCTP_HEADER_SIZE) {
        pkt_size = MCTP_PAYLOAD_SIZE + MCTP_HEADER_SIZE;
    }

    mctp_smbus_set_pkt_size(pkt_size);

    mctp = mctp_init();
    smbus = mctp_smbus_init();
    if (mctp == NULL || smbus == NULL || mctp_smbus_register_bus(smbus, mctp, src_eid) < 0) {
        mctp_prerr("%s: MCTP init failed", __func__);
        goto bail;
    }

    /* Setting the default slave address */
    mctp_smbus_set_src_slave_addr(smbus, src_addr);
    smbus_extra_params = (struct mctp_smbus_pkt_private *)
                          malloc(sizeof(struct mctp_smbus_pkt_private));
    if (smbus_extra_params == NULL) {
        mctp_prerr("%s: out of memory(mctp_smbus_pkt_private)", __func__);
        goto bail;
    }

    snprintf(dev, sizeof(dev), "/dev/i2c-%d", bus);
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        mctp_prerr("%s: open %s failed", __func__, dev);
        goto bail;
    }

    mctp_smbus_set_out_fd(smbus, fd);
    smbus_extra_params->mux_hold_timeout = 0;
    smbus_extra_params->mux_flags = 0;
    smbus_extra_params->fd = fd;
    smbus_extra_params->slave_addr = dst_addr;

    snprintf(slave_queue, sizeof(slave_queue), SYSFS_SLAVE_QUEUE, bus, src_addr_7bits);
    fd = open(slave_queue, O_RDONLY);
    if (fd < 0) {
        mctp_prerr("%s: open %s failed", __func__, slave_queue);
        goto bail;
    }

    mctp_smbus_set_in_fd(smbus, fd);
    mctp_ctx->mctp = mctp;
    mctp_ctx->prot = (void *)smbus;
    mctp_ctx->len = 0;
    return mctp_ctx;

bail:
    test_mctp_smbus_free(mctp_ctx);
    return NULL;
}

int test_mctp_smbus_send_data(struct test_mctp_ctx *ctx, uint8_t dst, uint8_t flag_tag,
                         void *req, size_t size)
{
    bool tag_owner = flag_tag & MCTP_HDR_FLAG_TO? true: false;
    uint8_t tag = MCTP_HDR_GET_TAG(flag_tag);
    struct mctp *mctp = ctx->mctp;

    // TODO:
    //  Function overloading

    if (mctp_message_tx(mctp, dst, req, size,
                        tag_owner, tag, smbus_extra_params) < 0) {
        mctp_prerr("%s: MCTP TX error", __func__);
        return -1;
    }

    return 0;
}

void test_mctp_smbus_free(struct test_mctp_ctx* ctx)
{
    mctp_smbus_free(ctx->prot);
    mctp_destroy(ctx->mctp);
    free(ctx);
    free(smbus_extra_params);
}

int test_send_mctp_cmd(uint8_t bus, uint8_t src_addr, uint8_t dst_addr, uint8_t src_eid, uint8_t dst_eid,
                       uint8_t *tbuf, int tlen, uint8_t *rbuf, int *rlen)
{
    struct test_mctp_ctx *ctx;
    uint8_t tag = 0;
    int ret = -1;

    ctx = test_mctp_smbus_init(bus, src_addr, dst_addr, src_eid, MAX_PAYLOAD_SIZE);
    if (ctx == NULL) {
        mctp_prerr("%s: Error: mctp binding failed", __func__);
        return -1;
    }

    tag |= MCTP_HDR_FLAG_TO;
    ret = test_mctp_smbus_send_data(ctx, dst_eid, tag, tbuf, tlen);
    if (ret < 0) {
        mctp_prerr("error: %s send failed\n", __func__);
        goto bail;
    }

    ctx->rx_buf = (uint8_t *)rbuf;

    ret = test_mctp_smbus_recv_data_timeout_raw(ctx, dst_eid, -1);
    if (ret < 0) {
        mctp_prerr("%s: error getting response\n", __func__);
        goto bail;
    } else {
        *rlen = ret;
        ret = 0;
    }

bail:
    test_mctp_smbus_free(ctx);
    return ret;
}

int test_mctp_fake_responder(uint8_t bus, uint8_t src_addr, uint8_t dst_addr, uint8_t src_eid, uint8_t dst_eid)
{
    struct test_mctp_ctx *ctx;

    ctx = test_mctp_smbus_init(bus, src_addr, dst_addr, src_eid, MAX_PAYLOAD_SIZE);
    if (ctx == NULL) {
        mctp_prerr("%s: Error: mctp binding failed", __func__);
        return -1;
    }

    wait_for_request(ctx);
    test_mctp_smbus_free(ctx);
    return 0;
}

int main(int argc, char *argv[])
{
    uint8_t msg_hdr_len = sizeof(struct mctp_ctrl_msg_hdr);
    uint8_t cmd = MCTP_CTRL_CMD_GET_MESSAGE_TYPE_SUPPORT;
    uint8_t tbuf[MAX_PAYLOAD_SIZE] = {0};
    uint8_t rbuf[MAX_PAYLOAD_SIZE] = {0};
    uint8_t src_eid = REQUESTER_EID;
    uint8_t dst_eid = RESPONDER_EID;
    uint8_t rq_dgram_inst = 0x80;
    uint8_t responder_flag = 0;
    uint8_t requester_flag = 0;
    uint8_t src_addr = 0x24;
    uint8_t dst_addr = 0x28;
    uint8_t test_status = 0;
    uint8_t debug_flag = 0;
    uint8_t mctp_type = 0;
    int loop_count = 1;
    int data_len = 0;
    uint8_t bus = 8;
    char option = 0;
    int minargc = 6;
    int tlen = 0;
    int rlen = 0;
    int ret = -1;
    int i = 0;

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
        case 't':
            requester_flag = 1;
            // request data should include mctp_type, rq_dgram_inst and cmd
            // 1 + msg_hdr_len
            minargc += 4;
            break;
        case 'r':
            responder_flag = 1;
            minargc += 1;
            break;
        case 'l':
            data_len = strtoul(optarg, NULL, 0);
            minargc += 2;
            break;
        case 'd':
            debug_flag = 1;
            minargc += 1;
            break;
        case 'c':
            loop_count = strtoul(optarg, NULL, 0);
            minargc += 2;
            break;
        default:
            usage(stdout, argc, argv);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (debug_flag) {
        mctp_set_log_stdio(MCTP_LOG_DEBUG);
        mctp_set_tracing_enabled(true);
    } else {
        mctp_set_log_stdio(MCTP_LOG_INFO);
    }

    if (argc < minargc) { // min params: mctp-smbus-test -r <bus> <dst_addr> <src_addr> <dst_eid> <src_eid>
        mctp_prerr("Error argc(%d) < minargc(%d)", argc, minargc);
        usage(stdout, argc, argv);
        exit(EXIT_FAILURE);
    }

    if (!requester_flag && !responder_flag) {
        mctp_prerr("Error -t or -r option should be added");
        usage(stdout, argc, argv);
        exit(EXIT_FAILURE);
    }

    if (requester_flag && responder_flag) {
        mctp_prerr("Error -t and -r options should not be added at the same time");
        usage(stdout, argc, argv);
        exit(EXIT_FAILURE);
    }

    bus = (uint8_t)strtoul(argv[optind++], NULL, 0);
    dst_addr = (uint8_t)strtoul(argv[optind++], NULL, 0);
    src_addr = (uint8_t)strtoul(argv[optind++], NULL, 0);
    dst_eid = (uint8_t)strtoul(argv[optind++], NULL, 0);
    src_eid = (uint8_t)strtoul(argv[optind++], NULL, 0);

    // requester
    if (requester_flag) {
        // min params: mctp-smbus-test -t <bus> <dst_addr> <src_addr> <dst_eid> <src_eid>
        // request data should include mctp_type, rq_dgram_inst and cmd
        if ((argc - minargc) > (SMBUS_TEST_TX_BUFF_SIZE - msg_hdr_len)) {
            mctp_prerr("data payload=%d exceeds max payload length %d\n", (argc - minargc + msg_hdr_len), SMBUS_TEST_TX_BUFF_SIZE);
            usage(stdout, argc, argv);
            exit(EXIT_FAILURE);
        }

        if (data_len > (SMBUS_TEST_TX_BUFF_SIZE - msg_hdr_len)) {
            mctp_prerr("length exceeds max payload length %d\n", SMBUS_TEST_TX_BUFF_SIZE - msg_hdr_len);
            usage(stdout, argc, argv);
            exit(EXIT_FAILURE);
        }

        // refer to struct mctp_ctrl_req and struct mctp_ctrl_msg_hdr
        mctp_type = (uint8_t)strtoul(argv[optind++], NULL, 0);
        rq_dgram_inst = (uint8_t)strtoul(argv[optind++], NULL, 0);
        cmd = (uint8_t)strtoul(argv[optind++], NULL, 0);
        tbuf[tlen++] = mctp_type;
        tbuf[tlen++] = rq_dgram_inst | MCTP_CTRL_HDR_FLAG_REQUEST;;
        tbuf[tlen++] = cmd;


        if (data_len > 0) {
            test_pattern_prepare(&tbuf[msg_hdr_len], data_len);
            tlen += data_len;
        } else {
            for (i=optind; i <argc; i++) {
                tbuf[tlen++] = (uint8_t)strtoul(argv[i], NULL, 0);
            }
        }

        if(debug_flag) {
            printf("argc=%d bus=%d dst_addr=0x%X src_addr=0x%X dst_eid=%d src_eid=%d mctp_type=%d rq_dgram_inst=0x%X cmd=%d\n",
                    argc, bus, dst_addr, src_addr, dst_eid, src_eid, mctp_type, rq_dgram_inst, cmd);
            printf("data: ");

            //0: mctp_type, 1: rq_dgram_inst, 2: cmd
            for (i=msg_hdr_len; i<tlen; ++i) {
                printf("0x%x ", tbuf[i]);
            }

            printf("\n");
        }

        if(mctp_type != MCTP_MESSAGE_TYPE_MCTP_CTRL && mctp_type != MCTP_MESSAGE_TYPE_ASPEED_CTRL) {
            mctp_prerr("Error not support message type 0x%X\n", mctp_type);
            return -1;
        }

        for(i=0; i<loop_count; i++) {
            ret = test_send_mctp_cmd(bus, src_addr, dst_addr, src_eid, dst_eid,
                          tbuf, tlen, rbuf, &rlen);
            if (ret < 0) {
                mctp_prerr("Error sending MCTP cmd, ret = %d\n count = %d\n", ret, i);
                test_status = -1;
            }
            else {
                print_raw_resp(rbuf, rlen);
            }
        }

        return test_status;
    }

    // responder
    if (responder_flag) {
        if(debug_flag) {
            printf("argc=%d bus=%d dst_addr=0x%X src_addr=0x%X dst_eid=%d src_eid=%d\n",
                    argc, bus, dst_addr, src_addr, dst_eid, src_eid);
        }

        ret = test_mctp_fake_responder(bus, src_addr, dst_addr, src_eid, dst_eid);
        if (ret < 0) {
            mctp_prerr("Error run fake responder failed");
            return -1;
        }
    }

    return 0;
}
