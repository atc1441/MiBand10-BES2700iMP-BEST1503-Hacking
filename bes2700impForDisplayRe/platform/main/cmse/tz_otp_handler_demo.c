/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * interface.c      Secure/non-secure callable application code
 *
 * Version 1.0
 *    Initial Release
 *---------------------------------------------------------------------------*/
#include "string.h"
#include "plat_types.h"
#include <arm_cmse.h>     // CMSE definitions
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_norflash.h"

//#define SE_OTP_DEMO_TEST

#if defined(FLASH_SECURITY_REGISTER) && defined(SE_OTP_DEMO_TEST)
#ifdef FLASH1_CTRL_BASE
#define SEC_REG_FLASH_CTRL_ID_SHIFT         24
#define SEC_REG_FLASH_CTRL_ID_MASK          0xFF000000
#define FLASH_SEC_REG_ADDR_MASK             (~(SEC_REG_FLASH_CTRL_ID_MASK))

static enum HAL_FLASH_ID_T cur_flash_ctrl_id = HAL_FLASH_ID_0;

static int se_otp_demo_set_security_register_flash_ctrl_id(unsigned int addr)
{
#if (FLASH_BASE != FLASH1_BASE)
    uint32_t id;

    id = (addr & SEC_REG_FLASH_CTRL_ID_MASK) >> SEC_REG_FLASH_CTRL_ID_SHIFT;
    if (id < HAL_FLASH_ID_NUM) {
        cur_flash_ctrl_id = id;
    } else {
        cur_flash_ctrl_id = HAL_FLASH_ID_0;
    }
#endif
    return 0;
}
#endif

static int se_otp_demo_read_security_register(unsigned int addr, unsigned char *data, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    int ret;

    ret = se_otp_demo_set_security_register_flash_ctrl_id(addr);
    if (ret) {
        return ret;
    }
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_read(cur_flash_ctrl_id, addr, data, len);
}

static int se_otp_demo_erase_security_register(unsigned int addr, unsigned int len) {
#ifdef FLASH1_CTRL_BASE
    int ret;

    ret = se_otp_demo_set_security_register_flash_ctrl_id(addr);
    if (ret) {
        return ret;
    }
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_erase(cur_flash_ctrl_id, addr, len);
}

static int se_otp_demo_get_security_register_lock_status(unsigned int addr, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    int ret;

    ret = se_otp_demo_set_security_register_flash_ctrl_id(addr);
    if (ret) {
        return ret;
    }
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_is_locked(cur_flash_ctrl_id, addr, len);
}

static int se_otp_demo_lock_security_register(unsigned int addr, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    int ret;

    ret = se_otp_demo_set_security_register_flash_ctrl_id(addr);
    if (ret) {
        return ret;
    }
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_lock(cur_flash_ctrl_id, addr, len);
}

static int se_otp_demo_burn_security_register(unsigned int addr, const unsigned char *data, unsigned int len)
{
#ifdef FLASH1_CTRL_BASE
    int ret;

    ret = se_otp_demo_set_security_register_flash_ctrl_id(addr);
    if (ret) {
        return ret;
    }
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif
    return hal_norflash_security_register_write(cur_flash_ctrl_id, addr, data, len);
}

static int se_otp_demo_verify_security_register_data(unsigned int addr, unsigned char *load_buf, const unsigned char *data, unsigned int len)
{
    int ret;
    const unsigned char *fptr;
    const unsigned char *mptr;
    unsigned char fval;
    unsigned char mval;
    int i;
    uint32_t sr_addr;

#ifdef FLASH1_CTRL_BASE
    ret = se_otp_demo_set_security_register_flash_ctrl_id(addr);
    if (ret) {
        return ret;
    }
    addr &= FLASH_SEC_REG_ADDR_MASK;
#endif

    sr_addr = addr;
    ret = hal_norflash_security_register_read(cur_flash_ctrl_id, sr_addr, load_buf, len);
    if (ret) {
        TRACE(1,"*** Failed to read sec reg: %d", ret);
        return 1;
    }

    fptr = load_buf;
    mptr = data;
    for (i = 0; i < len; i++) {
        fval = *fptr++;
        mval = *mptr++;
        if (fval != mval) {
            --fptr;
            --mptr;
            TRACE(0, "*** Verify sec reg data failed: 0x%02X @ %p != 0x%02X @ %p", fval, (void *)(addr + (fptr - load_buf)), mval, (void *)mptr);
            return fval - mval;
        }
    }

    return 0;
}

#define SE_OTP_DEMO_WRITE_LEN   (20)
#define SE_OTP_DEMO_TOTAL_LEN   (40)
#define SE_OTP_DEMO_WRITE_START_ADDR    (0)
static unsigned char se_otp_demo_write_buf[SE_OTP_DEMO_TOTAL_LEN];

static void se_otp_demo_init(void)
{
    for(unsigned int i =0;i<SE_OTP_DEMO_WRITE_LEN;i++){
        se_otp_demo_write_buf[i] = (0x55<<4) + i;
        if(i >= SE_OTP_DEMO_WRITE_LEN/2){
            se_otp_demo_write_buf[i] = (0xaa<<4) + (i - SE_OTP_DEMO_WRITE_LEN/2);
        }
    }
}

static void se_otp_demo_test_check(unsigned char * addr, int len)
{
    TRACE(0,"%s",__func__);

    DUMP8("0x%02x ",addr,len);
}
#ifdef SE_OTP_DEMO_TEST
int se_otp_demo_test_bes(void)
{
    int ret = 0;
    uint8_t otp_buf[50] = {0};

    TRACE(0, "%s start",__func__);
    ret = se_otp_demo_read_security_register(THIRD_ALGO_PRIVATE_KEY_ADDR, otp_buf, 50);
    if (ret) {
        TRACE(0,"--failed-- ret = %d se_otp_demo_read_security_register",ret);
    }
    else
    {
        DUMP8("%02X ",otp_buf, 50);
        TRACE(0,"\n");
        TRACE(0, "[DOLBY]private key start addr = 0x%04X", THIRD_ALGO_PRIVATE_KEY_ADDR);
        TRACE(0, "[DOLBY]private key len        = %d bytes", otp_buf[3]<<24 | otp_buf[2]<<16| otp_buf[1]<<8| otp_buf[0]<<0);
    }
    TRACE(0, "%s end",__func__);

/*     
    unsigned char se_otp_buf[0x04BF];
    uint32_t key_offset = 0x031C;
    uint32_t len = 0x04BF;
    otp_read(key_offset, &len, sizeof(size_t));
    otp_read(key_offset+sizeof(size_t), se_otp_buf, len);
    DUMP8("0x%02x ",se_otp_buf, 0x04BF);
 */
    return 0;
}
#endif
int se_otp_demo_test(void)
{
    int ret = 0;

    TRACE(1,"%s start",__func__);

    TRACE(0,"checking");
    ret = se_otp_demo_read_security_register(SE_OTP_DEMO_WRITE_START_ADDR, se_otp_demo_write_buf, SE_OTP_DEMO_WRITE_LEN);
    if (ret) {
        TRACE(0,"--failed-- ret = %d se_otp_demo_read_security_register",ret);
        return 1;
    }

    se_otp_demo_test_check(se_otp_demo_write_buf,SE_OTP_DEMO_WRITE_LEN);

    se_otp_demo_init();


    TRACE(0,"erasing");
    se_otp_demo_erase_security_register(SE_OTP_DEMO_WRITE_START_ADDR, SE_OTP_DEMO_WRITE_LEN);

    if (ret) {
        TRACE(0,"--failed-- ret = %d se_otp_demo_erase_security_register",ret);
        return -1;
    }
    
    TRACE(0,"burning");
    ret = se_otp_demo_burn_security_register(SE_OTP_DEMO_WRITE_START_ADDR, (const unsigned char *)se_otp_demo_write_buf, SE_OTP_DEMO_WRITE_LEN);
    if (ret) {
        TRACE(0,"--failed-- ret = %d se_otp_demo_burn_security_register",ret);
        return -2;
    }
    
    TRACE(0,"verifying");
    ret = se_otp_demo_verify_security_register_data(SE_OTP_DEMO_WRITE_START_ADDR, se_otp_demo_write_buf + SE_OTP_DEMO_WRITE_LEN,
        (const unsigned char *)se_otp_demo_write_buf, SE_OTP_DEMO_WRITE_LEN);
    if (ret) {
        TRACE(0,"--failed-- ret = %d se_otp_demo_verify_security_register_data",ret);
        return -3;
    }

    TRACE(0,"reading");
    ret = se_otp_demo_read_security_register(SE_OTP_DEMO_WRITE_START_ADDR, se_otp_demo_write_buf + SE_OTP_DEMO_WRITE_LEN, SE_OTP_DEMO_WRITE_LEN);
    if (ret) {
        TRACE(0,"--failed-- ret = %d se_otp_demo_read_security_register",ret);
        return -4;
    }

    if(memcmp(se_otp_demo_write_buf,(se_otp_demo_write_buf+SE_OTP_DEMO_WRITE_LEN),SE_OTP_DEMO_WRITE_LEN) ==0){
        TRACE(0," pass ");
    }else{
        TRACE(0," failed ");
    }

    ret = se_otp_demo_get_security_register_lock_status((unsigned int)SE_OTP_DEMO_WRITE_START_ADDR,SE_OTP_DEMO_WRITE_LEN);
    TRACE(0,"[%d] locked status = %d ",__LINE__,ret);

    if(0){
        TRACE(0,"locking");
        ret = se_otp_demo_lock_security_register((unsigned int)SE_OTP_DEMO_WRITE_START_ADDR,SE_OTP_DEMO_WRITE_LEN);
        if (ret) {
            TRACE(0,"--failed-- ret = %d se_otp_demo_lock_security_register",ret);
            return -5;
        }

        ret = se_otp_demo_get_security_register_lock_status((unsigned int)SE_OTP_DEMO_WRITE_START_ADDR,SE_OTP_DEMO_WRITE_LEN);
        TRACE(0,"[%d] locked status = %d ",__LINE__,ret);
    }


    TRACE(1,"%s end",__func__);

    return 0;
}

int se_otp_read_security_register(unsigned int addr, unsigned char *data, unsigned int len)
{
    TRACE(0, "%s",__func__);
    DUMP8("%02X ",data, len);
    return se_otp_demo_read_security_register(addr, data, len);
}
int se_otp_get_security_register_lock_status(unsigned int addr, unsigned int len)
{
    int ret;
    ret = se_otp_demo_lock_security_register((unsigned int)SE_OTP_DEMO_WRITE_START_ADDR,SE_OTP_DEMO_WRITE_LEN);
    TRACE(0, "%s, lock set ret = %d",__func__, ret);

    ret = se_otp_demo_get_security_register_lock_status(addr, len);
    TRACE(0, "%s, lock get ret = %d",__func__, ret);
    return ret;
}


#endif
