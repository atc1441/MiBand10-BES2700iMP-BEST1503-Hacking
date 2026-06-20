## ------------------------------------------------------------------- ##
## BT service features config
## ------------------------------------------------------------------- ##
export BT_SERVICE_ENABLE ?= 1
ifeq ($(BT_SERVICE_ENABLE),1)
KBUILD_CPPFLAGS += -DBT_SERVICE_ENABLE

export BTHOST_USE_IBRT ?= 1
ifeq ($(BTHOST_USE_IBRT),1)
export IBRT ?= 1
export IBRT_UI ?= 1
export BES_AUD ?= 1
ifeq ($(IBRT_UI),1)
KBUILD_CPPFLAGS += -DIBRT_UI
endif #IBRT_UI
endif #BTHOST_USE_IBRT

## ------------------------------------------------------------------- ##
## framework config
## ------------------------------------------------------------------- ##
# ****************************************** #
# BT_SVC_FW_PRODUCT Optional:
#     BT_SVC_FW_PRODUCT_DONGLE
#     BT_SVC_FW_PRODUCT_EARBUDS
#     BT_SVC_FW_PRODUCT_HEADSET
#     BT_SVC_FW_PRODUCT_GLASSES
#     BT_SVC_FW_PRODUCT_SPEAKER
#     BT_SVC_FW_PRODUCT_WATCH
# ****************************************** #
export BT_SVC_FW_PRODUCT ?= BT_SVC_FW_PRODUCT_EARBUDS
ifneq ($(BT_SVC_FW_PRODUCT),)
export BT_SVC_FW_ENABLED ?= 1
KBUILD_CPPFLAGS += -D$(BT_SVC_FW_PRODUCT)
endif

## ------------------------------------------------------------------- ##
## module config
## ------------------------------------------------------------------- ##
export BT_SVC_MODULE_ENABLED ?= 0
export BT_SVC_MODULE_BT_ENABLED ?= 0
ifeq ($(BT_SVC_MODULE_BT_ENABLED),1)
BT_SVC_MODULE_ENABLED := 1
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_BT_ENABLED
endif #BT_SVC_MODULE_BT_ENABLED

export BT_SVC_MODULE_TWS_ENABLED ?= 0
ifeq ($(BT_SVC_MODULE_TWS_ENABLED),1)
BT_SVC_MODULE_ENABLED := 1
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_TWS_ENABLED

export BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH ?= 0
ifeq ($(BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH),1)
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH
endif #BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH
endif #BT_SVC_MODULE_TWS_ENABLED

export BT_SVC_MODULE_IBRT_ENABLED ?= 1
ifeq ($(BT_SVC_MODULE_IBRT_ENABLED),1)
BT_SVC_MODULE_ENABLED := 1
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_IBRT_ENABLED
endif #BT_SVC_MODULE_IBRT_ENABLED

export BT_SVC_MODULE_LEA_ENABLED ?= 0
ifeq ($(BLE_AUDIO_ENABLED),1)
BT_SVC_MODULE_LEA_ENABLED := 1
endif

ifeq ($(BT_SVC_MODULE_LEA_ENABLED),1)
BT_SVC_MODULE_ENABLED := 1
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_LEA_ENABLED

BLE ?= 1  # ENABLE BTHOST BLE FRATURE
BLE_AUDIO_ENABLED ?= 1 #ENABLE BTHOST BLE AUDIO FEATURE

export BT_SVC_MODULE_LEA_BC_SRC_ENABLED ?= 0
ifeq ($(BT_SVC_MODULE_LEA_BC_SRC_ENABLED),1)
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_LEA_BC_SRC_ENABLED
export APP_BLE_BIS_SRC_ENABLE := 1
export BT_SERVICE_HEAP_SIZE ?= 100*1024
export AUDIO_OUTPUT_ROUTE_SELECT := 1
export PLAYBACK_FORCE_48K=1
export ISO_BUF_NB ?= 20
export ISO_BUF_SIZE ?= 250
endif #BT_SVC_MODULE_LEA_BC_SRC_ENABLED

endif #BT_SVC_MODULE_LEA_ENABLED

export BT_SVC_MODULE_WT_ENABLED ?= 0
ifeq ($(BT_SVC_MODULE_WT_ENABLED),1)
BT_SVC_MODULE_ENABLED := 1
KBUILD_CPPFLAGS += -DBT_SVC_MODULE_WT_ENABLED
endif #BT_SVC_MODULE_WT_ENABLED

## ------------------------------------------------------------------- ##
## BT service includes header file path
## ------------------------------------------------------------------- ##
export BT_SERVICE_UX_DIR_PATH = services/btservice
export BT_SERVICE_UX_INCLUDES = \
    -I$(BT_SERVICE_UX_DIR_PATH)/base/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/common/ibrt/app_ibrt/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/common/ibrt/ibrt_ui/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/common/ibrt/ibrt_core/inc  \
    -I$(BT_SERVICE_UX_DIR_PATH)/common/ibrt/custom_api \
    -I$(BT_SERVICE_UX_DIR_PATH)/common/ibrt/ibrt_middleware/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/common/ble_audio_core \
    -I$(BT_SERVICE_UX_DIR_PATH)/product/earbud_tws/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/tws/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/lea/api \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/lea/adapter_api \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/lea/adapter_api/ble_audio_central \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/bt/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/ibrt/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/module/tws/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/framework/product/earbuds/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/framework/product/dongle/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/framework/product/earphone/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/framework/product/glasses/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/framework/product/speaker/inc \
    -I$(BT_SERVICE_UX_DIR_PATH)/framework/product/watch/inc
endif #BT_SERVICE_ENABLE
