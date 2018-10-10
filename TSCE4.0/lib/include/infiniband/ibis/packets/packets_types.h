/*                  - Mellanox Confidential and Proprietary -
 *
 *  Copyright (C) 2010-2011, Mellanox Technologies Ltd.  ALL RIGHTS RESERVED.
 *
 *  Except as specifically permitted herein, no portion of the information,
 *  including but not limited to object code and source code, may be reproduced,
 *  modified, distributed, republished or otherwise exploited in any form or by
 *  any means for any purpose without the prior written permission of Mellanox
 *  Technologies Ltd. Use of software subject to the terms and conditions
 *  detailed in the file "LICENSE.txt".
 *
 */

/***
 *** This file was generated at "Fri Jul 22 00:05:18 2011"
 *** by:
 ***    % csp_pack_unpack.pm ../xml_files/packets_st.csp
 ***/

#ifndef packets_structs_H
#define packets_structs_H

#include <stdlib.h>
#include <stdio.h>

//for htonl etc...
#if defined(_WIN32) || defined(_WIN64)
    #include<Winsock2.h>
#else   /* Linux */
    #include<arpa/inet.h>
#endif  /* Windows */


/************************************/
/************************************/
/************************************/
/* Endianess Defines */
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define PLATFORM_MEM "Little Endianess"
    #define _LITTLE_ENDIANESS
#else           /* __BYTE_ORDER == __BIG_ENDIAN */
    #define PLATFORM_MEM "Big Endianess"
    #define _BIG_ENDIANESS
#endif


/* Bit manipulation macros */

/* MASK generate a bit mask S bits width */
//#define MASK32(S)     ( ((u_int32_t) ~0L) >> (32-(S)) )
#define MASK8(S)        ( ((u_int8_t) ~0) >> (8-(S)) )

/* BITS generate a bit mask with bits O+S..O set (assumes 32 / 8 bit integer) */
//#define BITS32(O,S)   ( MASK32(S) << (O) )
#define BITS8(O,S)      ( MASK8(S) << (O) )

/* EXTRACT32/8 macro extracts S bits from (u_int32_t/u_int8_t)W with offset O
 * and shifts them O places to the right (right justifies the field extracted) */
//#define EXTRACT32(W,O,S)  ( ((W)>>(O)) & MASK32(S) )
#define EXTRACT8(W,O,S)     ( ((W)>>(O)) & MASK8(S) )


/* INSERT32/8 macro inserts S bits with offset O from field F into word W (u_int32_t/u_int8_t) */
//#define INSERT32(W,F,O,S)     ((W)= ( ( (W) & (~BITS32(O,S)) ) | (((F) & MASK32(S))<<(O)) ))
#define INSERT8(W,F,O,S)        ((W)= ( ( (W) & (~BITS8(O,S)) ) | (((F) & MASK8(S))<<(O)) ))

//#define INSERTF_32(W,O1,F,O2,S)   (INSERT32(W, EXTRACT32(F, O2, S), O1, S) )
#define INSERTF_8(W,O1,F,O2,S)      (INSERT8(W, EXTRACT8(F, O2, S), O1, S) )


#define PTR_64_OF_BUFF(buf, offset)     ((u_int64_t*)((u_int8_t*)(buf) + (offset)))
#define PTR_32_OF_BUFF(buf, offset)     ((u_int32_t*)((u_int8_t*)(buf) + (offset)))
#define PTR_8_OF_BUFF(buf, offset)      ((u_int8_t*)((u_int8_t*)(buf) + (offset)))
#define FIELD_64_OF_BUFF(buf, offset)   (*PTR_64_OF_BUFF(buf, offset))
#define FIELD_32_OF_BUFF(buf, offset)   (*PTR_32_OF_BUFF(buf, offset))
#define FIELD_8_OF_BUFF(buf, offset)    (*PTR_8_OF_BUFF(buf, offset))
#define DWORD_N(buf, n)                 FIELD_32_OF_BUFF((buf), (n) * 4)
#define BYTE_N(buf, n)                  FIELD_8_OF_BUFF((buf), (n))


#define MIN(a, b)   ((a) < (b) ? (a) : (b))


#define CPU_TO_BE32(x)  htonl(x)
#define BE32_TO_CPU(x)  ntohl(x)
#define CPU_TO_BE16(x)  htons(x)
#define BE16_TO_CPU(x)  ntohs(x)
#ifdef _LITTLE_ENDIANESS
    #define CPU_TO_BE64(x) (((u_int64_t)htonl((u_int32_t)((x) & 0xffffffff)) << 32) | \
                            ((u_int64_t)htonl((u_int32_t)((x >> 32) & 0xffffffff))))

    #define BE64_TO_CPU(x) (((u_int64_t)ntohl((u_int32_t)((x) & 0xffffffff)) << 32) | \
                            ((u_int64_t)ntohl((u_int32_t)((x >> 32) & 0xffffffff))))
#else
    #define CPU_TO_BE64(x) (x)
    #define BE64_TO_CPU(x) (x)
#endif


/* define macros to the architecture of the CPU */
#if defined(__linux) || defined(__FreeBSD__)             /* __linux || __FreeBSD__ */
#   if defined(__i386__)
#       define ARCH_x86
#   elif defined(__x86_64__)
#       define ARCH_x86_64
#   elif defined(__ia64__)
#       define ARCH_ia64
#   elif defined(__PPC64__)
#       define ARCH_ppc64
#   elif defined(__PPC__)
#       define ARCH_ppc
#   else
#       error Unknown CPU architecture using the linux OS
#   endif
#elif defined(_WIN32) || defined(_WIN64)                /* Windows */
#   if defined(_WIN32)
#       define ARCH_x86
#   elif defined(_WIN64)
#       define ARCH_x86_64
#   else
#       error Unknown CPU architecture using the windows OS
#   endif
#else                                                   /* Unknown */
#   error Unknown OS
#endif


/* define macros for print fields */
#define U32D_FMT    "%u"
#define U32H_FMT    "0x%08x"
#define STR_FMT     "%s"
#define U16H_FMT    "0x%04x"
#define U8H_FMT     "0x%02x"
#if defined (ARCH_ia64) || defined(ARCH_x86_64) || defined(ARCH_ppc64)
#    define U64D_FMT    "%lu"
#    define U64H_FMT    "0x%016lx"
#elif defined(ARCH_x86) || defined(ARCH_ppc)
#    define U64D_FMT    "%llu"
#    define U64H_FMT    "0x%016llx"
#else
#   error Unknown architecture
#endif  /* ARCH */


#if !defined(_WIN32) && !defined(_WIN64)    /* Linux */
    #include <sys/types.h>
#else   /* Windows */
/*
    typedef char                    int8_t;
    typedef unsigned char           u_int8_t;
    typedef short int               int16_t;
    typedef unsigned short int      u_int16_t;
    typedef int                     int32_t;
    typedef unsigned int            u_int32_t;
    typedef long long               int64_t;
    typedef unsigned long long      u_int64_t;
*/
    typedef __int8                  int8_t;
    typedef unsigned __int8         u_int8_t;
    typedef __int16                 int16_t;
    typedef unsigned __int16        u_int16_t;
    typedef __int32                 int32_t;
    typedef unsigned __int32        u_int32_t;
    typedef __int64                 int64_t;
    typedef unsigned __int64        u_int64_t;
#endif


/************************************/
/************************************/
/************************************/


/*************************************/
/* Name: PSID_Block_Element
 * Size: 128 bits
 * Description: PSID_Block_Element */

struct PSID_Block_Element {
    u_int8_t	PSID[16];    /* bit_offset:0 */    /* element_size: 8 */    /* PS - ID */
};


/*************************************/
/* Name: GID_Block_Element
 * Size: 128 bits
 * Description: GID_Block_Element */

struct GID_Block_Element {
    u_int32_t	DWord[4];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: uint64bit
 * Size: 64 bits
 * Description: uint64bit */

struct uint64bit {
    u_int32_t	High;    /* bit_offset:0 */    /* element_size: 32 */
    u_int32_t	Low;    /* bit_offset:32 */    /* element_size: 32 */
};


/*************************************/
/* Name: CCTI_Entry_ListElement
 * Size: 16 bits
 * Description: CCTI_Entry_ListElement */

struct CCTI_Entry_ListElement {
    u_int16_t	CCT_Multiplier;    /* bit_offset:0 */    /* element_size: 14 */    /* This is the multiplier used when calculating the injection rate delay */
    u_int8_t	CCT_Shift;    /* bit_offset:14 */    /* element_size: 2 */    /* This is the shift value used when calculating the injection rate delay */
};


/*************************************/
/* Name: CACongestionEntryListElement
 * Size: 64 bits
 * Description: CACongestionEntryListElement */

struct CACongestionEntryListElement {
    u_int8_t	Trigger_Threshold;    /* bit_offset:0 */    /* element_size: 8 */    /* When the CCTI is equal to this value, an event is logged in the CAs cyclic event log. */
    u_int8_t	CCTI_Increase;    /* bit_offset:8 */    /* element_size: 8 */    /* The number to be added to the table Index (CCTI) on the receipt of a BECN. */
    u_int16_t	CCTI_Timer;    /* bit_offset:16 */    /* element_size: 16 */    /* When the timer expires it will be reset to its specified value, and 1 will be decremented from the CCTI. */
    u_int32_t	reserved0;    /* bit_offset:32 */    /* element_size: 24 */
    u_int8_t	CCTI_Min;    /* bit_offset:56 */    /* element_size: 8 */    /* The minimum value permitted for the CCTI. */
};


/*************************************/
/* Name: CongestionLogEventListCAElement
 * Size: 128 bits
 * Description:  */

struct CongestionLogEventListCAElement {
    u_int8_t	reserved0;    /* bit_offset:0 */    /* element_size: 8 */
    u_int32_t	Local_QP_CN_Entry;    /* bit_offset:8 */    /* element_size: 24 */    /* Local QP that reached CN Threshold. Set to zero if port threshold reached. */
    u_int8_t	Service_Type_CN_Entry;    /* bit_offset:32 */    /* element_size: 4 */    /* Service Type of local QP: 0 - RC, 1 - UC, 2 - RD, 3 - UD */
    u_int8_t	SL_CN_Entry;    /* bit_offset:36 */    /* element_size: 4 */    /* Service Level associated with local QP */
    u_int32_t	Remote_QP_Number_CN_Entry;    /* bit_offset:40 */    /* element_size: 24 */    /* Remote QP that is connected to local QP. Set to zero for datagram QPs. */
    u_int16_t	reserved1;    /* bit_offset:64 */    /* element_size: 16 */
    u_int16_t	Remote_LID_CN_Entry;    /* bit_offset:80 */    /* element_size: 16 */    /* LID of remote port that is connected to local QP. Set to zero for datagram service. */
    u_int32_t	Timestamp;    /* bit_offset:96 */    /* element_size: 32 */    /* Time stamp of congestion event */
};


/*************************************/
/* Name: CongestionEntryListSwitchElement
 * Size: 96 bits
 * Description:  */

struct CongestionEntryListSwitchElement {
    u_int16_t	DLID;    /* bit_offset:0 */    /* element_size: 16 */    /* Destination lid of congestion event */
    u_int16_t	SLID;    /* bit_offset:16 */    /* element_size: 16 */    /* Source lid of congestion event */
    u_int32_t	reserved0;    /* bit_offset:32 */    /* element_size: 28 */
    u_int8_t	SL;    /* bit_offset:60 */    /* element_size: 4 */    /* Service level of congestion event */
    u_int32_t	Timestamp;    /* bit_offset:64 */    /* element_size: 32 */    /* Time stamp of congestion event */
};


/*************************************/
/* Name: SWInfo_Block_Element
 * Size: 256 bits
 * Description: SW Info */

struct SWInfo_Block_Element {
    u_int8_t	SubMinor;    /* bit_offset:0 */    /* element_size: 8 */
    u_int8_t	Minor;    /* bit_offset:8 */    /* element_size: 8 */
    u_int8_t	Major;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	reserved0;    /* bit_offset:24 */    /* element_size: 8 */
    u_int32_t	reserved1[7];    /* bit_offset:32 */    /* element_size: 32 */
};


/*************************************/
/* Name: FWInfo_Block_Element
 * Size: 512 bits
 * Description: FW Info */

struct FWInfo_Block_Element {
    u_int8_t	SubMinor;    /* bit_offset:0 */    /* element_size: 8 */
    u_int8_t	Minor;    /* bit_offset:8 */    /* element_size: 8 */
    u_int8_t	Major;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	reserved0;    /* bit_offset:24 */    /* element_size: 8 */
    u_int32_t	BuildID;    /* bit_offset:32 */    /* element_size: 32 */
    u_int16_t	Year;    /* bit_offset:64 */    /* element_size: 16 */
    u_int8_t	Day;    /* bit_offset:80 */    /* element_size: 8 */
    u_int8_t	Month;    /* bit_offset:88 */    /* element_size: 8 */
    u_int16_t	Hour;    /* bit_offset:96 */    /* element_size: 16 */
    u_int16_t	reserved1;    /* bit_offset:112 */    /* element_size: 16 */
    struct PSID_Block_Element	PSID;    /* bit_offset:128 */    /* element_size: 128 */
    u_int32_t	INI_File_Version;    /* bit_offset:256 */    /* element_size: 32 */
    u_int32_t	Extended_Major;    /* bit_offset:288 */    /* element_size: 32 */
    u_int32_t	Extended_Minor;    /* bit_offset:320 */    /* element_size: 32 */
    u_int32_t	Extended_SubMinor;    /* bit_offset:352 */    /* element_size: 32 */
    u_int32_t	reserved2[4];    /* bit_offset:384 */    /* element_size: 32 */
};


/*************************************/
/* Name: HWInfo_Block_Element
 * Size: 256 bits
 * Description: HW Info */

struct HWInfo_Block_Element {
    u_int16_t	DeviceID;    /* bit_offset:0 */    /* element_size: 16 */
    u_int16_t	DeviceHWRevision;    /* bit_offset:16 */    /* element_size: 16 */
    u_int32_t	reserved0[6];    /* bit_offset:32 */    /* element_size: 32 */    /* Reserved */
    u_int32_t	UpTime;    /* bit_offset:224 */    /* element_size: 32 */    /* Time (in sec) since last reset */
};


/*************************************/
/* Name: CC_KeyViolation
 * Size: 448 bits
 * Description:  */

struct CC_KeyViolation {
    u_int8_t	reserved0;    /* bit_offset:0 */    /* element_size: 8 */
    u_int8_t	Method;    /* bit_offset:8 */    /* element_size: 8 */
    u_int16_t	SourceLID;    /* bit_offset:16 */    /* element_size: 16 */
    u_int16_t	reserved1;    /* bit_offset:32 */    /* element_size: 16 */
    u_int16_t	ArrtibuteID;    /* bit_offset:48 */    /* element_size: 16 */
    u_int32_t	AttributeModifier;    /* bit_offset:64 */    /* element_size: 32 */
    u_int8_t	reserved2;    /* bit_offset:96 */    /* element_size: 8 */
    u_int32_t	QP;    /* bit_offset:104 */    /* element_size: 24 */
    u_int64_t	CC_Key;    /* bit_offset:128 */    /* element_size: 64 */    /* Congestion Control key, is used to validate the source of Congestion Control Mads. */
    struct GID_Block_Element	SourceGID;    /* bit_offset:192 */    /* element_size: 128 */
    u_int8_t	Padding[16];    /* bit_offset:320 */    /* element_size: 8 */
};


/*************************************/
/* Name: CCTI_Entry_List
 * Size: 1024 bits
 * Description:  */

struct CCTI_Entry_List {
    struct CCTI_Entry_ListElement	CCTI_Entry_ListElement[64];    /* bit_offset:0 */    /* element_size: 16 */    /* List of up to 64 CongestionControlTableEntries. see table 521 p1686 for data format */
};


/*************************************/
/* Name: CACongestionEntryList
 * Size: 1024 bits
 * Description:  */

struct CACongestionEntryList {
    struct CACongestionEntryListElement	CACongestionEntryListElement[16];    /* bit_offset:0 */    /* element_size: 64 */    /* List of sixteen CACongestionEntries, one per service level. (See Table 519 on Page 1684.) */
};


/*************************************/
/* Name: SwitchPortCongestionSettingElement
 * Size: 32 bits
 * Description: SwitchPortCongestionSettingElement */

struct SwitchPortCongestionSettingElement {
    u_int16_t	Cong_Parm;    /* bit_offset:0 */    /* element_size: 16 */    /* When control_type=0 this field contains the port marking_rate. When control_type=1 this field is reserved */
    u_int8_t	Packet_Size;    /* bit_offset:16 */    /* element_size: 8 */    /* When control_type=0 this field contains the minimum size of packets that may be marked with a FECN. When control_type=1 this field is reserved. */
    u_int8_t	Threshold;    /* bit_offset:24 */    /* element_size: 4 */    /* When control_type=0 this field contains the congestion threshold value (Threshold) for this port. When Control Type is 1, contains the credit starvation threshold (CS_Threshold) value for this port. */
    u_int8_t	reserved0;    /* bit_offset:28 */    /* element_size: 2 */
    u_int8_t	Control_Type;    /* bit_offset:30 */    /* element_size: 1 */    /* Indicates which type of attribute is being set: 0b = Congestion Control parameters are being set. 1b = Credit Starvation parameters are being set. */
    u_int8_t	Valid;    /* bit_offset:31 */    /* element_size: 1 */    /* When set to 1, indicates this switch port congestion setting element is valid. */
};


/*************************************/
/* Name: UINT256
 * Size: 256 bits
 * Description: UINT256 */

struct UINT256 {
    u_int32_t	Mask_255_224;    /* bit_offset:0 */    /* element_size: 32 */
    u_int32_t	Mask_223_192;    /* bit_offset:32 */    /* element_size: 32 */
    u_int32_t	Mask_191_160;    /* bit_offset:64 */    /* element_size: 32 */
    u_int32_t	Mask_159_128;    /* bit_offset:96 */    /* element_size: 32 */
    u_int32_t	Mask_127_96;    /* bit_offset:128 */    /* element_size: 32 */
    u_int32_t	Mask_95_64;    /* bit_offset:160 */    /* element_size: 32 */
    u_int32_t	Mask_63_32;    /* bit_offset:192 */    /* element_size: 32 */
    u_int32_t	Mask_31_0;    /* bit_offset:224 */    /* element_size: 32 */    /* bit0: port 0, bit1: port1... bit254: port 254, bit255: reserved */
};


/*************************************/
/* Name: CC_SwitchCongestionSetting_Control_Map
 * Size: 32 bits
 * Description:  */

struct CC_SwitchCongestionSetting_Control_Map {
    u_int8_t	Marking_RateIsValid;    /* bit_offset:0 */    /* element_size: 1 */
    u_int8_t	CS_ThresholdAndCS_ReturnDelayIsValid;    /* bit_offset:1 */    /* element_size: 1 */
    u_int8_t	ThresholdAndPacket_SizeIsValid;    /* bit_offset:2 */    /* element_size: 1 */
    u_int8_t	Credit_MaskIsValid;    /* bit_offset:3 */    /* element_size: 1 */
    u_int8_t	Victim_MaskIsValid;    /* bit_offset:4 */    /* element_size: 1 */
    u_int32_t	reserved0;    /* bit_offset:5 */    /* element_size: 27 */
};


/*************************************/
/* Name: CongestionLogEventListCA
 * Size: 1664 bits
 * Description: array of at most 13 recent events */

struct CongestionLogEventListCA {
    struct CongestionLogEventListCAElement	CongestionLogEventListCAElement[13];    /* bit_offset:0 */    /* element_size: 128 */
};


/*************************************/
/* Name: CongestionEntryListSwitch
 * Size: 1440 bits
 * Description: array of at most 15 recent events */

struct CongestionEntryListSwitch {
    struct CongestionEntryListSwitchElement	CongestionEntryListSwitchElement[15];    /* bit_offset:0 */    /* element_size: 96 */
};


/*************************************/
/* Name: PortCountersExtended_Mask_Block_Element
 * Size: 16 bits
 * Description:  */

struct PortCountersExtended_Mask_Block_Element {
    u_int8_t	SetPortXmitData;    /* bit_offset:0 */    /* element_size: 1 */
    u_int8_t	SetPortRcvData;    /* bit_offset:1 */    /* element_size: 1 */
    u_int8_t	SetPortXmitPkts;    /* bit_offset:2 */    /* element_size: 1 */
    u_int8_t	SetPortRcvPkts;    /* bit_offset:3 */    /* element_size: 1 */
    u_int8_t	SetPortUnicastXmitPkts;    /* bit_offset:4 */    /* element_size: 1 */
    u_int8_t	SetPortUnicastRcvPkts;    /* bit_offset:5 */    /* element_size: 1 */
    u_int8_t	SetPortMulticastXmitPkts;    /* bit_offset:6 */    /* element_size: 1 */
    u_int8_t	SetPortMulticastRcvPkts;    /* bit_offset:7 */    /* element_size: 1 */
    u_int8_t	reserved0;    /* bit_offset:8 */    /* element_size: 8 */
};


/*************************************/
/* Name: PortCounters_Mask2_Block_Element
 * Size: 8 bits
 * Description:  */

struct PortCounters_Mask2_Block_Element {
    u_int8_t	SetPortXmitWait;    /* bit_offset:0 */    /* element_size: 1 */
    u_int8_t	reserved0;    /* bit_offset:1 */    /* element_size: 7 */
};


/*************************************/
/* Name: PortCounters_Mask_Block_Element
 * Size: 16 bits
 * Description:  */

struct PortCounters_Mask_Block_Element {
    u_int8_t	SetSymbolErrorCounter;    /* bit_offset:0 */    /* element_size: 1 */
    u_int8_t	SetLinkErrorRecoveryCounter;    /* bit_offset:1 */    /* element_size: 1 */
    u_int8_t	SetLinkDownedCounter;    /* bit_offset:2 */    /* element_size: 1 */
    u_int8_t	SetPortRcvErrors;    /* bit_offset:3 */    /* element_size: 1 */
    u_int8_t	SetPortRcvRemotePhysicalErrors;    /* bit_offset:4 */    /* element_size: 1 */
    u_int8_t	SetPortRcvSwitchRelayErrors;    /* bit_offset:5 */    /* element_size: 1 */
    u_int8_t	SetPortXmitDiscards;    /* bit_offset:6 */    /* element_size: 1 */
    u_int8_t	SetPortXmitConstraintErrors;    /* bit_offset:7 */    /* element_size: 1 */
    u_int8_t	SetPortRcvConstraintErrors;    /* bit_offset:8 */    /* element_size: 1 */
    u_int8_t	SetLocalLinkIntegrityErrors;    /* bit_offset:9 */    /* element_size: 1 */
    u_int8_t	SetExcessiveBufferOverrunErrors;    /* bit_offset:10 */    /* element_size: 1 */
    u_int8_t	SetVL15Dropped;    /* bit_offset:11 */    /* element_size: 1 */
    u_int8_t	SetPortXmitData;    /* bit_offset:12 */    /* element_size: 1 */
    u_int8_t	SetPortRcvData;    /* bit_offset:13 */    /* element_size: 1 */
    u_int8_t	SetPortXmitPkts;    /* bit_offset:14 */    /* element_size: 1 */
    u_int8_t	SetPortRcvPkts;    /* bit_offset:15 */    /* element_size: 1 */
};


/*************************************/
/* Name: LID_Port_Block_Element
 * Size: 32 bits
 * Description:  */

struct LID_Port_Block_Element {
    u_int16_t	LID;    /* bit_offset:0 */    /* element_size: 16 */
    u_int8_t	Valid;    /* bit_offset:16 */    /* element_size: 1 */
    u_int8_t	LMC;    /* bit_offset:17 */    /* element_size: 3 */
    u_int8_t	reserved0;    /* bit_offset:20 */    /* element_size: 4 */
    u_int8_t	Port;    /* bit_offset:24 */    /* element_size: 8 */
};


/*************************************/
/* Name: VL_Weight_Block_Element
 * Size: 16 bits
 * Description: VL Weight */

struct VL_Weight_Block_Element {
    u_int8_t	reserved0;    /* bit_offset:0 */    /* element_size: 4 */
    u_int8_t	VL;    /* bit_offset:4 */    /* element_size: 4 */
    u_int8_t	Weight;    /* bit_offset:8 */    /* element_size: 8 */
};


/*************************************/
/* Name: P_Key_Block_Element
 * Size: 16 bits
 * Description: Partition Key */

struct P_Key_Block_Element {
    u_int16_t	P_KeyBase;    /* bit_offset:0 */    /* element_size: 15 */
    u_int8_t	Membership_Type;    /* bit_offset:15 */    /* element_size: 1 */
};


/*************************************/
/* Name: GUID_Block_Element
 * Size: 512 bits
 * Description: GUID_Block_Element */

struct GUID_Block_Element {
    u_int64_t	GUID[8];    /* bit_offset:0 */    /* element_size: 64 */
};


/*************************************/
/* Name: TID_Block_Element
 * Size: 64 bits
 * Description: TID_Block_Element */

struct TID_Block_Element {
    u_int32_t	High;    /* bit_offset:0 */    /* element_size: 32 */
    u_int32_t	Low;    /* bit_offset:32 */    /* element_size: 32 */
};


/*************************************/
/* Name: VendorSpec_GeneralInfo
 * Size: 1024 bits
 * Description:  */

struct VendorSpec_GeneralInfo {
    struct HWInfo_Block_Element	HWInfo;    /* bit_offset:0 */    /* element_size: 256 */
    struct FWInfo_Block_Element	FWInfo;    /* bit_offset:256 */    /* element_size: 512 */
    struct SWInfo_Block_Element	SWInfo;    /* bit_offset:768 */    /* element_size: 256 */
};


/*************************************/
/* Name: CC_Notice
 * Size: 640 bits
 * Description:  */

struct CC_Notice {
    u_int32_t	ProducerType_VendorID;    /* bit_offset:0 */    /* element_size: 24 */
    u_int8_t	Type;    /* bit_offset:24 */    /* element_size: 7 */
    u_int8_t	IsGeneric;    /* bit_offset:31 */    /* element_size: 1 */
    u_int16_t	IssuerLID;    /* bit_offset:32 */    /* element_size: 16 */
    u_int16_t	TrapNumber;    /* bit_offset:48 */    /* element_size: 16 */
    struct CC_KeyViolation	CC_KeyViolation;    /* bit_offset:64 */    /* element_size: 448 */
    struct GID_Block_Element	IssuerGID;    /* bit_offset:512 */    /* element_size: 128 */
};


/*************************************/
/* Name: CC_TimeStamp
 * Size: 32 bits
 * Description:  */

struct CC_TimeStamp {
    u_int32_t	TimeStamp;    /* bit_offset:0 */    /* element_size: 32 */    /* Free running clock that provieds relative time infomation for a device. Time is kept in 1.024 usec units. */
};


/*************************************/
/* Name: CC_CongestionControlTable
 * Size: 1056 bits
 * Description:  */

struct CC_CongestionControlTable {
    u_int16_t	reserved0;    /* bit_offset:0 */    /* element_size: 16 */
    u_int16_t	CCTI_Limit;    /* bit_offset:16 */    /* element_size: 16 */    /* Maximum valid CCTI for this table. */
    struct CCTI_Entry_List	CCTI_Entry_List;    /* bit_offset:32 */    /* element_size: 1024 */    /* List of up to 64 CongestionControlTableEntries. */
};


/*************************************/
/* Name: CC_CACongestionSetting
 * Size: 1056 bits
 * Description:  */

struct CC_CACongestionSetting {
    u_int16_t	Control_Map;    /* bit_offset:0 */    /* element_size: 16 */    /* List of sixteen CACongestionEntries, one per service level. */
    u_int16_t	Port_Control;    /* bit_offset:16 */    /* element_size: 16 */    /* Congestion attributes for this port: bit0 = 0: QP based CC; bit0 = 1: SL/Port based CC; All other bits are reserved. */
    struct CACongestionEntryList	CACongestionEntryList;    /* bit_offset:32 */    /* element_size: 1024 */
};


/*************************************/
/* Name: CC_SwitchPortCongestionSetting
 * Size: 1024 bits
 * Description:  */

struct CC_SwitchPortCongestionSetting {
    struct SwitchPortCongestionSettingElement	SwitchPortCongestionSettingElement[32];    /* bit_offset:0 */    /* element_size: 32 */    /* see table 516 and table 517 p1681 for data format */
};


/*************************************/
/* Name: CC_SwitchCongestionSetting
 * Size: 608 bits
 * Description:  */

struct CC_SwitchCongestionSetting {
    struct CC_SwitchCongestionSetting_Control_Map	Control_Map;    /* bit_offset:0 */    /* element_size: 32 */
    struct UINT256	Victim_Mask;    /* bit_offset:32 */    /* element_size: 256 */    /* If the bit is set to 1, then the port corresponding to that bit shall mark packets that enconter congestion with a FECN, wheter they are the source or victim of congestion. */
    struct UINT256	Credit_Mask;    /* bit_offset:288 */    /* element_size: 256 */    /* If the bit is set to 1, then the port corresponding to that bit shall apply Credit Starvation. */
    u_int16_t	reserved0;    /* bit_offset:544 */    /* element_size: 12 */
    u_int8_t	CS_Threshold;    /* bit_offset:556 */    /* element_size: 4 */
    u_int8_t	Packet_Size;    /* bit_offset:560 */    /* element_size: 8 */
    u_int8_t	reserved1;    /* bit_offset:568 */    /* element_size: 4 */
    u_int8_t	Threshold;    /* bit_offset:572 */    /* element_size: 4 */
    u_int16_t	Marking_Rate;    /* bit_offset:576 */    /* element_size: 16 */
    u_int16_t	CS_ReturnDelay;    /* bit_offset:592 */    /* element_size: 16 */
};


/*************************************/
/* Name: CC_CongestionLogCA
 * Size: 1760 bits
 * Description:  */

struct CC_CongestionLogCA {
    u_int16_t	ThresholdEventCounter;    /* bit_offset:0 */    /* element_size: 16 */    /* Number of CongestionLogEvents since log last sent */
    u_int8_t	CongestionFlags;    /* bit_offset:16 */    /* element_size: 8 */    /* Bit0 - 1: - CC_Key lease period timer active, Bit0 - 0: lease timer incative */
    u_int8_t	LogType;    /* bit_offset:24 */    /* element_size: 8 */    /* 0x1 - switch , 0x2 - CA */
    u_int16_t	reserved0;    /* bit_offset:32 */    /* element_size: 16 */
    u_int16_t	ThresholdCongestionEventMap;    /* bit_offset:48 */    /* element_size: 16 */    /* List of sixteen entries, one per service level. */
    u_int32_t	CurrentTimeStamp;    /* bit_offset:64 */    /* element_size: 32 */    /* 2^CurrentTimeStamp*1.24uSec = time stamp, wrap around ~1.22hr */
    struct CongestionLogEventListCA	CongestionLogEvent;    /* bit_offset:96 */    /* element_size: 1664 */    /* for data format see spec 1.2.1 vol 1 p.1678 */
};


/*************************************/
/* Name: CC_CongestionLogSwitch
 * Size: 1760 bits
 * Description:  */

struct CC_CongestionLogSwitch {
    u_int16_t	LogEventsCounter;    /* bit_offset:0 */    /* element_size: 16 */    /* Number of CongestionLogEvents since log last sent */
    u_int8_t	CongestionFlags;    /* bit_offset:16 */    /* element_size: 8 */    /* Bit0 - 1: CC_Key lease period timer active, Bit0 - 0: lease timer incative */
    u_int8_t	LogType;    /* bit_offset:24 */    /* element_size: 8 */    /* 0x1 - switch , 0x2 - CA */
    u_int32_t	CurrentTimeStamp;    /* bit_offset:32 */    /* element_size: 32 */    /* 2^CurrentTimeStamp*1.24uSec = time stamp, wrap around ~1.22hr */
    struct UINT256	PortMap;    /* bit_offset:64 */    /* element_size: 256 */    /* If a bit is set then the corresponding port has marked packets with FECN, bit 0 - reserve, bit 1 - port 1 */
    struct CongestionEntryListSwitch	CongestionEntryList;    /* bit_offset:320 */    /* element_size: 1440 */    /* for data format see spec 1.2.1 vol 1 p.1678 */
};


/*************************************/
/* Name: CC_CongestionKeyInfo
 * Size: 128 bits
 * Description:  */

struct CC_CongestionKeyInfo {
    u_int64_t	CC_Key;    /* bit_offset:0 */    /* element_size: 64 */    /* Congestion Control key, is used to validate the source of Congestion Control Mads. A value of 0 means that no CC_Key checks is ever performed by the CCMgrA. */
    u_int16_t	CC_KeyLeasePeriod;    /* bit_offset:64 */    /* element_size: 16 */    /* How long the CC Key protect bit is to remain non-zero. */
    u_int16_t	reserved0;    /* bit_offset:80 */    /* element_size: 15 */
    u_int8_t	CC_KeyProtectBit;    /* bit_offset:95 */    /* element_size: 1 */
    u_int16_t	reserved1;    /* bit_offset:96 */    /* element_size: 16 */
    u_int16_t	CC_KeyViolations;    /* bit_offset:112 */    /* element_size: 16 */    /* Number of received MADs that violated CC Key. */
};


/*************************************/
/* Name: CC_CongestionInfo
 * Size: 32 bits
 * Description:  */

struct CC_CongestionInfo {
    u_int8_t	reserved0;    /* bit_offset:0 */    /* element_size: 8 */
    u_int8_t	ControlTableCap;    /* bit_offset:8 */    /* element_size: 8 */    /* CA only - Number of 64 entry block in CCT */
    u_int16_t	CongestionInfo;    /* bit_offset:16 */    /* element_size: 16 */    /* Bit0 - 1: Switch support Creadit Starvation, Bit0 - 0: Switch doesn't support CS */
};


/*************************************/
/* Name: PM_PortSamplesResult
 * Size: 1536 bits
 * Description: 0x40 */

struct PM_PortSamplesResult {
    u_int8_t	SampleStatus;    /* bit_offset:0 */    /* element_size: 2 */    /* Read-only copy of PortSamplesControl:SampleStatus. */
    u_int16_t	reserved0;    /* bit_offset:2 */    /* element_size: 14 */
    u_int16_t	Tag;    /* bit_offset:16 */    /* element_size: 16 */    /* Read-only copy of PortSamplesControl:Tag. */
    u_int32_t	Counter0;    /* bit_offset:32 */    /* element_size: 32 */
    u_int32_t	Counter1;    /* bit_offset:64 */    /* element_size: 32 */
    u_int32_t	Counter2;    /* bit_offset:96 */    /* element_size: 32 */
    u_int32_t	Counter3;    /* bit_offset:128 */    /* element_size: 32 */
    u_int32_t	Counter4;    /* bit_offset:160 */    /* element_size: 32 */
    u_int32_t	Counter5;    /* bit_offset:192 */    /* element_size: 32 */
    u_int32_t	Counter6;    /* bit_offset:224 */    /* element_size: 32 */
    u_int32_t	Counter7;    /* bit_offset:256 */    /* element_size: 32 */
    u_int32_t	Counter8;    /* bit_offset:288 */    /* element_size: 32 */
    u_int32_t	Counter9;    /* bit_offset:320 */    /* element_size: 32 */
    u_int32_t	Counter10;    /* bit_offset:352 */    /* element_size: 32 */
    u_int32_t	Counter11;    /* bit_offset:384 */    /* element_size: 32 */
    u_int32_t	Counter12;    /* bit_offset:416 */    /* element_size: 32 */
    u_int32_t	Counter13;    /* bit_offset:448 */    /* element_size: 32 */
    u_int32_t	Counter14;    /* bit_offset:480 */    /* element_size: 32 */
    u_int32_t	reserved1;    /* bit_offset:512 */    /* element_size: 1024 */
};


/*************************************/
/* Name: PM_PortRcvErrorDetails
 * Size: 512 bits
 * Description:  */

struct PM_PortRcvErrorDetails {
    u_int8_t	PortLocalPhysicalErrors_Sel;    /* bit_offset:0 */    /* element_size: 1 */
    u_int8_t	PortMalformedPacketErrors_Sel;    /* bit_offset:1 */    /* element_size: 1 */
    u_int8_t	PortBufferOverrunErrors_Sel;    /* bit_offset:2 */    /* element_size: 1 */
    u_int8_t	PortDLIDMappingErrors_Sel;    /* bit_offset:3 */    /* element_size: 1 */
    u_int8_t	PortVLMappingErrors_Sel;    /* bit_offset:4 */    /* element_size: 1 */
    u_int8_t	PortLoopingErrors_Sel;    /* bit_offset:5 */    /* element_size: 1 */
    u_int16_t	reserved0;    /* bit_offset:6 */    /* element_size: 10 */
    u_int8_t	PortSelect;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	reserved1;    /* bit_offset:24 */    /* element_size: 8 */    /* Reserved */
    u_int16_t	PortMalformedPacketErrors;    /* bit_offset:32 */    /* element_size: 16 */
    u_int16_t	PortLocalPhysicalErrors;    /* bit_offset:48 */    /* element_size: 16 */
    u_int16_t	PortDLIDMappingErrors;    /* bit_offset:64 */    /* element_size: 16 */
    u_int16_t	PortBufferOverrunErrors;    /* bit_offset:80 */    /* element_size: 16 */
    u_int16_t	PortLoopingErrors;    /* bit_offset:96 */    /* element_size: 16 */
    u_int16_t	PortVLMappingErrors;    /* bit_offset:112 */    /* element_size: 16 */
    u_int32_t	reserved2;    /* bit_offset:128 */    /* element_size: 384 */
};


/*************************************/
/* Name: PM_PortXmitDiscardDetails
 * Size: 512 bits
 * Description:  */

struct PM_PortXmitDiscardDetails {
    u_int8_t	PortInactiveDiscards_Sel;    /* bit_offset:0 */    /* element_size: 1 */
    u_int8_t	PortNeighborMTUDiscards_Sel;    /* bit_offset:1 */    /* element_size: 1 */
    u_int8_t	PortSwLifetimeLimitDiscards_Sel;    /* bit_offset:2 */    /* element_size: 1 */
    u_int8_t	PortSwHOQLifetimeLimitDiscards_Sel;    /* bit_offset:3 */    /* element_size: 1 */
    u_int16_t	reserved0;    /* bit_offset:4 */    /* element_size: 12 */
    u_int8_t	PortSelect;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	reserved1;    /* bit_offset:24 */    /* element_size: 8 */    /* Reserved */
    u_int16_t	PortNeighborMTUDiscards;    /* bit_offset:32 */    /* element_size: 16 */
    u_int16_t	PortInactiveDiscards;    /* bit_offset:48 */    /* element_size: 16 */
    u_int16_t	PortSwHOQLifetimeLimitDiscards;    /* bit_offset:64 */    /* element_size: 16 */
    u_int16_t	PortSwLifetimeLimitDiscards;    /* bit_offset:80 */    /* element_size: 16 */
    u_int32_t	reserved2;    /* bit_offset:96 */    /* element_size: 416 */
};


/*************************************/
/* Name: PM_PortCountersExtended
 * Size: 576 bits
 * Description:  */

struct PM_PortCountersExtended {
    struct PortCountersExtended_Mask_Block_Element	CounterSelect;    /* bit_offset:0 */    /* element_size: 16 */
    u_int8_t	PortSelect;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	reserved0;    /* bit_offset:24 */    /* element_size: 8 */
    u_int32_t	reserved1;    /* bit_offset:32 */    /* element_size: 32 */
    u_int64_t	PortXmitData;    /* bit_offset:64 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of data octets, divided by 4, transmitted on all VLs from the port. This includes all octets between (and not including) the start of packet delimiter and the VCRC, and may include packets containing errors. */
    u_int64_t	PortRcvData;    /* bit_offset:128 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of data octets, divided by 4, received on all VLs at the port. This includes all octets between (and not including) the start of packet delimiter and the VCRC, and may include packets containing errors. */
    u_int64_t	PortXmitPkts;    /* bit_offset:192 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of packets transmitted on all VLs from the port. */
    u_int64_t	PortRcvPkts;    /* bit_offset:256 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of packets, including packets containing errors */
    u_int64_t	PortUniCastXmitPkts;    /* bit_offset:320 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of packets, including packets containing errors */
    u_int64_t	PortUniCastRcvPkts;    /* bit_offset:384 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of packets, including packets containing errors */
    u_int64_t	PortMultiCastXmitPkts;    /* bit_offset:448 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of packets, including packets containing errors */
    u_int64_t	PortMultiCastRcvPkts;    /* bit_offset:512 */    /* element_size: 64 */    /* Optional; shall be zero if not implemented. Total number of packets, including packets containing errors */
};


/*************************************/
/* Name: PM_PortCounters
 * Size: 352 bits
 * Description:  */

struct PM_PortCounters {
    struct PortCounters_Mask_Block_Element	CounterSelect;    /* bit_offset:0 */    /* element_size: 16 */
    u_int8_t	PortSelect;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	reserved0;    /* bit_offset:24 */    /* element_size: 8 */
    u_int8_t	LinkDownedCounter;    /* bit_offset:32 */    /* element_size: 8 */
    u_int8_t	LinkErrorRecoveryCounter;    /* bit_offset:40 */    /* element_size: 8 */
    u_int16_t	SymbolErrorCounter;    /* bit_offset:48 */    /* element_size: 16 */
    u_int16_t	PortRcvRemotePhysicalErrors;    /* bit_offset:64 */    /* element_size: 16 */
    u_int16_t	PortRcvErrors;    /* bit_offset:80 */    /* element_size: 16 */
    u_int16_t	PortXmitDiscards;    /* bit_offset:96 */    /* element_size: 16 */
    u_int16_t	PortRcvSwitchRelayErrors;    /* bit_offset:112 */    /* element_size: 16 */
    u_int8_t	ExcessiveBufferOverrunErrors;    /* bit_offset:128 */    /* element_size: 4 */
    u_int8_t	LocalLinkIntegrityErrors;    /* bit_offset:132 */    /* element_size: 4 */
    struct PortCounters_Mask2_Block_Element	CounterSelect2;    /* bit_offset:136 */    /* element_size: 8 */
    u_int8_t	PortRcvConstraintErrors;    /* bit_offset:144 */    /* element_size: 8 */
    u_int8_t	PortXmitConstraintErrors;    /* bit_offset:152 */    /* element_size: 8 */
    u_int16_t	VL15Dropped;    /* bit_offset:160 */    /* element_size: 16 */
    u_int16_t	reserved1;    /* bit_offset:176 */    /* element_size: 16 */
    u_int32_t	PortXmitData;    /* bit_offset:192 */    /* element_size: 32 */
    u_int32_t	PortRcvData;    /* bit_offset:224 */    /* element_size: 32 */
    u_int32_t	PortXmitPkts;    /* bit_offset:256 */    /* element_size: 32 */
    u_int32_t	PortRcvPkts;    /* bit_offset:288 */    /* element_size: 32 */
    u_int32_t	PortXmitWait;    /* bit_offset:320 */    /* element_size: 32 */
};


/*************************************/
/* Name: SMP_MlnxExtPortInfo
 * Size: 512 bits
 * Description:  */

struct SMP_MlnxExtPortInfo {
    u_int8_t	StateChangeEnable;    /* bit_offset:0 */    /* element_size: 8 */
    u_int32_t	reserved0;    /* bit_offset:8 */    /* element_size: 24 */
    u_int8_t	LinkSpeedSupported;    /* bit_offset:32 */    /* element_size: 8 */
    u_int32_t	reserved1;    /* bit_offset:40 */    /* element_size: 24 */
    u_int8_t	LinkSpeedEnabled;    /* bit_offset:64 */    /* element_size: 8 */
    u_int32_t	reserved2;    /* bit_offset:72 */    /* element_size: 24 */
    u_int8_t	LinkSpeedActive;    /* bit_offset:96 */    /* element_size: 8 */
    u_int32_t	reserved3;    /* bit_offset:104 */    /* element_size: 24 */
    u_int32_t	reserved4;    /* bit_offset:128 */    /* element_size: 384 */
};


/*************************************/
/* Name: SMP_LedInfo
 * Size: 32 bits
 * Description:  */

struct SMP_LedInfo {
    u_int8_t	LedMask;    /* bit_offset:0 */    /* element_size: 1 */
    u_int32_t	reserved0;    /* bit_offset:1 */    /* element_size: 31 */
};


/*************************************/
/* Name: SMP_SMInfo
 * Size: 192 bits
 * Description:  */

struct SMP_SMInfo {
    u_int64_t	GUID;    /* bit_offset:0 */    /* element_size: 64 */
    u_int64_t	Sm_Key;    /* bit_offset:64 */    /* element_size: 64 */
    u_int32_t	ActCount;    /* bit_offset:128 */    /* element_size: 32 */
    u_int32_t	reserved0;    /* bit_offset:160 */    /* element_size: 24 */
    u_int8_t	SmState;    /* bit_offset:184 */    /* element_size: 4 */
    u_int8_t	Priority;    /* bit_offset:188 */    /* element_size: 4 */
};


/*************************************/
/* Name: SMP_MulticastForwardingTable
 * Size: 512 bits
 * Description:  */

struct SMP_MulticastForwardingTable {
    u_int16_t	PortMask[32];    /* bit_offset:0 */    /* element_size: 16 */
};


/*************************************/
/* Name: SMP_RandomForwardingTable
 * Size: 512 bits
 * Description:  */

struct SMP_RandomForwardingTable {
    struct LID_Port_Block_Element	LID_Port_Block_Element[16];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: SMP_LinearForwardingTable
 * Size: 512 bits
 * Description:  */

struct SMP_LinearForwardingTable {
    u_int8_t	Port[64];    /* bit_offset:0 */    /* element_size: 8 */
};


/*************************************/
/* Name: SMP_VLArbitrationTable
 * Size: 512 bits
 * Description:  */

struct SMP_VLArbitrationTable {
    struct VL_Weight_Block_Element	VLArb[32];    /* bit_offset:0 */    /* element_size: 16 */
};


/*************************************/
/* Name: SMP_SLToVLMappingTable
 * Size: 64 bits
 * Description:  */

struct SMP_SLToVLMappingTable {
    u_int8_t	SL0ToVL;    /* bit_offset:0 */    /* element_size: 4 */
    u_int8_t	SL1ToVL;    /* bit_offset:4 */    /* element_size: 4 */
    u_int8_t	SL2ToVL;    /* bit_offset:8 */    /* element_size: 4 */
    u_int8_t	SL3ToVL;    /* bit_offset:12 */    /* element_size: 4 */
    u_int8_t	SL4ToVL;    /* bit_offset:16 */    /* element_size: 4 */
    u_int8_t	SL5ToVL;    /* bit_offset:20 */    /* element_size: 4 */
    u_int8_t	SL6ToVL;    /* bit_offset:24 */    /* element_size: 4 */
    u_int8_t	SL7ToVL;    /* bit_offset:28 */    /* element_size: 4 */
    u_int8_t	SL8ToVL;    /* bit_offset:32 */    /* element_size: 4 */
    u_int8_t	SL9ToVL;    /* bit_offset:36 */    /* element_size: 4 */
    u_int8_t	SL10ToVL;    /* bit_offset:40 */    /* element_size: 4 */
    u_int8_t	SL11ToVL;    /* bit_offset:44 */    /* element_size: 4 */
    u_int8_t	SL12ToVL;    /* bit_offset:48 */    /* element_size: 4 */
    u_int8_t	SL13ToVL;    /* bit_offset:52 */    /* element_size: 4 */
    u_int8_t	SL14ToVL;    /* bit_offset:56 */    /* element_size: 4 */
    u_int8_t	SL15ToVL;    /* bit_offset:60 */    /* element_size: 4 */
};


/*************************************/
/* Name: SMP_PKeyTable
 * Size: 512 bits
 * Description:  */

struct SMP_PKeyTable {
    struct P_Key_Block_Element	PKey_Entry[32];    /* bit_offset:0 */    /* element_size: 16 */
};


/*************************************/
/* Name: SMP_PortInfo
 * Size: 512 bits
 * Description: 0x38 */

struct SMP_PortInfo {
    u_int64_t	MKey;    /* bit_offset:0 */    /* element_size: 64 */
    u_int64_t	GIDPrfx;    /* bit_offset:64 */    /* element_size: 64 */
    u_int16_t	MSMLID;    /* bit_offset:128 */    /* element_size: 16 */
    u_int16_t	LID;    /* bit_offset:144 */    /* element_size: 16 */    /* offset  128 */
    u_int32_t	CapMsk;    /* bit_offset:160 */    /* element_size: 32 */    /* offset  160 */
    u_int16_t	M_KeyLeasePeriod;    /* bit_offset:192 */    /* element_size: 16 */
    u_int16_t	DiagCode;    /* bit_offset:208 */    /* element_size: 16 */    /* offset  192 */
    u_int8_t	LinkWidthActv;    /* bit_offset:224 */    /* element_size: 8 */
    u_int8_t	LinkWidthSup;    /* bit_offset:232 */    /* element_size: 8 */
    u_int8_t	LinkWidthEn;    /* bit_offset:240 */    /* element_size: 8 */
    u_int8_t	LocalPortNum;    /* bit_offset:248 */    /* element_size: 8 */    /* offset  224 */
    u_int8_t	LinkSpeedEn;    /* bit_offset:256 */    /* element_size: 4 */
    u_int8_t	LinkSpeedActv;    /* bit_offset:260 */    /* element_size: 4 */
    u_int8_t	LMC;    /* bit_offset:264 */    /* element_size: 3 */
    u_int8_t	reserved0;    /* bit_offset:267 */    /* element_size: 3 */
    u_int8_t	MKeyProtBits;    /* bit_offset:270 */    /* element_size: 2 */
    u_int8_t	LinkDownDefState;    /* bit_offset:272 */    /* element_size: 4 */
    u_int8_t	PortPhyState;    /* bit_offset:276 */    /* element_size: 4 */
    u_int8_t	PortState;    /* bit_offset:280 */    /* element_size: 4 */
    u_int8_t	LinkSpeedSup;    /* bit_offset:284 */    /* element_size: 4 */    /* offset 256 */
    u_int8_t	VLArbHighCap;    /* bit_offset:288 */    /* element_size: 8 */
    u_int8_t	VLHighLimit;    /* bit_offset:296 */    /* element_size: 8 */
    u_int8_t	InitType;    /* bit_offset:304 */    /* element_size: 4 */
    u_int8_t	VLCap;    /* bit_offset:308 */    /* element_size: 4 */
    u_int8_t	MSMSL;    /* bit_offset:312 */    /* element_size: 4 */
    u_int8_t	NMTU;    /* bit_offset:316 */    /* element_size: 4 */    /* offset 288 */
    u_int8_t	FilterRawOutb;    /* bit_offset:320 */    /* element_size: 1 */
    u_int8_t	FilterRawInb;    /* bit_offset:321 */    /* element_size: 1 */
    u_int8_t	PartEnfOutb;    /* bit_offset:322 */    /* element_size: 1 */
    u_int8_t	PartEnfInb;    /* bit_offset:323 */    /* element_size: 1 */
    u_int8_t	OpVLs;    /* bit_offset:324 */    /* element_size: 4 */
    u_int8_t	HoQLife;    /* bit_offset:328 */    /* element_size: 5 */
    u_int8_t	VLStallCnt;    /* bit_offset:333 */    /* element_size: 3 */
    u_int8_t	MTUCap;    /* bit_offset:336 */    /* element_size: 4 */
    u_int8_t	InitTypeReply;    /* bit_offset:340 */    /* element_size: 4 */
    u_int8_t	VLArbLowCap;    /* bit_offset:344 */    /* element_size: 8 */    /* offset 320 */
    u_int16_t	PKeyViolations;    /* bit_offset:352 */    /* element_size: 16 */
    u_int16_t	MKeyViolations;    /* bit_offset:368 */    /* element_size: 16 */
    u_int8_t	SubnTmo;    /* bit_offset:384 */    /* element_size: 5 */
    u_int8_t	reserved1;    /* bit_offset:389 */    /* element_size: 2 */
    u_int8_t	ClientReregister;    /* bit_offset:391 */    /* element_size: 1 */
    u_int8_t	GUIDCap;    /* bit_offset:392 */    /* element_size: 8 */
    u_int16_t	QKeyViolations;    /* bit_offset:400 */    /* element_size: 16 */
    u_int16_t	MaxCreditHint;    /* bit_offset:416 */    /* element_size: 16 */
    u_int8_t	OverrunErrs;    /* bit_offset:432 */    /* element_size: 4 */
    u_int8_t	LocalPhyError;    /* bit_offset:436 */    /* element_size: 4 */
    u_int8_t	RespTimeValue;    /* bit_offset:440 */    /* element_size: 5 */
    u_int8_t	reserved2;    /* bit_offset:445 */    /* element_size: 3 */
    u_int32_t	LinkRoundTripLatency;    /* bit_offset:448 */    /* element_size: 24 */
    u_int8_t	reserved3;    /* bit_offset:472 */    /* element_size: 8 */
    u_int8_t	LinkSpeedExtEn;    /* bit_offset:480 */    /* element_size: 5 */    /* offset 507 */
    u_int8_t	reserved4;    /* bit_offset:485 */    /* element_size: 3 */
    u_int8_t	LinkSpeedExtSup;    /* bit_offset:488 */    /* element_size: 4 */    /* offset 500 */
    u_int8_t	LinkSpeedExtActv;    /* bit_offset:492 */    /* element_size: 4 */    /* offset 496 */
    u_int16_t	reserved5;    /* bit_offset:496 */    /* element_size: 16 */
};


/*************************************/
/* Name: SMP_GUIDInfo
 * Size: 512 bits
 * Description:  */

struct SMP_GUIDInfo {
    struct GUID_Block_Element	GUIDBlock;    /* bit_offset:0 */    /* element_size: 512 */
};


/*************************************/
/* Name: SMP_SwitchInfo
 * Size: 512 bits
 * Description:  */

struct SMP_SwitchInfo {
    u_int16_t	RandomFDBCap;    /* bit_offset:0 */    /* element_size: 16 */
    u_int16_t	LinearFDBCap;    /* bit_offset:16 */    /* element_size: 16 */
    u_int16_t	LinearFDBTop;    /* bit_offset:32 */    /* element_size: 16 */
    u_int16_t	MCastFDBCap;    /* bit_offset:48 */    /* element_size: 16 */
    u_int8_t	OptimizedSLVLMapping;    /* bit_offset:64 */    /* element_size: 2 */    /* bit[0] when set indicate switch support optimized SLtoVL Mapping
                                                 bit[1] - reserve */
    u_int8_t	PortStateChange;    /* bit_offset:66 */    /* element_size: 1 */
    u_int8_t	LifeTimeValue;    /* bit_offset:67 */    /* element_size: 5 */
    u_int8_t	DefMCastNotPriPort;    /* bit_offset:72 */    /* element_size: 8 */
    u_int8_t	DefMCastPriPort;    /* bit_offset:80 */    /* element_size: 8 */
    u_int8_t	DefPort;    /* bit_offset:88 */    /* element_size: 8 */
    u_int16_t	PartEnfCap;    /* bit_offset:96 */    /* element_size: 16 */
    u_int16_t	LidsPerPort;    /* bit_offset:112 */    /* element_size: 16 */
    u_int16_t	MCastFDBTop;    /* bit_offset:128 */    /* element_size: 16 */
    u_int16_t	reserved0;    /* bit_offset:144 */    /* element_size: 11 */
    u_int8_t	ENP0;    /* bit_offset:155 */    /* element_size: 1 */
    u_int8_t	FilterRawOutbCap;    /* bit_offset:156 */    /* element_size: 1 */
    u_int8_t	FilterRawInbCap;    /* bit_offset:157 */    /* element_size: 1 */
    u_int8_t	OutbEnfCap;    /* bit_offset:158 */    /* element_size: 1 */
    u_int8_t	InbEnfCap;    /* bit_offset:159 */    /* element_size: 1 */
    u_int32_t	reserved1;    /* bit_offset:160 */    /* element_size: 352 */
};


/*************************************/
/* Name: SMP_NodeInfo
 * Size: 320 bits
 * Description: 0x28 */

struct SMP_NodeInfo {
    u_int8_t	NumPorts;    /* bit_offset:0 */    /* element_size: 8 */    /* Number of physical ports on this node. */
    u_int8_t	NodeType;    /* bit_offset:8 */    /* element_size: 8 */    /* 1: Channel Adapter 2: Switch 3: Router 0, 4 - 255: Reserved
                                                  */
    u_int8_t	ClassVersion;    /* bit_offset:16 */    /* element_size: 8 */
    u_int8_t	BaseVersion;    /* bit_offset:24 */    /* element_size: 8 */
    u_int64_t	SystemImageGUID;    /* bit_offset:32 */    /* element_size: 64 */
    u_int64_t	NodeGUID;    /* bit_offset:96 */    /* element_size: 64 */
    u_int64_t	PortGUID;    /* bit_offset:160 */    /* element_size: 64 */
    u_int16_t	DeviceID;    /* bit_offset:224 */    /* element_size: 16 */    /* Device ID information as assigned by device manufacturer. */
    u_int16_t	PartitionCap;    /* bit_offset:240 */    /* element_size: 16 */    /* Number of entries in the Partition Table for CA, router, and the switch management port. This is at a minimum set to 1 for all nodes including switches. */
    u_int32_t	revision;    /* bit_offset:256 */    /* element_size: 32 */    /* Device revision, assigned by manufacturer. */
    u_int32_t	VendorID;    /* bit_offset:288 */    /* element_size: 24 */
    u_int8_t	LocalPortNum;    /* bit_offset:312 */    /* element_size: 8 */    /* Number of the link port which received this SMP. */
};


/*************************************/
/* Name: SMP_NodeDesc
 * Size: 512 bits
 * Description: SMP_NodeDesc */

struct SMP_NodeDesc {
    u_int8_t	Byte[64];    /* bit_offset:0 */    /* element_size: 8 */    /* UTF-8 encoded string to describe node in text format */
};


/*************************************/
/* Name: CC_Mgt_Data_Block_Element
 * Size: 1536 bits
 * Description: CC_Mgt_Data_Block_Element */

struct CC_Mgt_Data_Block_Element {
    u_int32_t	DWORD[48];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: CC_Log_Data_Block_Element
 * Size: 256 bits
 * Description: CC_Log_Data_Block_Element */

struct CC_Log_Data_Block_Element {
    u_int32_t	DWORD[8];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: MAD_Header_Common
 * Size: 192 bits
 * Description: MAD Header Common */

struct MAD_Header_Common {
    u_int8_t	Method;    /* bit_offset:0 */    /* element_size: 8 */    /* Method to perform Based on Management Class */
    u_int8_t	ClassVersion;    /* bit_offset:8 */    /* element_size: 8 */    /* Version of MAD class-specific format */
    u_int8_t	MgmtClass;    /* bit_offset:16 */    /* element_size: 8 */    /* Class of operation */
    u_int8_t	BaseVersion;    /* bit_offset:24 */    /* element_size: 8 */    /* Version of MAD base format */
    u_int16_t	ClassSpecific;    /* bit_offset:32 */    /* element_size: 16 */    /* This field is reserved except for the Subnet Management Class */
    u_int16_t	Status;    /* bit_offset:48 */    /* element_size: 16 */    /* Code indicating status of operation */
    u_int64_t	TID_Block_Element;    /* bit_offset:64 */    /* element_size: 64 */
    u_int16_t	Rsv16;    /* bit_offset:128 */    /* element_size: 16 */
    u_int16_t	AttributeID;    /* bit_offset:144 */    /* element_size: 16 */    /* Defines objects being operated by a management class (Page 658) */
    u_int32_t	AttributeModifier;    /* bit_offset:160 */    /* element_size: 32 */    /* Provides further scope to the attributes */
};


/*************************************/
/* Name: VendorSpecific_MAD_Data_Block_Element
 * Size: 1792 bits
 * Description: VendorSpecific_MAD_Data_Block_Element */

struct VendorSpecific_MAD_Data_Block_Element {
    u_int32_t	DWORD[56];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: SubnetAdministartion_MAD_Data_Block_Element
 * Size: 1600 bits
 * Description: SubnetAdministartion_MAD_Data_Block_Element */

struct SubnetAdministartion_MAD_Data_Block_Element {
    u_int32_t	DWORD[50];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: MAD_Header_Common_With_RMPP
 * Size: 288 bits
 * Description: MAD Header Common With RMPP */

struct MAD_Header_Common_With_RMPP {
    u_int8_t	Method;    /* bit_offset:0 */    /* element_size: 8 */    /* Method to perform Based on Management Class */
    u_int8_t	ClassVersion;    /* bit_offset:8 */    /* element_size: 8 */    /* Version of MAD class-specific format */
    u_int8_t	MgmtClass;    /* bit_offset:16 */    /* element_size: 8 */    /* Class of operation */
    u_int8_t	BaseVersion;    /* bit_offset:24 */    /* element_size: 8 */    /* Version of MAD base format */
    u_int16_t	ClassSpecific;    /* bit_offset:32 */    /* element_size: 16 */    /* This field is reserved except for the Subnet Management Class */
    u_int16_t	Status;    /* bit_offset:48 */    /* element_size: 16 */    /* Code indicating status of operation */
    u_int64_t	TID_Block_Element;    /* bit_offset:64 */    /* element_size: 64 */
    u_int16_t	Rsv16;    /* bit_offset:128 */    /* element_size: 16 */
    u_int16_t	AttributeID;    /* bit_offset:144 */    /* element_size: 16 */    /* Defines objects being operated by a management class (Page 658) */
    u_int32_t	AttributeModifier;    /* bit_offset:160 */    /* element_size: 32 */    /* Provides further scope to the attributes */
    u_int8_t	RMPPStatus;    /* bit_offset:192 */    /* element_size: 8 */
    u_int8_t	RMPPFlags;    /* bit_offset:200 */    /* element_size: 4 */
    u_int8_t	RRespTime;    /* bit_offset:204 */    /* element_size: 4 */
    u_int8_t	RMPPType;    /* bit_offset:208 */    /* element_size: 8 */    /* Indicates the type of RMPP packet being transferred. */
    u_int8_t	RMPPVersion;    /* bit_offset:216 */    /* element_size: 8 */    /* Version of RMPP. Shall be set to the version of RMPP implemented. */
    u_int32_t	Data1;    /* bit_offset:224 */    /* element_size: 32 */
    u_int32_t	Data2;    /* bit_offset:256 */    /* element_size: 32 */
};


/*************************************/
/* Name: PerfManagement_MAD_Data_Block_Element
 * Size: 1536 bits
 * Description: PerfManagement_MAD_Data_Block_Element */

struct PerfManagement_MAD_Data_Block_Element {
    u_int32_t	DWORD[48];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: SMP_MAD_Data_Block_Element
 * Size: 512 bits
 * Description: SMP_MAD_Data_Block_Element */

struct SMP_MAD_Data_Block_Element {
    u_int32_t	DWORD[16];    /* bit_offset:0 */    /* element_size: 32 */
};


/*************************************/
/* Name: DirRPath_Block_Element
 * Size: 512 bits
 * Description: DirRPath_Block_Element */

struct DirRPath_Block_Element {
    u_int8_t	BYTE[64];    /* bit_offset:0 */    /* element_size: 8 */
};


/*************************************/
/* Name: MAD_Header_SMP_Direct_Routed
 * Size: 192 bits
 * Description: MAD Header for SMP Direct Routed */

struct MAD_Header_SMP_Direct_Routed {
    u_int8_t	Method;    /* bit_offset:0 */    /* element_size: 8 */    /* Method to perform Based on Management Class */
    u_int8_t	ClassVersion;    /* bit_offset:8 */    /* element_size: 8 */    /* Version of MAD class-specific format */
    u_int8_t	MgmtClass;    /* bit_offset:16 */    /* element_size: 8 */    /* Class of operation */
    u_int8_t	BaseVersion;    /* bit_offset:24 */    /* element_size: 8 */    /* Version of MAD base format */
    u_int8_t	HopCounter;    /* bit_offset:32 */    /* element_size: 8 */    /* Used in The direct route */
    u_int8_t	HopPointer;    /* bit_offset:40 */    /* element_size: 8 */    /* Used in The direct route */
    u_int16_t	Status;    /* bit_offset:48 */    /* element_size: 15 */    /* Code indicating status of operation */
    u_int8_t	D;    /* bit_offset:63 */    /* element_size: 1 */    /* direction of direct route packet */
    u_int64_t	TID_Block_Element;    /* bit_offset:64 */    /* element_size: 64 */
    u_int16_t	Rsv16;    /* bit_offset:128 */    /* element_size: 16 */
    u_int16_t	AttributeID;    /* bit_offset:144 */    /* element_size: 16 */    /* Defines objects being operated by a management class (Page 658) */
    u_int32_t	AttributeModifier;    /* bit_offset:160 */    /* element_size: 32 */    /* Provides further scope to the attributes */
};


/*************************************/
/* Name: VENDOR_SPECS
 * Size: 1048576 bits
 * Description: VENDOR_SPECS */

struct VENDOR_SPECS {
    u_int32_t	reserved0;    /* bit_offset:0 */    /* element_size: 32768 */
    struct VendorSpec_GeneralInfo	VendorSpec_GeneralInfo;    /* bit_offset:32768 */    /* element_size: 1024 */
    u_int32_t	reserved1;    /* bit_offset:33792 */    /* element_size: 1014784 */
};


/*************************************/
/* Name: CONGESTION_CONTOL
 * Size: 1048576 bits
 * Description: CONGESTION_CONTOL */

struct CONGESTION_CONTOL {
    u_int32_t	reserved0;    /* bit_offset:0 */    /* element_size: 32768 */
    struct CC_CongestionInfo	CC_CongestionInfo;    /* bit_offset:32768 */    /* element_size: 32 */
    u_int32_t	reserved1;    /* bit_offset:32800 */    /* element_size: 10208 */
    struct CC_CongestionKeyInfo	CC_CongestionKeyInfo;    /* bit_offset:43008 */    /* element_size: 128 */
    u_int32_t	reserved2;    /* bit_offset:43136 */    /* element_size: 22400 */
    struct CC_CongestionLogSwitch	CC_CongestionLogSwitch;    /* bit_offset:65536 */    /* element_size: 1760 */
    u_int32_t	reserved3;    /* bit_offset:67296 */    /* element_size: 8480 */
    struct CC_CongestionLogCA	CC_CongestionLogCA;    /* bit_offset:75776 */    /* element_size: 1760 */
    u_int32_t	reserved4;    /* bit_offset:77536 */    /* element_size: 20768 */
    struct CC_SwitchCongestionSetting	CC_SwitchCongestionSetting;    /* bit_offset:98304 */    /* element_size: 608 */
    u_int32_t	reserved5;    /* bit_offset:98912 */    /* element_size: 9632 */
    struct CC_SwitchPortCongestionSetting	CC_SwitchPortCongestionSetting;    /* bit_offset:108544 */    /* element_size: 1024 */
    u_int32_t	reserved6;    /* bit_offset:109568 */    /* element_size: 21504 */
    struct CC_CACongestionSetting	CC_CACongestionSetting;    /* bit_offset:131072 */    /* element_size: 1056 */
    u_int32_t	reserved7;    /* bit_offset:132128 */    /* element_size: 9184 */
    struct CC_CongestionControlTable	CC_CongestionControlTable;    /* bit_offset:141312 */    /* element_size: 1056 */
    u_int32_t	reserved8;    /* bit_offset:142368 */    /* element_size: 21472 */
    struct CC_TimeStamp	CC_TimeStamp;    /* bit_offset:163840 */    /* element_size: 32 */
    u_int32_t	reserved9;    /* bit_offset:163872 */    /* element_size: 10208 */
    struct CC_Notice	CC_Notice;    /* bit_offset:174080 */    /* element_size: 640 */
    u_int32_t	reserved10;    /* bit_offset:174720 */    /* element_size: 873856 */
};


/*************************************/
/* Name: PERFORMANCE_MADS
 * Size: 1048576 bits
 * Description: PERFORMANCE_MADS */

struct PERFORMANCE_MADS {
    u_int32_t	reserved0;    /* bit_offset:0 */    /* element_size: 32768 */
    struct PM_PortCounters	PM_PortCounters;    /* bit_offset:32768 */    /* element_size: 352 */
    u_int32_t	reserved1;    /* bit_offset:33120 */    /* element_size: 9888 */
    struct PM_PortCountersExtended	PM_PortCountersExtended;    /* bit_offset:43008 */    /* element_size: 576 */
    u_int32_t	reserved2;    /* bit_offset:43584 */    /* element_size: 21952 */
    struct PM_PortXmitDiscardDetails	PM_PortXmitDiscardDetails;    /* bit_offset:65536 */    /* element_size: 512 */
    u_int32_t	reserved3;    /* bit_offset:66048 */    /* element_size: 9728 */
    struct PM_PortRcvErrorDetails	PM_PortRcvErrorDetails;    /* bit_offset:75776 */    /* element_size: 512 */
    u_int32_t	reserved4;    /* bit_offset:76288 */    /* element_size: 22016 */
    struct PM_PortSamplesResult	PM_PortSamplesResult;    /* bit_offset:98304 */    /* element_size: 1536 */
    u_int32_t	reserved5;    /* bit_offset:99840 */    /* element_size: 948736 */
};


/*************************************/
/* Name: SMP_MADS
 * Size: 1048576 bits
 * Description: SMP_MADS */

struct SMP_MADS {
    u_int32_t	reserved0;    /* bit_offset:0 */    /* element_size: 32768 */
    struct SMP_NodeDesc	SMP_NodeDesc;    /* bit_offset:32768 */    /* element_size: 512 */
    u_int32_t	reserved1;    /* bit_offset:33280 */    /* element_size: 9728 */
    struct SMP_NodeInfo	SMP_NodeInfo;    /* bit_offset:43008 */    /* element_size: 320 */
    u_int32_t	reserved2;    /* bit_offset:43328 */    /* element_size: 22208 */
    struct SMP_SwitchInfo	SMP_SwitchInfo;    /* bit_offset:65536 */    /* element_size: 512 */
    u_int32_t	reserved3;    /* bit_offset:66048 */    /* element_size: 9728 */
    struct SMP_GUIDInfo	SMP_GUIDInfo;    /* bit_offset:75776 */    /* element_size: 512 */
    u_int32_t	reserved4;    /* bit_offset:76288 */    /* element_size: 22016 */
    struct SMP_PortInfo	SMP_PortInfo;    /* bit_offset:98304 */    /* element_size: 512 */
    u_int32_t	reserved5;    /* bit_offset:98816 */    /* element_size: 9728 */
    struct SMP_PKeyTable	SMP_PKeyTable;    /* bit_offset:108544 */    /* element_size: 512 */
    u_int32_t	reserved6;    /* bit_offset:109056 */    /* element_size: 22016 */
    struct SMP_SLToVLMappingTable	SMP_SLToVLMappingTable;    /* bit_offset:131072 */    /* element_size: 64 */
    u_int32_t	reserved7;    /* bit_offset:131136 */    /* element_size: 10176 */
    struct SMP_VLArbitrationTable	SMP_VLArbitrationTable;    /* bit_offset:141312 */    /* element_size: 512 */
    u_int32_t	reserved8;    /* bit_offset:141824 */    /* element_size: 22016 */
    struct SMP_LinearForwardingTable	SMP_LinearForwardingTable;    /* bit_offset:163840 */    /* element_size: 512 */
    u_int32_t	reserved9;    /* bit_offset:164352 */    /* element_size: 9728 */
    struct SMP_RandomForwardingTable	SMP_RandomForwardingTable;    /* bit_offset:174080 */    /* element_size: 512 */
    u_int32_t	reserved10;    /* bit_offset:174592 */    /* element_size: 22016 */
    struct SMP_MulticastForwardingTable	SMP_MulticastForwardingTable;    /* bit_offset:196608 */    /* element_size: 512 */
    u_int32_t	reserved11;    /* bit_offset:197120 */    /* element_size: 9728 */
    struct SMP_SMInfo	SMP_SMInfo;    /* bit_offset:206848 */    /* element_size: 192 */
    u_int32_t	reserved12;    /* bit_offset:207040 */    /* element_size: 22336 */
    struct SMP_LedInfo	SMP_LedInfo;    /* bit_offset:229376 */    /* element_size: 32 */
    u_int32_t	reserved13;    /* bit_offset:229408 */    /* element_size: 10208 */
    struct SMP_MlnxExtPortInfo	SMP_MlnxExtPortInfo;    /* bit_offset:239616 */    /* element_size: 512 */
    u_int32_t	reserved14;    /* bit_offset:240128 */    /* element_size: 808448 */
};


/*************************************/
/* Name: IB_ClassPortInfo
 * Size: 576 bits
 * Description:  */

struct IB_ClassPortInfo {
    u_int16_t	CapMsk;    /* bit_offset:0 */    /* element_size: 16 */    /* Supported capabilities of this management class, 
                                                 Bit set to 1 for affirmation of management support. 
                                                 Bit 0 - If 1, the management class generates Trap() MADs Bit 1 - If 1, the management class implements Get(Notice) and Set(Notice) 
                                                 Bit 2-7: reserved 
                                                 Bit 8: Switch only - set if the EnhancedPort0 supports CA Congestion Control. (Note a switch can support Congestion control on data ports without supporting it on EnhancedPort0) 
                                                 Bit 9-15: class-specific capabilities. */
    u_int8_t	ClassVersion;    /* bit_offset:16 */    /* element_size: 8 */    /* Current supported management class version. 
                                                 Indicates that this channel adapter, switch, or router supports up to and including this version. */
    u_int8_t	BaseVersion;    /* bit_offset:24 */    /* element_size: 8 */    /* Current supported MAD Base Version. 
                                                 Indicates that this channel adapter, switch, or router supports up to and including this version. */
    u_int8_t	RespTimeValue;    /* bit_offset:32 */    /* element_size: 5 */    /* See 13.4.6.2 Timers and Timeouts . */
    u_int32_t	reserved0;    /* bit_offset:37 */    /* element_size: 27 */
    u_int32_t	RedirectGID;    /* bit_offset:64 */    /* element_size: 128 */    /* The GID a requester shall use as the destination GID in the GRH of messages used to access redirected class services. */
    u_int32_t	RedirectFL;    /* bit_offset:192 */    /* element_size: 20 */    /* The Flow Label a requester shall use in the GRH of messages used to access redirected class services. */
    u_int8_t	RedirectSL;    /* bit_offset:212 */    /* element_size: 4 */    /* The SL a requester shall use to access the class services. */
    u_int8_t	RedirectTC;    /* bit_offset:216 */    /* element_size: 8 */    /* The Traffic Class a requester shall use in the GRH of messages used to access redirected class services. */
    u_int16_t	RedirectPKey;    /* bit_offset:224 */    /* element_size: 16 */    /* The P_Key a requester shall use to access the class services. */
    u_int16_t	RedirectLID;    /* bit_offset:240 */    /* element_size: 16 */    /* If this value is non-zero, it is the DLID a requester shall use to access the class services. */
    u_int32_t	RedirectQP;    /* bit_offset:256 */    /* element_size: 24 */    /* The QP a requester shall use to access the class services. Zero is illegal. */
    u_int8_t	reserved1;    /* bit_offset:280 */    /* element_size: 8 */
    u_int32_t	RedirectQKey;    /* bit_offset:288 */    /* element_size: 32 */    /* The Q_Key associated with the RedirectQP. This Q_Key shall be set to the well known Q_Key. */
    u_int32_t	TrapGID;    /* bit_offset:320 */    /* element_size: 128 */    /* The GID to be used as the destination GID in the GRH of Trap() messages originated by this service. */
    u_int32_t	TrapFL;    /* bit_offset:448 */    /* element_size: 20 */    /* The Flow Label to be placed in the GRH of Trap() messages originated by this service. */
    u_int8_t	TrapSL;    /* bit_offset:468 */    /* element_size: 4 */    /* The SL that shall be used when sending Trap() messages originated by this service. */
    u_int8_t	TrapTC;    /* bit_offset:472 */    /* element_size: 8 */    /* The Traffic Class to be placed in the GRH of Trap() messages originated by this service. */
    u_int16_t	TrapPKey;    /* bit_offset:480 */    /* element_size: 16 */    /* The P_Key to be placed in the header for traps originated by this service. */
    u_int16_t	TrapLID;    /* bit_offset:496 */    /* element_size: 16 */    /* The DLID to where Trap() messages shall be sent by this service. */
    u_int32_t	TrapQP;    /* bit_offset:512 */    /* element_size: 24 */    /* The QP to which Trap() messages originated by this service shall be sent. The value shall not be zero. */
    u_int8_t	TrapHL;    /* bit_offset:536 */    /* element_size: 8 */    /* The Hop Limit to be placed in the GRH of Trap() messages originated by this service. The default value is 255. */
    u_int32_t	TrapQKey;    /* bit_offset:544 */    /* element_size: 32 */    /* The Q_Key associated with the TrapQP. */
};


/*************************************/
/* Name: MAD_CongestionControl
 * Size: 2048 bits
 * Description: MAD Performance management Data Format */

struct MAD_CongestionControl {
    struct MAD_Header_Common	MAD_Header_Common;    /* bit_offset:0 */    /* element_size: 192 */
    u_int64_t	CC_Key;    /* bit_offset:192 */    /* element_size: 64 */    /* Congestion Control key, is used to validate the source of Congestion Control Mads. */
    struct CC_Log_Data_Block_Element	CC_LogData;    /* bit_offset:256 */    /* element_size: 256 */
    struct CC_Mgt_Data_Block_Element	CC_MgtData;    /* bit_offset:512 */    /* element_size: 1536 */
};


/*************************************/
/* Name: MAD_VendorSpec
 * Size: 2048 bits
 * Description: MAD Vendor Specific Data Format */

struct MAD_VendorSpec {
    struct MAD_Header_Common	MAD_Header_Common;    /* bit_offset:0 */    /* element_size: 192 */
    u_int64_t	V_Key;    /* bit_offset:192 */    /* element_size: 64 */
    struct VendorSpecific_MAD_Data_Block_Element	Data;    /* bit_offset:256 */    /* element_size: 1792 */
};


/*************************************/
/* Name: SA_MCMMemberRecord
 * Size: 416 bits
 * Description:  */

struct SA_MCMMemberRecord {
    struct GID_Block_Element	MGID;    /* bit_offset:0 */    /* element_size: 128 */
    struct GID_Block_Element	PortGID;    /* bit_offset:128 */    /* element_size: 128 */
    u_int32_t	Q_key;    /* bit_offset:256 */    /* element_size: 32 */
    u_int8_t	TClass;    /* bit_offset:288 */    /* element_size: 8 */
    u_int8_t	MTU;    /* bit_offset:296 */    /* element_size: 6 */
    u_int8_t	MTUSelector;    /* bit_offset:302 */    /* element_size: 2 */
    u_int16_t	MLID;    /* bit_offset:304 */    /* element_size: 16 */
    u_int8_t	PacketLifeTime;    /* bit_offset:320 */    /* element_size: 6 */
    u_int8_t	PacketLifeTimeSelector;    /* bit_offset:326 */    /* element_size: 2 */
    u_int8_t	Rate;    /* bit_offset:328 */    /* element_size: 6 */
    u_int8_t	RateSelector;    /* bit_offset:334 */    /* element_size: 2 */
    u_int16_t	P_Key;    /* bit_offset:336 */    /* element_size: 16 */
    u_int8_t	HopLimit;    /* bit_offset:352 */    /* element_size: 8 */
    u_int32_t	FlowLabel;    /* bit_offset:360 */    /* element_size: 20 */
    u_int8_t	SL;    /* bit_offset:380 */    /* element_size: 4 */
    u_int32_t	reserved0;    /* bit_offset:384 */    /* element_size: 23 */
    u_int8_t	ProxyJoin;    /* bit_offset:407 */    /* element_size: 1 */
    u_int8_t	JoinState;    /* bit_offset:408 */    /* element_size: 4 */
    u_int8_t	Scope;    /* bit_offset:412 */    /* element_size: 4 */
};


/*************************************/
/* Name: MAD_SubnetAdministartion
 * Size: 2048 bits
 * Description: MAD Subnet Administration Data Format */

struct MAD_SubnetAdministartion {
    struct MAD_Header_Common_With_RMPP	MAD_Header_Common_With_RMPP;    /* bit_offset:0 */    /* element_size: 288 */
    u_int64_t	Sm_Key;    /* bit_offset:288 */    /* element_size: 64 */
    u_int16_t	reserved0;    /* bit_offset:352 */    /* element_size: 16 */
    u_int16_t	AttributeOffset;    /* bit_offset:368 */    /* element_size: 16 */
    u_int64_t	ComponentMask;    /* bit_offset:384 */    /* element_size: 64 */
    struct SubnetAdministartion_MAD_Data_Block_Element	Data;    /* bit_offset:448 */    /* element_size: 1600 */
};


/*************************************/
/* Name: MAD_PerformanceManagement
 * Size: 2048 bits
 * Description: MAD Performance management Data Format */

struct MAD_PerformanceManagement {
    struct MAD_Header_Common	MAD_Header_Common;    /* bit_offset:0 */    /* element_size: 192 */
    u_int32_t	reserved0[10];    /* bit_offset:192 */    /* element_size: 32 */
    struct PerfManagement_MAD_Data_Block_Element	Data;    /* bit_offset:512 */    /* element_size: 1536 */
};


/*************************************/
/* Name: MAD_SMP_LID_Routed
 * Size: 2048 bits
 * Description: MAD SMP Data Format - Lid Routed */

struct MAD_SMP_LID_Routed {
    struct MAD_Header_Common	MAD_Header_Common;    /* bit_offset:0 */    /* element_size: 192 */
    u_int64_t	M_Key;    /* bit_offset:192 */    /* element_size: 64 */
    u_int32_t	reserved0;    /* bit_offset:256 */    /* element_size: 256 */    /* Reserved 32 bytes. */
    struct SMP_MAD_Data_Block_Element	Data;    /* bit_offset:512 */    /* element_size: 512 */
    u_int32_t	reserved1;    /* bit_offset:1024 */    /* element_size: 1024 */    /* Reserved 128 bytes. */
};


/*************************************/
/* Name: MAD_SMP_Direct_Routed
 * Size: 2048 bits
 * Description: MAD SMP Data Format - Direct Routed */

struct MAD_SMP_Direct_Routed {
    struct MAD_Header_SMP_Direct_Routed	MAD_Header_SMP_Direct_Routed;    /* bit_offset:0 */    /* element_size: 192 */
    u_int64_t	M_Key;    /* bit_offset:192 */    /* element_size: 64 */
    u_int16_t	DrDLID;    /* bit_offset:256 */    /* element_size: 16 */    /* Directed route destination LID. Used in directed routing. */
    u_int16_t	DrSLID;    /* bit_offset:272 */    /* element_size: 16 */    /* Directed route source LID. Used in directed routing. */
    u_int32_t	reserved0;    /* bit_offset:288 */    /* element_size: 224 */    /* Reserved 28 bytes */
    struct SMP_MAD_Data_Block_Element	Data;    /* bit_offset:512 */    /* element_size: 512 */
    struct DirRPath_Block_Element	InitPath;    /* bit_offset:1024 */    /* element_size: 512 */
    struct DirRPath_Block_Element	RetPath;    /* bit_offset:1536 */    /* element_size: 512 */
};


/*************************************/
/* Name: VCRC
 * Size: 32 bits
 * Description: VCRC of IB Packet */

struct VCRC {
    u_int16_t	VCRC;    /* bit_offset:0 */    /* element_size: 16 */    /* Variant CRC */
    u_int16_t	Rsrvd;    /* bit_offset:16 */    /* element_size: 16 */    /* Reserved */
};


/*************************************/
/* Name: ICRC
 * Size: 32 bits
 * Description: ICRC of IB Packet */

struct ICRC {
    u_int32_t	ICRC;    /* bit_offset:0 */    /* element_size: 32 */    /* Invariant CRC */
};


/*************************************/
/* Name: IB_IETH
 * Size: 32 bits
 * Description: Invalidate Extended Transport Header */

struct IB_IETH {
    u_int32_t	R_Key;    /* bit_offset:0 */    /* element_size: 32 */    /* Remote Key */
};


/*************************************/
/* Name: IB_ImmDt
 * Size: 32 bits
 * Description: Immediate Extended Transport Header */

struct IB_ImmDt {
    u_int32_t	Immediate_Data;    /* bit_offset:0 */    /* element_size: 32 */    /* Immediate Extended Transport Header */
};


/*************************************/
/* Name: IB_AtomicAckETH
 * Size: 64 bits
 * Description: Atomic ACK Extended Transport Header */

struct IB_AtomicAckETH {
    u_int32_t	OrigRemDt;    /* bit_offset:0 */    /* element_size: 64 */    /* Return operand in atomic operations and contains the data in the remote memory location before the atomic operationIndicates if this is an ACK or NAK packet plus additional information about the ACK or NAKRemote virtual address */
};


/*************************************/
/* Name: IB_AETH
 * Size: 32 bits
 * Description: ACK Extended Transport Header */

struct IB_AETH {
    u_int32_t	MSN;    /* bit_offset:0 */    /* element_size: 24 */    /* Indicates the sequence number of the last message completed at the responder. Remote Key that authorizes access to the remote virtual address */
    u_int8_t	Syndrome;    /* bit_offset:24 */    /* element_size: 8 */    /* Indicates if this is an ACK or NAK packet plus additional information about the ACK or NAKRemote virtual address */
};


/*************************************/
/* Name: IB_AtomicETH
 * Size: 224 bits
 * Description: Atomic Extended Transport Header */

struct IB_AtomicETH {
    u_int32_t	VA;    /* bit_offset:0 */    /* element_size: 64 */    /* Remote virtual address */
    u_int32_t	R_Key;    /* bit_offset:64 */    /* element_size: 32 */    /* Remote Key that authorizes access to the remote virtual address */
    u_int32_t	SwapDt;    /* bit_offset:96 */    /* element_size: 64 */    /* Operand in atomic operations */
    u_int32_t	CmpDt;    /* bit_offset:160 */    /* element_size: 64 */    /* Operand in CmpSwap atomic operation */
};


/*************************************/
/* Name: IB_RETH
 * Size: 128 bits
 * Description: RDMA Extended Transport Header Fields */

struct IB_RETH {
    u_int32_t	VA;    /* bit_offset:0 */    /* element_size: 64 */    /* Virtual Address of the RDMA operation */
    u_int32_t	R_Key;    /* bit_offset:64 */    /* element_size: 32 */    /* Remote Key that authorizes access for the RDMA operation */
    u_int32_t	DMALen;    /* bit_offset:96 */    /* element_size: 32 */    /* Indicates the length (in Bytes) of the DMA operation */
};


/*************************************/
/* Name: IB_DETH
 * Size: 64 bits
 * Description: Datagram Extended Transport Header */

struct IB_DETH {
    u_int32_t	Q_Key;    /* bit_offset:0 */    /* element_size: 32 */    /* Queue Key */
    u_int32_t	SrcQP;    /* bit_offset:32 */    /* element_size: 24 */    /* Source QP */
    u_int8_t	Rsv8;    /* bit_offset:56 */    /* element_size: 8 */    /* TX - 0, RX - ignore */
};


/*************************************/
/* Name: IB_RDETH
 * Size: 32 bits
 * Description: Reliable Datagram Extended Transport Header */

struct IB_RDETH {
    u_int32_t	EECnxt;    /* bit_offset:0 */    /* element_size: 24 */    /* This field indicates which End-to-End Context should be used for this Reliable Datagram packet */
    u_int8_t	Rsv8;    /* bit_offset:24 */    /* element_size: 8 */    /* TX - 0, RX - ignore */
};


/*************************************/
/* Name: IB_BTH_CNP
 * Size: 96 bits
 * Description: Base Transport Header for CNP */

struct IB_BTH_CNP {
    struct P_Key_Block_Element	P_Key;    /* bit_offset:0 */    /* element_size: 16 */
    u_int8_t	TVer;    /* bit_offset:16 */    /* element_size: 4 */    /* Transport Header Version (is set to 0x0) */
    u_int8_t	PadCnt;    /* bit_offset:20 */    /* element_size: 2 */    /* Pad Count */
    u_int8_t	MigReq;    /* bit_offset:22 */    /* element_size: 1 */    /* If zero, it means there is no change in current migration state */
    u_int8_t	SE;    /* bit_offset:23 */    /* element_size: 1 */    /* Solicited Event */
    u_int8_t	OpCode;    /* bit_offset:24 */    /* element_size: 8 */    /* operation code (011_00100 - UD_SEND_only) */
    u_int32_t	DestQP;    /* bit_offset:32 */    /* element_size: 24 */    /* Destination QP */
    u_int8_t	Rsv6;    /* bit_offset:56 */    /* element_size: 6 */    /* Reserved(variant) - 6 bits. TX - 0, RX - ignore. Not included in ICRC */
    u_int8_t	Becn;    /* bit_offset:62 */    /* element_size: 1 */
    u_int8_t	Fecn;    /* bit_offset:63 */    /* element_size: 1 */
    u_int32_t	PSN;    /* bit_offset:64 */    /* element_size: 24 */    /* Packet Sequence Number */
    u_int8_t	Rsv7;    /* bit_offset:88 */    /* element_size: 7 */    /* TX - 0, RX - ignore. This field is included in the ICRC */
    u_int8_t	AckReq;    /* bit_offset:95 */    /* element_size: 1 */    /* Requests responder to schedule an ACK on the associated QP */
};


/*************************************/
/* Name: IB_BTH
 * Size: 96 bits
 * Description: Base Transport Header */

struct IB_BTH {
    struct P_Key_Block_Element	P_Key;    /* bit_offset:0 */    /* element_size: 16 */
    u_int8_t	TVer;    /* bit_offset:16 */    /* element_size: 4 */    /* Transport Header Version (is set to 0x0) */
    u_int8_t	PadCnt;    /* bit_offset:20 */    /* element_size: 2 */    /* Pad Count */
    u_int8_t	MigReq;    /* bit_offset:22 */    /* element_size: 1 */    /* If zero, it means there is no change in current migration state */
    u_int8_t	SE;    /* bit_offset:23 */    /* element_size: 1 */    /* Solicited Event */
    u_int8_t	OpCode;    /* bit_offset:24 */    /* element_size: 8 */    /* operation code (011_00100 - UD_SEND_only) */
    u_int32_t	DestQP;    /* bit_offset:32 */    /* element_size: 24 */    /* Destination QP */
    u_int8_t	Rsv8;    /* bit_offset:56 */    /* element_size: 8 */    /* Reserved(variant) - 8 bits. TX - 0, RX - ignore. Not included in ICRC */
    u_int32_t	PSN;    /* bit_offset:64 */    /* element_size: 24 */    /* Packet Sequence Number */
    u_int8_t	Rsv7;    /* bit_offset:88 */    /* element_size: 7 */    /* TX - 0, RX - ignore. This field is included in the ICRC */
    u_int8_t	AckReq;    /* bit_offset:95 */    /* element_size: 1 */    /* Requests responder to schedule an ACK on the associated QP */
};


/*************************************/
/* Name: IB_GRH
 * Size: 320 bits
 * Description: Global Routing Header */

struct IB_GRH {
    u_int32_t	FlowLabel;    /* bit_offset:0 */    /* element_size: 20 */    /* This field identifies sequences of packets requiring special handling. */
    u_int8_t	TClass;    /* bit_offset:20 */    /* element_size: 8 */    /* This field is used by IBA to communicate global service level. */
    u_int8_t	IPVer;    /* bit_offset:28 */    /* element_size: 4 */    /* This field indicates version of the GRH */
    u_int8_t	HopLmt;    /* bit_offset:32 */    /* element_size: 8 */    /* This field sets a strict bound on the number of hops 
                                                 between subnets a packet can make before being discarded. 
                                                 This is enforced only by routers. */
    u_int8_t	NxtHdr;    /* bit_offset:40 */    /* element_size: 8 */    /* This field identifies the header following the GRH. 
                                                 This field is included for compatibility with IPV6 headers. 
                                                 It should indicate IBA transport. */
    u_int16_t	PayLen;    /* bit_offset:48 */    /* element_size: 16 */    /* For an IBA packet this field specifies the number of bytes 
                                                 starting from the first byte after the GRH, 
                                                 up to and including the last byte of the ICRC. 
                                                 For a raw IPv6 datagram this field specifies the number 
                                                 of bytes starting from the first byte after the GRH, 
                                                 up to but not including either the VCRC or any padding, 
                                                 to achieve a multiple of 4 byte packet length. 
                                                 For raw IPv6 datagrams padding is determined 
                                                 from the lower 2 bits of this GRH:PayLen field. 
                                                 Note: GRH:PayLen is different from LRH:PkyLen. */
    struct GID_Block_Element	SGID;    /* bit_offset:64 */    /* element_size: 128 */
    struct GID_Block_Element	DGID;    /* bit_offset:192 */    /* element_size: 128 */
};


/*************************************/
/* Name: IB_RWH
 * Size: 32 bits
 * Description: Raw Header */

struct IB_RWH {
    u_int16_t	EtherType;    /* bit_offset:0 */    /* element_size: 16 */
    u_int16_t	reserved0;    /* bit_offset:16 */    /* element_size: 16 */
};


/*************************************/
/* Name: IB_LRH
 * Size: 64 bits
 * Description: Local Routed Header */

struct IB_LRH {
    u_int16_t	DLID;    /* bit_offset:0 */    /* element_size: 16 */    /* Destination Local Identifier */
    u_int8_t	LNH;    /* bit_offset:16 */    /* element_size: 2 */    /* Link Next Header (3-IBA/GRH, 2-IBA/BTH, 1-Raw/IPv6, 0-Raw/RWH) */
    u_int8_t	Rsv2;    /* bit_offset:18 */    /* element_size: 2 */    /* The 2 -bit Reserve field, shall be transmited as 00 and ignored on receive */
    u_int8_t	Sl;    /* bit_offset:20 */    /* element_size: 4 */    /* Service Level */
    u_int8_t	LVer;    /* bit_offset:24 */    /* element_size: 4 */    /* Link Version (shall be set to 0x0) */
    u_int8_t	Vl;    /* bit_offset:28 */    /* element_size: 4 */    /* Virtual Lane */
    u_int16_t	SLID;    /* bit_offset:32 */    /* element_size: 16 */    /* Source Local Identifier */
    u_int16_t	PktLen;    /* bit_offset:48 */    /* element_size: 11 */    /* Packet Length (from LRH upto Variant CRC in 4 byte words) */
    u_int8_t	Rsv5;    /* bit_offset:59 */    /* element_size: 5 */    /* The 5 - bit Reserve shall be transmited as 0x0 and ignored on receive */
};


/*************************************/
/* Name: PACKETS_EXTERNAL
 * Size: 67108864 bits
 * Description: PACKETS_EXTERNAL */

struct PACKETS_EXTERNAL {
    struct IB_LRH	IB_LRH;    /* bit_offset:0 */    /* element_size: 64 */
    u_int32_t	reserved0;    /* bit_offset:64 */    /* element_size: 10176 */
    struct IB_RWH	IB_RWH;    /* bit_offset:10240 */    /* element_size: 32 */
    u_int32_t	reserved1;    /* bit_offset:10272 */    /* element_size: 22496 */
    struct IB_GRH	IB_GRH;    /* bit_offset:32768 */    /* element_size: 320 */
    u_int32_t	reserved2;    /* bit_offset:33088 */    /* element_size: 9920 */
    struct IB_BTH	IB_BTH;    /* bit_offset:43008 */    /* element_size: 96 */
    u_int32_t	reserved3;    /* bit_offset:43104 */    /* element_size: 22432 */
    struct IB_BTH_CNP	IB_BTH_CNP;    /* bit_offset:65536 */    /* element_size: 96 */
    u_int32_t	reserved4;    /* bit_offset:65632 */    /* element_size: 10144 */
    struct IB_RDETH	IB_RDETH;    /* bit_offset:75776 */    /* element_size: 32 */
    u_int32_t	reserved5;    /* bit_offset:75808 */    /* element_size: 22496 */
    struct IB_DETH	IB_DETH;    /* bit_offset:98304 */    /* element_size: 64 */
    u_int32_t	reserved6;    /* bit_offset:98368 */    /* element_size: 10176 */
    struct IB_RETH	IB_RETH;    /* bit_offset:108544 */    /* element_size: 128 */
    u_int32_t	reserved7;    /* bit_offset:108672 */    /* element_size: 22400 */
    struct IB_AtomicETH	IB_AtomicETH;    /* bit_offset:131072 */    /* element_size: 224 */
    u_int32_t	reserved8;    /* bit_offset:131296 */    /* element_size: 10016 */
    struct IB_AETH	IB_AETH;    /* bit_offset:141312 */    /* element_size: 32 */
    u_int32_t	reserved9;    /* bit_offset:141344 */    /* element_size: 22496 */
    struct IB_AtomicAckETH	IB_AtomicAckETH;    /* bit_offset:163840 */    /* element_size: 64 */
    u_int32_t	reserved10;    /* bit_offset:163904 */    /* element_size: 10176 */
    struct IB_ImmDt	IB_ImmDt;    /* bit_offset:174080 */    /* element_size: 32 */
    u_int32_t	reserved11;    /* bit_offset:174112 */    /* element_size: 22496 */
    struct IB_IETH	IB_IETH;    /* bit_offset:196608 */    /* element_size: 32 */
    u_int32_t	reserved12;    /* bit_offset:196640 */    /* element_size: 2016 */
    struct ICRC	ICRC;    /* bit_offset:198656 */    /* element_size: 32 */
    u_int32_t	reserved13;    /* bit_offset:198688 */    /* element_size: 2016 */
    struct VCRC	VCRC;    /* bit_offset:200704 */    /* element_size: 32 */
    u_int32_t	reserved14;    /* bit_offset:200736 */    /* element_size: 6112 */
    struct MAD_Header_Common	MAD_Header_Common;    /* bit_offset:206848 */    /* element_size: 192 */
    u_int32_t	reserved15;    /* bit_offset:207040 */    /* element_size: 1856 */
    struct MAD_Header_Common_With_RMPP	MAD_Header_Common_With_RMPP;    /* bit_offset:208896 */    /* element_size: 288 */
    u_int32_t	reserved16;    /* bit_offset:209184 */    /* element_size: 1760 */
    struct MAD_Header_SMP_Direct_Routed	MAD_Header_SMP_Direct_Routed;    /* bit_offset:210944 */    /* element_size: 192 */
    u_int32_t	reserved17;    /* bit_offset:211136 */    /* element_size: 51008 */
    struct MAD_SMP_Direct_Routed	MAD_SMP_Direct_Routed;    /* bit_offset:262144 */    /* element_size: 2048 */
    u_int32_t	reserved18;    /* bit_offset:264192 */    /* element_size: 8192 */
    struct MAD_SMP_LID_Routed	MAD_SMP_LID_Routed;    /* bit_offset:272384 */    /* element_size: 2048 */
    u_int32_t	reserved19;    /* bit_offset:274432 */    /* element_size: 20480 */
    struct MAD_PerformanceManagement	MAD_PerformanceManagement;    /* bit_offset:294912 */    /* element_size: 2048 */
    u_int32_t	reserved20;    /* bit_offset:296960 */    /* element_size: 8192 */
    struct MAD_SubnetAdministartion	MAD_SubnetAdministartion;    /* bit_offset:305152 */    /* element_size: 2048 */
    u_int32_t	reserved21;    /* bit_offset:307200 */    /* element_size: 20480 */
    struct SA_MCMMemberRecord	SA_MCMMemberRecord;    /* bit_offset:327680 */    /* element_size: 416 */
    u_int32_t	reserved22;    /* bit_offset:328096 */    /* element_size: 9824 */
    struct MAD_VendorSpec	MAD_VendorSpec;    /* bit_offset:337920 */    /* element_size: 2048 */
    u_int32_t	reserved23;    /* bit_offset:339968 */    /* element_size: 20480 */
    struct MAD_CongestionControl	MAD_CongestionControl;    /* bit_offset:360448 */    /* element_size: 2048 */
    u_int32_t	reserved24;    /* bit_offset:362496 */    /* element_size: 8192 */
    struct IB_ClassPortInfo	IB_ClassPortInfo;    /* bit_offset:370688 */    /* element_size: 576 */
    u_int32_t	reserved25;    /* bit_offset:371264 */    /* element_size: 153024 */
    struct SMP_MADS	SMP_MADS;    /* bit_offset:524288 */    /* element_size: 1048576 */
    struct PERFORMANCE_MADS	PERFORMANCE_MADS;    /* bit_offset:1572864 */    /* element_size: 1048576 */
    struct CONGESTION_CONTOL	CONGESTION_CONTOL;    /* bit_offset:2621440 */    /* element_size: 1048576 */
    struct VENDOR_SPECS	VENDOR_SPECS;    /* bit_offset:3670016 */    /* element_size: 1048576 */
    u_int32_t	reserved26;    /* bit_offset:4718592 */    /* element_size: 62390272 */
};

#endif /* packets_structs_H */
