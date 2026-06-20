#ifndef __LHDC_CONFIG_H__
#define __LHDC_CONFIG_H__

#include "bluetooth_bt_api.h"
//
// Config for LHDC Encode
//

#define MALLOC_USED                     0
//#define ENC_LPF_FIXED_POINT			1


// 20190402 .start
//#define ENC_LPC_V_MAX_ORDER			1
#define ENC_LPC_V_USE_32BIT			1
// 20190402 .end



//
// Config for LHDC Decode
//

//#define DEC_LPF_FIXED_POINT			1



// 20190402 .start
//#define DEC_LPC_V_MAX_ORDER			1
//#define DEC_ADSP_LPC_V_USE_ADSP		1
//#define DEC_ADSP_LPC_V_MAX_ORDER		1
// 20190402 .end


#endif
