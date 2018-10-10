
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
#define SPSIZE		        "spsize"
#define	RPSIZE		        "rpsize"
#define NETINFOFORMAT  " face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"

#define NET_COL_NUM 4
int net_num = 0;
int number = 0;
static unsigned long long milliseconds = 0;

struct t_net {
    char if_name[LEN_32];
    unsigned long long if_ibytes;
    unsigned long long if_obytes;
    unsigned long long if_ipackets;
    unsigned long long if_opackets;
};

struct  c_net
{
	float send;
	float send_packet_size;
	float receive;
	float receive_packet_size;
};

struct t_net ifnet_i[NET_MAX];
struct c_net ifnet_r[NET_MAX];
struct t_net ifnet_avr;
struct c_net ifnet_age;

static struct mod_info eth_mod_info [] = {
	{"ibreceive_rate", "\0"},
	{"ibtransmit_rate", "\0"},
	{"ibreceive_pack", "\0"},
	{"ibtransmit_pack", "\0"}

};

char if_name[NET_MAX][LEN_32];


void 
eth_start()
{
      char buf[1024];
      int i=0;
      FILE *fp = NULL;
      int blank = 0;
      int count=0;
      int ret;
      
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
     

	for(i=0; i < net_num;i++) {
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
	    /* printf("%s%s:%llu %llu %llu %llu -----------------\n",
	                    buf, ifnet_i[i].if_name,ifnet_i[i].if_ibytes,ifnet_i[i].if_ipackets,ifnet_i[i].if_obytes,ifnet_i[i].if_opackets); */
	}
		
	for (i =0; i< net_num; i++) {
		if (strstr(ifnet_i[i].if_name, "lo") || strstr(ifnet_i[i].if_name, "virbr") || strstr(ifnet_i[i].if_name, "Receive") \
		|| strstr(ifnet_i[i].if_name, "packets") || strstr(ifnet_i[i].if_name, "e")) {
			continue;
		}	  
		ifnet_avr.if_ibytes = ifnet_avr.if_ibytes + ifnet_i[i].if_ibytes;
		ifnet_avr.if_ipackets = ifnet_avr.if_ipackets + ifnet_i[i].if_ipackets;
		ifnet_avr.if_obytes = ifnet_avr.if_obytes + ifnet_i[i].if_obytes;
		ifnet_avr.if_opackets = ifnet_avr.if_opackets + ifnet_i[i].if_opackets;
		count ++ ;
	}
	    ifnet_avr.if_ibytes = ifnet_avr.if_ibytes/count;
	    ifnet_avr.if_ipackets = ifnet_avr.if_ipackets/count;
	    ifnet_avr.if_obytes = ifnet_avr.if_obytes/count;
	    ifnet_avr.if_opackets = ifnet_avr.if_opackets/count;

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
    int i = 0, flag = 0;

    memset(&network_card_traffic, 0, sizeof(struct t_net));
    memset(&tmp, 0, sizeof(struct c_net));

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

    /* caculate the network card rate
    */

    ifnet_r[sequence_number].receive = ((network_card_traffic.if_ibytes >= ifnet_i[sequence_number].if_ibytes) ? \
                                 (network_card_traffic.if_ibytes-ifnet_i[sequence_number].if_ibytes) : 0) \
                                 /1024.0/1024.0;
                                 
    ifnet_r[sequence_number].receive_packet_size = (network_card_traffic.if_ipackets > ifnet_i[sequence_number].if_ipackets) ? \
                                                     ifnet_r[sequence_number].receive/(network_card_traffic.if_ipackets-ifnet_i[sequence_number].if_ipackets):0;
                                                     
    ifnet_r[sequence_number].send = ((network_card_traffic.if_obytes >= ifnet_i[sequence_number].if_obytes) ? \
                            (network_card_traffic.if_obytes-ifnet_i[sequence_number].if_obytes) : 0) \
                            /1024.0/1024.0;
    
    ifnet_r[sequence_number].send_packet_size = (network_card_traffic.if_opackets > ifnet_i[sequence_number].if_opackets) ? \
                                                          ifnet_r[sequence_number].send/(network_card_traffic.if_opackets-ifnet_i[sequence_number].if_opackets):0;
                                                          
    ifnet_i[sequence_number].if_ibytes       = network_card_traffic.if_ibytes;
    ifnet_i[sequence_number].if_ipackets   = network_card_traffic.if_ipackets;
    ifnet_i[sequence_number].if_obytes      = network_card_traffic.if_obytes;
    ifnet_i[sequence_number].if_opackets  = network_card_traffic.if_opackets;
        


    return 0;   
}

void 
eth_read(struct module *mod)
{
	char buf[1024];
	int i=0, j=0;
	FILE *fp = NULL;
	unsigned long long tmp_millisecond = 0;
	unsigned long long cycle_millisecond = 0;
	double seconds = 0.0;

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
	if (strstr(buf, "lo") || strstr(buf, "virbr") || strstr(buf, "Receive") \
            || strstr(buf, "packets") || strstr(buf, "e")) {
             continue;
        }
        parse_net_traffic(buf, j, mod, cycle_millisecond);
	j++;
    }

    memset(&ifnet_age, 0, sizeof(ifnet_age));

    for (i =0; i< number; i++) {
    	ifnet_age.send = ifnet_age.send + ifnet_r[i].send;
    	ifnet_age.send_packet_size = ifnet_age.send_packet_size + ifnet_r[i].send_packet_size;
    	ifnet_age.receive = ifnet_age.receive + ifnet_r[i].receive;
    	ifnet_age.receive_packet_size = ifnet_age.receive_packet_size + ifnet_r[i].receive_packet_size;
    }
    ifnet_age.send = ifnet_age.send/number;
    ifnet_age.send_packet_size = ifnet_age.send_packet_size/number;
    ifnet_age.receive = ifnet_age.receive/number;
    ifnet_age.receive_packet_size = ifnet_age.receive_packet_size/number;

    seconds = cycle_millisecond / 1000.0;
    snprintf(mod->info[0].index_data, LEN_32, "%.2f", ifnet_age.receive / seconds);
    snprintf(mod->info[2].index_data, LEN_32, "%.2f", ifnet_age.receive_packet_size); 
    snprintf(mod->info[1].index_data, LEN_32, "%.2f", ifnet_age.send / seconds);
    snprintf(mod->info[3].index_data, LEN_32, "%.2f", ifnet_age.send_packet_size);
  

    fclose(fp);
    return;
}


int get_number_net()
{
    unsigned int  un_number = 0;
    FILE *fp = NULL;
    char buf[256] = "";
    char eth_name[LEN_32];
    int i = 0;
    int ret = 0;
    int blank = 0;

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
	        blank = 0;
	        while(' ' == buf[blank]) {
	            blank++;
	        }
	        sscanf(buf+blank, "%[^:]", eth_name);
		    if (strstr(eth_name, "lo") || strstr(eth_name, "virbr") || strstr(eth_name, "Receive") \
			|| strstr(eth_name, "packets") || strstr(eth_name, "e")) {
			
				un_number++;
				continue;
		    }
	    	strcpy(if_name[number], eth_name);
            number++;
    	}

    }
    fclose(fp);
    net_num = number + un_number;
    return net_num;
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
						  NET_COL_NUM, eth_start, eth_read);
	return 0;
}
