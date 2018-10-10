#include <stdio.h>
#include <stdlib.h>
#include "email_interface.h"
#include "config.h"

#define ALARM_ERRMSG_LEN 1024 
int main(int argc, char *argv[])
{
	int ret;
	email_config_t email_config;
	char error_msg [ ALARM_ERRMSG_LEN ];

	bzero ( &email_config, sizeof ( email_config ) );
	init_config (CONFIG_FILE_PATH, &email_config, error_msg);


	char *check_err_msg = (char *)malloc(ALARM_ERRMSG_LEN *16);	
	if (check_err_msg == NULL) {
		exit ( 0 );
	}
        ret = send_email(&(email_config), \
		ALARM_TEST_SUBJECT, ALARM_TEST_INFO, check_err_msg, ALARM_ERRMSG_LEN * 16);
	if ( ret != 0 ) {
		fprintf ( stderr, "Test email error\n%s\n", check_err_msg );
		free ( check_err_msg );
		exit ( 0 );
	}
	free ( check_err_msg );
	printf("Success\n");

        return 0;
}
