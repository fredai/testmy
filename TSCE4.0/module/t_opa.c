/*
 * Copyright (C) Inspur(Beijing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define OPA_MODULE_COL_NUM 4

static struct mod_info opa_info[] = {
	{"OPATxWords","\0"},
	{"OPARxWords","\0"},
	{"OPATxPkt","\0"},
	{"OPARxPkt","\0"},
};

struct opa_value {
	float opa_txw;
	float opa_rxw;
	float opa_txp;
	float opa_rxp;
};

struct opa_type {
	char name[LEN_32];
	char value[LEN_32];
};

struct opa_value prevalue;
struct opa_value newvalue;
struct opa_value calcu;
struct opa_type type;

void 
opa_start()
{
	char buf[1024] = {0};
	int blank = 0;
	int ret = 0;
	FILE *fp;
	char tmp[LEN_32];
	
	if ((fp = popen("hfi1stats", "r")) == NULL) 
//	if ((fp = fopen("test", "r")) == NULL) 
	{
		perror("failed to open dmidecode");
		return;
	}
	
	while (fgets(buf,1024,fp)) 
	{
		blank = 0;
		while(' ' == buf[blank]) {
			blank ++;
		}
		if (strlen(buf) <= 0)
		{
			continue;
		}
		ret =  sscanf (buf+blank, "%s %s", type.name, type.value);
		if (!strcasecmp(type.name, "RxWords"))
		{
			sscanf (type.value, "%[^K]", tmp);
			newvalue.opa_txw = atof(tmp);
		}
		if (!strcasecmp(type.name, "TxWords"))
		{
			sscanf (type.value, "%[^K]", tmp);
			newvalue.opa_rxw = atof(tmp);
		}
		if (!strcasecmp(type.name, "TxPkt"))
		{
			newvalue.opa_txp = atof(type.value);
		}
		if (!strcasecmp(type.name, "RxPkt"))
		{
			newvalue.opa_rxp = atof(type.value);
		}
		
		memset(buf, 0, sizeof(buf));	
	}
	fclose(fp);

}


void 
opa_read(struct module *mod)
{

	assert(mod != NULL);

	char buf[1024] = {0};
	int blank = 0;
	int ret = 0;
	FILE *fp;
	char tmp[LEN_32];
	
	if ((fp = popen("hfi1stats 2>/dev/null", "r")) == NULL) 
//	if ((fp = fopen("test", "r")) == NULL) 
	{
		perror("failed to open dmidecode");
		return; 
	}

	assert (mod -> col == OPA_MODULE_COL_NUM);
	while (fgets(buf,1024,fp)) 
	{
		bzero (tmp, sizeof(tmp));
		blank = 0;
		while(' ' == buf[blank]) {
			blank ++;
		}
		if (strlen(buf) <= 0)
		{
			continue;
		}
		ret =  sscanf (buf+blank, "%s %s", type.name, type.value);
		if (!strcasecmp(type.name, "RxWords"))
		{
			sscanf (type.value, "%[^K]", tmp);
			newvalue.opa_txw = atof(tmp);
			calcu.opa_txw = (newvalue.opa_txw > prevalue.opa_txw)?(newvalue.opa_txw - prevalue.opa_txw)*2/1024.0:0;
			snprintf((mod->info[0]).index_data, LEN_32, "%.2f", calcu.opa_txw);
		}
		if (!strcasecmp(type.name, "TxWords"))
		{
			sscanf (type.value, "%[^K]", tmp);
			newvalue.opa_rxw = atof(tmp);
			calcu.opa_rxw = (newvalue.opa_rxw > prevalue.opa_rxw)?(newvalue.opa_rxw - prevalue.opa_rxw)*2/1024.0:0;
			snprintf((mod->info[1]).index_data, LEN_32, "%.2f", calcu.opa_rxw);
		}
		if (!strcasecmp(type.name, "TxPkt"))
		{
			newvalue.opa_txp = atof(type.value);
			calcu.opa_txp = (newvalue.opa_txp > prevalue.opa_txp) ?calcu.opa_txw*2*1024.0/(newvalue.opa_txp - prevalue.opa_txp):0;
			snprintf((mod->info[2]).index_data, LEN_32, "%.2f", calcu.opa_txp);
		}
		if (!strcasecmp(type.name, "RxPkt"))
		{
			newvalue.opa_rxp = atof(type.value);
			calcu.opa_rxp = (newvalue.opa_rxp > prevalue.opa_rxp) ?calcu.opa_rxw*2*1024.0/(newvalue.opa_rxp - prevalue.opa_rxp):0;
			snprintf((mod->info[3]).index_data, LEN_32, "%.2f", calcu.opa_rxp);
		}
		
		prevalue.opa_txw = newvalue.opa_txw;	
		prevalue.opa_rxw = newvalue.opa_rxw;	
		prevalue.opa_txp = newvalue.opa_txp;	
		prevalue.opa_rxp = newvalue.opa_rxp;	

		memset(buf, 0, sizeof(buf));	
	}
	fclose(fp);
	
}

int
mod_register(struct module *mod)
{
	assert(mod != NULL);

	FILE *fp;
	char error_buf[LEN_64];

	fp =popen("hfi1stats 2>&1", "r");
	fgets(error_buf,LEN_64, fp);

	if (strstr(error_buf, "not found")) 
	{
		return MODULE_FLAG_NOT_USEABLE;
	}
	fclose(fp);
	/* TODO: add decide module is useable in current HW and SW environment */

	register_module_fields(mod, opa_info, OPA_MODULE_COL_NUM, \
				 opa_start, opa_read);
	return 0;
}
