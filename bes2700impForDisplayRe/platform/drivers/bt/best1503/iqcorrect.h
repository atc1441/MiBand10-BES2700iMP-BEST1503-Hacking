#ifndef __IQ_CORRECT_H__
#define __IQ_CORRECT_H__

#ifdef __cplusplus
extern "C" {
#endif


// configure the bt iq calibration on system boot-up
void bt_iq_calibration_setup(void);

void btdrv_rx_iq_cal(void);

void dc_iq_calib_1306(void);

#ifdef __cplusplus
}
#endif

#endif // __IQ_CORRECT_H__

