
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define ETH_MODULE_COL_NUM       4
#define NET_MAX                                   8
#define ACCESS_PROC_NET_DEV         "/proc/net/dev"

#define	SEND		        "send"
#define	RECEIVE		        "receive"
#define     SPSIZE		        "spsize"
#define	RPSIZE		        "rpsize"
#define     NETINFOFORMAT  " face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"

static int net_total_col_num = 0;
static unsigned long long milliseconds = 0;

struct t_net {
        char if_name[24];
        unsigned long long if_ibytes;
        unsigned long long if_obytes;
        unsigned long long if_ipackets;
        unsigned long long if_opackets;
};

struct  c_net
{
  char dev_name[32];
  float send;
  float send_packet_size;
  float receive;
  float receive_packet_size;
};

struct t_net ifnet_i[NET_MAX];
struct t_net ifnet_r[NET_MAX];

#if 0
static struct mod_info eth_mod_info [] = {
	{"send_mb", "\0", "\0"},
	{"receive_mb", "\0", "\0"},
	{"spsize_mb", "\0", "\0"},
	{"rpsize_mb", "\0", "\0"}
};
#endif

static struct mod_info eth_mod_info[NET_MAX * ETH_MODULE_COL_NUM];

void 
eth_start()
    {
      char buf[1024];
      int i=0;
      int ret;
      FILE *fp = NULL;
      int real_net_number = 0;
      int blank = 0;
      
      if( (fp = fopen(ACCESS_PROC_NET_DEV, "r")) == NULL) {
        perror(ACCESS_PROC_NET_DEV);
        return;
      }
     
      /* throw away the header lines */
      if((fgets(buf,1024,fp) == NULL)||(fgets(buf,1024,fp) == NULL))
      {
        printf("wrong\n");
      }

      milliseconds = get_current_millisecond();
     
    /*
    * Inter-|   Receive                                                |  Transmit
    * face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    * lo:    1956      30    0    0    0     0          0         0     1956      30    0    0    0     0       0          0
    * eth0:       0       0    0    0    0     0          0         0   458718       0  781    0    0     0     781          0
    * sit0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    * eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    */
        real_net_number = net_total_col_num / ETH_MODULE_COL_NUM;
            for(i=0;i<real_net_number;i++) {
                    if(fgets(buf,1024,fp) == NULL)
                            break;

                    blank = 0;
                    while(' ' == buf[blank]) {
                        blank ++;
                    }
                   
                                         /* 1   2   3    4   5   6   7   8   9   10   11   12  13  14  15  16 */
                    ret = sscanf(buf+blank, "%[^:]%*[:] %Lu %Lu %*s %*s %*s %*s %*s %*s %Lu %Lu", \
                                        (char *)&ifnet_i[i].if_name,
                            &ifnet_i[i].if_ibytes,
                            &ifnet_i[i].if_ipackets,
                            &ifnet_i[i].if_obytes,
                            &ifnet_i[i].if_opackets);
        /* printf("%s%s:%llu %llu %llu %llu\n",
                            buf, ifnet_i[i].if_name,ifnet_i[i].if_ibytes,ifnet_i[i].if_ipackets,ifnet_i[i].if_obytes,ifnet_i[i].if_opackets); */
            }     
                    fclose(fp);
    }

int parse_net_traffic(char *traffic_data, int sequence_number, struct module *mod, unsigned long long milli_cycles)
{
    assert(traffic_data != NULL);
    assert(mod != NULL);
    
    int ret = -1;
    int blank = 0;
    struct t_net  network_card_traffic;
    struct c_net tmp;
    double seconds = 0.0;

    memset(&network_card_traffic, 0, sizeof(struct t_net));
    memset(&tmp, 0, sizeof(struct c_net));
    
    /*
    * Inter-|   Receive                                                |  Transmit
    * face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    * lo:    1956      30    0    0    0     0          0         0     1956      30    0    0    0     0       0          0
    * eth0:       0       0    0    0    0     0          0         0   458718       0  781    0    0     0     781          0
    * sit0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    * eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    */
    while(' ' == traffic_data[blank]) {
        blank++;
    }
                                                /* 1   2   3    4   5   6   7   8   9   10   11   12  13  14  15  16 */
    ret = sscanf(traffic_data+blank, "%[^:]%*[:] %Lu %Lu %*s %*s %*s %*s %*s %*s %Lu %Lu",
                                                    network_card_traffic.if_name,
                                                    &network_card_traffic.if_ibytes,
                                                    &network_card_traffic.if_ipackets,
                                                    &network_card_traffic.if_obytes,
                                                    &network_card_traffic.if_opackets);

    /* compare the network_card name
    * then save the network card traffic data to corresponding ifnet_i array
    * and caculate the network card rate
    */
    if((!strncmp(network_card_traffic.if_name,  ifnet_i[sequence_number].if_name, strlen(network_card_traffic.if_name))) && \
         (!strncmp(network_card_traffic.if_name,  eth_mod_info[sequence_number *4].index_hdr, strlen(network_card_traffic.if_name)))) {
                  
         
        tmp.receive = ((network_card_traffic.if_ibytes >= ifnet_i[sequence_number].if_ibytes) ? \
                                     (network_card_traffic.if_ibytes-ifnet_i[sequence_number].if_ibytes) : 0) \
                                     /1024.0/1024.0;
                                     
        tmp.receive_packet_size = (network_card_traffic.if_ipackets > ifnet_i[sequence_number].if_ipackets) ? \
                                                         tmp.receive/(network_card_traffic.if_ipackets-ifnet_i[sequence_number].if_ipackets) : 0;
                                                         
        tmp.send = ((network_card_traffic.if_obytes >= ifnet_i[sequence_number].if_obytes) ? \
                                (network_card_traffic.if_obytes-ifnet_i[sequence_number].if_obytes) : 0) \
                                /1024.0/1024.0;
        
        tmp.send_packet_size = (network_card_traffic.if_opackets > ifnet_i[sequence_number].if_opackets) ? \
                                                              tmp.send/(network_card_traffic.if_opackets-ifnet_i[sequence_number].if_opackets) : \
                                                              0;

        if(milli_cycles != 0) {
            seconds = milli_cycles / 1000.0;
            snprintf(mod->info[sequence_number *4 + 0].index_data, LEN_32, "%.2f", tmp.receive / seconds);
            snprintf(mod->info[sequence_number *4 + 1].index_data, LEN_32, "%.2f", tmp.receive_packet_size); 
            snprintf(mod->info[sequence_number *4 + 2].index_data, LEN_32, "%.2f", tmp.send / seconds);
            snprintf(mod->info[sequence_number *4 + 3].index_data, LEN_32, "%.2f", tmp.send_packet_size);
        } else {
            snprintf(mod->info[sequence_number *4 + 0].index_data, LEN_32, "%.2f", 0.0);
            snprintf(mod->info[sequence_number *4 + 1].index_data, LEN_32, "%.2f", 0.0); 
            snprintf(mod->info[sequence_number *4 + 2].index_data, LEN_32, "%.2f", 0.0);
            snprintf(mod->info[sequence_number *4 + 3].index_data, LEN_32, "%.2f", 0.0);
        }

        ifnet_i[sequence_number].if_ibytes       = network_card_traffic.if_ibytes;
        ifnet_i[sequence_number].if_ipackets   = network_card_traffic.if_ipackets;
        ifnet_i[sequence_number].if_obytes      = network_card_traffic.if_obytes;
        ifnet_i[sequence_number].if_opackets  = network_card_traffic.if_opackets;
        
#if DEBUG
        printf("%s:sequence_number = %d\nrbytes = %s, rpackets = %s, sbytes = %s, spackets = %s\n",
                                                                                                                            network_card_traffic.if_name,
                                                                                                                            sequence_number,
                                                                                                                            eth_mod_info[sequence_number *4 + 0].index_data,
                                                                                                                            eth_mod_info[sequence_number *4 + 1].index_data,
                                                                                                                            eth_mod_info[sequence_number *4 + 2].index_data,
                                                                                                                            eth_mod_info[sequence_number *4 + 3].index_data);
        printf("if_ibytes = %llu, if_ipackets = %llu, if_obytes = %llu, if_opackets = %llu\n",
                                                                                                                            ifnet_i[sequence_number].if_ibytes,
                                                                                                                            network_card_traffic.if_ipackets,
                                                                                                                            network_card_traffic.if_obytes,
                                                                                                                            network_card_traffic.if_opackets); 

#endif
    }

    return 0;
    
}

void 
eth_read(struct module *mod)
{
  char buf[1024];
  int i=0;
  FILE *fp = NULL;
  unsigned long long tmp_millisecond = 0;
  unsigned long long cycle_millisecond = 0;
  
  if( (fp = fopen(ACCESS_PROC_NET_DEV,"r")) == NULL) {
    printf("failed to open - %s\n", ACCESS_PROC_NET_DEV);
    return;
  }
  /* throw away the header lines */
  if((fgets(buf,1024,fp) == NULL)||(fgets(buf,1024,fp) == NULL))
  {
    printf("wrong\n");
  }

  tmp_millisecond = get_current_millisecond();

   cycle_millisecond = (tmp_millisecond >= milliseconds) ? (tmp_millisecond - milliseconds) : tmp_millisecond;
  milliseconds = tmp_millisecond;
 
    /* parse all network card traffic info */
    for(i=0;i<NET_MAX;i++) {
        if(fgets(buf,1024,fp) == NULL) {
            break;
        }
        parse_net_traffic(buf, i, mod, cycle_millisecond);
    }

    fclose(fp);
    return;
}


int get_number_net()
{
    unsigned int number = 0;
    FILE *fp = NULL;
    char buf[256] = "";
    int i = 0;
	int tmp = 0;
	char net_name[256] = "";
	int ret = 0;
    int blank = 0;

    memset(eth_mod_info, 0, sizeof(eth_mod_info));

    fp = fopen(ACCESS_PROC_NET_DEV, "r");
    if (NULL == fp) {
        perror(ACCESS_PROC_NET_DEV);
        return -1;
    } else {
        for (i = 0; i < 2; i++) {
            if (NULL == fgets(buf, sizeof(buf), fp)) {
                fclose(fp);
                return -1;
            }
        }
        if (0 != (ret = strncmp(NETINFOFORMAT, buf, strlen(buf)))) {
            printf("strlen(buf) = %d\n", strlen(buf));
            printf("strlen(NETINFOFORMAT) = %d\n", strlen(NETINFOFORMAT));
            printf("ret = %d, the second line format error of the %s:\n%s", ret, ACCESS_PROC_NET_DEV, buf);
            printf("NETINFOFORMAT:\n%s", NETINFOFORMAT);
            fclose(fp);
            return -1;
        }
        while((NULL != fgets(buf, sizeof(buf), fp)) && (number < NET_MAX)) {
			tmp = number * ETH_MODULE_COL_NUM;
            blank = 0;
            while(' ' == buf[blank]) {
                blank++;
            }
            sscanf(buf+blank, "%[^:]", net_name);
			sprintf(eth_mod_info[tmp+0].index_hdr, "%s_%s", net_name, RECEIVE);
			sprintf(eth_mod_info[tmp+1].index_hdr, "%s_%s", net_name, RPSIZE);
			sprintf(eth_mod_info[tmp+2].index_hdr, "%s_%s", net_name, SEND);
			sprintf(eth_mod_info[tmp+3].index_hdr, "%s_%s", net_name, SPSIZE);
            number++;
        }

    }
    fclose(fp);
    net_total_col_num = number * ETH_MODULE_COL_NUM;
    return number;
}


int
mod_register(struct module* mod)
{
	/* TODO: add decide module is usealbe in current HW and SW environment */
	assert(mod != NULL);

	/* get the number of Network card */
	if(-1 == get_number_net()) {
            printf("get net number error\n");
            return MODULE_FLAG_NOT_USEABLE;
	}
	
	register_module_fields(mod, eth_mod_info, \
						  net_total_col_num, eth_start, eth_read);
	return 0;
}
