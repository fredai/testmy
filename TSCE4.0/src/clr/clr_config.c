
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "clr_config.h"
#include "clr_define.h"
#include "error_handle.h"
#include "util.h"


static int read_config_from_file(char *path, app_clr_config_t *app_clr_config);
static int parse_clr_config_line(char *config_line, app_clr_config_t *app_clr_config, \
	   	char *section_str);
static int parse_clr_section_svr(char *config_line, app_clr_config_t *app_clr_config);
static int parse_clr_section_interval(char *config_line, \
		app_clr_config_t *app_clr_config);
static int parse_clr_section_log(char * config_line, \
		app_clr_config_t *app_clr_config);


static int 
read_config_from_file(char *path, app_clr_config_t *app_clr_config)
{

    assert(path != NULL && app_clr_config != NULL && \
            path [ 0 ] != '\0'); 

    int ret = -1;
    int str_len = 0;
    FILE * config_file;
    char line_buffer[MAX_CONFIG_FILE_LINE_LEN];
    char section_str[MAX_SECTION_STR_LEN] = {0};

    /* init */
    bzero(app_clr_config, sizeof(app_clr_config_t));
    app_clr_config->itvl = DEFAULT_APP_INTERVAL;
    app_clr_config->log_level = LOG_INFO;

    /* open file */
    config_file = fopen(path, "r");
    if (config_file == NULL) {
        err_sys("open client configure \"%s\" error", path);
        return -1;
    }

    /* get line */
    while (fgets(line_buffer, MAX_CONFIG_FILE_LINE_LEN, config_file)) {
        str_len = strlen(line_buffer);
        if (str_len == MAX_CONFIG_FILE_LINE_LEN -1 && \
               line_buffer[str_len - 1] != '\n') {
            err_msg("client configure file line too long: %s", line_buffer);
            return -1;
        }

        line_buffer[str_len - 1] = 0;
        str_len--; 

        if (line_buffer[str_len - 1] == '\r') { 
            line_buffer[str_len - 1] = 0;
            str_len--; 
        }

        /* too long */
        if (str_len > MAX_CONFIG_FILE_LINE_LEN) { 
            err_msg("line too long");
            return -1;
        }

        char *line_buffer_trim_head = trim_head(line_buffer, str_len);

        /* start with # */
        if (*line_buffer_trim_head == '#') {
            continue;
        }
        if (*line_buffer_trim_head == 0 ) {
            continue;
        }

        /* section string */
        if (line_buffer_trim_head[0] == '[' && line_buffer_trim_head[ \
                strlen(line_buffer_trim_head) - 1] == ']') {

            if (strlen(line_buffer_trim_head) > MAX_SECTION_STR_LEN) {
                err_msg("section string too long : \"%s\"", line_buffer_trim_head);
                return -1;
            }
            strcpy(section_str, line_buffer_trim_head);

        } else {
            if (section_str[0] == '\0') {
                err_msg("no section for config item \"%s\"", line_buffer_trim_head);
                return -1;
            }
            /* parse config line */
            ret = parse_clr_config_line ( line_buffer_trim_head, \
                    app_clr_config, section_str);
            if (ret < 0) {
                err_msg("parse client configure file line error");
                return -1;
            }
        }
    }

    fclose(config_file);

    return 0;
}


static int 
parse_clr_config_line(char *config_line, app_clr_config_t *app_clr_config, \
        char * section_str)
{
    assert(config_line != NULL && app_clr_config != NULL && \
            section_str != NULL && config_line[0] != '\0' && \
            section_str[0] != '\0');

    int ret;

    /* section svr */
    if (strcmp (section_str, CF_STR_SECTION_SVR) == 0) {
        ret = parse_clr_section_svr(config_line, app_clr_config);
        if (ret < 0) {
            err_msg("parse client configure section svr error");
            return -1;
        }
    }
    /* section interval */
    else if (strcmp(section_str, CF_STR_SECTION_INTERVAL) == 0) {
        ret = parse_clr_section_interval(config_line, app_clr_config);
        if (ret < 0) {
            err_msg("parse client configure section interval error");
            return -1;
        }
    }
    /* section log */
    else if ( strcmp ( section_str, CF_STR_SECTION_LOG ) == 0 ) {
        ret = parse_clr_section_log (config_line, app_clr_config);
        if ( ret < 0 ) {
            err_msg("parse client configure section log error");
            return -1;
        }
    }

    return 0;
}


static int 
parse_clr_section_svr(char *config_line, app_clr_config_t *app_clr_config) 
{

    assert(config_line != NULL && app_clr_config != NULL && \
            config_line[0] != '\0');

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, MAX_CONFIG_ITEM_LEN, \
         config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        err_msg("parse client section server item and value error");
        return -1;
    }

    /* ip */
    if (strcmp(config_item, CF_STR_SVR_IP) == 0) {

        if (!ip_str_is_valid(config_value)) {
            err_msg("app svr ip %s is invalid", config_value);
            return -1;
        }
        if (strlen(config_value) > sizeof(app_clr_config -> \
                    svr_config.svr_ip ) - 1) {
            err_msg("app svr ip %s is too long", config_value);
            return -1;
        }
        strcpy (app_clr_config -> svr_config.svr_ip, config_value);
    }

    /* port */
    else if ( strcmp ( config_item, CF_STR_SVR_PORT ) == 0 ) {
        unsigned short int svr_port;
        ret = parse_port(config_value, &svr_port);
        if (ret < 0) {
            err_msg("app svr port %s is invalid", config_value);
            return -1;
        }
        app_clr_config -> svr_config.svr_port = svr_port;
    }

    return 0;

}


static int 
parse_clr_section_interval(char *config_line, app_clr_config_t *app_clr_config) 
{ 
    assert(config_line != NULL && app_clr_config != NULL && \
            config_line [ 0 ] != '\0' );

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, MAX_CONFIG_ITEM_LEN, \
         config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        err_msg("parse client section interval item and value error");
        return -1;
    }

    /* interval */
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

        app_clr_config -> itvl = (app_interval_t)app_interval;
    }

    return 0;
}


static int 
parse_clr_section_log(char *config_line, app_clr_config_t *app_clr_config)
{

    assert(config_line != NULL && app_clr_config != NULL && \
			config_line [0] != '\0');

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];
    ret = get_config_item_and_value(config_line, config_item, MAX_CONFIG_ITEM_LEN, \
         config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        err_msg("parse client section log item and value error");
        return -1;
    }

    /* log */
    if (strcmp(config_item, CF_STR_LOG_LEVEL) == 0) {
        if (strcasecmp(config_value, "DEBUG") == 0 ) {
            app_clr_config -> log_level = LOG_DEBUG;
        } else if (strcasecmp(config_value, "INFO") == 0) {
            app_clr_config -> log_level = LOG_INFO;
        } else if (strcasecmp(config_value, "WARN") == 0) {
            app_clr_config -> log_level = LOG_WARN;
        } else if (strcasecmp( config_value, "ERROR") == 0) {
            app_clr_config -> log_level = LOG_ERROR;
        } else if (strcasecmp(config_value, "FATAL") == 0) {
            app_clr_config -> log_level = LOG_FATAL;
        }
    }

    return 0;

}


int 
init_app_clr_config(char *path, app_clr_config_t *app_clr_config)
{

    assert(path != NULL && path[0] != 0 && app_clr_config != NULL);

    int ret;
    ret = read_config_from_file (path, app_clr_config);
    if (ret < 0) {
        err_msg("read configure file error");
        return -1;
    }
    if (app_clr_config -> svr_config.svr_ip [0] == '\0') {
        err_msg("svr ip is null");
        return -1;
    }
    if (app_clr_config -> svr_config.svr_port == 0) {
        err_msg("svr port is null" );
        return -1;
    }
    if (app_clr_config -> itvl == 0) {
        app_clr_config -> itvl = DEFAULT_APP_INTERVAL;
    }
    return 0;
}


static int 
config_add_monitor(const char * monitor_name, \
        clr_monitor_list_t *monitor_list)
{
    assert(monitor_name != NULL && monitor_list != NULL );
    
    clr_cf_monitor_t *monitor = &(monitor_list -> head);
    int *count = &(monitor_list -> count);
    
	while (monitor->next) {
        monitor = monitor->next;
    }

	monitor->next = (clr_cf_monitor_t *) malloc(sizeof(clr_cf_monitor_t));
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
parse_clr_section_monitor_list(char *config_line, clr_monitor_list_t *monitor_list)
{
    assert(config_line != NULL && monitor_list != NULL);
	
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
    ret = config_add_monitor(monitor_name, monitor_list);
    if (ret < 0) {
        err_msg("app_config_add_node error for %s", monitor_name);
        free(str);
        return -1;
    }
    free(str);
	return 0;
}


static int 
parse_clr_monitor_config(char *config_line, const char *section_str, \
        clr_monitor_list_t *monitor_list)
{
	
    int ret = -1;
    if (strcmp(section_str, CF_STR_SECTION_MONITORLIST) == 0) {
		ret = parse_clr_section_monitor_list(config_line, monitor_list);
        if (ret < 0) {
            err_msg("parse server configure section monitorlist error");
            return -1;
        }
	}	
	return 0;
}
static int 
analyse_monitor_list (const char *path, clr_monitor_list_t *monitor_list)
{
    assert(path != NULL && monitor_list != NULL);

    int ret = -1;
    int str_len = 0;
    FILE * config_file;
    char line_buffer[MAX_CONFIG_FILE_LINE_LEN] = {0};
    char section_str[MAX_SECTION_STR_LEN] = {0};

    /* init */
	bzero(monitor_list, sizeof(clr_monitor_list_t));

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
		
			if (!strcmp(section_str, CF_STR_SECTION_MONITORLIST))
				ret = parse_clr_monitor_config(line_buffer_trim_head, \
					    section_str, monitor_list);
			else
				continue;
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


int
init_monitor_list_config(const char * path, clr_monitor_list_t *monitor_list)
{
    assert(path != NULL && path[0] != 0 && monitor_list != NULL);
    
	int ret;
    ret = analyse_monitor_list (path, monitor_list);
    if (ret < 0) {
        err_msg("read configure file error");
        return -1;
    }
	return 0;	
}


int
monitor_data(config_mod_info_t *g_mod_info, clr_monitor_list_t *monitor_list, config_mod_info_t *monitor_mod_info_head)
{
    config_mod_info_t *head, *head_monitor;
	int j;

	/* monitor */
	const int monitor_count = monitor_list -> count;
	clr_cf_monitor_t *clr_cf_monitor = monitor_list -> head.next;


	head_monitor = monitor_mod_info_head;
	for (j = 0; j<monitor_count; j++) {//monitor
		
		head = g_mod_info -> next;
		while (head != NULL) {// base.conf
	
			if(!strcmp(head->info.mod_name, clr_cf_monitor->monitor_name)) {// if base == monitor
				head_monitor->next = head;
				head_monitor = head_monitor->next;
			}

			head = head -> next;
		}
		clr_cf_monitor = clr_cf_monitor->next;
	}
	head_monitor->next=NULL;
	
/*	while (head_monitor != NULL) {
		printf("*******llllllllll*******=mod_name = %s\n", head_monitor->info.mod_name);
		head_monitor = head_monitor -> next;
	}
*/
	return 0;
}

