
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "con_config.h"
#include "error_handle.h"
#include "util.h"
#include "buffer.h"
#include "svr_config.h"
#include <signal.h>


static int analyse_line_index(char *line, config_mod_info_t *mod);
static int check_modinfo(const config_mod_info_t *p, \
        const config_mod_info_t *head);
static int analyse_line_xml(char *line, xml_config_t *conf);


int 
get_config_item_and_value ( char *config_line, \
        char *config_item, const int config_item_len, \
        char *config_value, const int config_value_len) 
{

    assert(config_line != NULL && config_item != NULL && \
			config_value != NULL && config_line [ 0 ] != '\0' && \
			config_item_len > 1 && config_value_len > 1);

    char *delim = " \t\r\n\f\v";
    char *saveptr;
    char *token;
    char *str = config_line;

    token = strtok_r(str, delim, & saveptr);
    if (token == NULL) {
        err_msg("empty line");
        return -1;
    }
    /* item too long */
    if (strlen(token) > config_item_len) {
        err_msg("config item %s too long", token);
        return -1;
    }

    strcpy(config_item, token);

    token = strtok_r(NULL, delim, &saveptr);
    /* no value */
    if (token == NULL) {
        err_msg("config item %s have no value", config_item);
        return -1;
    }
    /* too many value */
    char *remain_str = strtok_r(NULL, delim, & saveptr);
    if (remain_str != NULL) {
        err_msg("config item %s have too many values",config_item);
        return -1;
    }
    /* value too long */
    if (strlen(token) > config_value_len) {
        err_msg("config value %s too long", token);
        return -1;
    }

    strcpy (config_value, token);

    return 0;

}


int 
ignore_all_signals(void) 
{

    int ret;

    struct sigaction sa_alrm;
    sa_alrm.sa_handler = SIG_IGN;
    sigemptyset ( & sa_alrm.sa_mask );
    sa_alrm.sa_flags = 0; 
    ret = sigaction ( SIGALRM, & sa_alrm, NULL );
    if ( ret == -1 ) {
        return -1;
    }
    struct sigaction sa_int;
    sa_int.sa_handler = SIG_IGN;
    sigemptyset ( & sa_int.sa_mask );
    sa_int.sa_flags = 0;
    ret = sigaction ( SIGINT, & sa_int, NULL );
    if ( ret == -1 ) {
        return -2;
    }
    struct sigaction sa_hup;
    sa_hup.sa_handler = SIG_IGN;
    sigemptyset ( & sa_hup.sa_mask );
    sa_hup.sa_flags = 0;
    ret = sigaction ( SIGHUP, & sa_hup, NULL );
    if ( ret == -1 ) {
        return -3;
    }
    struct sigaction sa_quit;
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset ( & sa_quit.sa_mask );
    sa_quit.sa_flags = 0;
    ret = sigaction ( SIGQUIT, & sa_quit, NULL );
    if ( ret == -4 ) {
        return -1;
    }
    /*struct sigaction sa_usr1;
    sa_usr1.sa_handler = SIG_IGN;
    sigemptyset ( & sa_usr1.sa_mask );
    sa_usr1.sa_flags = 0;
    ret = sigaction ( SIGUSR1, & sa_usr1, NULL );
    if ( ret == -1 ) {
        return -5;
    }*/
    struct sigaction sa_usr2;
    sa_usr2.sa_handler = SIG_IGN;
    sigemptyset ( & sa_usr2.sa_mask );
    sa_usr2.sa_flags = 0;
    ret = sigaction ( SIGUSR2, & sa_usr2, NULL );
    if ( ret == -1 ) {
        return -6;
    }
    struct sigaction sa_cont;
    sa_cont.sa_handler = SIG_IGN;
    sigemptyset ( & sa_cont.sa_mask );
    sa_cont.sa_flags = 0;
    ret = sigaction ( SIGCONT, & sa_cont, NULL );
    if ( ret == -1 ) {
        return -7;
    }
    struct sigaction sa_tstp;
    sa_tstp.sa_handler = SIG_IGN;
    sigemptyset ( & sa_tstp.sa_mask );
    sa_tstp.sa_flags = 0;
    ret = sigaction ( SIGTSTP, & sa_tstp, NULL );
    if ( ret == -1 ) {
        return -8;
    }
    return 0;
}


static int 
analyse_line_index(char *line, config_mod_info_t *mod)
{
	assert (line != NULL && mod != NULL);

	char *str, *token, *saveptr;
	config_mod_info_item_t *item;

	item = &(mod->info);
	str = line;
	token = strtok_r(str, CONFIG_ITEM_DELIM, &saveptr);
	if (token == NULL) {
		err_msg("index configure empty line");
		return -1;
	}

	strncpy(item -> mod_name, token, strlen(token));
	while ( (token = strtok_r(NULL, CONFIG_ITEM_DELIM, &saveptr)) != NULL) {
		strncpy( (item->index_info[item->index_cnt]).index_name, \
				token, strlen(token));
		
		/* analyse unit */
		token = strtok_r(NULL, CONFIG_ITEM_DELIM, &saveptr);
		if (NULL == token) {
			return -1;
		}
		strncpy( (item->index_info[item->index_cnt]).index_unit, \
				token, strlen(token));

        /* analyse data type */
        token = strtok_r(NULL, CONFIG_ITEM_DELIM, &saveptr);
        if (NULL == token) {
            return -1;
        }
        strncpy( (item->index_info[(item->index_cnt)++]).data_type, \
                token, strlen(token));

	}

	return 0;
}

static int
check_modinfo(const config_mod_info_t *p, const config_mod_info_t *head)
{
    assert(p != NULL && head != NULL);

    int i, j;

    while ( (head = head->next) != NULL) {
        if (strcmp(p->info.mod_name, head->info.mod_name) == 0) {
            err_msg("same module name: %s", p->info.mod_name);
            return -1;
        }

        for (i = 0; i < p->info.index_cnt; i++) {
            for (j = 0; j < head->info.index_cnt; j++) {
                if (strcmp(p->info.index_info[i].index_name, \
                            head->info.index_info[j].index_name) == 0) {
                    err_msg("same index name: %s", \
                            p->info.index_info[i].index_name);
                    return -1;
                }
            }
        }
    }

    return 0;
}

        


int 
analyse_index_info_config(const char *config_path, \
						config_mod_info_t *g_mod_info)
{
	assert(config_path != NULL && config_path[0] != '\0' && g_mod_info != NULL);
	assert(g_mod_info->next == NULL);

	int str_len;
	FILE *index_file;
	char line_buffer[MAX_CONFIG_FILE_LINE_LEN];
    	config_mod_info_t *head;

        head = g_mod_info;
	
	index_file = fopen(config_path, "r");
	if (index_file == NULL) {
		err_sys("open index configure file %s error", config_path);
		return -1;
	}

	while (fgets(line_buffer, MAX_CONFIG_FILE_LINE_LEN, index_file)) {
		str_len = strlen(line_buffer);

        if (str_len == MAX_CONFIG_FILE_LINE_LEN - 1 && \
                line_buffer[str_len - 1] != '\n') {
            err_msg("index configure line too long: %s", line_buffer);
            return -1;
        }

		line_buffer[--str_len] = '\0';
		/* for Windows text */
		if (line_buffer [str_len - 1] == 'r') {
			line_buffer[--str_len] = '\0';
		}

		char *line_buffer_head = trim_head(line_buffer, str_len);
		if (line_buffer_head[0] == '#' || line_buffer_head[0] == '\0') {
			continue;
		}
		
		config_mod_info_t *mod_info = (config_mod_info_t*) \
									  (malloc(sizeof(config_mod_info_t)));
		if (mod_info == NULL) {
			err_sys("malloc error");
			continue;
		}

        memset(mod_info, 0, sizeof(config_mod_info_t));
		if (0 != analyse_line_index(line_buffer, mod_info)) {
			free(mod_info);
            fclose(index_file);
            err_msg("index configure file line format error");
            return -1;
		}

        if (check_modinfo(mod_info, g_mod_info) < 0) {
            free(mod_info);
            fclose(index_file);
            err_msg("index configure file content error");
            return -1;
        }

		head->next = mod_info;
	    head = head->next;
	}

    fclose(index_file);
	return 0;
}

/*
int
get_index_ix_by_name(const config_mod_info_t *g_mod_info, \
        char (*index_name)[LEN_32], int *index_ix)
{
    assert(g_mod_info != NULL && index_name != NULL && index_ix != NULL);
   
    int i, j; 
    const config_mod_info_t *p;
    const config_mod_info_item_t  *t;
    int curr_ix = 0, cnt = 0;

    extern app_svr_config_t g_app_svr_config;
    monitor_list_t *monitor_list = &(g_app_svr_config.monitor_list);
    const int  monitor_count = monitor_list->count;
    cf_monitor_t *cf_monitor = monitor_list->head.next;
	


NEXT_INDEX:
    while (index_name[cnt][0] != '\0') {
        p = g_mod_info;
        curr_ix = 0;

        while ( (p = p->next) != NULL) {
            t = &(p->info);

		cf_monitor = monitor_list->head.next; 
		for (j = 0; j < monitor_count; j++) {

			if(0==strcmp(t->mod_name, cf_monitor -> monitor_name)) {
				for (i = 0; i < t->index_cnt; i++) {
					if (strcmp(index_name[cnt], t->index_info[i].index_name) == 0) { 
						index_ix[cnt++] = curr_ix + i;
						 goto NEXT_INDEX;
					}
				}  
				curr_ix += t->index_cnt;
			}

			cf_monitor = cf_monitor -> next;
		}


        }
    }

    return cnt;
}
*/

void
free_index_info_config(config_mod_info_t *g_mod_info)
{
	assert(g_mod_info != NULL);
	
	config_mod_info_t *t;
	while ( (t = g_mod_info -> next) != NULL) {
		g_mod_info -> next = t -> next;
		free(t);
	}
}


static int
analyse_line_xml(char *line, xml_config_t *conf)
{
    assert(line != NULL && conf != NULL);

    char *str, *token, *saveptr;
    xml_format_t *t;

    t = &(conf->item);
    str = line;
    token = strtok_r(str, CONFIG_ITEM_DELIM, &saveptr);
    if (token == NULL) {
        err_msg("missing xml label name");
        return -1;
    }
    strncpy(t->label_name, token, LEN_32);

    str = NULL;
    while ( (token = strtok_r(str, CONFIG_ITEM_DELIM, &saveptr)) != NULL) {
        strncpy(t->attribute_name[t->attribute_cnt++], token, LEN_32);
    }

    return 0;
}


int
analyse_xml_format_config(const char *path, xml_config_t *conf)
{
    assert(path != NULL && conf != NULL);

    FILE *f;
    char line[MAX_CONFIG_FILE_LINE_LEN] = {0};

    if ( (f = fopen(path, "r")) == NULL) {
        err_sys("open file %s error", path);
        return -1;
    }

    while (fgets(line, MAX_CONFIG_FILE_LINE_LEN, f) != NULL) {
        int str_len;
        str_len = strlen(line);

        if (str_len == MAX_CONFIG_FILE_LINE_LEN -1 && \
               line[str_len - 1] != '\n') { 
            err_msg("xml configure file line too long: %s", line);
        }

        line[--str_len] = '\0';
        if (line[str_len-1] == '\r') {
            line[--str_len] = '\0';
        }

        char *line_buffer_head = trim_head(line, str_len);
        if (line_buffer_head[0] == '#' || line_buffer_head[0] == '\0') {
            continue;
        }

        xml_config_t *xml_item = malloc(sizeof(xml_config_t));
        if (xml_item == NULL) {
            err_sys("malloc error");
            continue;
        }

        memset(xml_item, 0, sizeof(xml_config_t));
        if (0 != analyse_line_xml(line_buffer_head, xml_item)) {
            free(xml_item);
        }

        conf->next = xml_item;
        conf = conf->next;
    }
    
    fclose(f);
    return 0;
}


void
free_xml_format_config(xml_config_t *conf)
{
    assert(conf != NULL);

    xml_config_t *t;
    while ( (t = conf->next) != NULL) {
        conf->next = t->next;
        free(t);
    }
}

