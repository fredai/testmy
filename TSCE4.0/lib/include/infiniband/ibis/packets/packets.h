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

#ifndef packets_functions_H
#define packets_functions_H

#include "packets_types.h"


/*************************************/
/* Name: PSID_Block_Element
 * Size: 128 bits
 * Description: PSID_Block_Element */

u_int32_t PSID_Block_Element_pack(struct PSID_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void PSID_Block_Element_unpack(struct PSID_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void PSID_Block_Element_dump(struct PSID_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: GID_Block_Element
 * Size: 128 bits
 * Description: GID_Block_Element */

u_int32_t GID_Block_Element_pack(struct GID_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void GID_Block_Element_unpack(struct GID_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void GID_Block_Element_dump(struct GID_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: uint64bit
 * Size: 64 bits
 * Description: uint64bit */

u_int32_t uint64bit_pack(struct uint64bit *data_to_pack, u_int8_t *packed_buffer);
void uint64bit_unpack(struct uint64bit *unpacked_data, u_int8_t *buffer_to_unpack);
void uint64bit_dump(struct uint64bit *data_to_print, FILE *out_port);


/*************************************/
/* Name: CCTI_Entry_ListElement
 * Size: 16 bits
 * Description: CCTI_Entry_ListElement */

u_int32_t CCTI_Entry_ListElement_pack(struct CCTI_Entry_ListElement *data_to_pack, u_int8_t *packed_buffer);
void CCTI_Entry_ListElement_unpack(struct CCTI_Entry_ListElement *unpacked_data, u_int8_t *buffer_to_unpack);
void CCTI_Entry_ListElement_dump(struct CCTI_Entry_ListElement *data_to_print, FILE *out_port);


/*************************************/
/* Name: CACongestionEntryListElement
 * Size: 64 bits
 * Description: CACongestionEntryListElement */

u_int32_t CACongestionEntryListElement_pack(struct CACongestionEntryListElement *data_to_pack, u_int8_t *packed_buffer);
void CACongestionEntryListElement_unpack(struct CACongestionEntryListElement *unpacked_data, u_int8_t *buffer_to_unpack);
void CACongestionEntryListElement_dump(struct CACongestionEntryListElement *data_to_print, FILE *out_port);


/*************************************/
/* Name: CongestionLogEventListCAElement
 * Size: 128 bits
 * Description:  */

u_int32_t CongestionLogEventListCAElement_pack(struct CongestionLogEventListCAElement *data_to_pack, u_int8_t *packed_buffer);
void CongestionLogEventListCAElement_unpack(struct CongestionLogEventListCAElement *unpacked_data, u_int8_t *buffer_to_unpack);
void CongestionLogEventListCAElement_dump(struct CongestionLogEventListCAElement *data_to_print, FILE *out_port);


/*************************************/
/* Name: CongestionEntryListSwitchElement
 * Size: 96 bits
 * Description:  */

u_int32_t CongestionEntryListSwitchElement_pack(struct CongestionEntryListSwitchElement *data_to_pack, u_int8_t *packed_buffer);
void CongestionEntryListSwitchElement_unpack(struct CongestionEntryListSwitchElement *unpacked_data, u_int8_t *buffer_to_unpack);
void CongestionEntryListSwitchElement_dump(struct CongestionEntryListSwitchElement *data_to_print, FILE *out_port);


/*************************************/
/* Name: SWInfo_Block_Element
 * Size: 256 bits
 * Description: SW Info */

u_int32_t SWInfo_Block_Element_pack(struct SWInfo_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void SWInfo_Block_Element_unpack(struct SWInfo_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void SWInfo_Block_Element_dump(struct SWInfo_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: FWInfo_Block_Element
 * Size: 512 bits
 * Description: FW Info */

u_int32_t FWInfo_Block_Element_pack(struct FWInfo_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void FWInfo_Block_Element_unpack(struct FWInfo_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void FWInfo_Block_Element_dump(struct FWInfo_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: HWInfo_Block_Element
 * Size: 256 bits
 * Description: HW Info */

u_int32_t HWInfo_Block_Element_pack(struct HWInfo_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void HWInfo_Block_Element_unpack(struct HWInfo_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void HWInfo_Block_Element_dump(struct HWInfo_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_KeyViolation
 * Size: 448 bits
 * Description:  */

u_int32_t CC_KeyViolation_pack(struct CC_KeyViolation *data_to_pack, u_int8_t *packed_buffer);
void CC_KeyViolation_unpack(struct CC_KeyViolation *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_KeyViolation_dump(struct CC_KeyViolation *data_to_print, FILE *out_port);


/*************************************/
/* Name: CCTI_Entry_List
 * Size: 1024 bits
 * Description:  */

u_int32_t CCTI_Entry_List_pack(struct CCTI_Entry_List *data_to_pack, u_int8_t *packed_buffer);
void CCTI_Entry_List_unpack(struct CCTI_Entry_List *unpacked_data, u_int8_t *buffer_to_unpack);
void CCTI_Entry_List_dump(struct CCTI_Entry_List *data_to_print, FILE *out_port);


/*************************************/
/* Name: CACongestionEntryList
 * Size: 1024 bits
 * Description:  */

u_int32_t CACongestionEntryList_pack(struct CACongestionEntryList *data_to_pack, u_int8_t *packed_buffer);
void CACongestionEntryList_unpack(struct CACongestionEntryList *unpacked_data, u_int8_t *buffer_to_unpack);
void CACongestionEntryList_dump(struct CACongestionEntryList *data_to_print, FILE *out_port);


/*************************************/
/* Name: SwitchPortCongestionSettingElement
 * Size: 32 bits
 * Description: SwitchPortCongestionSettingElement */

u_int32_t SwitchPortCongestionSettingElement_pack(struct SwitchPortCongestionSettingElement *data_to_pack, u_int8_t *packed_buffer);
void SwitchPortCongestionSettingElement_unpack(struct SwitchPortCongestionSettingElement *unpacked_data, u_int8_t *buffer_to_unpack);
void SwitchPortCongestionSettingElement_dump(struct SwitchPortCongestionSettingElement *data_to_print, FILE *out_port);


/*************************************/
/* Name: UINT256
 * Size: 256 bits
 * Description: UINT256 */

u_int32_t UINT256_pack(struct UINT256 *data_to_pack, u_int8_t *packed_buffer);
void UINT256_unpack(struct UINT256 *unpacked_data, u_int8_t *buffer_to_unpack);
void UINT256_dump(struct UINT256 *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_SwitchCongestionSetting_Control_Map
 * Size: 32 bits
 * Description:  */

u_int32_t CC_SwitchCongestionSetting_Control_Map_pack(struct CC_SwitchCongestionSetting_Control_Map *data_to_pack, u_int8_t *packed_buffer);
void CC_SwitchCongestionSetting_Control_Map_unpack(struct CC_SwitchCongestionSetting_Control_Map *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_SwitchCongestionSetting_Control_Map_dump(struct CC_SwitchCongestionSetting_Control_Map *data_to_print, FILE *out_port);


/*************************************/
/* Name: CongestionLogEventListCA
 * Size: 1664 bits
 * Description: array of at most 13 recent events */

u_int32_t CongestionLogEventListCA_pack(struct CongestionLogEventListCA *data_to_pack, u_int8_t *packed_buffer);
void CongestionLogEventListCA_unpack(struct CongestionLogEventListCA *unpacked_data, u_int8_t *buffer_to_unpack);
void CongestionLogEventListCA_dump(struct CongestionLogEventListCA *data_to_print, FILE *out_port);


/*************************************/
/* Name: CongestionEntryListSwitch
 * Size: 1440 bits
 * Description: array of at most 15 recent events */

u_int32_t CongestionEntryListSwitch_pack(struct CongestionEntryListSwitch *data_to_pack, u_int8_t *packed_buffer);
void CongestionEntryListSwitch_unpack(struct CongestionEntryListSwitch *unpacked_data, u_int8_t *buffer_to_unpack);
void CongestionEntryListSwitch_dump(struct CongestionEntryListSwitch *data_to_print, FILE *out_port);


/*************************************/
/* Name: PortCountersExtended_Mask_Block_Element
 * Size: 16 bits
 * Description:  */

u_int32_t PortCountersExtended_Mask_Block_Element_pack(struct PortCountersExtended_Mask_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void PortCountersExtended_Mask_Block_Element_unpack(struct PortCountersExtended_Mask_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void PortCountersExtended_Mask_Block_Element_dump(struct PortCountersExtended_Mask_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: PortCounters_Mask2_Block_Element
 * Size: 8 bits
 * Description:  */

u_int32_t PortCounters_Mask2_Block_Element_pack(struct PortCounters_Mask2_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void PortCounters_Mask2_Block_Element_unpack(struct PortCounters_Mask2_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void PortCounters_Mask2_Block_Element_dump(struct PortCounters_Mask2_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: PortCounters_Mask_Block_Element
 * Size: 16 bits
 * Description:  */

u_int32_t PortCounters_Mask_Block_Element_pack(struct PortCounters_Mask_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void PortCounters_Mask_Block_Element_unpack(struct PortCounters_Mask_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void PortCounters_Mask_Block_Element_dump(struct PortCounters_Mask_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: LID_Port_Block_Element
 * Size: 32 bits
 * Description:  */

u_int32_t LID_Port_Block_Element_pack(struct LID_Port_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void LID_Port_Block_Element_unpack(struct LID_Port_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void LID_Port_Block_Element_dump(struct LID_Port_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: VL_Weight_Block_Element
 * Size: 16 bits
 * Description: VL Weight */

u_int32_t VL_Weight_Block_Element_pack(struct VL_Weight_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void VL_Weight_Block_Element_unpack(struct VL_Weight_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void VL_Weight_Block_Element_dump(struct VL_Weight_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: P_Key_Block_Element
 * Size: 16 bits
 * Description: Partition Key */

u_int32_t P_Key_Block_Element_pack(struct P_Key_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void P_Key_Block_Element_unpack(struct P_Key_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void P_Key_Block_Element_dump(struct P_Key_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: GUID_Block_Element
 * Size: 512 bits
 * Description: GUID_Block_Element */

u_int32_t GUID_Block_Element_pack(struct GUID_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void GUID_Block_Element_unpack(struct GUID_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void GUID_Block_Element_dump(struct GUID_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: TID_Block_Element
 * Size: 64 bits
 * Description: TID_Block_Element */

u_int32_t TID_Block_Element_pack(struct TID_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void TID_Block_Element_unpack(struct TID_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void TID_Block_Element_dump(struct TID_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: VendorSpec_GeneralInfo
 * Size: 1024 bits
 * Description:  */

u_int32_t VendorSpec_GeneralInfo_pack(struct VendorSpec_GeneralInfo *data_to_pack, u_int8_t *packed_buffer);
void VendorSpec_GeneralInfo_unpack(struct VendorSpec_GeneralInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void VendorSpec_GeneralInfo_dump(struct VendorSpec_GeneralInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_Notice
 * Size: 640 bits
 * Description:  */

u_int32_t CC_Notice_pack(struct CC_Notice *data_to_pack, u_int8_t *packed_buffer);
void CC_Notice_unpack(struct CC_Notice *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_Notice_dump(struct CC_Notice *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_TimeStamp
 * Size: 32 bits
 * Description:  */

u_int32_t CC_TimeStamp_pack(struct CC_TimeStamp *data_to_pack, u_int8_t *packed_buffer);
void CC_TimeStamp_unpack(struct CC_TimeStamp *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_TimeStamp_dump(struct CC_TimeStamp *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_CongestionControlTable
 * Size: 1056 bits
 * Description:  */

u_int32_t CC_CongestionControlTable_pack(struct CC_CongestionControlTable *data_to_pack, u_int8_t *packed_buffer);
void CC_CongestionControlTable_unpack(struct CC_CongestionControlTable *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_CongestionControlTable_dump(struct CC_CongestionControlTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_CACongestionSetting
 * Size: 1056 bits
 * Description:  */

u_int32_t CC_CACongestionSetting_pack(struct CC_CACongestionSetting *data_to_pack, u_int8_t *packed_buffer);
void CC_CACongestionSetting_unpack(struct CC_CACongestionSetting *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_CACongestionSetting_dump(struct CC_CACongestionSetting *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_SwitchPortCongestionSetting
 * Size: 1024 bits
 * Description:  */

u_int32_t CC_SwitchPortCongestionSetting_pack(struct CC_SwitchPortCongestionSetting *data_to_pack, u_int8_t *packed_buffer);
void CC_SwitchPortCongestionSetting_unpack(struct CC_SwitchPortCongestionSetting *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_SwitchPortCongestionSetting_dump(struct CC_SwitchPortCongestionSetting *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_SwitchCongestionSetting
 * Size: 608 bits
 * Description:  */

u_int32_t CC_SwitchCongestionSetting_pack(struct CC_SwitchCongestionSetting *data_to_pack, u_int8_t *packed_buffer);
void CC_SwitchCongestionSetting_unpack(struct CC_SwitchCongestionSetting *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_SwitchCongestionSetting_dump(struct CC_SwitchCongestionSetting *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_CongestionLogCA
 * Size: 1760 bits
 * Description:  */

u_int32_t CC_CongestionLogCA_pack(struct CC_CongestionLogCA *data_to_pack, u_int8_t *packed_buffer);
void CC_CongestionLogCA_unpack(struct CC_CongestionLogCA *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_CongestionLogCA_dump(struct CC_CongestionLogCA *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_CongestionLogSwitch
 * Size: 1760 bits
 * Description:  */

u_int32_t CC_CongestionLogSwitch_pack(struct CC_CongestionLogSwitch *data_to_pack, u_int8_t *packed_buffer);
void CC_CongestionLogSwitch_unpack(struct CC_CongestionLogSwitch *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_CongestionLogSwitch_dump(struct CC_CongestionLogSwitch *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_CongestionKeyInfo
 * Size: 128 bits
 * Description:  */

u_int32_t CC_CongestionKeyInfo_pack(struct CC_CongestionKeyInfo *data_to_pack, u_int8_t *packed_buffer);
void CC_CongestionKeyInfo_unpack(struct CC_CongestionKeyInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_CongestionKeyInfo_dump(struct CC_CongestionKeyInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_CongestionInfo
 * Size: 32 bits
 * Description:  */

u_int32_t CC_CongestionInfo_pack(struct CC_CongestionInfo *data_to_pack, u_int8_t *packed_buffer);
void CC_CongestionInfo_unpack(struct CC_CongestionInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_CongestionInfo_dump(struct CC_CongestionInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: PM_PortSamplesResult
 * Size: 1536 bits
 * Description: 0x40 */

u_int32_t PM_PortSamplesResult_pack(struct PM_PortSamplesResult *data_to_pack, u_int8_t *packed_buffer);
void PM_PortSamplesResult_unpack(struct PM_PortSamplesResult *unpacked_data, u_int8_t *buffer_to_unpack);
void PM_PortSamplesResult_dump(struct PM_PortSamplesResult *data_to_print, FILE *out_port);


/*************************************/
/* Name: PM_PortRcvErrorDetails
 * Size: 512 bits
 * Description:  */

u_int32_t PM_PortRcvErrorDetails_pack(struct PM_PortRcvErrorDetails *data_to_pack, u_int8_t *packed_buffer);
void PM_PortRcvErrorDetails_unpack(struct PM_PortRcvErrorDetails *unpacked_data, u_int8_t *buffer_to_unpack);
void PM_PortRcvErrorDetails_dump(struct PM_PortRcvErrorDetails *data_to_print, FILE *out_port);


/*************************************/
/* Name: PM_PortXmitDiscardDetails
 * Size: 512 bits
 * Description:  */

u_int32_t PM_PortXmitDiscardDetails_pack(struct PM_PortXmitDiscardDetails *data_to_pack, u_int8_t *packed_buffer);
void PM_PortXmitDiscardDetails_unpack(struct PM_PortXmitDiscardDetails *unpacked_data, u_int8_t *buffer_to_unpack);
void PM_PortXmitDiscardDetails_dump(struct PM_PortXmitDiscardDetails *data_to_print, FILE *out_port);


/*************************************/
/* Name: PM_PortCountersExtended
 * Size: 576 bits
 * Description:  */

u_int32_t PM_PortCountersExtended_pack(struct PM_PortCountersExtended *data_to_pack, u_int8_t *packed_buffer);
void PM_PortCountersExtended_unpack(struct PM_PortCountersExtended *unpacked_data, u_int8_t *buffer_to_unpack);
void PM_PortCountersExtended_dump(struct PM_PortCountersExtended *data_to_print, FILE *out_port);


/*************************************/
/* Name: PM_PortCounters
 * Size: 352 bits
 * Description:  */

u_int32_t PM_PortCounters_pack(struct PM_PortCounters *data_to_pack, u_int8_t *packed_buffer);
void PM_PortCounters_unpack(struct PM_PortCounters *unpacked_data, u_int8_t *buffer_to_unpack);
void PM_PortCounters_dump(struct PM_PortCounters *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_MlnxExtPortInfo
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_MlnxExtPortInfo_pack(struct SMP_MlnxExtPortInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_MlnxExtPortInfo_unpack(struct SMP_MlnxExtPortInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_MlnxExtPortInfo_dump(struct SMP_MlnxExtPortInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_LedInfo
 * Size: 32 bits
 * Description:  */

u_int32_t SMP_LedInfo_pack(struct SMP_LedInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_LedInfo_unpack(struct SMP_LedInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_LedInfo_dump(struct SMP_LedInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_SMInfo
 * Size: 192 bits
 * Description:  */

u_int32_t SMP_SMInfo_pack(struct SMP_SMInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_SMInfo_unpack(struct SMP_SMInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_SMInfo_dump(struct SMP_SMInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_MulticastForwardingTable
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_MulticastForwardingTable_pack(struct SMP_MulticastForwardingTable *data_to_pack, u_int8_t *packed_buffer);
void SMP_MulticastForwardingTable_unpack(struct SMP_MulticastForwardingTable *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_MulticastForwardingTable_dump(struct SMP_MulticastForwardingTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_RandomForwardingTable
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_RandomForwardingTable_pack(struct SMP_RandomForwardingTable *data_to_pack, u_int8_t *packed_buffer);
void SMP_RandomForwardingTable_unpack(struct SMP_RandomForwardingTable *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_RandomForwardingTable_dump(struct SMP_RandomForwardingTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_LinearForwardingTable
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_LinearForwardingTable_pack(struct SMP_LinearForwardingTable *data_to_pack, u_int8_t *packed_buffer);
void SMP_LinearForwardingTable_unpack(struct SMP_LinearForwardingTable *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_LinearForwardingTable_dump(struct SMP_LinearForwardingTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_VLArbitrationTable
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_VLArbitrationTable_pack(struct SMP_VLArbitrationTable *data_to_pack, u_int8_t *packed_buffer);
void SMP_VLArbitrationTable_unpack(struct SMP_VLArbitrationTable *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_VLArbitrationTable_dump(struct SMP_VLArbitrationTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_SLToVLMappingTable
 * Size: 64 bits
 * Description:  */

u_int32_t SMP_SLToVLMappingTable_pack(struct SMP_SLToVLMappingTable *data_to_pack, u_int8_t *packed_buffer);
void SMP_SLToVLMappingTable_unpack(struct SMP_SLToVLMappingTable *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_SLToVLMappingTable_dump(struct SMP_SLToVLMappingTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_PKeyTable
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_PKeyTable_pack(struct SMP_PKeyTable *data_to_pack, u_int8_t *packed_buffer);
void SMP_PKeyTable_unpack(struct SMP_PKeyTable *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_PKeyTable_dump(struct SMP_PKeyTable *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_PortInfo
 * Size: 512 bits
 * Description: 0x38 */

u_int32_t SMP_PortInfo_pack(struct SMP_PortInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_PortInfo_unpack(struct SMP_PortInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_PortInfo_dump(struct SMP_PortInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_GUIDInfo
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_GUIDInfo_pack(struct SMP_GUIDInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_GUIDInfo_unpack(struct SMP_GUIDInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_GUIDInfo_dump(struct SMP_GUIDInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_SwitchInfo
 * Size: 512 bits
 * Description:  */

u_int32_t SMP_SwitchInfo_pack(struct SMP_SwitchInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_SwitchInfo_unpack(struct SMP_SwitchInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_SwitchInfo_dump(struct SMP_SwitchInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_NodeInfo
 * Size: 320 bits
 * Description: 0x28 */

u_int32_t SMP_NodeInfo_pack(struct SMP_NodeInfo *data_to_pack, u_int8_t *packed_buffer);
void SMP_NodeInfo_unpack(struct SMP_NodeInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_NodeInfo_dump(struct SMP_NodeInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_NodeDesc
 * Size: 512 bits
 * Description: SMP_NodeDesc */

u_int32_t SMP_NodeDesc_pack(struct SMP_NodeDesc *data_to_pack, u_int8_t *packed_buffer);
void SMP_NodeDesc_unpack(struct SMP_NodeDesc *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_NodeDesc_dump(struct SMP_NodeDesc *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_Mgt_Data_Block_Element
 * Size: 1536 bits
 * Description: CC_Mgt_Data_Block_Element */

u_int32_t CC_Mgt_Data_Block_Element_pack(struct CC_Mgt_Data_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void CC_Mgt_Data_Block_Element_unpack(struct CC_Mgt_Data_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_Mgt_Data_Block_Element_dump(struct CC_Mgt_Data_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: CC_Log_Data_Block_Element
 * Size: 256 bits
 * Description: CC_Log_Data_Block_Element */

u_int32_t CC_Log_Data_Block_Element_pack(struct CC_Log_Data_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void CC_Log_Data_Block_Element_unpack(struct CC_Log_Data_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void CC_Log_Data_Block_Element_dump(struct CC_Log_Data_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_Header_Common
 * Size: 192 bits
 * Description: MAD Header Common */

u_int32_t MAD_Header_Common_pack(struct MAD_Header_Common *data_to_pack, u_int8_t *packed_buffer);
void MAD_Header_Common_unpack(struct MAD_Header_Common *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_Header_Common_dump(struct MAD_Header_Common *data_to_print, FILE *out_port);


/*************************************/
/* Name: VendorSpecific_MAD_Data_Block_Element
 * Size: 1792 bits
 * Description: VendorSpecific_MAD_Data_Block_Element */

u_int32_t VendorSpecific_MAD_Data_Block_Element_pack(struct VendorSpecific_MAD_Data_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void VendorSpecific_MAD_Data_Block_Element_unpack(struct VendorSpecific_MAD_Data_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void VendorSpecific_MAD_Data_Block_Element_dump(struct VendorSpecific_MAD_Data_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: SubnetAdministartion_MAD_Data_Block_Element
 * Size: 1600 bits
 * Description: SubnetAdministartion_MAD_Data_Block_Element */

u_int32_t SubnetAdministartion_MAD_Data_Block_Element_pack(struct SubnetAdministartion_MAD_Data_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void SubnetAdministartion_MAD_Data_Block_Element_unpack(struct SubnetAdministartion_MAD_Data_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void SubnetAdministartion_MAD_Data_Block_Element_dump(struct SubnetAdministartion_MAD_Data_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_Header_Common_With_RMPP
 * Size: 288 bits
 * Description: MAD Header Common With RMPP */

u_int32_t MAD_Header_Common_With_RMPP_pack(struct MAD_Header_Common_With_RMPP *data_to_pack, u_int8_t *packed_buffer);
void MAD_Header_Common_With_RMPP_unpack(struct MAD_Header_Common_With_RMPP *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_Header_Common_With_RMPP_dump(struct MAD_Header_Common_With_RMPP *data_to_print, FILE *out_port);


/*************************************/
/* Name: PerfManagement_MAD_Data_Block_Element
 * Size: 1536 bits
 * Description: PerfManagement_MAD_Data_Block_Element */

u_int32_t PerfManagement_MAD_Data_Block_Element_pack(struct PerfManagement_MAD_Data_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void PerfManagement_MAD_Data_Block_Element_unpack(struct PerfManagement_MAD_Data_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void PerfManagement_MAD_Data_Block_Element_dump(struct PerfManagement_MAD_Data_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_MAD_Data_Block_Element
 * Size: 512 bits
 * Description: SMP_MAD_Data_Block_Element */

u_int32_t SMP_MAD_Data_Block_Element_pack(struct SMP_MAD_Data_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void SMP_MAD_Data_Block_Element_unpack(struct SMP_MAD_Data_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_MAD_Data_Block_Element_dump(struct SMP_MAD_Data_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: DirRPath_Block_Element
 * Size: 512 bits
 * Description: DirRPath_Block_Element */

u_int32_t DirRPath_Block_Element_pack(struct DirRPath_Block_Element *data_to_pack, u_int8_t *packed_buffer);
void DirRPath_Block_Element_unpack(struct DirRPath_Block_Element *unpacked_data, u_int8_t *buffer_to_unpack);
void DirRPath_Block_Element_dump(struct DirRPath_Block_Element *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_Header_SMP_Direct_Routed
 * Size: 192 bits
 * Description: MAD Header for SMP Direct Routed */

u_int32_t MAD_Header_SMP_Direct_Routed_pack(struct MAD_Header_SMP_Direct_Routed *data_to_pack, u_int8_t *packed_buffer);
void MAD_Header_SMP_Direct_Routed_unpack(struct MAD_Header_SMP_Direct_Routed *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_Header_SMP_Direct_Routed_dump(struct MAD_Header_SMP_Direct_Routed *data_to_print, FILE *out_port);


/*************************************/
/* Name: VENDOR_SPECS
 * Size: 1048576 bits
 * Description: VENDOR_SPECS */

u_int32_t VENDOR_SPECS_pack(struct VENDOR_SPECS *data_to_pack, u_int8_t *packed_buffer);
void VENDOR_SPECS_unpack(struct VENDOR_SPECS *unpacked_data, u_int8_t *buffer_to_unpack);
void VENDOR_SPECS_dump(struct VENDOR_SPECS *data_to_print, FILE *out_port);


/*************************************/
/* Name: CONGESTION_CONTOL
 * Size: 1048576 bits
 * Description: CONGESTION_CONTOL */

u_int32_t CONGESTION_CONTOL_pack(struct CONGESTION_CONTOL *data_to_pack, u_int8_t *packed_buffer);
void CONGESTION_CONTOL_unpack(struct CONGESTION_CONTOL *unpacked_data, u_int8_t *buffer_to_unpack);
void CONGESTION_CONTOL_dump(struct CONGESTION_CONTOL *data_to_print, FILE *out_port);


/*************************************/
/* Name: PERFORMANCE_MADS
 * Size: 1048576 bits
 * Description: PERFORMANCE_MADS */

u_int32_t PERFORMANCE_MADS_pack(struct PERFORMANCE_MADS *data_to_pack, u_int8_t *packed_buffer);
void PERFORMANCE_MADS_unpack(struct PERFORMANCE_MADS *unpacked_data, u_int8_t *buffer_to_unpack);
void PERFORMANCE_MADS_dump(struct PERFORMANCE_MADS *data_to_print, FILE *out_port);


/*************************************/
/* Name: SMP_MADS
 * Size: 1048576 bits
 * Description: SMP_MADS */

u_int32_t SMP_MADS_pack(struct SMP_MADS *data_to_pack, u_int8_t *packed_buffer);
void SMP_MADS_unpack(struct SMP_MADS *unpacked_data, u_int8_t *buffer_to_unpack);
void SMP_MADS_dump(struct SMP_MADS *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_ClassPortInfo
 * Size: 576 bits
 * Description:  */

u_int32_t IB_ClassPortInfo_pack(struct IB_ClassPortInfo *data_to_pack, u_int8_t *packed_buffer);
void IB_ClassPortInfo_unpack(struct IB_ClassPortInfo *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_ClassPortInfo_dump(struct IB_ClassPortInfo *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_CongestionControl
 * Size: 2048 bits
 * Description: MAD Performance management Data Format */

u_int32_t MAD_CongestionControl_pack(struct MAD_CongestionControl *data_to_pack, u_int8_t *packed_buffer);
void MAD_CongestionControl_unpack(struct MAD_CongestionControl *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_CongestionControl_dump(struct MAD_CongestionControl *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_VendorSpec
 * Size: 2048 bits
 * Description: MAD Vendor Specific Data Format */

u_int32_t MAD_VendorSpec_pack(struct MAD_VendorSpec *data_to_pack, u_int8_t *packed_buffer);
void MAD_VendorSpec_unpack(struct MAD_VendorSpec *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_VendorSpec_dump(struct MAD_VendorSpec *data_to_print, FILE *out_port);


/*************************************/
/* Name: SA_MCMMemberRecord
 * Size: 416 bits
 * Description:  */

u_int32_t SA_MCMMemberRecord_pack(struct SA_MCMMemberRecord *data_to_pack, u_int8_t *packed_buffer);
void SA_MCMMemberRecord_unpack(struct SA_MCMMemberRecord *unpacked_data, u_int8_t *buffer_to_unpack);
void SA_MCMMemberRecord_dump(struct SA_MCMMemberRecord *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_SubnetAdministartion
 * Size: 2048 bits
 * Description: MAD Subnet Administration Data Format */

u_int32_t MAD_SubnetAdministartion_pack(struct MAD_SubnetAdministartion *data_to_pack, u_int8_t *packed_buffer);
void MAD_SubnetAdministartion_unpack(struct MAD_SubnetAdministartion *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_SubnetAdministartion_dump(struct MAD_SubnetAdministartion *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_PerformanceManagement
 * Size: 2048 bits
 * Description: MAD Performance management Data Format */

u_int32_t MAD_PerformanceManagement_pack(struct MAD_PerformanceManagement *data_to_pack, u_int8_t *packed_buffer);
void MAD_PerformanceManagement_unpack(struct MAD_PerformanceManagement *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_PerformanceManagement_dump(struct MAD_PerformanceManagement *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_SMP_LID_Routed
 * Size: 2048 bits
 * Description: MAD SMP Data Format - Lid Routed */

u_int32_t MAD_SMP_LID_Routed_pack(struct MAD_SMP_LID_Routed *data_to_pack, u_int8_t *packed_buffer);
void MAD_SMP_LID_Routed_unpack(struct MAD_SMP_LID_Routed *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_SMP_LID_Routed_dump(struct MAD_SMP_LID_Routed *data_to_print, FILE *out_port);


/*************************************/
/* Name: MAD_SMP_Direct_Routed
 * Size: 2048 bits
 * Description: MAD SMP Data Format - Direct Routed */

u_int32_t MAD_SMP_Direct_Routed_pack(struct MAD_SMP_Direct_Routed *data_to_pack, u_int8_t *packed_buffer);
void MAD_SMP_Direct_Routed_unpack(struct MAD_SMP_Direct_Routed *unpacked_data, u_int8_t *buffer_to_unpack);
void MAD_SMP_Direct_Routed_dump(struct MAD_SMP_Direct_Routed *data_to_print, FILE *out_port);


/*************************************/
/* Name: VCRC
 * Size: 32 bits
 * Description: VCRC of IB Packet */

u_int32_t VCRC_pack(struct VCRC *data_to_pack, u_int8_t *packed_buffer);
void VCRC_unpack(struct VCRC *unpacked_data, u_int8_t *buffer_to_unpack);
void VCRC_dump(struct VCRC *data_to_print, FILE *out_port);


/*************************************/
/* Name: ICRC
 * Size: 32 bits
 * Description: ICRC of IB Packet */

u_int32_t ICRC_pack(struct ICRC *data_to_pack, u_int8_t *packed_buffer);
void ICRC_unpack(struct ICRC *unpacked_data, u_int8_t *buffer_to_unpack);
void ICRC_dump(struct ICRC *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_IETH
 * Size: 32 bits
 * Description: Invalidate Extended Transport Header */

u_int32_t IB_IETH_pack(struct IB_IETH *data_to_pack, u_int8_t *packed_buffer);
void IB_IETH_unpack(struct IB_IETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_IETH_dump(struct IB_IETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_ImmDt
 * Size: 32 bits
 * Description: Immediate Extended Transport Header */

u_int32_t IB_ImmDt_pack(struct IB_ImmDt *data_to_pack, u_int8_t *packed_buffer);
void IB_ImmDt_unpack(struct IB_ImmDt *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_ImmDt_dump(struct IB_ImmDt *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_AtomicAckETH
 * Size: 64 bits
 * Description: Atomic ACK Extended Transport Header */

u_int32_t IB_AtomicAckETH_pack(struct IB_AtomicAckETH *data_to_pack, u_int8_t *packed_buffer);
void IB_AtomicAckETH_unpack(struct IB_AtomicAckETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_AtomicAckETH_dump(struct IB_AtomicAckETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_AETH
 * Size: 32 bits
 * Description: ACK Extended Transport Header */

u_int32_t IB_AETH_pack(struct IB_AETH *data_to_pack, u_int8_t *packed_buffer);
void IB_AETH_unpack(struct IB_AETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_AETH_dump(struct IB_AETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_AtomicETH
 * Size: 224 bits
 * Description: Atomic Extended Transport Header */

u_int32_t IB_AtomicETH_pack(struct IB_AtomicETH *data_to_pack, u_int8_t *packed_buffer);
void IB_AtomicETH_unpack(struct IB_AtomicETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_AtomicETH_dump(struct IB_AtomicETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_RETH
 * Size: 128 bits
 * Description: RDMA Extended Transport Header Fields */

u_int32_t IB_RETH_pack(struct IB_RETH *data_to_pack, u_int8_t *packed_buffer);
void IB_RETH_unpack(struct IB_RETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_RETH_dump(struct IB_RETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_DETH
 * Size: 64 bits
 * Description: Datagram Extended Transport Header */

u_int32_t IB_DETH_pack(struct IB_DETH *data_to_pack, u_int8_t *packed_buffer);
void IB_DETH_unpack(struct IB_DETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_DETH_dump(struct IB_DETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_RDETH
 * Size: 32 bits
 * Description: Reliable Datagram Extended Transport Header */

u_int32_t IB_RDETH_pack(struct IB_RDETH *data_to_pack, u_int8_t *packed_buffer);
void IB_RDETH_unpack(struct IB_RDETH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_RDETH_dump(struct IB_RDETH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_BTH_CNP
 * Size: 96 bits
 * Description: Base Transport Header for CNP */

u_int32_t IB_BTH_CNP_pack(struct IB_BTH_CNP *data_to_pack, u_int8_t *packed_buffer);
void IB_BTH_CNP_unpack(struct IB_BTH_CNP *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_BTH_CNP_dump(struct IB_BTH_CNP *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_BTH
 * Size: 96 bits
 * Description: Base Transport Header */

u_int32_t IB_BTH_pack(struct IB_BTH *data_to_pack, u_int8_t *packed_buffer);
void IB_BTH_unpack(struct IB_BTH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_BTH_dump(struct IB_BTH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_GRH
 * Size: 320 bits
 * Description: Global Routing Header */

u_int32_t IB_GRH_pack(struct IB_GRH *data_to_pack, u_int8_t *packed_buffer);
void IB_GRH_unpack(struct IB_GRH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_GRH_dump(struct IB_GRH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_RWH
 * Size: 32 bits
 * Description: Raw Header */

u_int32_t IB_RWH_pack(struct IB_RWH *data_to_pack, u_int8_t *packed_buffer);
void IB_RWH_unpack(struct IB_RWH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_RWH_dump(struct IB_RWH *data_to_print, FILE *out_port);


/*************************************/
/* Name: IB_LRH
 * Size: 64 bits
 * Description: Local Routed Header */

u_int32_t IB_LRH_pack(struct IB_LRH *data_to_pack, u_int8_t *packed_buffer);
void IB_LRH_unpack(struct IB_LRH *unpacked_data, u_int8_t *buffer_to_unpack);
void IB_LRH_dump(struct IB_LRH *data_to_print, FILE *out_port);


/*************************************/
/* Name: PACKETS_EXTERNAL
 * Size: 67108864 bits
 * Description: PACKETS_EXTERNAL */

u_int32_t PACKETS_EXTERNAL_pack(struct PACKETS_EXTERNAL *data_to_pack, u_int8_t *packed_buffer);
void PACKETS_EXTERNAL_unpack(struct PACKETS_EXTERNAL *unpacked_data, u_int8_t *buffer_to_unpack);
void PACKETS_EXTERNAL_dump(struct PACKETS_EXTERNAL *data_to_print, FILE *out_port);

#endif /* packets_functions_H */
