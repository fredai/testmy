#ifndef _TEYE_H_
#define _TEYE_H_

#define COMPANYINFO "Inspur (Beijing) Electronic Information Industry Co., Ltd."
#define VER_ID "2.0.1"

#define TPORT 9999

#define SI_CHECK  9001

#define SI_START  101
#define SI_READ   102
#define SI_STOP   108
#define SI_END    109

#define MACHINE_MAX 256
#define NET_MAX 6

struct  c_sock
{
  char name[256];
  int sockfd;
  int check;
};

struct  c_net
{
  char dev_name[32];
  float send;
  float send_packet_size;
  float receive;
  float receive_packet_size;
};

struct  c_ib
{
  float XmitData; //MBs
  float RcvData;  //MBs
  float XmitPktsize;
  float RcvPktsize;
};

struct  c_cpu
{
  float user;
  float sys;
  float idle;
  float iowait;
};

struct  c_disk
{
  char disk_name[32];
  float reads;
  float r_size;    
  float writes;     //MB
  float w_size;   //MB
};

struct  c_mem
{
  unsigned long MemTotal;
  unsigned long MemUsed;    
  unsigned long Buffers;
  unsigned long Cached;   
};

struct  c_nfs
{
  unsigned long long read;
  unsigned long long write;
  float read_nfsd;  // MB/s
  float write_nfsd; // MB/s
};

struct  teye_st
{
  unsigned long long id;
  int single;
  char hostname[225];
  int cpu_num;
  int cache_line_size;
  
  float microstructure_values[19];
  struct c_cpu cpu;
  struct c_ib  ib;  
  
  //int net_num;
  struct c_net net[3];
  
  struct c_disk disk;
  struct c_mem mem;
  struct c_nfs nfs; 
};
#endif
