/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include <string.h>
#include "plat_types.h"
#include "bt_drv.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "bt_drv_1306_internal.h"
#include "bt_drv_1306_config.h"
#include "hal_chipid.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "pmu.h"
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
#include "nvrecord_dev.h"
#endif

#ifndef BT_RF_MAX_XTAL_TUNE_PPB
// Default 10 ppm/bit or 10000 ppb/bit
#define BT_RF_MAX_XTAL_TUNE_PPB                 10000
#endif

#ifndef BT_RF_XTAL_TUNE_FACTOR
// Default 0.2 ppm/bit or 200 ppb/bit
#define BT_RF_XTAL_TUNE_FACTOR                  200
#endif


#define XTAL_FCAP_NORMAL_SHIFT                  0
#define XTAL_FCAP_NORMAL_MASK                   (0x1FF << XTAL_FCAP_NORMAL_SHIFT)
#define XTAL_FCAP_NORMAL(n)                     BITFIELD_VAL(XTAL_FCAP_NORMAL, n)

#define RF_REG_XTAL_FCAP                        0x168
#define RF_REG_XTAL_CMOM_DR                     0x168
static uint16_t xtal_fcap = DEFAULT_XTAL_FCAP;
static uint16_t init_xtal_fcap = DEFAULT_XTAL_FCAP;

struct RF_SYS_INIT_ITEM
{
    uint16_t reg;
    uint16_t set;
    uint16_t mask;
    uint16_t delay;
};

struct TX_PWR_SELECT_ITEM
{
    float pwr_dbm;
    uint16_t pwr_val;
};

static const struct TX_PWR_SELECT_ITEM tx_pwr_select[] =
{
    {16, 0x0023},    //16dbm
    {15, 0x0022},    //15dbm
    {14, 0x0021},    //14dbm
    {13, 0x0020},    //13dbm
    {12, 0x0016},    //12dbm
    {11, 0x0015},    //11dbm
    {10, 0x0014},    //10dbm
    {9,  0x0013},    //9dbm
    {8,  0x0012},    //8dbm
    {7,  0x0012},    //7dbm
    {6,  0x0011},    //6dbm
    {5,  0x0010},    //5dbm
    {4,  0x0006},    //3dbm
    {3,  0x0006},    //3dbm
    {2,  0x0005},    //2dbm
    {1,  0x0004},    //1dbm
    {0,  0x0003},    //0dbm
    {-1, 0x0002},    //-1dbm
    {-2, 0x0001},    //-2dbm
    {-3, 0x0000},    //-3dbm
};

static const struct RF_SYS_INIT_ITEM rf_sys_init_tbl[] =
{
    {0x00E9,0x0000,0x000F,1},
};

#define REG_EB_VAL 0x083f
#define REG_181_VAL (0x00bf)
#define REG_EC_VAL 0x081f
#define REG_182_VAL 0x00bf
#define REG_ED_VAL 0x091f
#define REG_183_VAL 0x00bf
#define REG_EE_VAL 0x0b1f
#define REG_184_VAL 0x00bf
#define REG_EF_VAL 0x00c7
#define REG_185_VAL 0x00bf
#define REG_F0_VAL 0x0147
#define REG_186_VAL 0x00bf
#define REG_F1_VAL 0x0347
#define REG_187_VAL 0x00af
#define REG_F2_VAL 0x0347
#define REG_188_VAL 0x008d

const uint16_t rf_init_tbl_common[][3] =
{
    {0x02,0x1717,0},
    {0x03,0x1918,0},
    {0x04,0x1A1D,0},
    {0x09,0x5556,0},//increase  prechar time
    {0x0A,0x1501,0},
    {0x0B,0x1615,0},
    {0x0C,0x1717,0},
    {0x0E,0x0804,0},//rf_ldo precharge time
    {0x0F,0x1000,0},//lna_pdt_pu_dr
    {0x12,0x0022,0},
    {0x17,0x4400,0},//rfpll dither bypass rx
    {0x18,0x0195,0},
    {0x23,0x28BF,0},
    {0x27,0x2500,0},
    {0x28,0x6E10,0},    //dither_byp
    {0x2B,0x0600,0},
    {0x2c,0x240C,0},//txflt fixed gain
    {0x51,0x0350,0},
    {0x81,0x0602,0},
    {0x90,0x3848,0},
    {0x91,0x8D22,0},
    {0x92,0x83B0,0},//tmx bias
    {0x98,0x2463,0},
    {0x9C,0x7E61,0},//increase vco vbit pdtref
    {0x9E,0x1414,0},
    {0xA0,0x2908,0},
    {0xA2,0x168C,0},
    {0xBF,0x1802,0},
    {0xC1,0x1A80,0},//padrv fixed gain
    {0xC2,0x5848,0},
    {0xC4,0x6C80,0},
    {0xCB,0x0E80,0},//pa cascode bias
    {0xCD,0x903F,0},//in band flatness tuing
    {0xCE,0x00A4,0},
    {0xCF,0xF000,0},
    {0x120,0x3016,0},//logen in div25 mode
    {0x121,0x0350,0},//vco tx in div25 mode
    {0x132,0x0800,0},
    {0x133,0x0800,0},
    {0x134,0x0800,0},
    {0x135,0x0800,0},
    {0x136,0x0800,0},
    {0x137,0x0800,0},
    {0x138,0x0800,0},
    {0x139,0x0800,0},
    {0x13A,0x6000,0},
    {0x13B,0x0025,0},
    {0x13C,0x2050,0},
    {0x13D,0x02E4,0},
    {0x13E,0x1578,0},
    {0x13F,0x0C45,0},
    {0x15D,0x0025,0},
    {0x15E,0x1224,0},
    {0x15F,0x1224,0},
    {0x160,0x1224,0},
    {0x161,0x1224,0},
    {0x162,0x2040,0},
    {0x163,0x2040,0},
    {0x164,0x2040,0},
    {0x165,0x2040,0},
    {0x209,0x0117,0},

    //rx gain
    {0x0039,0X2078,1},//gain idx 0
    {0x0041,0X0354,1},

    {0x003a,0X401A,1},//gain idx 1
    {0x0042,0X034B,1},

    {0x003b,0X3011,1},//gain idx 2
    {0x0043,0X034B,1},

    {0x003c,0X1012,1},//gain idx 3
    {0x0044,0X02C3,1},

    {0x003d,0X1012,1},//gain idx 4
    {0x0045,0X02C2,1},

    {0x003e,0X0013,1},//gain idx 5
    {0x0046,0X023A,1},

    {0x003f,0X0107,1},//gain idx 6
    {0x0047,0x0191,1},

    {0x0040,0X0307,1},//gain idx 7
    {0x0048,0X0109,1},

#ifdef PMU_HIGH_VPA
    {0x00CC,0x805F,0},//pa bias
#else
    {0x00CC,0x80DF,0},//pa bias
#endif

#ifdef RF_TX_LOW_POWER_MODE
    {0x00B8,0x3000,0},
    {0x00BB,0xFF9B,0},
    {0x0152,0x0F12,0},
    {0x0154,0x4006,0},
#else
    {0x00B8,0x7C00,0},
    {0x00BB,0xFFFF,0},
    {0x0152,0xFF12,0},
    {0x0154,0x403E,0},
#endif

#ifdef RF_RX_LOW_POWER_MODE
    {0x0096,0x1917,0},
    {0x009B,0x1744,0},
    {0x009D,0x0302,0},
    {0x00A1,0x1104,0},
    {0x00A4,0x9164,0},
    {0x00D0,0x0390,0},
#else
    {0x0096,0x1957,0},
    {0x009B,0x3746,0},
    {0x009D,0x0404,0},
    {0x00A1,0x1304,0},
    {0x00A4,0x91F4,0},
    {0x00D0,0x03A0,0},//improve acbuf drive voltage
#endif
};

const uint16_t rf_init_tbl_t0[][3] =
{
    {0x008F,0x9F52,0},
};

const uint16_t rf_init_tbl_t1[][3] =
{
    {0x008F,0x9F42,1},
};

#ifdef __HW_AGC__
const uint16_t rf_init_tbl_1_hw_agc[][3] = //hw agc table
{
    {0x027d, 0x0778, 0},
    {0x000E, 0x0F02, 0},

    {0x0209, 0x8116, 0},
    {0x020b, 0x00CC, 0},
    {0x020c, 0x0090, 0},
    {0x020d, 0x0000, 0},
    {0x020e, 0x021C, 0},
    {0x020f, 0x0006, 0},
    {0x0210, 0x0260, 0},
    {0x0211, 0x00CC, 0},
    {0x0212, 0x0090, 0},
    {0x0213, 0x006C, 0},
    {0x0214, 0x021C, 0},
    {0x0215, 0x7F7F, 0},
    {0x0216, 0x7F7F, 0},
    {0x027e, 0x01D4, 0},
    {0x027f, 0x01D4, 0},
    {0x0280, 0x01D4, 0},      //wait fast rssi
    {0x0281, 0x01D4, 0},

    // gain table
    {0x021D, 0x21C6, 0},
    {0x0220, 0x21B6, 0},     //wait EDR guard time，52.5us
    {0x0223, 0x21AE, 0},     //wait slow rssi
    {0x0226, 0x21AE, 0},
    {0x0229, 0x2DB6, 0},
    {0x022C, 0x2DA6, 0},
    {0x022F, 0x2DBE, 0},     //rssi diff ble2m & lr
    {0x0232, 0x2DAE, 0},     // wait rf stable

    {0x021E, 0x2078, 0},
    {0x0221, 0x2038, 0},
    {0x0224, 0x2018, 0},
    {0x0227, 0x1018, 0},
    {0x022A, 0x1108, 0},     //gain 0
    {0x022D, 0x1108, 0},
    {0x0230, 0x1008, 0},
    {0x0233, 0x1008, 0},

    {0x021F, 0x007C, 0},
    {0x0222, 0x007C, 0},
    {0x0225, 0x007C, 0},     //gain 7
    {0x0228, 0x047C, 0},
    {0x022B, 0x007C, 0},     //gain 0
    {0x022E, 0x007C, 0},
    {0x0231, 0x007C, 0},
    {0x0234, 0x007C, 0},

    //gain offset
    {0x0235, 0x2C34, 0},
    {0x0236, 0x2125, 0},     //gain 7
    {0x0237, 0x181D, 0},
    {0x0238, 0x0C11, 0},     //gain 0

    //jump point
    {0x0239, 0xB0AA, 0},
    {0x023A, 0xBAB7, 0},
    {0x023B, 0xC6C0, 0},
    {0x023C, 0x00CB, 0},     //gain 7

    //mode jump offset
    {0x0219, 0x0000, 0},
    {0x021A, 0xFD0A, 0},
    {0x021B, 0x0003, 0},
    {0x021C, 0x0000, 0},
};
#endif //__HW_AGC__

void bt_drv_rf_set_bt_hw_agc_enable(bool enable)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0x27D,&val_e2);
    if(enable)
    {
        //open rf bt hw agc mode
        val_e2 |= (1<<3);
    }
    else
    {
        //close rf bt hw agc mode
        val_e2 &= ~(1<<3);
    }
    btdrv_write_rf_reg(0x27D,val_e2);
}

void bt_drv_rf_set_ble_hw_agc_enable(bool enable)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0x27D,&val_e2);
    if(enable)
    {
        //open rf ble hw agc mode
        val_e2 |= (3<<4);
    }
    else
    {
        //close rf ble hw agc mode
        val_e2 &= ~(3<<4);
    }
    btdrv_write_rf_reg(0x27D,val_e2);
}

void bt_drv_rf_set_afh_monitor_gain(void)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0x1e4,&val_e2);//reg_bt_i2v_rin_afh[6:4]
    //i2v_rin_afh = 2 i2v_stb_afh=1
    val_e2 &= ~(7<<4);
    val_e2 |= (2<<4);
    val_e2 &= ~(7<<12); //reg_bt_i2v_stb_afh[14:12]
    val_e2 |= (1<<12); //reg_bt_i2v_stb_afh[14:12]
    btdrv_write_rf_reg(0x1e4,val_e2);


    btdrv_read_rf_reg(0x3cb,&val_e2); //lna_ic = 0 , i2v_gain = 7  lna_ldo_ic = 0xb
    val_e2 &= ~(0xf<<0);//reg_bt_i2v_gain_afh[3:0]
    val_e2 |= (7<<0);

    val_e2 &= ~(3<<4);//reg_bt_adc_vin_sel_afh[5:4]
    val_e2 |= (1<<4);

    val_e2 &= ~(0xF<<8);//reg_bt_lna_ldo_ic_afh[11:8]
    val_e2 |= (0x5<<8);

    val_e2 &= ~(7<<12);// reg_bt_lna_ic_afh[14:12]

    val_e2 &= ~(3<<6);//reg_bt_adc_vin_sel_normal[7:6]
    val_e2 |= (1<<6);

    btdrv_write_rf_reg(0x3cb,val_e2);


    btdrv_read_rf_reg(0x3cc,&val_e2); //flt = 7, hg_en=0, hg=0, rpass=11, rin=00,rfflt=0

    val_e2 = 0x1E0C;

    btdrv_write_rf_reg(0x3cc,val_e2);
}

uint32_t btdrv_rf_get_max_xtal_tune_ppb(void)
{
    return BT_RF_MAX_XTAL_TUNE_PPB;
}

uint32_t btdrv_rf_get_xtal_tune_factor(void)
{
    return BT_RF_XTAL_TUNE_FACTOR;
}

void btdrv_rf_init_xtal_fcap(uint32_t fcap)
{
    xtal_fcap = SET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL, fcap);
    btdrv_write_rf_reg(RF_REG_XTAL_FCAP, xtal_fcap);
    init_xtal_fcap = xtal_fcap;
}

uint32_t btdrv_rf_get_init_xtal_fcap(void)
{
    return GET_BITFIELD(init_xtal_fcap, XTAL_FCAP_NORMAL);
}

uint32_t btdrv_rf_get_xtal_fcap(void)
{
    return GET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL);
}

void btdrv_rf_set_xtal_fcap(uint32_t fcap, uint8_t is_direct)
{
}

int btdrv_rf_xtal_fcap_busy(uint8_t is_direct)
{
    return 0;
}

uint32_t btdrv_rf_bit_offset_get(void)
{
    return 0;
}

uint16_t btdrv_rf_bitoffset_get(uint8_t conidx)
{
    return bt_drv_reg_op_bitoff_getf(conidx);
}

void btdrv_rf_turn_off_tx(void)
{
    btdrv_write_rf_reg(0x000F,0x0001); /* turn off tmx */
}

void btdrv_rf_log_delay_cal(void)
{
    unsigned short read_value;
    unsigned short write_value;
    BT_DRV_TRACE(1,"%s", __func__);
    BTDIGITAL_REG(0xd0340020) = 0x010e01c0;
    BT_DRV_TRACE(1,"0xd0340020 =%x\n",BTDIGITAL_REG(0xd0340020) );

    btdrv_write_rf_reg(0xd4, 0x000f);
    btdrv_write_rf_reg(0xd5, 0x4000);
    btdrv_write_rf_reg(0xd2, 0x1003);
    btdrv_write_rf_reg(0xa7, 0x004e);
    btdrv_write_rf_reg(0xd4, 0x0000);
    btdrv_write_rf_reg(0xd5, 0x4002);

    BTDIGITAL_REG(0xd0340020) = 0x030e01c1;
    BT_DRV_TRACE(1,"0xd0340020 =%x\n",BTDIGITAL_REG(0xd0340020) );

    btdrv_delay(1);

    btdrv_write_rf_reg(0xd2, 0x5003);

    btdrv_delay(1);

    btdrv_read_rf_reg(0x1e2, &read_value);
    BT_DRV_TRACE(1,"0x1e2 read_value:%x\n",read_value);
    if(read_value == 0xff80)
    {
        btdrv_write_rf_reg(0xd3, 0xffff);
    }
    else
    {
        write_value = ((read_value>>7)&0x0001) | ((read_value & 0x007f)<<1) | ((read_value&0x8000)>>7) | ((read_value&0x7f00)<<1);
        BT_DRV_TRACE(1,"d3 write_value:%x\n",write_value);
        btdrv_write_rf_reg(0xd3, write_value);
    }
    btdrv_delay(1);

    BTDIGITAL_REG(0xd0340020) = 0x010e01c0;
    BT_DRV_TRACE(1,"0xd0340020 =%x\n",BTDIGITAL_REG(0xd0340020) );


    btdrv_write_rf_reg(0xd4, 0x000f);
    btdrv_write_rf_reg(0xd2, 0x1003);
    btdrv_write_rf_reg(0xd5, 0x4000);

}

void btdrv_rf_rx_gain_adjust_req(uint32_t user, bool lowgain)
{
    return;
}

//rf Image calib
void btdtv_rf_image_calib(void)
{
    uint16_t read_val = 0;
    //read calibrated val from efuse 0x05 register
    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &read_val);
    //check if bit 11 has been set
    uint8_t calb_done_flag = ((read_val &0x800)>>11);
    if(calb_done_flag)
    {
        BT_DRV_TRACE(1,"EFUSE REG[5]=%x",read_val);
    }
    else
    {
        BT_DRV_TRACE(0,"EFUSE REG[5] rf image has not been calibrated!");
        return;
    }
    //[bit 12] calib flag
    uint8_t calib_val = ((read_val &0x1000)>>12);
    btdrv_read_rf_reg(0x9b,&read_val);
    read_val&=0xfcff;

    if(calib_val==0)
    {
        read_val|= 1<<8;
    }
    else if(calib_val== 1)
    {
        read_val|= 1<<9;
    }

    BT_DRV_TRACE(1,"write rf image calib val=%x in REG[0x9b]", read_val);
    btdrv_write_rf_reg(0x9b,read_val);
}

void btdrv_turn_off_bias_current(void)
{
    // only for version B
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();

    if (metal_id >= HAL_CHIP_METAL_ID_1) {
        btdrv_write_rf_reg(0x00f2,0x0000);
        btdrv_write_rf_reg(0x00f3,0x0000);
        btdrv_write_rf_reg(0x00f4,0x007c);
    }
}

#ifdef RX_IQ_CAL

int bt_iqimb_test_ex (int mismatch_type);
extern int iq_gain;
extern int iq_phy;

void dataSort(int *arry,int len)
{
    for (int i = 0; i < len - 1; i++){
        for (int j = 0; j < len - 1 - i; j++){
            if (arry[j] > arry[j + 1]){
                int temp = arry[j + 1];
                arry[j + 1] = arry[j];
                arry[j] = temp;
            }
        }
    }
}

const uint16_t rx_cal_rfreg_set[][2] =
{
    {0xE8, 0x36B1},
    {0xE9, 0x1175},
    {0x3AD, 0x0038},
    {0xBF, 0x0D00},
    {0xC0,0x1E17},
    {0x1E4, 0x0025},
};

const uint16_t rx_cal_rfreg_store[][1] =
{
    {0xE8},
    {0xE9},
    {0x3AD},
    {0xBF},
    {0xC0},
    {0x1E4},
};

void btdrv_rx_iq_cal(void)
{
    //uint16_t val;
    uint8_t i;
    uint16_t value = 0;
    uint32_t tx_cal_digreg_store[6];

    //const uint16_t (*rx_cal_rfreg_set_p)[2];
    const uint16_t (*rx_cal_rfreg_store_p)[1];
    //uint32_t rx_reg_set_tbl_size = 0;

    tx_cal_digreg_store[0] = BTDIGITAL_REG(0xD0350214);
    tx_cal_digreg_store[1] = BTDIGITAL_REG(0xd0350218);
    tx_cal_digreg_store[2] = BTDIGITAL_REG(0xd0350240);
    tx_cal_digreg_store[3] = BTDIGITAL_REG(0xD0350300);
    tx_cal_digreg_store[4] = BTDIGITAL_REG(0xD035031c);
    tx_cal_digreg_store[5] = BTDIGITAL_REG(0xD0350320);

    rx_cal_rfreg_store_p = &rx_cal_rfreg_store[0];
    uint32_t rx_reg_store_tbl_size = ARRAY_SIZE(rx_cal_rfreg_store);
    uint16_t rx_cal_rfreg_store[rx_reg_store_tbl_size];
    BT_DRV_TRACE(0,"rx reg_store:\n");
    for(i=0; i< rx_reg_store_tbl_size; i++)
    {
        btdrv_read_rf_reg(rx_cal_rfreg_store_p[i][0],&value);
        rx_cal_rfreg_store[i] = value;
        BT_DRV_TRACE(2,"rx reg=%x,v=%x",rx_cal_rfreg_store_p[i][0],value);
    }


    BT_DRV_TRACE(1,"0xd0350214:%x\n",tx_cal_digreg_store[0]);
    BT_DRV_TRACE(1,"0xD0350218:%x\n",tx_cal_digreg_store[1]);
    BT_DRV_TRACE(1,"0xD0350240:%x\n",tx_cal_digreg_store[2]);
    BT_DRV_TRACE(1,"0xD0350300:%x\n",tx_cal_digreg_store[3]);
    BT_DRV_TRACE(1,"0xD035031c:%x\n",tx_cal_digreg_store[4]);
    BT_DRV_TRACE(1,"0xD0350320:%x\n",tx_cal_digreg_store[5]);

    BTDIGITAL_REG(0xd0350218) = 0x00000583;
    BTDIGITAL_REG(0xD0350214) = 0x0;
    BTDIGITAL_REG(0xD0350320) = 0x0001000F;
    BTDIGITAL_REG_SET_FIELD(0xD0350300, 7, 0, 3);
    BTDIGITAL_REG_SET_FIELD(0xD035031c, 0xf, 0, 1);
#ifdef BT_IF_750K
    BTDIGITAL_REG_SET_FIELD(0xD0350320, 0x7f, 0, 0xc);
#elif defined(BT_IF_1P05M)
    BTDIGITAL_REG_SET_FIELD(0xD0350320, 0x7f, 0, 0xf);
#endif
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0x1, 12, 1);

    while(1) {
        BTDIGITAL_REG(0xd0220c00) = 0x0;
        btdrv_delay(1);
        BTDIGITAL_REG(0xd0220c00) = 0x001BA096;
        BT_DRV_TRACE(1, "0xd0220c00=0x%x", BTDIGITAL_REG(0xd0220c00));
        btdrv_delay(1);
        bt_iqimb_test_ex(1);
        btdrv_delay(5*1000);
    }

    BTDIGITAL_REG(0xd0220c00) = 0x0;
    BTDIGITAL_REG_SET_FIELD(0xD035025c, 0x3, 2, 0);
    BT_DRV_TRACE(0,"25c:%x\n",BTDIGITAL_REG(0xd035025c));

    BT_DRV_TRACE(0,"reg_reset:\n");

    for(i=0; i< rx_reg_store_tbl_size; i++)
    {
        btdrv_write_rf_reg(rx_cal_rfreg_store_p[i][0],rx_cal_rfreg_store[i]);
     //   BTRF_REG_DUMP(rx_cal_rfreg_store_p[i][0]);
    }

    BTDIGITAL_REG(0xD0350214) = tx_cal_digreg_store[0];
    BTDIGITAL_REG(0xd0350218) = tx_cal_digreg_store[1];
    BTDIGITAL_REG(0xd0350240) = tx_cal_digreg_store[2];
    BTDIGITAL_REG(0xD0350300) = tx_cal_digreg_store[3];
    BTDIGITAL_REG(0xD035031c) = tx_cal_digreg_store[4];
    BTDIGITAL_REG(0xD0350320) = tx_cal_digreg_store[5];

    BT_DRV_TRACE(1,"0xD0350214:%x\n",BTDIGITAL_REG(0xD0350214));
    BT_DRV_TRACE(1,"0xd0350218:%x\n",BTDIGITAL_REG(0xd0350218));
    BT_DRV_TRACE(1,"0xD0350240:%x\n",BTDIGITAL_REG(0xD0350240));
    BT_DRV_TRACE(1,"0xD0350300:%x\n",BTDIGITAL_REG(0xD0350300));
    BT_DRV_TRACE(1,"0xD035031c:%x\n",BTDIGITAL_REG(0xD035031c));
    BT_DRV_TRACE(1,"0xD0350320:%x\n",BTDIGITAL_REG(0xD0350320));
    //BT_DRV_TRACE(1,"0xd0350240:%x\n",BTDIGITAL_REG(0xd0350240));
}
#endif

#define TX_PWR_16DBM 16
#define TX_PWR_15DBM 15
#define TX_PWR_14DBM 14
#define TX_PWR_13DBM 13
#define TX_PWR_12DBM 12
#define TX_PWR_11DBM 11
#define TX_PWR_10DBM 10
#define TX_PWR_9DBM  9
#define TX_PWR_8DBM  8
#define TX_PWR_7DBM  7
#define TX_PWR_6DBM  6
#define TX_PWR_5DBM  5
#define TX_PWR_4DBM  4
#define TX_PWR_3DBM  3
#define TX_PWR_2DBM  2
#define TX_PWR_1DBM  1
#define TX_PWR_0DBM  0
#define TX_PWR_N1DBM -1
#define TX_PWR_N2DBM -2
#define TX_PWR_N3DBM -3

struct btdrv_customer_rf_config_t btdrv_rf_customer_config;
void bt_drv_rf_sdk_init(void)
{
    BT_DRV_TRACE(0,"SDK default rf config");
    btdrv_rf_customer_config.config_xtal_en = false;//get value from NV
    btdrv_rf_customer_config.xtal_cap_val = 0;//get value from NV
    //config TX power in dbm
    btdrv_rf_customer_config.config_tx_pwr_en = true;
    btdrv_rf_customer_config.bt_tx_page_pwr =TX_PWR_9DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_max_pwr  =TX_PWR_15DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx4_pwr =TX_PWR_11DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx3_pwr =TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx2_pwr =TX_PWR_4DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx1_pwr =TX_PWR_0DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx0_pwr =TX_PWR_N3DBM;//in dbm

    btdrv_rf_customer_config.le_tx_max_pwr  =TX_PWR_15DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx4_pwr =TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx3_pwr =TX_PWR_9DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx2_pwr =TX_PWR_5DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx1_pwr =TX_PWR_1DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx0_pwr =TX_PWR_N3DBM;//in dbm
    //init BLE convert table
    btdrv_txpwr_conv_tbl[0] = btdrv_rf_customer_config.le_tx_idx0_pwr;
    btdrv_txpwr_conv_tbl[1] = btdrv_rf_customer_config.le_tx_idx1_pwr;
    btdrv_txpwr_conv_tbl[2] = btdrv_rf_customer_config.le_tx_idx2_pwr;
    btdrv_txpwr_conv_tbl[3] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[4] = btdrv_rf_customer_config.le_tx_idx4_pwr;
    btdrv_txpwr_conv_tbl[5] = btdrv_rf_customer_config.le_tx_max_pwr;
    btdrv_txpwr_conv_tbl[6] = btdrv_rf_customer_config.le_tx_max_pwr;
    btdrv_txpwr_conv_tbl[7] = btdrv_rf_customer_config.le_tx_max_pwr;
}

void bt_drv_rf_set_customer_config(struct btdrv_customer_rf_config_t* config)
{
    memset(&btdrv_rf_customer_config, 0, sizeof(struct btdrv_customer_rf_config_t));
    if(config !=NULL)
    {
        btdrv_rf_customer_config = *config;
    }
    else
    {
        bt_drv_rf_sdk_init();
    }
}

static uint16_t bt_rf_txpwr_reg_get(uint8_t idx)
{
    uint16_t rf_reg = 0;
    switch(idx)
    {
        case TX_PWR_IDX_0:
            rf_reg = RF_BT_TX_PWR_IDX0_REG;
            break;
        case TX_PWR_IDX_1:
            rf_reg = RF_BT_TX_PWR_IDX1_REG;
            break;
        case TX_PWR_IDX_2:
            rf_reg = RF_BT_TX_PWR_IDX2_REG;
            break;
        case TX_PWR_IDX_3:
            rf_reg = RF_BT_TX_PWR_IDX3_REG;
            break;
        case TX_PWR_IDX_4:
            rf_reg = RF_BT_TX_PWR_IDX4_REG;
            break;
        case TX_PWR_IDX_MAX:
            rf_reg = RF_BT_TX_PWR_IDX5_REG;
            break;
        case TX_PWR_IDX_PAGE:
            rf_reg = RF_BT_TX_PWR_IDX6_REG;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR tx pwr idx=%d", idx);
            break;
    }

    return rf_reg;
}

uint16_t ble_rf_txpwr_reg_get(uint8_t idx)
{
    uint16_t rf_reg = 0;
    switch(idx)
    {
        case TX_PWR_IDX_0:
            rf_reg = RF_BLE_TX_PWR_IDX0_REG;
            break;
        case TX_PWR_IDX_1:
            rf_reg = RF_BLE_TX_PWR_IDX1_REG;
            break;
        case TX_PWR_IDX_2:
            rf_reg = RF_BLE_TX_PWR_IDX2_REG;
            break;
        case TX_PWR_IDX_3:
            rf_reg = RF_BLE_TX_PWR_IDX3_REG;
            break;
        case TX_PWR_IDX_4:
            rf_reg = RF_BLE_TX_PWR_IDX4_REG;
            break;
        case TX_PWR_IDX_MAX:
            rf_reg = RF_BLE_TX_PWR_IDX5_REG;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR BLE tx pwr idx=%d", idx);
            break;
    }

    return rf_reg;
}

int rf_reg_txpwr_val_get(int8_t txpwr_dbm, uint16_t *reg_pwr)
{
    uint32_t txpwr_val = 0;
    uint16_t i = 0;
    uint16_t tbl_size;

    tbl_size = sizeof(tx_pwr_select)/sizeof(tx_pwr_select[0]);
    for(i=0; i< tbl_size; i++){
        if(tx_pwr_select[i].pwr_dbm == txpwr_dbm){
            *reg_pwr = tx_pwr_select[i].pwr_val;
        }
    }
     return txpwr_val;
}

void bt_drv_ble_rf_reg_txpwr_set(uint16_t rf_reg, uint16_t val)
{
    btdrv_write_rf_reg(rf_reg, val);
}

static void bt_drv_rf_bt_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint16_t tx_pwr_val = 0;

    uint16_t rf_reg = bt_rf_txpwr_reg_get(idx);
    rf_reg_txpwr_val_get(val_in_dbm, &tx_pwr_val);
    BTRF_REG_SET_FIELD(rf_reg, 0x3F, 0, tx_pwr_val);

    BT_DRV_TRACE(2, "BT:TX pwr reg=0x%x,[in:%d dbm,out:0x%04x]", rf_reg, val_in_dbm, tx_pwr_val);
}

static void bt_drv_rf_ble_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint16_t tx_pwr_val = 0;

    uint16_t rf_reg = ble_rf_txpwr_reg_get(idx);
    rf_reg_txpwr_val_get(val_in_dbm, &tx_pwr_val);
    BTRF_REG_SET_FIELD(rf_reg, 0x3F, 6, tx_pwr_val);

    BT_DRV_TRACE(2, "BLE:TX pwr reg=0x%x,[in:%d dbm,out:0x%04x]", rf_reg, val_in_dbm, tx_pwr_val);
}

void bt_drv_tx_pwr_init(void)
{
    struct btdrv_customer_rf_config_t cfg = btdrv_rf_customer_config;

    if(cfg.config_tx_pwr_en == true)
    {
        //BT
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_0, cfg.bt_tx_idx0_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_1, cfg.bt_tx_idx1_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_2, cfg.bt_tx_idx2_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_3, cfg.bt_tx_idx3_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_4, cfg.bt_tx_idx4_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_MAX, cfg.bt_tx_max_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_PAGE, cfg.bt_tx_page_pwr);
        //BLE
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_0, cfg.le_tx_idx0_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_1, cfg.le_tx_idx1_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_2, cfg.le_tx_idx2_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_3, cfg.le_tx_idx3_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_4, cfg.le_tx_idx4_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_MAX, cfg.le_tx_max_pwr);
    }
    else
    {
        ASSERT(0, "BT_DRV:pls check TX pwr config");
    }
}

void bt_drv_rf_init_xtal_fcap(void)
{
    uint16_t xtal_fcap_temp = DEFAULT_XTAL_FCAP;
    if(btdrv_rf_customer_config.config_xtal_en == true)
    {
        xtal_fcap_temp = btdrv_rf_customer_config.xtal_cap_val;
        BT_DRV_TRACE(1,"btdrv customer set xtal fcap=0x%x", xtal_fcap_temp);
        btdrv_rf_init_xtal_fcap(xtal_fcap_temp);
    }
    else
    {
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
        unsigned int xtal_fcap;
        if (!nvrec_dev_get_xtal_fcap(&xtal_fcap))
        {
            btdrv_rf_init_xtal_fcap(xtal_fcap);
            btdrv_delay(1);
            BT_DRV_TRACE(2,"%s XTAL 0x03=0x%x", __func__, xtal_fcap);
        }
        else
#endif
        {
            btdrv_rf_init_xtal_fcap(DEFAULT_XTAL_FCAP);
            BT_DRV_TRACE(1,"%s failed", __func__);
        }
    }
}

void bt_drv_ble_adv_txpwr_via_advhdl(uint8_t adv_hdl, uint8_t idx, int8_t txpwr_dbm)
{
    BT_DRV_TRACE(2,"set adv_txpwr_via_advhdl hdl 0x%02x idx %d dbm %d", adv_hdl, idx, txpwr_dbm);

    if(idx > 5)
    {
        return;
    }

    if(txpwr_dbm <-3)
    {
        txpwr_dbm += 18;
        bt_drv_reg_op_low_txpwr(true, FACTOR_ATTENUATION_18DBM, BLE_ADV_LOW_TXPWR_MODE, adv_hdl);
    }
    else
    {
        bt_drv_reg_op_low_txpwr(false, FACTOR_ATTENUATION_18DBM, BLE_ADV_LOW_TXPWR_MODE, adv_hdl);
    }

    bt_drv_rf_ble_txpwr_set(idx, txpwr_dbm);
}

void btdrv_cap_delay_cal(void)
{
    uint16_t value = 0;;
    uint8_t tx_capdly, rx_capdly = 0;

    tx_capdly = value & 0xF;
    rx_capdly = ((value & 0xF0) >> 4);

    if ((0 == tx_capdly) || (0 == rx_capdly)) {
        BT_DRV_TRACE(0, "invalid efuse ,so use default value");
        tx_capdly = rx_capdly = 0x8;
    }

    BTRF_REG_SET_FIELD(0x295, 0xF, 6, tx_capdly);
    BTRF_REG_SET_FIELD(0x2C5, 0xF, 6, rx_capdly);
}

void btdrv_rf_log_delay_calibration_self(void)
{
    uint32_t dig_reg_default;
    uint16_t log_dll_dly2;
    uint16_t log_dll_dly1;
    uint16_t val;
    const float attenuation_factor = 0.8f;

    BT_DRV_TRACE(0, "%s start", __func__);
    dig_reg_default = BTDIGITAL_REG(0xd0220c00);

    // tx on 2448 test_freq 2MHz
    BTRF_REG_SET_FIELD(0xc4, 0x1, 0, 0x1);
    BTDIGITAL_REG_WR(0xd0220c00, 0xa0028);
    btdrv_delay(1);

    BTRF_REG_GET_FIELD(0x58, 0x7F, 7, log_dll_dly2);
    BTRF_REG_GET_FIELD(0x58, 0x7F, 0, log_dll_dly1);
    BT_DRV_TRACE(0, "log_dll_dly2:0x%x, log_dll_dly1:0x%x", log_dll_dly2, log_dll_dly1);

    if (log_dll_dly1 >= 117 && log_dll_dly1 <= 127) {
        log_dll_dly1 = (uint16_t)((float)log_dll_dly1 * attenuation_factor + 0.5);
        BT_DRV_TRACE(0, "calib val too high, 0.8 * log_dll_dly1:0x%x", log_dll_dly1);
    }

    if (log_dll_dly2 >= 117 && log_dll_dly2 <= 127) {
        log_dll_dly2 = (uint16_t)((float)log_dll_dly2 * attenuation_factor + 0.5);
        BT_DRV_TRACE(0, "calib val too high, 0.8 * log_dll_dly2:0x%x", log_dll_dly2);
    }

    BTRF_REG_SET_FIELD(0x2e, 0x7F, 7, log_dll_dly2);
    BTRF_REG_SET_FIELD(0x2e, 0x7F, 0, log_dll_dly1);

    btdrv_read_rf_reg(0x2e, &val);
    BT_DRV_TRACE(0, "check 0x2E=0x%x", val);

    BTRF_REG_SET_FIELD(0x2d, 0x7, 11, 0x7);
    BTRF_REG_SET_FIELD(0xc4, 0x1, 0, 0x0);
    BTDIGITAL_REG_WR(0xd0220c00, dig_reg_default);
    BT_DRV_TRACE(0, "%s end", __func__);
}

#ifdef __NEW_SWAGC_MODE__
void bt_drv_rf_set_bt_sync_agc_enable(bool enable)
{
    uint16_t val = 0;
    btdrv_read_rf_reg(0x22,&val);
    if(enable)
    {
        //open rf new sync agc mode
        val |= (1<<3);
    }
    else
    {
        //close rf new sync agc mode
        val &= ~(1<<3);
    }
    btdrv_write_rf_reg(0x22,val);
}
#endif

#ifdef __BLE_NEW_SWAGC_MODE__
void bt_drv_rf_set_ble_sync_agc_enable(bool enable)
{
    uint16_t val = 0;
    btdrv_read_rf_reg(0x22,&val);
    if(enable)
    {
        //open rf new sync agc mode
        val |= (1<<2);
    }
    else
    {
        //close rf new sync agc mode
        val &= ~(1<<2);
    }
    btdrv_write_rf_reg(0x22,val);
}
#endif

void bt_drv_rf_fixed_tx_power_idx(uint8_t txpwr_idx, bool enable)
{
    if (enable) {
        BTRF_REG_SET_FIELD(0x24, 0x1, 4, 1);            //dr tx gain enbale
        BTRF_REG_SET_FIELD(0x24, 0x7, 5, txpwr_idx);    //sel txpwr idx
    } else {
        BTRF_REG_SET_FIELD(0x24, 0xF, 4, 0);            //clr tx gain dr
    }
}

#ifdef __HW_AGC__
static uint16_t hw_agc_rx_gain0_adc_reg_val;
static uint16_t hw_agc_rx_gain0_lna_reg_val;
static uint16_t hw_agc_rx_gain1_adc_reg_val;
static uint16_t hw_agc_rx_gain1_lna_reg_val;
static uint16_t hw_agc_rx_gain0_offset_reg_val;
static uint16_t hw_agc_rx_gain1_offset_reg_val;
static uint8_t bt_drv_hw_agc_mode_status = 0xFF;

void bt_drv_rf_hw_agc_mode_init(void)
{
    btdrv_read_rf_reg(0x21D, &hw_agc_rx_gain0_adc_reg_val);
    btdrv_read_rf_reg(0x21E, &hw_agc_rx_gain0_lna_reg_val);
    btdrv_read_rf_reg(0x220, &hw_agc_rx_gain1_adc_reg_val);
    btdrv_read_rf_reg(0x221, &hw_agc_rx_gain1_lna_reg_val);
    BTRF_REG_GET_FIELD(0x235, 0xFF, 0, hw_agc_rx_gain0_offset_reg_val);
    BTRF_REG_GET_FIELD(0x235, 0xFF, 8, hw_agc_rx_gain1_offset_reg_val);
}

void bt_drv_rf_get_hw_agc_reg_addr(int8_t rx_idx, uint16_t *adc_reg, uint16_t *lna_reg, uint16_t *offset_reg)
{
    *adc_reg = 0x21D + 3 * rx_idx;
    *lna_reg = 0x21E + 3 * rx_idx;
    *offset_reg = 0x235 + rx_idx / 2;
}

void bt_drv_rf_hw_agc_mode_set(int8_t rx_idx, int8_t hwagc_mode)
{
    uint16_t adc_reg;
    uint16_t lna_reg;
    uint16_t offset_reg;

    if (bt_drv_hw_agc_mode_status == hwagc_mode) {
        return;
    }

    bt_drv_hw_agc_mode_status = hwagc_mode;

    bt_drv_rf_get_hw_agc_reg_addr(rx_idx, &adc_reg, &lna_reg, &offset_reg);

    BT_DRV_TRACE(0, "%s, mode=%d, idx=%d",__func__, hwagc_mode, rx_idx);

    switch(rx_idx) {
        case RX_GAIN_IDX_0:
            if (hwagc_mode == HW_AGC_PWR_SAVE_MODE){
                btdrv_write_rf_reg(adc_reg, hw_agc_rx_gain1_adc_reg_val);
                btdrv_write_rf_reg(lna_reg, hw_agc_rx_gain1_lna_reg_val);
                BTRF_REG_SET_FIELD(offset_reg, 0xFF, 0, hw_agc_rx_gain1_offset_reg_val);
            }else if (hwagc_mode == HW_AGC_HIGH_PWEF_MODE){
                btdrv_write_rf_reg(adc_reg, hw_agc_rx_gain0_adc_reg_val);
                btdrv_write_rf_reg(lna_reg, hw_agc_rx_gain0_lna_reg_val);
                BTRF_REG_SET_FIELD(offset_reg, 0xFF, 0, hw_agc_rx_gain0_offset_reg_val);
            }
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR rx gain idx=%d don't have low pwr mode", rx_idx);
            break;
    }
}

void bt_drv_rf_high_efficency_tx_pwr_ctrl(bool limit_tx_idx, bool limit_pa_en)
{
#ifdef HIGH_EFFICIENCY_TX_PWR_CTRL
    bool limit_hwagc_init_gain = limit_tx_idx;
    if(limit_hwagc_init_gain)
    {
        bt_drv_rf_hw_agc_mode_set(RX_GAIN_IDX_0, HW_AGC_PWR_SAVE_MODE);
    }
    else
    {
        bt_drv_rf_hw_agc_mode_set(RX_GAIN_IDX_0, HW_AGC_HIGH_PWEF_MODE);
    }
#endif
}
#endif
enum RF_VCO_CALIB_TYPE_T {
    RF_VCO_CALIB_TYPE_0,
    RF_VCO_CALIB_TYPE_1,
    RF_VCO_CALIB_TYPE_2,
    RF_VCO_CALIB_TYPE_3,

    RF_VCO_CALIB_TYPE_QTY
};

void bt_drv_rf_vco_current(void)
{
    // calib by efuse
    uint16_t read_val;
    uint16_t calib_val_bit_0;
    uint16_t calib_val_bit_6;
    uint16_t chip_type;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &read_val);
    BT_DRV_TRACE(2, "%s PMU_EFUSE_PAGE_RESERVED_C val=0x%x",__func__, read_val);

    calib_val_bit_0 = getbit(read_val, 0);
    calib_val_bit_6 = getbit(read_val, 6);
    chip_type = (calib_val_bit_6 << 1) | calib_val_bit_0;
    BT_DRV_TRACE(2, "%s PMU_EFUSE_PAGE_RESERVED_C val=0x%x",__func__, chip_type);

    switch (chip_type) {
        case RF_VCO_CALIB_TYPE_0:
            BTRF_REG_SET_FIELD(0x18, 0x3F, 0, 0x12);
            break;
        case RF_VCO_CALIB_TYPE_1:
            BTRF_REG_SET_FIELD(0x18, 0x3F, 0, 0x18);
            break;
        case RF_VCO_CALIB_TYPE_2:
            BTRF_REG_SET_FIELD(0x18, 0x3F, 0, 0x26);
            break;
        case RF_VCO_CALIB_TYPE_3:
            BTRF_REG_SET_FIELD(0x18, 0x3F, 0, 0x3F);
            break;
        default:
            break;
    }
}

// PMU_EFUSE_PAGE_RESERVED_C
#define PMU_EFUSE_BT_LOGEN_CAL_SHIFT                            9
#define PMU_EFUSE_BT_LOGEN_CAL_MASK                             (0x7 << PMU_EFUSE_BT_LOGEN_CAL_SHIFT)
#define PMU_EFUSE_BT_LOGEN_CAL(n)                               BITFIELD_VAL(PMU_EFUSE_BT_LOGEN_CAL, n)

void btdrv_logen_calib_by_efuse(void)
{
    uint16_t read_val = 0;
    uint16_t calib_val = 0;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_C, &read_val);
    calib_val = (read_val & PMU_EFUSE_BT_LOGEN_CAL_MASK) >> 9;
    TRACE(0,"%s, read val: %x, calib val: %x", __func__, read_val, calib_val);

    if (calib_val != 0) {
        BTRF_REG_SET_FIELD(0x8F, 0x7, 4, calib_val);
    }
}

uint8_t btdrv_rf_init(void)
{
    uint16_t value = 0;
    const uint16_t (*rf_init_tbl_p)[3] = NULL;
    uint32_t tbl_size = 0;
    uint8_t i;
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();
    //system RF register init
    for (i = 0; i < ARRAY_SIZE(rf_sys_init_tbl); i++) {
        btdrv_read_rf_reg(rf_sys_init_tbl[i].reg, &value);
        value = (value & ~rf_sys_init_tbl[i].mask) | (rf_sys_init_tbl[i].set & rf_sys_init_tbl[i].mask);
        if (rf_sys_init_tbl[i].delay) {
            btdrv_delay(rf_sys_init_tbl[i].delay);
        }
        btdrv_write_rf_reg(rf_sys_init_tbl[i].reg, value);
    }
    //common RF register init
    rf_init_tbl_p = &rf_init_tbl_common[0];
    tbl_size = sizeof(rf_init_tbl_common) / sizeof(rf_init_tbl_common[0]);

    for (i = 0; i < tbl_size; i++){
        btdrv_write_rf_reg(rf_init_tbl_p[i][0],rf_init_tbl_p[i][1]);
        if (rf_init_tbl_p[i][2] != 0)
            btdrv_delay(rf_init_tbl_p[i][2]);//delay
        btdrv_read_rf_reg(rf_init_tbl_p[i][0],&value);
        if (value != rf_init_tbl_p[i][1]){
            BT_DRV_TRACE(3,"write false rf reg=0x%x,write=0x%x,read=0x%x",rf_init_tbl_p[i][0],rf_init_tbl_p[i][1],value);
        }
    }
    //different metal version RF register init
    if (metal_id == HAL_CHIP_METAL_ID_0) {
        rf_init_tbl_p = &rf_init_tbl_t0[0];
        tbl_size = sizeof(rf_init_tbl_t0) / sizeof(rf_init_tbl_t0[0]);
    }
    else if (metal_id >= HAL_CHIP_METAL_ID_1) {
        rf_init_tbl_p = &rf_init_tbl_t1[0];
        tbl_size = sizeof(rf_init_tbl_t1) / sizeof(rf_init_tbl_t1[0]);
    }
    for (i = 0; i < tbl_size; i++) {
        btdrv_write_rf_reg(rf_init_tbl_p[i][0], rf_init_tbl_p[i][1]);
        if(rf_init_tbl_p[i][2] != 0)
            btdrv_delay(rf_init_tbl_p[i][2]);//delay
        btdrv_read_rf_reg(rf_init_tbl_p[i][0],&value);
        BT_DRV_TRACE(2,"reg=%x,v=%x",rf_init_tbl_p[i][0],value);
    }
    // logen ldo vres calib (only for version B)
    if (metal_id >= HAL_CHIP_METAL_ID_1) {
        btdrv_logen_calib_by_efuse();
    }
    //fix rx low frequency no signal
    bt_drv_rf_vco_current();
    //customer option
    bt_drv_rf_set_customer_config(NULL);
    //TX power init
    bt_drv_tx_pwr_init();
    //bt log calib
    btdrv_rf_log_delay_calibration_self();
    //XTAL cap init
    bt_drv_rf_init_xtal_fcap();

#ifdef __NEW_SWAGC_MODE__
    bt_drv_rf_set_bt_sync_agc_enable(true);
#endif

#ifdef __HW_AGC__
    bt_drv_rf_set_bt_hw_agc_enable(true);
    bt_drv_rf_set_ble_hw_agc_enable(true);
    for(i=0; i< ARRAY_SIZE(rf_init_tbl_1_hw_agc); i++)
    {
        btdrv_write_rf_reg(rf_init_tbl_1_hw_agc[i][0],rf_init_tbl_1_hw_agc[i][1]);
        if(rf_init_tbl_1_hw_agc[i][2] !=0)
            btdrv_delay(rf_init_tbl_1_hw_agc[i][2]);
    }
    bt_drv_rf_hw_agc_mode_init();
    bt_drv_rf_hw_agc_mode_set(RX_GAIN_IDX_0, HW_AGC_HIGH_PWEF_MODE);
#endif  //__HW_AGC__

    return 0;
}
