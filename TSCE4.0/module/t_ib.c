
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "common.h"
#include <infiniband/mad.h>
#include <infiniband/iba/ib_types.h>
#include <sys/time.h>
#include <stdio.h>


#define IB_MODULE_COL_NUM 4 


static struct mod_info ib_mod_info [] = {
    {"ib_xmitdata", "\0"},
    {"ib_rcvdata", "\0"},
    {"ib_xmitpktsize", "\0"},
    {"ib_rcvpktsize", "\0"}
};


typedef struct {
    unsigned long long data_xmt;
    unsigned long long data_rcv;
    unsigned long long pkt_xmt;
    unsigned long long pkt_rcv;
} IBflux;

static IBflux ib;
static char *ibd_ca;
static int ibd_ca_port;
static int ibd_timeout;
/* HCA port */
int port; 
ib_portid_t portid = {0};
int mask;
uint16_t cap_mask; 
static struct ibmad_port *srcport;
static uint8_t pc[1024];

static struct timeval start_time, end_time;

static int open_port(void);
static void close_port(void);
static void read_perf_counters(void);
static int reset_perf_counters(void);


static int 
open_port(void)
{
    int mgmt_classes[4] = {IB_SMI_CLASS, IB_SMI_DIRECT_CLASS, \
        IB_SA_CLASS, IB_PERFORMANCE_CLASS};

    port = 1;
    mask = 0xffff;

    /* open ibd_ca, port ibd_ca_port */
    srcport = mad_rpc_open_port(ibd_ca, ibd_ca_port, mgmt_classes, 4);
    if (srcport == NULL) {
        return -1;
    }

    ib_resolve_self_via(&portid, &port, 0, srcport);

    /* PerfMgt ClassPortInfo is a required attribute */
    memset(pc, 0, sizeof(pc));
    if (!pma_query_via(pc, &portid, port, ibd_timeout, \
                CLASS_PORT_INFO, srcport)) {
        return -1;
    }
    memcpy(&cap_mask, pc + 2, sizeof(cap_mask));

    return 0;
}


static void 
close_port(void)
{
    mad_rpc_close_port(srcport);
}


static void 
read_perf_counters(void)
{
    int i;
    char val[64] = {0};
    char buf_xmt[64] = {0}, buf_rcv[64] = {0};
    char buf_xmt_pkt[64] = {0}, buf_rcv_pkt[64] = {0};

    memset(pc, 0, sizeof(pc));
    pma_query_via(pc, &portid, port, ibd_timeout, \
            IB_GSI_PORT_COUNTERS, srcport);

    /* data send */
    mad_decode_field(pc, IB_PC_XMT_BYTES_F, val);
    mad_dump_field(IB_PC_XMT_BYTES_F, buf_xmt, sizeof(buf_xmt), val);

    /* data receive */
    mad_decode_field(pc, IB_PC_RCV_BYTES_F, val);
    mad_dump_field(IB_PC_RCV_BYTES_F, buf_rcv, sizeof(buf_rcv), val);

    /* packet send */
    mad_decode_field(pc, IB_PC_XMT_PKTS_F, val);
    mad_dump_field(IB_PC_XMT_PKTS_F, buf_xmt_pkt, sizeof(buf_xmt_pkt), val);

    /* packet receive */
    mad_decode_field(pc, IB_PC_RCV_PKTS_F, val);
    mad_dump_field(IB_PC_RCV_PKTS_F, buf_rcv_pkt, sizeof(buf_rcv_pkt), val);

    for ( i = 0; i < 64; i++) {
        if ((buf_xmt[i] > 47) && (buf_xmt[i] < 58)) {
            ib.data_xmt = atoll(&buf_xmt[i]) * 4.0 / (1024.0 * 1024.0);
            break;
        }
    }
    for (i = 0; i < 64; i++) {
        if ((buf_rcv[i] > 47) && (buf_rcv[i] < 58)) {
            ib.data_rcv = atoll(&buf_rcv[i]) * 4.0 / (1024.0 * 1024.0);
            break;
        }
    }
    for (i = 0; i < 64; i++) {
        if ((buf_xmt_pkt[i] > 47) && (buf_xmt_pkt[i] < 58)) {
            ib.pkt_xmt = atoll(&buf_xmt_pkt[i]);
            break;
        }
    }
    for (i = 0; i < 64; i++) {
        if ((buf_rcv_pkt[i] > 47) && (buf_rcv_pkt[i] < 58)) {
            ib.pkt_rcv = atoll(&buf_rcv_pkt[i]);
            break;
        }
    }

    return;
}


static int 
reset_perf_counters(void)
{
    memset(pc, 0, sizeof(pc));
    if (!performance_reset_via(pc, &portid, port, mask, \
                ibd_timeout, IB_GSI_PORT_COUNTERS, srcport)) {
        return -1;
    }
    return 0;
}


void 
ib_start(void)
{
    open_port();

    reset_perf_counters();
    gettimeofday(&start_time, NULL);
}


void 
ib_read(struct module *mod)
{
    assert(mod != NULL);
    double time;

    bzero(&ib, sizeof(ib));
    read_perf_counters();

    gettimeofday(&end_time, NULL);
    time = (end_time.tv_sec - start_time.tv_sec) + \
           (end_time.tv_usec - start_time.tv_usec) / (double) 1000000;


    assert(mod->col == IB_MODULE_COL_NUM);
    snprintf((mod->info[0]).index_data, LEN_32, "%.2f", ib.data_xmt / time);
    snprintf((mod->info[1]).index_data, LEN_32, "%.2f", ib.data_rcv / time);
    snprintf((mod->info[2]).index_data, LEN_32, "%.2f", ib.pkt_xmt / time);
    snprintf((mod->info[3]).index_data, LEN_32, "%.2f", ib.pkt_rcv / time);

    // printf("data_xmt: %lld\n", ib.data_xmt);
    // printf("data_rcv: %lld\n", ib.data_rcv);
    // printf("pkt_xmt: %lld\n", ib.pkt_xmt);
    // printf("pkt_rcv: %lld\n", ib.pkt_rcv);

    reset_perf_counters();
    start_time = end_time;
}


void
ib_stop(void)
{
    close_port();
}


int
mod_register(struct module *mod)
{
    assert(mod != NULL);

    if (open_port() < 0) {
        return MODULE_FLAG_NOT_USEABLE;
    }
    close_port();

    register_module_fields(mod, ib_mod_info, \
            IB_MODULE_COL_NUM, ib_start, ib_read);  

    return MODULE_FLAG_NORMAL;
}


/*
int
main(void)
{
    printf("test begin\n");
    if (ib_start() < 0) {
        printf("open port failed\n");
    }
    ib_read();
    ib_stop();

    return 0;
}
*/
