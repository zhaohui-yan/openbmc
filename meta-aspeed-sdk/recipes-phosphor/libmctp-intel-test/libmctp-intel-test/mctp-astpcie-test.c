/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */
/*
 * Copyright 2021 Aspeed Technology Inc.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include "mctp-astpcie-test.h"

struct mctp_astpcie_pkt_private *astpcie_extra_params = NULL;
const char short_options [] = "htrdl:c:g";
const struct option
    long_options [] = {
    { "help",   no_argument,        NULL,   'h' },
    { "req",    no_argument,        NULL,   't' },
    { "resp",   no_argument,        NULL,   'r' },
    { "deb",    no_argument,        NULL,   'd' },
    { "len",    required_argument,  NULL,   'l' },
    { "count",  required_argument,  NULL,   'c' },
    { "gbdf",   required_argument,  NULL,   'g' },
    { 0, 0, 0, 0 }
};

void usage(FILE *fp, int argc, char **argv)
{
    fprintf(fp,
        "Usage: %s [options] <bus_num> <routing_type> <dst_dev> <des_func> <dst_eid> <src_eid> <message_type> <cmd payload>\n\n"
        "Sends MCTP data over PCIE\n"
        "Options:\n"
        " -h | --help       print this message\n"
        " -t | --req        requester\n"
        " -r | --resp       responder\n"
        " -d | --deb        debug\n"
        " -l | --len        data length\n"
        " -c | --count      test times\n"
        " -g | --gbdf       get BDF information\n"
        "Command fields\n"
        " <bus_num>         destination PCIE bus number\n"
        " <routing_type>    PCIE routing type 0: route to RC, 2: route by ID, 3: Broadcast from RC\n"
        " <dst_dev>         destination PCIE device number\n"
        " <dst_func>        destination PCIE function number\n"
        " <dst_eid>         destination EID\n"
        " <src_eid>         source EID\n"
        " <type>            MCTP message type\n"
        "   0x00                - MCTP Control Message\n"
        "   0x85                - ASPEED Control Message\n"
        " example: get BDF: mctp-astpcie-test -g\n"
        " example: rx : mctp-astpcie-test -r 2 2 0 0 8 9\n"
        " example: tx :\n"
        "   MCTP Control Message\n"
        "       GET MESSAGE TYPE SUPPORT : mctp-astpcie-test -t 10 2 0 0 9 8 0x00 0x80 0x05\n"
        "   MCTP ASPEED Control Message\n"
        "       ECHO : mctp-astpcie-test -t 10 2 0 0 9 8 0x85 0x80 0x00 0x01 0x02 0x03 0x04 0x05\n"
        "       ECHO LARGE: mctp-astpcie-test -t -l 32 10 2 0 0 9 8 0x85 0x80 0x01\n"
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
 * if we received a response by calling mctp_astpcie_rx().
 * It should be useful to reply a basic request if we are not the bus owner.
 */
void rx_response_handler(uint8_t eid, void *data, void *msg, size_t len,
                       bool tag_owner, uint8_t tag, void *prv)
{
    // TODO:
    //    We should send device the response according to the EID
    struct test_mctp_ctx *p = (struct test_mctp_ctx *)data;

    mctp_prinfo("%s: Received a response", __func__);

    //notify test_mctp_astpcie_recv_data_timeout_raw
    p->len = len;
    memcpy(p->rx_buf, msg, len);
}

/*
 * This rx_request_handler is the callback function which is registered by mctp_set_rx_all()
 * if we received a request for none control message by calling mctp_astpcie_rx().
 * It should be useful to reply a basic request if we are not the bus owner.
 */
void rx_request_handler(mctp_eid_t src, void *data, void *msg, size_t len,
               bool tag_owner, uint8_t tag, void *msg_binding_private)
{
    struct mctp_astpcie_pkt_private *pkt_prv =
        (struct mctp_astpcie_pkt_private *)msg_binding_private;
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
        mctp_prerr("%s: private astpcie message not found", __func__);
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

    mctp_binding_set_tx_enabled(ctx->astpcie_binding, true);
    rc = mctp_message_tx(ctx->mctp, src, &resp, resp_len, false, tag,
                 (void *)pkt_prv);

    if (rc < 0) {
        mctp_prerr("%s: send response failed", __func__);
    }
}

/*
 * This rx_request_control_handler is the callback function which is registered by mctp_set_rx_all()
 * if we received a request for control message by calling mctp_astpcie_rx().
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
        mctp_prerr("%s: private astpcie message not found", __func__);
        return;
    }

    cmd = req->hdr.command_code;
    mctp_prinfo("Received Control Command: %d", cmd);

    memcpy(&resp.hdr, &req->hdr, sizeof(struct mctp_ctrl_msg_hdr));
    resp.hdr.rq_dgram_inst &= ~(MCTP_CTRL_HDR_FLAG_REQUEST);

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

    mctp_binding_set_tx_enabled(ctx->astpcie_binding, true);
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
    struct mctp_binding_astpcie *astpcie = (struct mctp_binding_astpcie *)ctx->prot;
    struct mctp *mctp = ctx->mctp;
    int count = 0;
    int r;

    mctp_set_rx_all(mctp, rx_request_handler, ctx);
    mctp_set_rx_ctrl(mctp, rx_request_control_handler, ctx);

    while (count <= 10000) {
        r = mctp_astpcie_poll(astpcie, 1000);

        if (r & POLLIN) {
            r = mctp_astpcie_rx(astpcie);

            if (r < 0)
            {
                mctp_prerr("%s: MCTP RX read error", __func__);
                break;
            }

            count++;
            mctp_prinfo("%s: MCTP RX count = %d", __func__, count);
        }
    }
}

// reads MCTP response off astpcie
// return byte count, do not interpret data (e.g. MCTP msg type)
int test_mctp_astpcie_recv_data_timeout_raw(struct test_mctp_ctx *ctx, uint8_t dst, int TOsec)
{
    struct mctp_binding_astpcie *astpcie = (struct mctp_binding_astpcie *)ctx->prot;
    struct test_mctp_ctx *p = (struct test_mctp_ctx *)ctx;
    struct mctp *mctp = ctx->mctp;
    int retry = 6*10; // Default 6 secs
    int r;

    mctp_set_rx_all(mctp, rx_response_handler, ctx);

    if (TOsec > 0){
        retry = TOsec*10;
    }

    while (retry--) {
        usleep(100*1000);

        r = mctp_astpcie_poll(astpcie, 1000);

        if (r & POLLIN) {
            r = mctp_astpcie_rx(astpcie);

            if (r < 0)
            {
                mctp_prerr("%s: MCTP RX read error", __func__);
                break;
            }

            mctp_prdebug("%s: MCTP Rx received response", __func__);
            // Total size of raw message
            return p->len;
        }

        mctp_prdebug("%s: MCTP retry %d", __func__, retry);
    }

    mctp_prerr("%s: MCTP timeout", __func__);
    return -1;
}

int test_mctp_astpcie_get_bdf(uint8_t *src_bus, uint8_t *src_dev, uint8_t *src_func)
{
    struct mctp_binding_astpcie *astpcie;
    struct mctp_binding *astpcie_binding;
    struct mctp *mctp;
    uint16_t func;
    uint16_t bus;
    uint16_t dev;
    uint16_t bdf;

    mctp = mctp_init();
    astpcie = mctp_astpcie_init();
    astpcie_binding = mctp_astpcie_core(astpcie);
    if (mctp == NULL || astpcie == NULL || astpcie_binding == NULL || mctp_register_bus_dynamic_eid(mctp, astpcie_binding) < 0) {
        mctp_prerr("%s: MCTP init failed", __func__);
        goto bail;
    }

    if (mctp_astpcie_get_bdf(astpcie, &bdf) < 0) {
        mctp_prerr("%s: MCTP get BDF failed", __func__);
        goto bail;
    }

    mctp_prdebug("%s: BDF = 0x%02X", __func__, bdf);
    bus = (bdf & 0xff00) >> 8;
    dev = (bdf & 0xf8) >> 3;
    func = (bdf & 0x07);
    mctp_prdebug("%s: bus=%d, dev=%d, func=%d", __func__, bus, dev, func);
    *src_bus = (uint8_t)bus;
    *src_dev = (uint8_t)dev;
    *src_func = (uint8_t)func;
    mctp_astpcie_free(astpcie);
    mctp_destroy(mctp);
    return 0;

bail:
    mctp_astpcie_free(astpcie);
    mctp_destroy(mctp);
    return -1;
}

struct test_mctp_ctx* test_mctp_astpcie_init(uint8_t bus, uint8_t routing, uint8_t dst_dev, uint8_t dst_func,
                                                       uint8_t dst_eid, uint8_t src_eid, int pkt_size)
{
    struct mctp_binding_astpcie *astpcie;
    struct mctp_binding *astpcie_binding;
    struct test_mctp_ctx *mctp_ctx;
    struct mctp *mctp;

    mctp_ctx = (struct test_mctp_ctx *)malloc(sizeof(struct test_mctp_ctx));

    if (mctp_ctx == NULL) {
        mctp_prerr("%s: out of memory(test_mctp_ctx)", __func__);
        return NULL;
    }

    mctp = mctp_init();
    astpcie = mctp_astpcie_init();
    astpcie_binding = mctp_astpcie_core(astpcie);
    if (mctp == NULL || astpcie == NULL || astpcie_binding == NULL || mctp_register_bus_dynamic_eid(mctp, astpcie_binding) < 0) {
        mctp_prerr("%s: MCTP init failed", __func__);
        goto bail;
    }

    if(mctp_dynamic_eid_set(astpcie_binding, src_eid) < 0) {
        mctp_prerr("%s: MCTP set dynamic eid failed", __func__);
        goto bail;
    }

    if(mctp_astpcie_register_default_handler(astpcie) <0) {
        mctp_prerr("%s: MCTP register default handler failed", __func__);
        goto bail;
    }

    /* Setting the default slave address */
    astpcie_extra_params = (struct mctp_astpcie_pkt_private *)
                          malloc(sizeof(struct mctp_astpcie_pkt_private));
    if (astpcie_extra_params == NULL) {
        mctp_prerr("%s: out of memory(mctp_astpcie_pkt_private)", __func__);
        goto bail;
    }

    astpcie_extra_params->routing = routing;
    astpcie_extra_params->remote_id = bus << 8 | (dst_dev & 0x1f) << 3 | (dst_func & 0x07);
    mctp_prdebug("%s: astpcieextra_params remote_id=0x%02X", __func__, astpcie_extra_params->remote_id);
    mctp_ctx->mctp = mctp;
    mctp_ctx->prot = (void *)astpcie;
    mctp_ctx->astpcie_binding = astpcie_binding;
    return mctp_ctx;

bail:
    test_mctp_astpcie_free(mctp_ctx);
    return NULL;
}

int test_mctp_astpcie_send_data(struct test_mctp_ctx *ctx, uint8_t dst, uint8_t flag_tag,
                         void *req, size_t size)
{
    struct mctp_binding *astpcie_binding = ctx->astpcie_binding;
    bool tag_owner = flag_tag & MCTP_HDR_FLAG_TO? true: false;
    uint8_t tag = MCTP_HDR_GET_TAG(flag_tag);
    struct mctp *mctp = ctx->mctp;

    mctp_binding_set_tx_enabled(astpcie_binding, true);
    if (mctp_message_tx(mctp, dst, req, size,
                        tag_owner, tag, astpcie_extra_params) < 0) {
        mctp_prerr("%s: MCTP TX error", __func__);
        return -1;
    }

    return 0;
}

void test_mctp_astpcie_free(struct test_mctp_ctx* ctx)
{
    mctp_astpcie_free(ctx->prot);
    mctp_destroy(ctx->mctp);
    free(ctx);
    free(astpcie_extra_params);
}

int test_send_mctp_cmd(uint8_t bus, uint8_t routing, uint8_t dst_dev, uint8_t dst_func, uint8_t dst_eid, uint8_t src_eid,
                       uint8_t *tbuf, int tlen, uint8_t *rbuf, int *rlen)
{
    struct test_mctp_ctx *ctx;
    uint8_t tag = 0;
    int ret = -1;

    ctx = test_mctp_astpcie_init(bus, routing, dst_dev, dst_func, dst_eid, src_eid, MAX_PAYLOAD_SIZE);
    if (ctx == NULL) {
        mctp_prerr("%s: Error: mctp binding failed", __func__);
        return -1;
    }

    tag |= MCTP_HDR_FLAG_TO;
    ret = test_mctp_astpcie_send_data(ctx, dst_eid, tag, tbuf, tlen);
    if (ret < 0) {
        mctp_prerr("error: %s send failed\n", __func__);
        goto bail;
    }

    ctx->rx_buf = (uint8_t *)rbuf;

    ret = test_mctp_astpcie_recv_data_timeout_raw(ctx, dst_eid, -1);
    if (ret < 0) {
        mctp_prerr("%s: error getting response\n", __func__);
        goto bail;
    } else {
        *rlen = ret;
        ret = 0;
    }

bail:
    test_mctp_astpcie_free(ctx);
    return ret;
}

int test_mctp_fake_responder(uint8_t bus, uint8_t routing, uint8_t dst_dev, uint8_t dst_func,
                                        uint8_t dst_eid, uint8_t src_eid)
{
    struct test_mctp_ctx *ctx;

    ctx = test_mctp_astpcie_init(bus, routing, dst_dev, dst_func, dst_eid, src_eid, MAX_PAYLOAD_SIZE);
    if (ctx == NULL) {
        mctp_prerr("%s: Error: mctp binding failed", __func__);
        return -1;
    }

    wait_for_request(ctx);
    test_mctp_astpcie_free(ctx);
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
    uint8_t getbdf_flag = 0;
    uint8_t test_status = 0;
    uint8_t debug_flag = 0;
    uint8_t mctp_type = 0;
    uint8_t src_func = 0;
    uint8_t dst_func = 0;
    uint8_t routing = 2;
    uint8_t src_dev = 0;
    uint8_t dst_dev = 0;
    uint8_t src_bus = 0;
    uint8_t dst_bus = 0;
    int loop_count = 1;
    int data_len = 0;
    char option = 0;
    int minargc = 7;
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
        case 'g':
            getbdf_flag = 1;
            minargc += 1;
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

    if (getbdf_flag) {
        ret = test_mctp_astpcie_get_bdf(&src_bus, &src_dev, &src_func);
        if (ret < 0) {
            mctp_prerr("Error get BDF failed");
        } else {
            printf("src_bus=%d, src_dev=%d, src_func=%d\n", src_bus, src_dev, src_func);
        }

        return ret;
    }

    if (argc < minargc) { // min params: mctp-astpcie-test -r <bus_num> <routing_type> <dst_dev> <des_func> <dst_eid> <src_eid>
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

    if (data_len > MCTP_BTU - msg_hdr_len) {
        mctp_prerr("length exceeds max payload length %d\n", MCTP_BTU - msg_hdr_len);
        usage(stdout, argc, argv);
        exit(EXIT_FAILURE);
    }

    dst_bus = (uint8_t)strtoul(argv[optind++], NULL, 0);
    routing = (uint8_t)strtoul(argv[optind++], NULL, 0);
    dst_dev = (uint8_t)strtoul(argv[optind++], NULL, 0);
    dst_func = (uint8_t)strtoul(argv[optind++], NULL, 0);
    dst_eid = (uint8_t)strtoul(argv[optind++], NULL, 0);
    src_eid = (uint8_t)strtoul(argv[optind++], NULL, 0);

    // requester
    if (requester_flag) {
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
            printf("argc=%d bus=%d dst_dev=%d dst_func=%d dst_eid=%d src_eid=%d mctp_type=%d rq_dgram_inst=0x%X cmd=%d\n",
                    argc, dst_bus, dst_dev, dst_func, dst_eid, src_eid, mctp_type, rq_dgram_inst, cmd);
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
            ret = test_send_mctp_cmd(dst_bus, routing, dst_dev, dst_func, dst_eid, src_eid,
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
            printf("argc=%d bus=%d dst_dev=%d dst_func=%d dst_eid=%d src_eid=%d\n",
                    argc, dst_bus, dst_dev, dst_func, dst_eid, src_eid);
        }

        ret = test_mctp_fake_responder(dst_bus, routing, dst_dev, dst_func, dst_eid, src_eid);
        if (ret < 0) {
            mctp_prerr("Error run fake responder failed");
            return -1;
        }
    }

    return 0;
}
