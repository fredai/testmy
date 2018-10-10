/*
 *
 * t_get_data.h
 *
 */

#ifndef _T_GET_DATA_H_
#define _T_GET_DATA_H_


#if MAX_DATA_ITEM_NAME_LEN > MAX_DATA_VALUE_LEN
#define MAX_GET_DATA_LEN MAX_DATA_ITEM_NAME_LEN
#else
#define MAX_GET_DATA_LEN MAX_DATA_VALUE_LEN 
#endif


#define MAX_GET_NODES_NUM (MAX_NODES_NUM+1+1)
#define MAX_GET_ITEMS_NUM (MAX_DATA_ITEM_NUM_PER_SCRIPT*MAX_SCRIPT_NUM_PER_NODE+1)





#endif
/*end of file*/
