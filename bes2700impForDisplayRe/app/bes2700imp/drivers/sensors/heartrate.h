/* heartrate.h — PPG / heart-rate AFE (MiBand9). [SCAFFOLD]
 * Optical PPG front-end over I2C (LED drive + photodiode ADC -> sample stream -> HR algo).
 * Part + address TODO(probe) (wrist PPG AFEs e.g. 0x44/0x57 class). The HR algorithm itself
 * (peak/FFT) runs on the sample stream this driver delivers. */
#ifndef BES2700IMP_HEARTRATE_H
#define BES2700IMP_HEARTRATE_H
#include <stdint.h>
#include <stdbool.h>
int  hr_init(void);
void hr_start(void);                 /* enable LEDs + sampling */
void hr_stop(void);
bool hr_read_sample(uint32_t *ppg);  /* one PPG sample from the AFE FIFO */
#endif
