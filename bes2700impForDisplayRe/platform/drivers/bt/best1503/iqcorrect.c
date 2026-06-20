#ifdef RX_IQ_CAL
#ifndef NOSTD

#include <math.h>
#include <stdio.h>

#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "heap_api.h"

#include "string.h"
#include "besbt_string.h"
#include "bt_drv.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_btdump.h"
#include "iqcorrect.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"

#include "bt_drv_iq_common.h"

#define IQ_CALIB
#define DC_CALIB
//#define IQ_DEBUG
//#define DC_DEBUG
#define DC_ENERGY_THD 200
#define DC_RANGE_MIN -200
#define DC_RANGE_MAX 200
#define IQ_ENERGY_THD 100
#define IQ_RANGE_MIN -200
#define IQ_RANGE_MAX 200
#define SWEEP_STEP 50
#define LOG_MODULE                          HAL_TRACE_MODULE_APP

#define NTbl (1024)
#define FFT_SIZE 4096//2048//4096
#define CAL_BIN 1877*2
#define CAL_OFFSET 4*2
#define TIME_CAL_TABLE

#define bitshift 10
#define pi (3.1415926535898)
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define READ_REG(b,a) *(volatile uint32_t *)(uintptr_t)((uint32_t)(b)+(a))
#define WRITE_REG(v,b,a) *(volatile uint32_t *)(uintptr_t)((uint32_t)(b)+(a)) = v

#define   BUF_SIZE   (1024)

#define MAX_COUNT 5
volatile int iqimb_dma_status = 0;
//short M0data[BUF_SIZE];
#define MED_MEM_POOL_SIZE (88*1024)
static uint8_t *g_medMemPool = NULL;

extern void *rt_malloc(unsigned int size);
extern void rt_free(void *rmem);

typedef struct ComplexInt_
{
    int re;
    int im;
} ComplexInt;
typedef struct Complexflt_
{
    float re;
    float im;
} ComplexFlt;
typedef struct ComplexShort_
{
    short re;
    short im;
} ComplexShort;

typedef struct IQMismatchPreprocessState_
{
    short *M0data;
    ComplexFlt *Table;
    float *win;
    ComplexShort *Table0;
    ComplexShort *Table1;
    ComplexShort *Table2;
    short fft_len;
} IQMismatchPreprocessState;

#if !defined(TIME_CAL_TABLE)
static void Tblgen_new(IQMismatchPreprocessState *st)
{
    short len = st->fft_len;
    ComplexFlt *w = st->Table;
    float target_tone = CAL_BIN;

    for (int j=-CAL_OFFSET; j<= CAL_OFFSET; j++)
    {
        float tone_tmp = target_tone + j;
        for (int i=0; i<len; i++)
        {
            w[i].re = (float)(cos(2*pi*i*tone_tmp/len));
            w[i].im = (float)(-1*sin(2*pi*i*tone_tmp/len));
        }
        w += len;
    }
    BT_DRV_TRACE(1,"[%s]table ok",__func__);
/*
    for (int i=0; i<len; i++)
    {
        st->Table0[i].re = (short)(cos(2*pi*i*target_tone/len)*32767);
        st->Table0[i].im = (short)(-1*sin(2*pi*i*target_tone/len)*32767);
        st->Table1[i].re = (short)(cos(2*pi*i*(target_tone-1)/len)*32767);
        st->Table1[i].im = (short)(-1*sin(2*pi*i*(target_tone-1)/len)*32767);
        st->Table2[i].re = (short)(cos(2*pi*i*(target_tone+1)/len)*32767);
        st->Table2[i].im = (short)(-1*sin(2*pi*i*(target_tone+1)/len)*32767);
    }
*/
}
#else
static void Tblgen_new_offset(IQMismatchPreprocessState *st, int offset)
{
    short len = st->fft_len;
    ComplexFlt *w = st->Table;
    float target_tone = CAL_BIN;

    int j = offset;
    float tone_tmp = target_tone + j;
    for (int i=0; i<len; i++)
    {
        w[i].re = (float)(cosf(2*pi*i*tone_tmp/len));
        w[i].im = (float)(-1*sinf(2*pi*i*tone_tmp/len));
    }
}
#endif
IQMismatchPreprocessState *IQMismatchPreprocessState_init(int fft_size)
{
    BT_DRV_TRACE(0,"malloc ini");
    int sweep_num = 1;
    //int need_ram = fft_size*2*2 + fft_size*sweep_num*sizeof(ComplexFlt);
    BT_DRV_TRACE(0,"fft_size = %d", fft_size);

    IQMismatchPreprocessState *st = (IQMismatchPreprocessState *)med_calloc(1, sizeof(IQMismatchPreprocessState));
    if (st){
        st->M0data = (short*)med_calloc(fft_size*2, sizeof(short));
        st->fft_len = fft_size;
        st->Table = (ComplexFlt*)med_calloc(fft_size*sweep_num, sizeof(ComplexFlt));
        st->win = (float *)med_calloc(fft_size, sizeof(float));
        for(int i=0;i<fft_size/2;i++)
        {
            st->win[i] = 0.5F*(1-cosf(2*pi*i/(float)fft_size));
            st->win[fft_size - i -1] = st->win[i];
        }

        /*
        st->Table0 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
        st->Table1 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
        st->Table2 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));

        BT_DRV_TRACE(5,"st:%p, st->M0data:%p, st->Table0:%p, st->Table1:%p, st->Table2:%p", st, st->M0data, st->Table0, st->Table1, st->Table2);
        */
        BT_DRV_TRACE(1,"[%s]malloc ok",__func__);
    }
    return st;
}

int32_t IQMismatchPreprocessState_destroy(IQMismatchPreprocessState *st)
{
    BT_DRV_TRACE(5,"st:%p, st->M0data:%p, st->Table0:%p, st->Table1:%p, st->Table2:%p", st, st->M0data, st->Table0, st->Table1, st->Table2);
    med_free(st->M0data);
    med_free(st->Table);
/*
    med_free(st->Table0);
    med_free(st->Table1);
    med_free(st->Table2);
*/
    med_free(st);
    return 0;
}

int get_cpx_fft(IQMismatchPreprocessState *st)
{
    int i,j;
    ComplexFlt tmp0;
    short fftsize = st->fft_len;
    tmp0.re = 0.0f;
    tmp0.im = 0.0f;
    float tmp_i = 0;
    float tmp_q = 0;
    float sqare_tmp = 0;
    float sqare_max = 0;
    float sqare_sum = 0;
    ComplexFlt *w = st->Table;
    for(j=-CAL_OFFSET;j<=CAL_OFFSET;j++)
    {
        Tblgen_new_offset(st,j);
        for (i=0; i<fftsize; i++)
        {
           tmp_i = (float)st->M0data[2*i];//*st->win[i];
           tmp_q = (float)st->M0data[2*i + 1];//*st->win[i];

           tmp0.re = tmp0.re + (tmp_i*w[i].re - tmp_q*w[i].im);
           tmp0.im = tmp0.im + (tmp_i*w[i].im + tmp_q*w[i].re);
        }
#if !defined(TIME_CAL_TABLE)
        w += fftsize;
#endif
        //TRACE(1,"tmp[%d]=%d+1j*%d",j,(int)tmp0.re,(int)tmp0.im);
        tmp0.re = tmp0.re/sqrtf(fftsize);
        tmp0.im = tmp0.im/sqrtf(fftsize);

        sqare_tmp = tmp0.re*tmp0.re + tmp0.im*tmp0.im;
        sqare_sum += sqare_tmp/fftsize;
        if(sqare_max < sqare_tmp)
            sqare_max = sqare_tmp;
    }

    TRACE(1,"sqare_sum=%de-2",(int)(100*sqare_sum));

/*
    for (i=0; i<fftsize; i++)
    {
        tmp_i = st->M0data[2*i];
        tmp_q = st->M0data[2*i + 1];
        tmp0.re = tmp0.re + (tmp_i*st->Table0[i].re - tmp_q*st->Table0[i].im)/32768;
        tmp0.im = tmp0.im + (tmp_i*st->Table0[i].im + tmp_q*st->Table0[i].re)/32768;
    }
    TRACE(1,"tmp0=%d+1j*%d",(int)tmp0.re,(int)tmp0.im);
*/
   return (int)sqare_max;
}

int IQMismatchParameterCalc_ex(const short *M0data,
                               short Cohcnt, ComplexShort *Table0, ComplexShort *Table1, ComplexShort *Table2,int fftsize)
{
    int i,j,k;
    float M0 = 0;
    //ComplexInt tmp0;
    ComplexFlt tmp0;
    ComplexFlt tmp1;
    ComplexFlt tmp2;
    for (j = 0; j < Cohcnt; j++)
    {
        tmp0.re = 0.0f;
        tmp0.im = 0.0f;
        tmp1.re = 0.0f;
        tmp1.im = 0.0f;
        tmp2.re = 0.0f;
        tmp2.im = 0.0f;
        k = 0;
        for (i=0; i<fftsize; i++)
        {
            if(k>=fftsize)
                k = 0;
            tmp0.re = tmp0.re + (((int)M0data[i] *(int)Table0[k].re) >> 15) ;
            tmp0.im = tmp0.im + (((int)M0data[i] * (int)Table0[k].im) >> 15) ;
            tmp1.re = tmp1.re + (((int)M0data[i] *(int)Table1[k].re) >> 15) ;
            tmp1.im = tmp1.im + (((int)M0data[i] * (int)Table1[k].im) >> 15) ;
            tmp2.re = tmp2.re + (((int)M0data[i] *(int)Table2[k].re) >> 15) ;
            tmp2.im = tmp2.im + (((int)M0data[i] * (int)Table2[k].im) >> 15) ;
            k = k + 1;
        }
        //TRACE(1,"tmp0=%d+1j*%d",(int)tmp0.re,(int)tmp0.im);
        //TRACE(1,"tmp1=%d+1j*%d",(int)tmp1.re,(int)tmp1.im);
        //TRACE(1,"tmp2=%d+1j*%d",(int)tmp2.re,(int)tmp2.im);
        tmp0.re = tmp0.re/ fftsize;// >> bitshift;
        tmp0.im = tmp0.im/ fftsize;// >>bitshift;
        tmp1.re = tmp1.re/ fftsize;// >> bitshift;
        tmp1.im = tmp1.im/ fftsize;// >>bitshift;
        tmp2.re = tmp2.re/ fftsize;// >> bitshift;
        tmp2.im = tmp2.im/ fftsize;// >>bitshift;
        M0 = M0 + tmp0.re*tmp0.re + tmp0.im*tmp0.im;
        M0 = M0 + tmp1.re*tmp1.re + tmp1.im*tmp1.im;
        M0 = M0 + tmp2.re*tmp2.re + tmp2.im*tmp2.im;
    }
    return (int)(M0/Cohcnt);
}

void caculate_energy_main_test(IQMismatchPreprocessState *st,int* Energy,int* Energy1,int fftsize)
{
    short Cohcnt = 1;
    *Energy1 = IQMismatchParameterCalc_ex(st->M0data, Cohcnt, st->Table0, st->Table1, st->Table2,fftsize);
}

static struct HAL_DMA_DESC_T iqimb_dma_desc[32];
static uint8_t g_dma_channel = HAL_DMA_CHAN_NONE;

static void iqimb_dma_dout_handler(uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if(g_dma_channel != HAL_DMA_CHAN_NONE)
    {
        hal_audma_free_chan(g_dma_channel);
    }
    hal_btdump_disable();
    iqimb_dma_status = 0;

    return;
}

#define TRANSFER_SIZE    512
int bt_iqimb_dma_enable ( short * dma_dst_data, uint16_t size)
{
    int sRet = 0;
    struct HAL_DMA_CH_CFG_T iqimb_dma_cfg;
    // BT_DRV_TRACE(0,"bt_iqimb_dma_enable :");
    int times = 0;

    times = size/TRANSFER_SIZE;
    iqimb_dma_status = 1;
    TRACE(0, "times:%d", times);

    sRet = memset_s(&iqimb_dma_cfg, sizeof(iqimb_dma_cfg), 0, sizeof(iqimb_dma_cfg));
    if (sRet){
        TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }

    //iqimb_dma_cfg.ch = hal_audma_get_chan(HAL_AUDMA_DSD_RX,HAL_DMA_HIGH_PRIO);

    iqimb_dma_cfg.ch = hal_audma_get_chan(HAL_AUDMA_BTDUMP,HAL_DMA_HIGH_PRIO);
    ASSERT((HAL_DMA_CHAN_NONE != iqimb_dma_cfg.ch), "hal_audma_get_chan failed.");
    g_dma_channel = iqimb_dma_cfg.ch;
    iqimb_dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    iqimb_dma_cfg.dst_periph = 0; //useless
    iqimb_dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    iqimb_dma_cfg.handler = (HAL_DMA_IRQ_HANDLER_T)iqimb_dma_dout_handler;
    iqimb_dma_cfg.src = 0; // useless
    iqimb_dma_cfg.src_bsize = HAL_DMA_BSIZE_4;

    //iqimb_dma_cfg.src_periph = HAL_AUDMA_DSD_RX;
    iqimb_dma_cfg.src_periph = HAL_AUDMA_BTDUMP;
    iqimb_dma_cfg.src_tsize = TRANSFER_SIZE;//size;//1600; //1600*2/26=123us
    iqimb_dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    //iqimb_dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    iqimb_dma_cfg.try_burst = 1;
    iqimb_dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    //iqimb_dma_cfg.dst = (uintptr_t)(dma_dst_data);

    //hal_audma_init_desc(&iqimb_dma_desc[0], &iqimb_dma_cfg, 0, 1);

	for(int i = 0;i < times; i++)
	{
        memset(&iqimb_dma_desc[i], 0, sizeof(iqimb_dma_desc[i]));
        iqimb_dma_cfg.dst = (uint32_t)(dma_dst_data + TRANSFER_SIZE * 2 * i);
        if(i < (times - 1))
        {
            hal_audma_init_desc(&iqimb_dma_desc[i], &iqimb_dma_cfg, &iqimb_dma_desc[i+1], 0);
        }
        else
        {
            hal_audma_init_desc(&iqimb_dma_desc[i], &iqimb_dma_cfg, 0, 1);
        }
    }

    hal_audma_sg_start(&iqimb_dma_desc[0], &iqimb_dma_cfg);

    //configed after mismatch parameter done, or apb clock muxed.

    //wait
    for(volatile int i=0; i<5000; i++);
    hal_btdump_enable();

    return 1;

}

void check_mem_data(void* data, int len)
{
    short* share_mem = (short*)data;
    BT_DRV_TRACE(3,"check_mem_data :share_mem= %p, 0x%x, 0x%x",share_mem,share_mem[0],share_mem[1]);

    int32_t i =0;
    int32_t remain = len;

    while(remain > 0)
    {
        for(i=0; i<32; i++)//output two line
        {
            if (remain >16)
            {
                DUMP16("%04X ",share_mem,16);
                share_mem +=16;
                remain -= 16;
            }
            else
            {
                DUMP16("%04X ",share_mem,remain);
                remain =0;
                return;
            }
        }
        //  BT_DRV_TRACE(0,"\n");
        //BT_DRV_TRACE(1,"addr :0x%08x\n",share_mem);
        hal_sys_timer_delay(MS_TO_TICKS(100));
    }
}

int bt_Txdc_cal_set(int ch_num, int dc_add)
{
    uint32_t val;
    uint32_t tmp = (uint32_t)dc_add;
    //sel apb clock
    val = READ_REG(0xd0350348,0x0);
    if(ch_num==0)
    {
        val &= 0xfffffc00; //bit31 1 int_en_mismatch
        val |= tmp & 0x3ff;
    }
    else
    {
        // BT_DRV_TRACE(1,"bt_Txdc_cal_set, dcadd : 0x%08x",tmp);
        val &= 0xfc00ffff; //bit31 1 int_en_mismatch
        val |= (tmp & 0x3ff) <<16;
    }
    WRITE_REG(val,0xd0350348,0x0);


    // BT_DRV_TRACE(1,"bt_Txdc_cal_set, 0xd0350348 : 0x%08x",val);

    return 1;
}

//g/p mismatch base addr 0xd0310000
//dc_i: 0xd0350348 9:0
//dc_q: 0xd0350348 25:16
int bt_iqimb_add_mismatch(int ch_num, int gain_mis, int phase_mis, int dc_i, int dc_q, uint32_t addr)
{
    uint32_t val;
    //sel apb clock
    val = READ_REG(0xd0350348,0x0);
    val &= 0x7fffffff; //bit31 1 int_en_mismatch
    WRITE_REG(val,0xd0350348,0x0);

    val = (phase_mis << 16) | (gain_mis & 0x0000ffff);
    WRITE_REG(val,addr,ch_num*4);
    //tval = READ_REG(addr,0x0);
    //BT_DRV_TRACE(1,"bt_iqimb_add_mismatch, iq : 0x%08x",tval);

    WRITE_REG(0x400000,0xd0350220,0);
    WRITE_REG(0x400000,0xd0350224,0);
    // bt_Txdc_cal_set(0,dc_i);
    //bt_Txdc_cal_set(1,dc_q);

    /*
        val = READ_REG(0xd0350348,0x0);
        val &= 0xfffffc00; //bit9:0
        val |= dc_i;
        val &= 0xfc00ffff; //bit25:16
        val |= (dc_q<<16);
        WRITE_REG(val,0xd0350348,0x0);
    */
    //sel 26m clock
    val = READ_REG(0xd0350348,0x0);
    val &= 0x7fffffff; //bit31 1 int_en_mismatch
    val |= (0x1<<31);
    WRITE_REG(val,0xd0350348,0x0);

    return 1;
}

void DC_correction(IQMismatchPreprocessState *st,int *dc_i_r,int *dc_q_r,int fftsize)
{
    uint8_t k;
    int Energy,Energy1,tmp;
    int dc_iters = 2;
    int dc_i;
    int dc_q;
    int dc_step = 4;
    int dc_i_base;
    int dc_q_base;
    int P0,PIplus,PIneg,PQplus,PQneg,PIPQ;
    int CI,CQ,tmp_D;
    dc_i_base =0;
    dc_q_base = 0;
    for(k=0; k<dc_iters; k++)
    {
        dc_i = dc_i_base;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        P0 = Energy1;
        dc_i = dc_i_base + dc_step;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIplus = Energy1;
        dc_i = dc_i_base - dc_step;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIneg = Energy1;
        dc_i = dc_i_base;
        dc_q = dc_q_base + dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PQplus = Energy1;
        dc_i = dc_i_base;
        dc_q = dc_q_base - dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PQneg = Energy1;
        dc_i = dc_i_base + dc_step;
        dc_q = dc_q_base + dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIPQ = Energy1;
        tmp = 2*PIplus*PIplus + 2*PQplus*PQplus + 2*PIPQ*PIPQ;
        tmp = tmp - 6*P0*P0;
        tmp = tmp + 4*P0*(PIneg+PQneg+PIPQ);
        tmp = tmp + 2*(PIplus*(PQplus - PQneg)-PIneg*(PQplus+PQneg));
        tmp_D = tmp - 4*PIPQ*(PIplus+PQplus);
        tmp = P0*(2*(PIplus - PIneg)+PQplus -PQneg);
        tmp = tmp - PIPQ*(PQneg-PQplus)+PIneg*(PQplus+PQneg);
        tmp = tmp + PQplus*(PQneg-PQplus-2*PIplus);
        CI = -dc_step*tmp;
        tmp = P0*(2*(PQplus - PQneg)+PIplus-PIneg);
        tmp = tmp - PIPQ*(PIneg-PIplus)+PQneg*(PIplus+PIneg);
        tmp = tmp + PIplus*(PIneg-PIplus-2*PQplus);
        CQ = -dc_step*tmp;
        dc_i_base = dc_i_base + CI/tmp_D;
        dc_q_base = dc_q_base + CQ/tmp_D;
        //dc_step = dc_step/2;
    }
    *dc_i_r = dc_i_base;
    *dc_q_r = dc_q_base;
}
int IQ_GAIN_Mismatch_Correction(IQMismatchPreprocessState *st,int phase_mis_base,int fftsize,uint32_t addr)
{
    uint8_t k = 0;
    int phase_mis_tmp =0;
    int gain_mis_tmp =0;
    int tmp = 0;
    int energy_ret0_last = 1048576;
    int energy_ret0 = 0;
    int energy_ret1 = 0;
    int energy_ret2 = 0;
    int Energy,Energy1 ;
    int iters = 4;
    int gainstep =8;
    int gain_mis_base = 0;

    phase_mis_tmp = phase_mis_base;
    energy_ret0_last = 1048576;
    for(k=0; k<iters+2; k++)
    {
        gain_mis_tmp = gain_mis_base;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret0 = Energy1;
        if(energy_ret0<50)
            break;
        if(k>0)
        {
            if(energy_ret0_last < energy_ret0)
            {
                gain_mis_base = gain_mis_base + tmp;
                gain_mis_tmp = gain_mis_base;
                bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                energy_ret0 = Energy1;
            }
        }
        //BT_DRV_TRACE(1,"IQ gain correct energy_ret0 = %d",energy_ret0);
        gainstep = 4;///////////////////
        gain_mis_tmp = gain_mis_base + gainstep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret2 = Energy1;
        gain_mis_tmp = gain_mis_base - gainstep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret1 = Energy1;
        tmp = energy_ret2 -2*energy_ret0 + energy_ret1;
        //BT_DRV_TRACE(1,"IQ gain correct energy_ret2 -2*energy_ret0 + energy_ret1 = %d",tmp);
        if(tmp>0)
        {
            tmp = (energy_ret2-energy_ret1)*gainstep/tmp/2;
            tmp = min(tmp,4*gainstep);
            gainstep = gainstep/2;
        }
        else
        {
            tmp = 0;
            iters = iters+1;
        }
        if(iters>8)
            break;
        if((gain_mis_base - tmp>60)||(gain_mis_base - tmp<-60))
            tmp = 0;
        //BT_DRV_TRACE(2,"IQ gain correct gain_mis = %d ,gain_adj = %d",gain_mis_base,tmp);
        gain_mis_base = gain_mis_base - tmp;
        energy_ret0_last = energy_ret0;
    }
    return gain_mis_base;
}
int IQ_Phase_Mismatch_Correction(IQMismatchPreprocessState *st,int gain_mis_base,int fftsize,uint32_t addr)
{
    uint8_t k = 0;
    int phase_mis_tmp =0;
    int gain_mis_tmp =0;
    int tmp = 0;
    int energy_ret0_last = 1048576;
    int energy_ret0 = 0;
    int energy_ret1 = 0;
    int energy_ret2 = 0;
    int Energy,Energy1 ;
    int iters = 4;
    int phasestep =4;
    int phase_mis_base = 0;
    tmp = 0;
    energy_ret0_last = 1048576;
    gain_mis_tmp = gain_mis_base;
    for(k=0; k<iters; k++)
    {
        phase_mis_tmp = phase_mis_base;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret0 = Energy1;
        if(k>0)
        {
            if(energy_ret0_last < energy_ret0)
            {
                phase_mis_base = phase_mis_base + tmp;
                phase_mis_tmp = phase_mis_base;
                bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                energy_ret0 = Energy1;
            }
        }
        if(energy_ret0<50)
            break;
        // BT_DRV_TRACE(1,"IQ phase correct energy_ret0 = %d",energy_ret0);
        phasestep = 4;//////////////////////////
        phase_mis_tmp = phase_mis_base + phasestep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret2 = Energy1;
        // BT_DRV_TRACE(1,"IQ phase correct energy_ret2 = %d",energy_ret2);
        phase_mis_tmp = phase_mis_base - phasestep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret1 = Energy1;
        //BT_DRV_TRACE(1,"IQ phase correct energy_ret1 = %d",energy_ret1);
        tmp = energy_ret2 -2*energy_ret0 + energy_ret1;
        // BT_DRV_TRACE(1,"IQ phase correct energy_ret2 -2*energy_ret0 + energy_ret1 = %d",tmp);
        if(tmp>0)
        {
            tmp = (energy_ret2-energy_ret1)*phasestep/tmp/2;
            tmp = min(tmp,4*phasestep);
            phasestep = phasestep/2;
        }
        else
        {
            tmp = 0;
            iters = iters + 1;
        }
        if(iters>8)
            break;
        if((phase_mis_base - tmp>60)||(phase_mis_base - tmp<-60))
            tmp = 0;
        // BT_DRV_TRACE(2,"IQ phase correct phase_mis = %d ,phase_adj = %d",phase_mis_base,tmp);
        phase_mis_base = phase_mis_base - tmp;
        energy_ret0_last = energy_ret0;

    }
    return phase_mis_base;
}

static BT_IQ_CALIBRATION_CONFIG_T config;
#if 0
static void bt_update_local_iq_calibration_val(uint32_t freq_range, uint16_t gain_cal_val, uint16_t phase_cal_val)
{
    config.validityMagicNum = BT_IQ_VALID_MAGIC_NUM;
    config.gain_cal_val[freq_range] = gain_cal_val;
    config.phase_cal_val[freq_range] = phase_cal_val;
}
#endif


// do QA calibration and get the calibration value
static POSSIBLY_UNUSED void btIqCalibration(void)
{
    btdrv_rx_iq_cal();
#if 0
    BT_DRV_TRACE(0,"Acquired calibration value:");
    for (uint32_t range = 0;range < BT_FREQENCY_RANGE_NUM;range++)
    {
        BT_DRV_TRACE(3,"%d: 0x%x - 0x%x", range,
            config.gain_cal_val[range],
            config.phase_cal_val[range]);
    }
#endif
}

#if 0
// get IQ calibration from NV
static bool bt_get_iq_calibration_val_from_nv(BT_IQ_CALIBRATION_CONFIG_T* pConfig)
{
    return false;
    if (nv_record_get_extension_entry_ptr() &&
        (BT_IQ_VALID_MAGIC_NUM ==
        nv_record_get_extension_entry_ptr()->btIqCalConfig.validityMagicNum))
    {
        *pConfig = nv_record_get_extension_entry_ptr()->btIqCalConfig;
        return true;
    }
    else
    {
        return false;
    }
}
#endif

void bt_iq_calibration_setup(void)
{
    if(0)//(bt_get_iq_calibration_val_from_nv(&config))
    {
        BT_DRV_TRACE(0,"Calibration value in NV:");

        for (uint32_t range = 0;range < BT_FREQENCY_RANGE_NUM;range++)
        {
            TRACE(3,"%d: 0x%x - 0x%x", range,
                config.gain_cal_val[range],
                config.phase_cal_val[range]);
        }
        uint32_t addr = 0xd0310000;
        int chs = 27;

        for (uint32_t range = 0;range < 3;range++)
        {
            if (0 == range)
            {
                addr = 0xd0310034;
                chs= 27;
                addr = addr-13*4;
            }

            for (uint32_t ch=0;ch < chs;ch++)
            {
                bt_iqimb_add_mismatch(0,
                    config.gain_cal_val[range],
                    config.phase_cal_val[range],
                    0,0,addr);
                addr = addr +4;
            }
        }
    }
    else
    {
        btIqCalibration();
#if 0
        uint32_t lock = nv_record_pre_write_operation();
        nv_record_get_extension_entry_ptr()->btIqCalConfig = config;
        nv_record_post_write_operation(lock);

        nv_record_update_runtime_userdata();
#endif
    }
}

int iq_gain,iq_phy;

void bt_IQ_DC_Mismatch_Correction_Release()
{
 //   int phase_mis_base = 0;
 //   int gain_mis_base  = 0;
 //   int Energy,Energy1 = 0;
    uint32_t time_start = hal_sys_timer_get();
    int fftsize;
//    uint32_t addr = 0xd0310000;
//    int chs = 27;
	IQMismatchPreprocessState *st = NULL;
    syspool_init_specific_size(MED_MEM_POOL_SIZE);
    syspool_get_buff(&g_medMemPool, MED_MEM_POOL_SIZE);
    med_heap_init(&g_medMemPool[0], MED_MEM_POOL_SIZE);
    //config gpadc 26m
    READ_REG(0xd0310000,0x0);

// BT_DRV_TRACE(1,"use time: %d ticks", (hal_sys_timer_get()-time_start));
    //fftsize = 1024;
    fftsize = FFT_SIZE;
    st = IQMismatchPreprocessState_init(fftsize);
#if !defined(TIME_CAL_TABLE)
    Tblgen_new(st);
#endif
    BT_DRV_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
    for (int k = 0; k<1; k++)
    {
        //BT_DRV_TRACE(1,"dump From DMA!");
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        //BT_DRV_TRACE(1,"GET DMA READY!");
        //check_mem_data((uint8_t *)st->M0data, fftsize*2);
        int energy_val = get_cpx_fft(st);
        BT_DRV_TRACE(1,"----MAX E=%d,E/N=%de-2----",energy_val,(int)(100.0*(float)energy_val/(float)fftsize));

#if 0
        lib_iqmis_getiq((short *)st->M0data,4096,(float *)st->data_i,(float *)st->data_q);
        BT_DRV_TRACE(2,"iq data:0x%x, 0x%x",st->data_i[0],st->data_q[0]);
        BT_DRV_TRACE(2,"iq data1:0x%x, 0x%x",st->data_i[1],st->data_q[1]);
        lib_iqmis_esti((float *)st->data_i,(float *)st->data_q,2048,&iq_gain,&iq_phy);
        BT_DRV_TRACE(2,"gain:0x%x phy: 0x%x",iq_gain,iq_phy);
#endif
        BT_DRV_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
    }
}

int bt_iqimb_test_ex (int mismatch_type)
{

    bt_IQ_DC_Mismatch_Correction_Release();

    return 1;
}

static float get_DC_energy(uint32_t addr, short dc_i, short dc_q,int num)
{
//0xD0350250
    float E_ave = 0;
    for(int cc = 0; cc < num; cc++)
    {
        int32_t i_data, q_data = 0;
        uint32_t val;

        //BT_DRV_TRACE(1,"1 0x%x=0x%x",addr,BTDIGITAL_REG(addr));

        val &= 0;
        val |= dc_i;
        val &= 0x3FFF;
        BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 0, val);
        //BT_DRV_TRACE(1,"2 0x%x=0x%x",addr,BTDIGITAL_REG(addr));

        val &= 0;
        val |= dc_q;
        val &= 0x3FFF;
        BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 14, val);
        //BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);
        hal_sys_timer_delay(US_TO_TICKS(1));
        //BT_DRV_TRACE(1,"0x%x=0x%x",addr,BTDIGITAL_REG(addr));

        //BTDIGITAL_REG_WR(0xD0220C00, 0x80020000);       //rx modem on
        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 0);//reset start mix
        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 1);//start mix

        hal_sys_timer_delay(US_TO_TICKS(400));
        i_data = *(volatile uint32_t *)(0xD0350338);
        q_data = *(volatile uint32_t *)(0xD035033C);
        i_data &= 0x3FFFFF;
        q_data &= 0x3FFFFF;
        if(0x200000 <= i_data)
            i_data -= 0x400000;
        if(0x200000 <= q_data)
            q_data -= 0x400000;
        i_data = i_data/8;
        q_data = q_data/8;
        float E = sqrtf(i_data*i_data + q_data*q_data);
        //BT_DRV_TRACE(3,"geti=%d,q=%d,e_tmp=%d",dc_i,dc_q,(int)E);
        E_ave = E_ave + E;
    }
    E_ave = E_ave / num;
    return E_ave;
}


void set_gain_phase(uint32_t addr, short gain, short phase)
{
    BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);
    uint32_t val;
    val &= 0;
    val |= gain;
    val &= 0x3FFF;
    BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 0, val);
    val &= 0;
    val |= phase;
    val &= 0x3FFF;
    BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 16, val);
}

static float get_iq_energy(uint32_t addr, short gain,short phase,int num)
{
//0xD0310000
    float E_ave = 0;
    for(int cc = 0; cc < num; cc++) {
        int32_t i_data, q_data = 0;
        uint32_t val;

        BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);

        val &= 0;
        val |= gain;
        val &= 0x3FFF;
        BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 0, val);
        val &= 0;
        val |= phase;
        val &= 0x3FFF;
        BTDIGITAL_REG_SET_FIELD(addr, 0x3FFF, 16, val);
        hal_sys_timer_delay(US_TO_TICKS(1));
        //BT_DRV_TRACE(1,"0xD0310000=0x%x",BTDIGITAL_REG(0xD0310000));
        BTDIGITAL_REG_WR(0xD0330058, 0x00800030);
        BTDIGITAL_REG_WR(0xD0330060, 0xFFFFFFFF);       //rst
        BTDIGITAL_REG_WR(0xD0330064, 0xFFFFFFFF);       //clr rst
        BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 1);

        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 0);//reset start mix
        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 1);//start mix

        hal_sys_timer_delay(US_TO_TICKS(400));
        i_data = *(volatile uint32_t *)(0xD0350338);
        q_data = *(volatile uint32_t *)(0xD035033C);
        i_data &= 0x3FFFFF;
        q_data &= 0x3FFFFF;
        //BT_DRV_TRACE(2,"i=0x%x,q=0x%x",i_data,q_data);
        if(0x200000 <= i_data)
            i_data -= 0x400000;
        if(0x200000 <= q_data)
            q_data -= 0x400000;
        i_data = i_data/8;
        q_data = q_data/8;
        float E = sqrtf(i_data*i_data + q_data*q_data);
#ifdef IQ_DEBUG
        //BT_DRV_TRACE(3,"n=%d,g=%d,p=%d,e=%d",cc,gain,phase,(int)E);
#endif
        E_ave = E_ave + E;
    }
    E_ave = E_ave / num;
    return E_ave;
}


int get_best_i(int min_i,float min_energy,int base_q,int step,int* im, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t e_addr = 0xD0350250;
    int i_left = 0;
    int i_right = 0;
    int i_middle = 0;
    int base_i = 0;

#if 0//only for debug
    float Energy1;
    int i_reg,q_reg;
    q_reg = base_q;
	for (i_reg = min_i - 50; i_reg <= min_i + 50; i_reg+=2)
    {
        Energy1 = get_DC_energy(e_addr, i_reg, q_reg, 3);
        BT_DRV_TRACE(3, "i_reg=%d,q_reg=%d,E=%d", i_reg, q_reg, (int)Energy1);
    }
#endif

    i_left = min_i - step;
    energy_left = get_DC_energy(e_addr, i_left, base_q, 1);
    i_right = min_i + step;
    energy_right = get_DC_energy(e_addr, i_right, base_q, 1);
    while(1)
    {
        i_middle = (i_left + i_right) / 2;
        if(ABS(i_left - i_right) < 17)
        {
            energy_middle = get_DC_energy(e_addr, i_middle, base_q, 3);

        }
        else
        {
            energy_middle = get_DC_energy(e_addr, i_middle, base_q, 2);
        }
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"Mi=%d,q=%d,e=%d,DC_Emin=%d",i_middle,base_q,(int)energy_middle,(int)*em);
        BT_DRV_TRACE(4,"Li=%d,ri=%d,le=%d,re=%d",i_left,i_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right)
        {
            i_left = i_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_i = i_right;
        }
        else
        {
            i_right = i_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_i = i_left;
        }
        if(min_energy < *em)
        {
            *im = min_i;
            *em = min_energy;
        }
#ifdef DC_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_i=%d,best_q=%d,energy_min=%d,DC_Emin=%d",min_i,base_q,(int)min_energy,(int)*em);
#endif
        if(ABS(i_left - i_right) < 4)
            break;
    }

    if(energy_left > energy_right)
    {
        min_energy = energy_right;
        min_i = i_right;
    }
    else
    {
        min_energy = energy_left;
        min_i = i_left;
    }
    if(min_energy < *em)
    {
        *im = min_i;
        *em = min_energy;
    }
#ifdef DC_DEBUG
    BT_DRV_TRACE(4,"LAST_i=%d,q=%d,e=%d,DC_Emin=%d",min_i,base_q,(int)min_energy,(int)*em);
#endif
    if(min_i != *im)
    {
        *em = get_DC_energy(e_addr, *im, base_q, 3);
        if(*em > min_energy)
        {
            *im = min_i;
            *em = min_energy;
        }
    }
    base_i = *im;
    return base_i;
}

int get_best_q(int min_q,float min_energy,int base_i,int step,int* qm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t e_addr = 0xD0350250;
    int q_left = 0;
    int q_right = 0;
    int q_middle = 0;
    int base_q = 0;

#if 0//only for debug
    float Energy1;
    int i_reg,q_reg;
    i_reg = base_i;
	for (q_reg = min_q - 50; q_reg <= min_q + 50; q_reg+=2)
    {
        Energy1 = get_DC_energy(e_addr, i_reg, q_reg, 3);
        BT_DRV_TRACE(3, "i_reg=%d,q_reg=%d,E=%d", i_reg, q_reg, (int)Energy1);
    }
#endif

    q_left = min_q - step;
    energy_left = get_DC_energy(e_addr, base_i, q_left, 1);
    q_right = min_q + step;
    energy_right = get_DC_energy(e_addr, base_i, q_right, 1);
    while(1)
    {
        q_middle = (q_left+q_right) / 2;
        if(ABS(q_left - q_right) < 17)
        {
            energy_middle = get_DC_energy(e_addr, base_i, q_middle, 3);
        }
        else
        {
            energy_middle = get_DC_energy(e_addr, base_i, q_middle, 2);
        }
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"Mi=%d,q=%d,e=%d,DC_Emin=%d",base_i,q_middle,(int)energy_middle,(int)*em);
#endif
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"Lq=%d,rq=%d,le=%d,re=%d",q_left,q_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right)
        {
            q_left = q_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_q = q_right;
        }
        else
        {
            q_right = q_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_q = q_left;
        }
        if(min_energy < *em)
        {
            *qm = min_q;
            *em = min_energy;
        }
#ifdef DC_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_i=%d,best_q=%d,energy_min=%d,DC_Emin=%d",base_i,min_q,(int)min_energy,(int)*em);
#endif
        if(ABS(q_left - q_right) < 4)
            break;
    }

    if(energy_left > energy_right)
    {
        min_energy = energy_right;
        min_q = q_right;
    }
    else
    {
        min_energy = energy_left;
        min_q = q_left;
    }
    if(min_energy < *em)
    {
        *qm = min_q;
        *em = min_energy;
    }
#ifdef DC_DEBUG
    BT_DRV_TRACE(4,"LAST_i=%d,q=%d,e=%d,DC_Emin=%d",base_i,min_q,(int)min_energy,(int)*em);
#endif
    if(min_q != *qm)
    {
        *em = get_DC_energy(e_addr, base_i, *qm, 3);
        if(*em > min_energy)
        {
            *qm = min_q;
            *em = min_energy;
        }
    }
    base_q = *qm;
    return base_q;
}

static int get_best_gain(uint32_t addr,int min_gain,float min_energy,int base_phase,int step,int* gm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t en_addr = addr;
    int gain_left = 0;
    int gain_right = 0;
    int gain_middle = 0;
    int base_gain = 0;

#if 0//only for debug
    float Energy1;
    int gain_reg,phase_reg;
    phase_reg = base_phase;
	for (gain_reg = min_gain - 50; gain_reg <= min_gain + 50; gain_reg+=2)
    {
        Energy1 = get_iq_energy(en_addr, gain_reg, phase_reg, 2);
        BT_DRV_TRACE(3, "gain_reg=%d,phase_reg=%d,E=%d", gain_reg, phase_reg, (int)Energy1);
        // hal_iomux_set_analog_i2c();
        // while(1){}
    }
#endif

    gain_left = min_gain - step;
    energy_left = get_iq_energy(en_addr, gain_left, base_phase, 2);
    gain_right = min_gain + step;
    energy_right = get_iq_energy(en_addr, gain_right, base_phase, 2);
    while(1)
    {
        gain_middle = (gain_left+gain_right) / 2;
        if(ABS(gain_left - gain_right) < 8)
        {
            energy_middle = get_iq_energy(en_addr, gain_middle, base_phase, 4);
        }
        else
        {
            energy_middle = get_iq_energy(en_addr, gain_middle, base_phase, 2);
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(4,"Mg=%d,p=%d,e=%d,IQ_Emin=%d",gain_middle,base_phase,(int)energy_middle,(int)*em);
        BT_DRV_TRACE(4,"Lg=%d,rg=%d,le=%d,re=%d",gain_left,gain_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right)
        {
            gain_left = gain_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_gain = gain_right;
        }
        else
        {
            gain_right = gain_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_gain = gain_left;
        }
        if(min_energy < *em)
        {
            *gm = min_gain;
            *em = min_energy;
        }

#ifdef IQ_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",min_gain,base_phase,(int)min_energy,(int)*em);
#endif
        if(ABS(gain_left-gain_right) < 2)
            break;
    }

    if(energy_left > energy_right)
    {
        min_energy = energy_right;
        min_gain = gain_right;
    }
    else
    {
        min_energy = energy_left;
        min_gain = gain_left;
    }
    if(min_energy < *em)
    {
        *gm = min_gain;
        *em = min_energy;
    }
#ifdef IQ_DEBUG
    //BT_DRV_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",min_gain,base_phase,(int)min_energy,(int)*em);
#endif
    if(min_gain != *gm)
    {
        *em = get_DC_energy(en_addr, *gm, base_phase, 3);
        if(*em > min_energy)
        {
            *gm = min_gain;
            *em = min_energy;
        }
    }
    base_gain = *gm;
    return base_gain;
}

static int get_best_phase(uint32_t addr,int min_phase,float min_energy,int base_gain,int step,int* pm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t en_addr = addr;
    int phase_left = 0;
    int phase_right = 0;
    int phase_middle = 0;
    int base_phase = 0;

#if 0//only for debug
    float Energy1;
    int gain_reg,phase_reg;
    gain_reg = base_gain;
	for (phase_reg = min_phase - 50; phase_reg <= min_phase + 50; phase_reg+=2)
    {
        Energy1 = get_iq_energy(en_addr, gain_reg, phase_reg, 2);
        BT_DRV_TRACE(3, "gain_reg=%d,phase_reg=%d,E=%d", gain_reg, phase_reg, (int)Energy1);
    }
#endif

    phase_left = min_phase - step;
    energy_left = get_iq_energy(en_addr, base_gain, phase_left, 2);
    phase_right = min_phase + step;
    energy_right = get_iq_energy(en_addr, base_gain, phase_right, 2);
    while(1)
    {
        phase_middle = (phase_left + phase_right) / 2;
        if(ABS(phase_left - phase_right) < 8)
        {
            energy_middle = get_iq_energy(en_addr,base_gain,phase_middle,4);
        }
        else
        {
            energy_middle = get_iq_energy(en_addr,base_gain,phase_middle,2);
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(4,"Mg=%d,p=%d,e=%d,IQ_Emin=%d",base_gain,phase_middle,(int)energy_middle,(int)*em);
        BT_DRV_TRACE(4,"Lp=%d,rp=%d,le=%d,re=%d",phase_left,phase_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right)
        {
            phase_left = phase_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_phase = phase_right;
        }
        else
        {
            phase_right = phase_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_phase = phase_left;
        }
        if(min_energy < *em)
        {
            *pm = min_phase;
            *em = min_energy;
        }
#ifdef IQ_DEBUG
        //BT_DRV_TRACE(4,"DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
#endif
        if(ABS(phase_left - phase_right) < 2)
            break;
    }

    if(energy_left > energy_right)
    {
        min_energy = energy_right;
        min_phase = phase_right;
    }
    else
    {
        min_energy = energy_left;
        min_phase = phase_left;
    }
    if(min_energy < *em)
    {
        *pm = min_phase;
        *em = min_energy;
    }
#ifdef IQ_DEBUG
    //BT_DRV_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
#endif
    BT_DRV_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
    if(min_phase != *pm)
    {
        *em = get_DC_energy(en_addr, base_gain, *pm, 3);
        if(*em > min_energy)
        {
            *pm = min_phase;
            *em = min_energy;
        }
    }
    base_phase = *pm;
    BT_DRV_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
    return base_phase;
}

void dc_iq_calib_1306(void)
{
    uint32_t time_start = hal_sys_timer_get();

#ifdef DC_CALIB
    int DC_i_min = 0;
    int DC_q_min = 0;
    uint32_t e_addr = 0xD0350250;
    float DC_energy_min = 1000000;

    int base_i = 0;
    int base_q = 0;
    int min_i = 0;
    int min_q = 0;
    int step = SWEEP_STEP;
    float min_energy = 1000000;
    for (int q1=DC_RANGE_MIN; q1<=DC_RANGE_MAX; q1+=step) {
        float e_tmp = get_DC_energy(e_addr,base_i,q1,1);
        if(min_energy > e_tmp) {
            min_q = q1;
            min_energy = e_tmp;
            DC_q_min = min_q;
            DC_energy_min = min_energy;
        }
#ifdef DC_DEBUG
    //BT_DRV_TRACE(4,"i=%d,q=%d,e_tmp=%d,DC_Emin=%d",base_i,q1,(int)e_tmp,(int)DC_energy_min);
#endif
    }
#ifdef DC_DEBUG
    BT_DRV_TRACE(4,"--------------SWEEP q=%d,e_tmp=%d--------------",min_q,(int)min_energy);
#endif
    base_q = get_best_q(min_q,min_energy,base_i,step,&DC_q_min,&DC_energy_min);
#ifdef DC_DEBUG
    BT_DRV_TRACE(2,"Step1:best_q=%d,energy_min=%d",base_q,(int)DC_energy_min);
#endif

    if(DC_ENERGY_THD < DC_energy_min) {
        min_energy = 1000000;
        for (int i1=DC_RANGE_MIN; i1<=DC_RANGE_MAX; i1+=step) {
            float e_tmp=get_DC_energy(e_addr,i1,base_q,1);
            if(min_energy > e_tmp) {
                min_i = i1;
                min_energy = e_tmp;
                DC_i_min = min_i;
                DC_energy_min = min_energy;
            }
#ifdef DC_DEBUG
            //BT_DRV_TRACE(4,"i=%d,q=%d,e_tmp=%d,DC_Emin=%d",i1,base_q,(int)e_tmp,(int)DC_energy_min);
#endif
        }
#ifdef DC_DEBUG
        BT_DRV_TRACE(4,"--------------SWEEP i=%d,e_tmp=%d--------------",min_i,(int)min_energy);
#endif
        base_i = get_best_i(min_i,min_energy,base_q,step,&DC_i_min,&DC_energy_min);
#ifdef DC_DEBUG
        BT_DRV_TRACE(3,"Step2:abest_i=%d,best_q=%d,energy_min=%d",base_i,base_q,(int)DC_energy_min);
#endif
        if (DC_ENERGY_THD < DC_energy_min) {
            base_q = get_best_q(min_q,min_energy,base_i,step,&DC_q_min,&DC_energy_min);
#ifdef DC_DEBUG
            BT_DRV_TRACE(3,"Step3:abest_i=%d,best_q=%d,energy_min=%d",base_i,base_q,(int)DC_energy_min);
#endif
        }
    }
#endif

    float teste = get_DC_energy(e_addr,base_i,base_q,1);
    BT_DRV_TRACE(3,"FINAL DC best_i=%d,best_q=%d,e=%d,energy_min=%d",
    base_i,base_q,(int)teste,(int)DC_energy_min);

    //IQ
    BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 23, 0);
#ifdef IQ_CALIB
    uint32_t en_addr_base = 0xD0310000;
    uint32_t en_addr = 0x0;
    for (int k = 0; k<3; k++) {
        uint32_t freq_add = 0x80020000;
        if(k==0) {
            freq_add += 13;
            BTDIGITAL_REG_WR(0xd0350248, 0xBF8E39);
            BTDIGITAL_REG_WR(0xD0220C00, freq_add);
            en_addr = en_addr_base + 13 * 4;
        } else if(k==1) {
            freq_add += 39;
            BTDIGITAL_REG_WR(0xd0350248, 0xC24E90);
            BTDIGITAL_REG_WR(0xD0220C00, freq_add);
            en_addr = en_addr_base + 39 * 4;
        } else {
            freq_add += 65;
            BTDIGITAL_REG_WR(0xd0350248, 0xC50EE6);
            BTDIGITAL_REG_WR(0xD0220C00, freq_add);
            en_addr = en_addr_base + 65 * 4;
        }
        int iq_gain_min = 0;
        int iq_phase_min = 0;
        float iq_energy_min = 1000000;
        int base_phase = 0;
        int base_gain = 0;
        int min_phase = 0;
        int min_gain = 0;
        step = SWEEP_STEP;
        min_energy = 1000000;
        for(int gain1=IQ_RANGE_MIN; gain1<=IQ_RANGE_MAX; gain1+=step) {
            float e_tmp=get_iq_energy(en_addr,gain1,base_phase,1);
            if(min_energy > e_tmp) {
                min_gain = gain1;
                min_energy = e_tmp;
                iq_gain_min = min_gain;
                iq_energy_min = min_energy;
            }
#ifdef IQ_DEBUG
        //BT_DRV_TRACE(4,"gain=%d,phase=%d,e_tmp=%d,IQ_Emin=%d",gain1,base_phase,(int)e_tmp,(int)iq_energy_min);
#endif
        }
#ifdef IQ_DEBUG
        BT_DRV_TRACE(4,"--------------SWEEP gain=%d,e_tmp=%d--------------",min_gain,(int)min_energy);
#endif
        base_gain=get_best_gain(en_addr,min_gain,min_energy,base_phase,step,&iq_gain_min,&iq_energy_min);
#ifdef IQ_DEBUG
        BT_DRV_TRACE(2,"Step1:best_gain=%d,energy_min=%d",base_gain,(int)iq_energy_min);
#endif

        if(IQ_ENERGY_THD < iq_energy_min) {
            min_energy = 1000000;
            for(int phase1=IQ_RANGE_MIN; phase1<=IQ_RANGE_MAX; phase1+=step){
                float e_tmp=get_iq_energy(en_addr,base_gain,phase1,1);
                if(min_energy > e_tmp) {
                    min_phase = phase1;
                    min_energy = e_tmp;
                    iq_phase_min = min_phase;
                    iq_energy_min = min_energy;
                }
#ifdef IQ_DEBUG
            //BT_DRV_TRACE(4,"gain=%d,phase=%d,e_tmp=%d,IQ_Emin=%d",base_gain,phase1,(int)e_tmp,(int)iq_energy_min);
#endif
            }
#ifdef IQ_DEBUG
            BT_DRV_TRACE(4,"--------------SWEEP phase=%d,e_tmp=%d--------------",min_phase,(int)min_energy);
#endif
            base_phase=get_best_phase(en_addr,min_phase,min_energy,base_gain,step,&iq_phase_min,&iq_energy_min);
#ifdef IQ_DEBUG
            BT_DRV_TRACE(3,"Step2:best_gain=%d,best_phase=%d,energy_min=%d",base_gain,base_phase,(int)iq_energy_min);
#endif

            if(IQ_ENERGY_THD < iq_energy_min) {
                base_gain=get_best_gain(en_addr,min_gain,min_energy,base_phase,step,&iq_gain_min,&iq_energy_min);
#ifdef IQ_DEBUG
                BT_DRV_TRACE(3,"Step3:best_gain=%d,best_phase=%d,energy_min=%d",base_gain,base_phase,(int)iq_energy_min);
#endif
            }
        }

        float testen=get_iq_energy(en_addr,base_gain,base_phase,1);
        BT_DRV_TRACE(3,"FINAL %d,best_g=%d,best_p=%d,e=%d,energy_min=%d",k,base_gain,base_phase,(int)testen,(int)iq_energy_min);

        int en_addr_start = 0;
        int en_addr_end = 0;
        if(k==0) {
            en_addr_start= 0;
            en_addr_end= 25;        //channel 2402-2427
        } else if(k==1) {
            en_addr_start= 26;
            en_addr_end= 51;        //channel 2428-2453
        } else {
            en_addr_start= 52;
            en_addr_end= 78;        //channel 2454-2480
        }
        for (int channel = en_addr_start; channel <= en_addr_end; channel++) {
            uint32_t addre=en_addr_base+channel*4;
            set_gain_phase(addre,base_gain,base_phase);

        }
    }
#endif

    BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 1);
    BT_DRV_TRACE(2,"%s use time: %d ms", __func__,__TICKS_TO_MS(hal_sys_timer_get()-time_start));
}


#endif//RX_IQ_CAL
#endif

