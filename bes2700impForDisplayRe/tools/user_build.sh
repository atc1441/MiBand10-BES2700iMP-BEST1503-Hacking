#! /bin/bash

#编译目标
TARGET_PRJ="best1306"

make T=$TARGET_PRJ -j \
        CHIP=best1306  \
        LEA_ENABLE=1 A2DP_LC3_ON=1 \
        BES_OTA=1 \
        FLASH_SIZE=0x400000 \
        DEBUG=1 ALLOW_WARNING=1 \
        SPEECH_TX_2MIC_NS8=1 \
        RAM_SIZE=340*1024  TRACE_BUF_SIZE=30*1024 \
        SEARCH_UI_COMPATIBLE_UI_V2=1 IBRT_SEARCH_UI=1 \
        ANC_ENABLE=1
        # A2DP_VIRTUAL_SURROUND=1


make T=prod_test/ota_copy -s -j \
        CHIP=best1306 \
        FLASH_SIZE=0x400000 \
        BES_OTA=1 \
        TOTA_v2=1 \
        DEBUG=1
