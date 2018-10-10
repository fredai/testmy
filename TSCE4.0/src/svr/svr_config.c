
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "svr_config.h"
#include "error_handle.h"
#include "util.h"


static int read_config_from_file(const char *path, \
        app_svr_config_t *app_svr_config, char *flag);
static int parse_svr_config_line(char *config_line, \
        const char *section_str, app_svr_config_t *app_svr_config);

static int parse_svr_config_base_line(char *config_line, \
        const char *section_str, app_svr_config_t *app_svr_config);

static int parse_svr_section_svr (char *config_line, \
        app_svr_config_t *app_svr_config);
static int parse_svr_section_db(char *config_line, \
        app_svr_config_t *app_svr_config);
static int parse_clr_section_interval(char *config_line, \
        app_svr_config_t *app_svr_config);
static int parse_svr_section_log(char *config_line, \
        app_svr_config_t *app_svr_config);
static int parse_svr_section_nodelist (char *config_line, \
        app_svr_config_t *app_svr_config);
static int app_config_add_node(const char *node_name, \
        app_svr_config_t *app_svr_config);
static int parse_svr_section_monitorlist(char *config_line, \
		app_svr_config_t  *app_svr_config);
static int app_config_add_monitor(const char * monitor_name, \
        app_svr_config_t *app_svr_config);

static int 
read_config_from_file(const char *path, app_svr_config_t *app_svr_config, char *flag)
{
    assert(path != NULL && app_svr_config != NULL);

    int ret = -1;
    int str_len = 0;
    FILE * config_file;
    char line_buffer[MAX_CONFIG_FILE_LINE_LEN] = {0};
    char section_str[MAX_SECTION_STR_LEN] = {0};

    /* init */
	if (strcmp(flag, "base")==0)
		bzero(app_svr_config, sizeof(app_svr_config_t));
    app_svr_config->log_level = LOG_INFO;
    app_svr_config->itvl = DEFAULT_APP_INTERVAL;

    /* open file */
    config_file = fopen(path, "r");
    if (config_file == NULL) {
        err_sys("open server configure file error: %s", path);
        return -1;
    }

    /* get line */
    while (fgets(line_buffer, MAX_CONFIG_FILE_LINE_LEN, config_file)) {

        str_len = strlen(line_buffer);
        if (str_len == MAX_CONFIG_FILE_LINE_LEN - 1 && \
                line_buffer[str_len - 1] != '\n') {
            err_msg("server configure line too long: %s", line_buffer);
            return -1;
        }

        line_buffer[str_len - 1] = '\0';
        str_len--; 
        if (line_buffer[str_len - 1] == '\r') { 
            line_buffer[str_len - 1] = 0;
            str_len--;
        }

        char *line_buffer_trim_head = trim_head(line_buffer, str_len);
        if (*line_buffer_trim_head == '#' || \
                *line_buffer_trim_head == 0) {
            continue;
        }

        /* section */
        if (line_buffer_trim_head[0] == '['  && line_buffer_trim_head \
                [strlen(line_buffer_trim_head) - 1 ] == ']') {

            if (strlen(line_buffer_trim_head) >= MAX_SECTION_STR_LEN) {
                err_msg("section string too long \"%s\"", \
                        line_buffer_trim_head);
                return -1;
            }
            strcpy(section_str, line_buffer_trim_head);

        } else {
            /* config item */

            if (section_str[0] == '\0') {
                err_msg("no section for config item \"%s\"", \
                        line_buffer_trim_head);
                return -1;
            }
			if(strcmp(flag, SVR_CONFIG_BASE)==0) {
				ret = parse_svr_config_base_line(line_buffer_trim_head, \
					    section_str, app_svr_config);
			}
			if(strcmp(flag, SVR_CONFIG_TSCE_INDEX)==0) {
				ret = parse_svr_config_line(line_buffer_trim_head, \
					    section_str, app_svr_config);
			}
            if (ret < 0) {
                err_msg("parse config item for %s error", \
                        line_buffer_trim_head);
                return -1;
            }
        }
    }

    fclose(config_file);
    return 0;
}

static int 
parse_svr_config_base_line(char *config_line, const char *section_str, \
        app_svr_config_t *app_svr_config)
{
    assert(config_line != NULL && section_str != NULL && \
            app_svr_config != NULL);

    int ret;

    if (strcmp(section_str, CF_STR_SECTION_SVR) == 0) {
        ret = parse_svr_section_svr(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section svr error");
            return -1;
        }

    } else if (strcmp(section_str, CF_STR_SECTION_DB) == 0) {
        ret = parse_svr_section_db(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section db error");
            return -1;
        }

    } /*else if (strcmp(section_str, CF_STR_SECTION_INTERVAL) == 0) {
        ret = parse_clr_section_interval(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse client configure section interval error");
            return -1;
        }

    } else if (strcmp(section_str, CF_STR_SECTION_NODELIST) == 0) {
        ret = parse_svr_section_nodelist(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section nodelist error");
            return -1;
        }

    } */
	else if (strcmp(section_str, CF_STR_SECTION_LOG) == 0) {
        ret = parse_svr_section_log(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section log error");
            return -1;
        }
    } /*else if (strcmp(section_str, CF_STR_SECTION_MONITORLIST) == 0) {
		ret = parse_svr_section_monitorlist(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section monitorlist error");
            return -1;
        }
	}*/

    return 0;
}


static int 
parse_svr_config_line(char *config_line, const char *section_str, \
        app_svr_config_t *app_svr_config)
{
    assert(config_line != NULL && section_str != NULL && \
            app_svr_config != NULL);

    int ret;

   /* if (strcmp(section_str, CF_STR_SECTION_SVR) == 0) {
        ret = parse_svr_section_svr(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section svr error");
            return -1;
        }

    } else if (strcmp(section_str, CF_STR_SECTION_DB) == 0) {
        ret = parse_svr_section_db(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section db error");
            return -1;
        }

    } else*/ if (strcmp(section_str, CF_STR_SECTION_INTERVAL) == 0) {
        ret = parse_clr_section_interval(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse client configure section interval error");
            return -1;
        }

    } else if (strcmp(section_str, CF_STR_SECTION_NODELIST) == 0) {
        ret = parse_svr_section_nodelist(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section nodelist error");
            return -1;
        }

    }/* else if (strcmp(section_str, CF_STR_SECTION_LOG) == 0) {
        ret = parse_svr_section_log(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section log error");
            return -1;
        }
    } */else if (strcmp(section_str, CF_STR_SECTION_MONITORLIST) == 0) {
		ret = parse_svr_section_monitorlist(config_line, app_svr_config);
        if (ret < 0) {
            err_msg("parse server configure section monitorlist error");
            return -1;
        }
	}

    return 0;
}


static int 
parse_svr_section_svr (char *config_line, \
        app_svr_config_t *app_svr_config) 
{
    assert(config_line != NULL && app_svr_config != NULL);

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, \
            MAX_CONFIG_ITEM_LEN, config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        return -1;
    }

    if (strcmp(config_item, CF_STR_SVR_IP) == 0) {
        if (!ip_str_is_valid(config_value)) {
            err_msg("app svr ip is invalid");
            return -1;
        }
        if (strlen(config_value) > sizeof(app_svr_config-> \
                    svr_config.svr_ip) - 1) {
            err_msg("app svr ip too long");
            return -1;
        }
        strcpy(app_svr_config->svr_config.svr_ip, config_value);

    } else if (strcmp(config_item, CF_STR_SVR_PORT) == 0) {
        unsigned short int svr_port;
        ret = parse_port(config_value, &svr_port);
        if (ret < 0) {
            err_msg("app svr port is invalid");
            return -1;
        }
        app_svr_config->svr_config.svr_port = svr_port;
    }

    return 0;

}


static int 
parse_svr_section_db(char *config_line, \
        app_svr_config_t *app_svr_config)
{
    assert(config_line != NULL && app_svr_config != NULL);

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, MAX_CONFIG_ITEM_LEN, \
            config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        return -1;
    }

    db_config_t *dfg = &(app_svr_config->db_config);

    if (strcmp(config_item, CF_STR_DB_SERVER_IP) == 0) {
        if (!ip_str_is_valid(config_value)) {
            err_msg( "svr db ip is invalid: %s",  config_value );
            return -1;
        }
        if (strlen(config_value) > sizeof(dfg ->db_server_ip) - 1) {
            err_msg("svr dp ib too long: %s", config_value);
            return -1;
        }
        strcpy(dfg->db_server_ip, config_value) ;

    } else if ( strcmp ( config_item, CF_STR_DB_SERVER_PORT ) == 0 ) {
        unsigned short int db_port;
        ret = parse_port(config_value, &db_port);
        if (ret < 0) {
            err_msg("svr db port is invalid: %s", config_value );
            return -1;
        }
        dfg->db_server_port = db_port;

    } else if (strcmp(config_item, CF_STR_DB_NAME) == 0) {
        if (strlen(config_value) > sizeof(dfg->db_name) - 1) {
            err_msg ("db name too long: %s", config_value );
            return -1;
        }
        strcpy ( dfg -> db_name, config_value );

    } else if (strcmp(config_item, CF_STR_DB_USERNAME) == 0) {
        if (strlen(config_value) > sizeof(dfg->db_username) - 1) {
            err_msg("db username too long: %s", config_value);
            return -1;
        }
        strcpy(dfg->db_username, config_value);

    } else if (strcmp(config_item, CF_STR_DB_PASSWORD) == 0) {
        if(strlen(config_value) > sizeof(dfg->db_password) - 1) {
            err_msg("db password too long: %s", config_value );
            return -1;
        }
        strcpy ( dfg -> db_password, config_value );

    } 

    return 0;
}


static int 
parse_clr_section_interval(char *config_line, \
        app_svr_config_t *app_svr_config) 
{
    assert(config_line != NULL && app_svr_config != NULL);

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, \
            MAX_CONFIG_ITEM_LEN, config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        err_msg("parse server section interval item and value error");
        return -1;
    }

    if (strcmp(config_item, CF_STR_APP_INTERVAL) == 0) {
        long int app_interval;
        ret = parse_long_int(config_value, &app_interval);

        if (ret < 0) {
            err_msg("app interval is invalid %s", config_value);
            return -1;
        } else if (app_interval < MIN_APP_INTERVAL) {
            err_msg("invalid app interval < %d", MIN_APP_INTERVAL);
            return -1;
        } else if (app_interval > MAX_APP_INTERVAL) {
            err_msg("invalid app interval > %d", MAX_APP_INTERVAL);
            return -1;
        }
        app_svr_config->itvl = (app_interval_t)app_interval;

    }

    return 0;
}


static int 
parse_svr_section_log(char *config_line, \
        app_svr_config_t *app_svr_config)
{
    assert(config_line != NULL && app_svr_config != NULL);

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, \
            MAX_CONFIG_ITEM_LEN, config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        return -1;
    }

    /* log level */
    if (strcmp(config_item, CF_STR_LOG_LEVEL) == 0) {
        if (strcasecmp(config_value, "DEBUG") == 0) {
            app_svr_config->log_level = LOG_DEBUG;
        } else if (strcasecmp(config_value, "INFO") == 0) {
            app_svr_config->log_level = LOG_INFO;
        } else if (strcasecmp(config_value, "WARN" ) == 0 ) {
            app_svr_config -> log_level = LOG_WARN;
        } else if (strcasecmp(config_value, "ERROR") == 0 ) {
            app_svr_config -> log_level = LOG_ERROR;
        } else if (strcasecmp(config_value, "FATAL") == 0 ) {
            app_svr_config -> log_level = LOG_FATAL;
        }
    }

    return 0;
}

static 
int parse_svr_section_nodelist (char *config_line, \
        app_svr_config_t * app_svr_config) 
{
    assert(config_line != NULL && app_svr_config != NULL);

    int ret;
    char node_name[MAX_NODE_NAME_LEN + 1];
    char * delim = " \t\r\n\f\v";
    char * saveptr;
    char * token;

    char *str = strdup(config_line);
    if (str == NULL) {
        err_sys("error for strdup");
        return -1;
    }

    token = strtok_r(str, delim, &saveptr);
    if (token == NULL) {
        err_msg("empty line");
        return -1;
    }

    /* node name too long */
    if (strlen(token) > MAX_NODE_NAME_LEN) {
        err_msg("node name too long: %s", token);
        free(str);
        return -1;
    }
    strcpy(node_name, token);

    token = strtok_r (NULL, delim, & saveptr );
    if (token != NULL) {
        err_msg("invalid node name: %s", config_line);
        free(str);
        return -1;
    }

    /* node ip */
    ip_t tmp_ip;
    ret = hostname_to_ip(node_name, &tmp_ip);
    if (ret < 0) { 
        err_msg("can not resolve host name: %s", node_name );
        free(str);
        return -1;
    }

    /* add node */
    ret = app_config_add_node(node_name, app_svr_config);
    if (ret < 0) {
        err_msg("app_config_add_node error for %s", node_name);
        free(str);
        return -1;
    }

    free(str);
    return 0;
}

static int
parse_svr_section_monitorlist(char *config_line, app_svr_config_t *app_svr_config)
{
    assert(config_line != NULL && app_svr_config != NULL);
	
    int ret;
    char monitor_name[MAX_NODE_NAME_LEN + 1];
    char * delim = " \t\r\n\f\v";
    char * saveptr;
    char * token;
	
	char *str = strdup(config_line);
	if (str == NULL) {
        err_sys("error for strdup");
        return -1;
    }
	token = strtok_r(str, delim, &saveptr);
    if (token == NULL) {
        err_msg("empty line");
        return -1;
    }
	/* monitor name too long */
    if (strlen(token) > MAX_NODE_NAME_LEN) {
        err_msg("node name too long: %s", token);
        free(str);
        return -1;
	} 
    strcpy(monitor_name, token);
	
	token = strtok_r (NULL, delim, & saveptr );
    if (token != NULL) {
        err_msg("invalid node name: %s", config_line);
        free(str);
        return -1;
    }

	/* add monitor */
    ret = app_config_add_monitor(monitor_name, app_svr_config);
    if (ret < 0) {
        err_msg("app_config_add_node error for %s", monitor_name);
        free(str);
        return -1;
    }
    free(str);
	return 0;
}


static int 
app_config_add_monitor(const char * monitor_name, \
        app_svr_config_t *app_svr_config)
{
    assert(monitor_name != NULL && app_svr_config != NULL );
    
	monitor_list_t *monitor_list = &(app_svr_config->monitor_list);
    cf_monitor_t *monitor = &(monitor_list -> head);
    int *count = &(monitor_list -> count);
    
	while (monitor->next) {
        monitor = monitor->next;
    }

	monitor->next = (cf_monitor_t *) malloc(sizeof(cf_monitor_t));
    if (monitor -> next == NULL) {
        err_sys("malloc error");
        return -1;
    }

    strcpy(monitor->next->monitor_name, monitor_name);
    monitor->next->next = NULL;

    (*count)++;

    return 0;

}

static int 
app_config_add_node(const char * node_name, \
        app_svr_config_t *app_svr_config) 
{
    assert(node_name != NULL && app_svr_config != NULL );
    
    node_list_t *node_list = &(app_svr_config->node_list);
    cf_node_t *node = &(node_list -> head);
    int *count = &(node_list -> count);

    while (node->next) {
        node = node->next;
    }

    node->next = (cf_node_t *) malloc(sizeof(cf_node_t));
    if (node -> next == NULL) {
        err_sys("malloc error");
        return -1;
    }

    strcpy(node->next->node_name, node_name);
    node->next->next = NULL;
    (*count)++;

    return 0;
}



int 
app_config_free_node_list(node_list_t *node_list)
{
    assert(node_list != NULL);

    cf_node_t *node = &(node_list->head);
    int *count = &(node_list->count);

    cf_node_t *del_node = NULL;
    while (node->next != NULL ) {
        del_node = node->next;
        node->next = del_node->next;
        free(del_node);
        (*count)--;
    }

    node_list->head.next = NULL;
    return 0;
}


int 
init_app_svr_base_config(const char *path, app_svr_config_t *app_svr_config, char *flag) 
{
    assert(path != NULL && app_svr_config != NULL);

    int ret;
    ret = read_config_from_file(path, app_svr_config, flag);
    if (ret < 0) {
        err_msg("read configure file error for %s", path);
        return -1;
    }

    if (app_svr_config->svr_config.svr_ip[0] == '\0') {
        err_msg("svr ip is null");
        return -1;
    }
    if (app_svr_config->svr_config.svr_port == 0) {
        err_msg("svr port is null");
        return -1;
    }
 
    db_config_t *t = &(app_svr_config->db_config);
    if (t->db_server_ip[0] == '\0') {
        err_msg("db server ip is null");
        return -1;
    }
    if (t->db_server_port == 0) {
        t->db_server_port = DEFAULT_DB_SERVER_PORT;
    }
    if (t->db_name[0] == '\0') {
        err_msg("db name is null");
        return -1;
    }
    if (t->db_username[0] == '\0' ) {
        err_msg("db username is null");
        return -1;
    }
    if (t->db_password[0] == '\0') {
        err_msg("db password is null");
        return -1;
    }
/*
    if (app_svr_config->node_list.count == 0) {
        err_msg("no node in nodelist");
        return -1;
    }
    if (app_svr_config->monitor_list.count == 0) {
        err_msg("no monitor in monitorlist");
        return -1;
    }
*/
    return 0;
}

int 
init_app_svr_tsce_index_config(const char *path, app_svr_config_t *app_svr_config, char *flag) 
{
    assert(path != NULL && app_svr_config != NULL);

    int ret;
    ret = read_config_from_file(path, app_svr_config, flag);
    if (ret < 0) {
        err_msg("read configure file error for %s", path);
        return -1;
    }

/*    if (app_svr_config->svr_config.svr_ip[0] == '\0') {
        err_msg("svr ip is null");
        return -1;
    }
    if (app_svr_config->svr_config.svr_port == 0) {
        err_msg("svr port is null");
        return -1;
    }
 
    db_config_t *t = &(app_svr_config->db_config);
    if (t->db_server_ip[0] == '\0') {
        err_msg("db server ip is null");
        return -1;
    }
    if (t->db_server_port == 0) {
        t->db_server_port = DEFAULT_DB_SERVER_PORT;
    }
    if (t->db_name[0] == '\0') {
        err_msg("db name is null");
        return -1;
    }
    if (t->db_username[0] == '\0' ) {
        err_msg("db username is null");
        return -1;
    }
    if (t->db_password[0] == '\0') {
        err_msg("db password is null");
        return -1;
    }
*/
    if (app_svr_config->node_list.count == 0) {
        err_msg("no node in nodelist");
        return -1;
    }
    if (app_svr_config->monitor_list.count == 0) {
        err_msg("no monitor in monitorlist");
        return -1;
    }
    return 0;
}

