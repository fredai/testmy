#include "db_config.h"


static int read_config_from_file(char *path, app_db_config_t *app_db_config);
static int parse_db_config_line(char *config_line, const char *section_str, \
        app_db_config_t *app_db_config);
static int parse_app_section_computing_node (char *config_line, \
		app_db_config_t *app_db_config);
static int parse_app_section_updb(char *config_line, \
        app_db_config_t *app_db_config);
static int parse_app_section_sql(char *config_line, app_db_config_t *app_db_config);



int 
init_app_db_config(char *path, app_db_config_t *app_db_config)
{

    assert(path != NULL && path[0] != 0 && app_db_config != NULL);

    int ret;
    ret = read_config_from_file (path, app_db_config);
    if (ret < 0) {
        err_msg("read configure file error");
		cs_log(LOG_INFO, "read configure file error");
        return -1;
    }
	cs_log(LOG_DEBUG, "updb dsn count: %d", (app_db_config->updb_config).updb_cnt);
	cs_log(LOG_DEBUG, "updb sql count: %d", (app_db_config->updb_sql).sql_cnt);

	for (int i = 0; i < (app_db_config->updb_config).updb_cnt; i++) {
		printf("dsn num[%d]:\n", i+1);
		updb_config_info_t updb_con_info = (app_db_config->updb_config).updb_info[i];
		printf("dsn:%s\n", updb_con_info.db_dsn);
		cs_log(LOG_INFO, "updb dsn: %s", updb_con_info.db_dsn);
		printf("dsn:%s\n", updb_con_info.db_username);
		cs_log(LOG_INFO, "updb username: %s", updb_con_info.db_username);
		printf("dsn:%s\n", updb_con_info.db_password);
		cs_log(LOG_INFO, "updb password: %s", updb_con_info.db_password);
	}

	for (int i = 0; i < (app_db_config->updb_sql).sql_cnt; i++) {
		printf("sql num[%d]:\n", i+1);
		updb_collect_sql_t updb_sql_info = (app_db_config->updb_sql).updb_sql_info[i];
		printf("sql:%s\n", updb_sql_info.updb_sql);
		cs_log(LOG_INFO, "sql: %s", updb_sql_info.updb_sql);
	}

//    if (app_db_config -> svr_config.svr_ip [0] == '\0') {
//        err_msg("svr ip is null");
//        return -1;
//    }
//    if (app_clr_config -> svr_config.svr_port == 0) {
//        err_msg("svr port is null" );
//        return -1;
//    }
//    if (app_clr_config -> itvl == 0) {
//        app_clr_config -> itvl = DEFAULT_APP_INTERVAL;
//    }
    return 0;
}

static int 
read_config_from_file(char *path, app_db_config_t *app_db_config)
{

    assert(path != NULL && app_db_config != NULL && \
            path [ 0 ] != '\0'); 

    int ret = -1;
    int str_len = 0;
    FILE * config_file;
    char line_buffer[MAX_CONFIG_FILE_LINE_LEN];
    char section_str[MAX_SECTION_STR_LEN] = {0};

    /* init */
    bzero(app_db_config, sizeof(app_db_config_t));
	app_db_config->computing_node = DEFAULT_COMPUTING_NODE;

    /* open file */
    config_file = fopen(path, "r");
    if (config_file == NULL) {
        err_sys("open db configure \"%s\" error", path);
        return -1;
    }

    /* get line */
    while (fgets(line_buffer, MAX_CONFIG_FILE_LINE_LEN, config_file)) {
        str_len = strlen(line_buffer);
        if (str_len == MAX_CONFIG_FILE_LINE_LEN -1 && \
               line_buffer[str_len - 1] != '\n') {
            err_msg("db configure file line too long: %s", line_buffer);
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
			if (strcmp(section_str, CF_STR_SECTION_UPDB) == 0) {
				(app_db_config->updb_config).updb_cnt++;
				if((app_db_config->updb_config).updb_cnt > MAX_UPDB_SUPPORT) {
					err_msg("updb count is too more,max updb support count is :%d", MAX_UPDB_SUPPORT);
					return -1;
				}
			}

        } else {
            if (section_str[0] == '\0') {
                err_msg("no section for config item \"%s\"", line_buffer_trim_head);
                return -1;
            }
            /* parse config line */
            ret = parse_db_config_line ( line_buffer_trim_head, section_str, \
                    app_db_config);
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
parse_db_config_line(char *config_line, const char *section_str, \
        app_db_config_t *app_db_config)
{
    assert(config_line != NULL && section_str != NULL && \
            app_db_config != NULL);

    int ret;

    if (strcmp(section_str, CF_STR_SECTION_COMPUTING_NODE) == 0) {
        ret = parse_app_section_computing_node(config_line, app_db_config);
        if (ret < 0) {
            err_msg("parse server configure section [computing_node] error");
            return -1;
        }

    } else if (strcmp(section_str, CF_STR_SECTION_UPDB) == 0) {
        ret = parse_app_section_updb(config_line, app_db_config);
        if (ret < 0) {
            err_msg("parse server configure section db error");
            return -1;
        }

    } else if (strcmp(section_str, CF_STR_SECTION_SQL) == 0) {
        ret = parse_app_section_sql(config_line, app_db_config);
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
parse_app_section_computing_node (char *config_line, \
		app_db_config_t *app_db_config) 
{
	assert(config_line != NULL && app_db_config != NULL);


	int ret;
	unsigned short int tem;
    ret = parse_node(config_line, &tem);
    if (ret < 0) {
        err_msg("app computing node is invalid");
        return -1;
    }
    app_db_config->computing_node = tem;

	return 0;

}

int parse_node ( char * str, unsigned short int * computing_node ) {

	if ( str == NULL || str [ 0 ] == '\0' || computing_node == NULL ) {
		return -1;
	}
	int ret = -1;
	long int compute_value;
	ret = parse_long_int ( str, & compute_value );
	if ( ret < 0 ) {
		return -2;
	}
	if ( compute_value != 0 && compute_value != 1 ) {
		return -3;
	}

	* computing_node = ( unsigned short int ) compute_value;

	return 0;
}

static int 
parse_app_section_updb(char *config_line, \
        app_db_config_t *app_db_config)
{
    assert(config_line != NULL && app_db_config != NULL);

    int ret;
    char config_item[MAX_CONFIG_ITEM_LEN + 1];
    char config_value[MAX_CONFIG_VALUE_LEN + 1];

    ret = get_config_item_and_value(config_line, config_item, MAX_CONFIG_ITEM_LEN, \
            config_value, MAX_CONFIG_VALUE_LEN);
    if (ret < 0) {
        return -1;
    }

    updb_config_t *dfg = &(app_db_config->updb_config);

	if (strcmp(config_item, CF_STR_UPDB_DSN) == 0) {
        if (strlen(config_value) > sizeof((dfg->updb_info[(dfg->updb_cnt)-1]).db_dsn) - 1) {
            err_msg ("updb dsn too long: %s", config_value );
            return -1;
        }
        strcpy ( (dfg->updb_info[(dfg->updb_cnt)-1]).db_dsn, config_value );
		//dfg->updb_cnt++;

    } else if (strcmp(config_item, CF_STR_UPDB_USERNAME) == 0) {
        if (strlen(config_value) > sizeof((dfg->updb_info[(dfg->updb_cnt)-1]).db_username) - 1) {
            err_msg("updb username too long: %s", config_value);
            return -1;
        }
        strcpy((dfg->updb_info[(dfg->updb_cnt)-1]).db_username, config_value);

    } else if (strcmp(config_item, CF_STR_UPDB_PASSWORD) == 0) {
        if(strlen(config_value) > sizeof((dfg->updb_info[(dfg->updb_cnt)-1]).db_password) - 1) {
            err_msg("updb password too long: %s", config_value );
            return -1;
        }
        strcpy ( (dfg->updb_info[(dfg->updb_cnt)-1]).db_password, config_value );

    } 
	//dfg->updb_cnt++;
    return 0;
}

//static int 
//parse_app_section_sql(char *config_line, const char *section_str, \
//		clr_monitor_list_t *monitor_list)
//{
//	
//	int ret = -1;
//	if (strcmp(section_str, CF_STR_SECTION_MONITORLIST) == 0) {
//		ret = parse_clr_section_monitor_list(config_line, monitor_list);
//		if (ret < 0) {
//			err_msg("parse server configure section monitorlist error");
//			return -1;
//		}
//	}	
//	return 0;
//}

static int
parse_app_section_sql(char *config_line, app_db_config_t *app_db_config)
{
	assert(config_line != NULL && app_db_config != NULL);
	
	int ret;
	char szSql[MAX_SQL_LEN + 1];
	char * delim = " \t\r\n\f\v";
	char * saveptr;
	char * token;

	int index = (app_db_config->updb_sql).sql_cnt;
	char *dest = (app_db_config->updb_sql).updb_sql_info[index].updb_sql;
	strncpy(dest, config_line, strlen(config_line));
	dest[strlen(config_line)] = '\0';
//	strncpy(((app_db_config->updb_sql).updb_info[(app_db_config->updb_sql).sql_cnt]), config_line, strlen(config_line));
//	((app_db_config->updb_sql).updb_info[(app_db_config->updb_sql).sql_cnt])[strlen(config_line)] = '\0';
	
    (app_db_config->updb_sql).sql_cnt++;
	if((app_db_config->updb_sql).sql_cnt > MAX_SQL_NUM) {
		err_msg("sql count is too more, the max sql count for each dsn is :%d", MAX_SQL_NUM);
		return -1;
	}
	return 0;
}


