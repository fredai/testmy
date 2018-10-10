/*
 *
 * alarm.h
 *
 */

#ifndef _ALARM_H
#define _ALARM_H

#define ALARM_TEST_INFO "This is a test message from the cluster alarm monitoring software\n"
#define ALARM_TEST_SUBJECT "Test message from the cluster monitoring alarm software"

#define ALARM_SUBJECT "Cluster-Engine alarm:"

#define IS_END_MSG 1

#define SCRIPT_PATH "../scripts/"

void * send_alarm_msg_to_buffer ( void * arg );

void * send_alarm_msg_from_buffer ( void * arg );

int check_sms ( );

#endif /* _ALARM_H */

/*end of file*/

