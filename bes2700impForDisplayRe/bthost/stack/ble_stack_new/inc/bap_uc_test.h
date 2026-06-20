/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __BAP_UC_TEST__
#define __BAP_UC_TEST__
#include "bluetooth.h"

#define BAP_UC_TEST_ENABLE  (0)

#if (BAP_UC_TEST_ENABLE)
#ifdef __cplusplus
extern "C" {
#endif

#include "generic_audio.h"
#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

/// FUCNTIONS DECLARATIONS
#ifdef AOB_MOBILE_ENABLED
#if (BAP_ASCC_ENABLE)
void bap_uc_test_ascc_codec_cfg(void);
void bap_uc_test_ascc_qos_cfg(void);
// For diferrent stream situation
void bap_uc_test_ascc_enable(uint8_t ase_num, const uint8_t *ase_id_list);
void bap_uc_test_ascc_disable(uint8_t ase_num, const uint8_t *ase_id_list);
void bap_uc_test_ascc_upd_md(uint8_t ase_num, const uint8_t *ase_id_list);
void bap_uc_test_ascc_release(void);
#endif /// (BAP_ASCC_ENABLE)
#if (BAP_ASCS_ENABLE)
void bap_uc_test_ascs_codec_cfg(uint8_t ase_num, const uint8_t *ase_id_list);
void bap_uc_test_ascs_disable(uint8_t ase_num, const uint8_t *ase_id_list);
void bap_uc_test_ascs_release(uint8_t ase_num, const uint8_t *ase_id_list);
#endif /// (BAP_ASCS_ENABLE)
#if (ACC_MCS_ENABLE)
void bap_uc_test_mcs_play(void);
void bap_uc_test_mcs_pause(void);
#endif /// (ACC_MCC_ENABLE)
#if (ARC_VCC_ENABLE)
void bap_uc_test_vcc_discovery(void);
void bap_uc_test_vcc_vol_up(void);
void bap_uc_test_vcc_vol_down(void);
#endif /// (ARC_VCS_ENABLE)
#endif /// AOB_MOBILE_ENABLED

#if (BAP_PACS_ENABLE)
void bap_uc_test_pacs_set_location(void);
void bap_uc_test_pacs_set_ava(void);
void bap_uc_test_pacs_set_supp(void);
void bap_uc_test_pacs_add_pac_record(void);
void bap_uc_test_pacs_del_pac_record(void);
#endif /// (BAP_PACS_ENABLE)
#if (ARC_VCS_ENABLE)
void bap_uc_test_vcs_vol_up(void);
void bap_uc_test_vcs_vol_down(void);
#endif /// (ARC_VCS_ENABLE)
#if (ACC_MCC_ENABLE)
void bap_uc_test_mcc_discovery(void);
void bap_uc_test_mcc_play(void);
void bap_uc_test_mcc_pause(void);
#endif /// (ACC_MCC_ENABLE)
#if (ACC_TBC_ENABLE)
void bap_uc_test_tbc_discovery(void);
void bap_uc_test_tbc_accept(void);
void bap_uc_test_tbc_terminate(void);
#endif /// (ACC_TBC_ENABLE)

void bap_uc_test_init(void);

#ifdef __cplusplus
}
#endif

#endif /// (BAP_UC_TEST_ENABLE)

#endif /// __BAP_UC_TEST__