CHIP        ?= best1503

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

# best1503: MUST be 0. LIBC_ROM=1 makes libc (memset/memcpy/...) ROM thunks that
# read the function address from the chip ROM's jump table at a fixed offset — but
# that table holds best1306 ROM addresses, so on best1503 the thunk branches to a
# garbage address and faults (INVSTATE) before main(). =0 uses the in-flash libc.
LIBC_ROM    ?= 0

export LIBC_OVERRIDE ?= 1

KERNEL      ?= RTX5
VERSION_INFO ?= best1306_ibrt

export USER_SECURE_BOOT	?= 0
# enable:1
# disable:0

WATCHER_DOG ?= 1

# Single selector for the whole debug serial (early dbg_boot_init checkpoints in
# main.cpp *and* the SDK hal_trace). Override on the make cmd line, e.g.
# `make T=best1503 DEBUG_PORT=1`.
DEBUG_PORT  ?= 2
# 0: usb
# 1: uart0 -> TXD on P2_3 = GPIO19
# 2: uart1 -> TXD on P2_1 = GPIO17 (auto-enables UART1_IOMUX_INDEX=20 in common.mk)

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

export RF_INIT_XTAL_CAP_FROM_NV ?= 1

export BLE_EXT_ADV_TX_PWR_INDEPEND ?= 0

export AFH_ASSESS ?= 0 #disable afh to avoid ble adv assert

export BT_RAMRUN ?= 0

export HW_AGC ?= 0

export NEW_SWAGC_MODE ?= 0

export BLE_NEW_SWAGC_MODE ?= 1

export BT_INFO_CHECKER ?= 1

export APP_RSSI ?= 1

export BT_TEST_CURRENT_KEY ?= 0

export BT_UART_LOG_P16 ?= 0

export 3WIRE_TPORTS ?= 0

export 2WIRE_TPORTS ?= 0

export BT_UART_LOG ?= 0

export BT_SYSTEM_52M ?= 0

export BES_FA_MODE ?= 0

export LL_MONITOR ?= 0

export SOFTBIT_EN ?= 0

export ACL_DATA_CRC_TEST ?= 0

export FORCE_SIGNALINGMODE ?= 0

export FORCE_NOSIGNALINGMODE ?= 0

export BT_FA_ECC ?= 0

ifeq ($(BT_FA_ECC),1)
export BT_FA_SCO_ECC ?= 0
export BT_FA_ACL_ECC ?= 0
endif

export BT_ECC_USER_DEFINE ?= 0
ifeq ($(BT_ECC_USER_DEFINE),1)
export BT_ECC_CONFIG_BLK ?= 1
export BT_ECC_USER_DEFINE_LENGTH ?= 8 #byte
else
export BT_ECC_CONFIG_BLK ?= 3
endif

export BT_BID_ECC_EN ?= 0

export BT_FAST_LOCK_ENABLE ?= 0

export IBRT_TESTMODE ?= 0

export CONTROLLER_DUMP_ENABLE ?= 1

export INTERSYS_DEBUG ?= 1

export PROFILE_DEBUG ?= 0

export BTDUMP_ENABLE ?= 0

export BT_DEBUG_TPORTS ?= 0

export TPORTS_KEY_COEXIST ?= 0

export BT_SIGNALTEST_SLEEP_EN ?= 0

export TX_PULLING_CAL ?= 1
export DISPLAY_PREFIX_HCI_CMD_EVT_ ?= 0

AUDIO_OUTPUT_MONO ?= 0

AUDIO_OUTPUT_DIFF ?= 0

HW_FIR_EQ_PROCESS ?= 0

SW_IIR_EQ_PROCESS ?= 0

SW_IIR_PROMPT_EQ_PROCESS ?= 0

HW_DAC_IIR_EQ_PROCESS ?= 1

IIR_EQ_PROCESS_LR_2CH ?= 0

HW_IIR_EQ_PROCESS ?= 0

HW_DC_FILTER_WITH_IIR ?= 0

AUDIO_DYNAMIC_BOOST ?= 0

AUDIO_DRC ?= 0

AUDIO_LIMITER ?= 0

export AUDIO_EQ_DYNAMICS_MEM ?= 1

AUDIO_HEARING_COMPSATN ?= 0

PC_CMD_UART ?= 0

TOTA_EQ_TUNING ?= 0

AUDIO_SECTION_ENABLE ?= 0

AUDIO_RESAMPLE ?= 1

RESAMPLE_ANY_SAMPLE_RATE ?= 1

OSC_26M_X4_AUD2BB ?= 0

export SYS_USE_BBPLL ?= 1

AUDIO_OUTPUT_VOLUME_DEFAULT ?= 16

# range:1~16

CODEC_DAC_MULTI_VOLUME_TABLE ?= 0

AUDIO_INPUT_CAPLESSMODE ?= 0

AUDIO_INPUT_LARGEGAIN ?= 0

AUDIO_CODEC_ASYNC_CLOSE ?= 0

AUDIO_SCO_BTPCM_CHANNEL ?= 1

export A2DP_KARAOKE ?= 0

export KARAOKE_ALGO_EQ ?= 0

export KARAOKE_ALGO_NS ?= 0

export A2DP_CP_ACCEL ?= 1

export SCO_CP_ACCEL ?= 1

export SCO_TRACE_CP_ACCEL ?= 0

LARGE_RAM ?= 1

HSP_ENABLE ?= 0

SBC_FUNC_IN_ROM ?= 0

ifneq ($(filter 1 ,$(ARM_CMNS)),)
ROM_UTILS_ON ?= 0
else
ROM_UTILS_ON ?= 1
export LC3_IN_ROM ?=1
endif

APP_LINEIN_A2DP_SOURCE ?= 0

APP_I2S_A2DP_SOURCE ?= 0

VOICE_PROMPT ?= 1

TWS_PROMPT_SYNC ?= 1

REPORT_CONNECTIVITY_LOG ?= 0

ifneq ($(AUDIO_RESAMPLE),1)
export AUDIO_USE_BBPLL := 1
endif

# TOTA1: old tota, TOTA2: new tota(debuging)
export TOTA ?= 0
export TOTA_v2 ?= 0

ifeq ($(TOTA_v2),1)
export TOTA := 0
endif


BT_DIP_SUPPORT ?= 1

AUDIO_RMS_MONITOR_ENABLE ?= 0

BES_OTA ?= 0

TILE_DATAPATH_ENABLED ?= 0

CUSTOM_INFORMATION_TILE_ENABLE ?= 0

INTERCONNECTION ?= 0

INTERACTION ?= 0

INTERACTION_FASTPAIR ?= 0

BT_ONE_BRING_TWO ?= 0

DSD_SUPPORT ?= 0

A2DP_EQ_24BIT ?= 1

A2DP_AAC_ON ?= 1

AAC_REDUCE_SIZE ?= 1

A2DP_SCALABLE_ON ?= 0

A2DP_LHDC_ON ?= 0

A2DP_LHDCV5_ON ?= 0

A2DP_LDAC_ON ?= 0

export A2DP_LC3_ON ?= 1
export A2DP_LC3_HR ?= 0

export TX_RX_PCM_MASK ?= 0

FACTORY_MODE ?= 1

ENGINEER_MODE ?= 1

ULTRA_LOW_POWER	?= 1

DAC_CLASSG_ENABLE ?= 0

NO_SLEEP ?= 0

CORE_DUMP ?= 1

CORE_DUMP_TO_FLASH ?= 0

export SYNC_BT_CTLR_PROFILE ?= 0

export A2DP_AVDTP_CP ?= 0

export A2DP_DECODER_VER := 2

export AUDIO_TRIGGER_VER := 1

ifneq ($(BT_SERVICE_ENABLE),1)
ifneq ($(BT_BUILD_WITH_CUSTOMER_HOST),1)
export IBRT ?= 1

export SEARCH_UI_COMPATIBLE_UI_V2 ?= 1

export IBRT_UI ?= 1
ifeq ($(IBRT_UI),1)
KBUILD_CPPFLAGS += -DIBRT_UI
endif

export BES_AUD ?= 1
endif
endif

export POWER_MODE   ?= DIG_DCDC

export BT_RF_PREFER ?= 2M

export MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED ?= 0

export PROMPT_SELF_MANAGEMENT ?= 1

export IOS_MFI ?= 0

# MiBand9/best1503 has a 4MB SPI flash (dumps are 0x400000). The SDK default of
# 2MB artificially halved the usable flash; the DOOM IWAD (~1.3MB) needs the room.
export FLASH_SIZE ?= 0x400000
export FLASH_SUSPEND ?= 1
export FLASH_PROTECTION_BOOT_SECTION_FIRST ?= 0
export NORFLASH_API_FLUSH_IN_SLEEP_HOOK ?= 1
export HOST_GEN_ECDH_KEY ?= 1

export SW_TRIG ?= 1

USE_THIRDPARTY ?= 0
export USE_KNOWLES ?= 0

export LAURENT_ALGORITHM ?= 1

export RX_IQ_CAL ?= 1

export TX_IQ_CAL ?= 1

export IBRT_DUAL_ANT_CTRL ?= 0

export BT_XTAL_SYNC ?= 0

export POWERKEY_I2C_SWITCH ?=0

AUTO_TEST ?= 0

BES_AUTOMATE_TEST ?= 0

export DUMP_LOG_ENABLE ?= 0

export DUMP_CRASH_LOG ?= 0

SUPPORT_BATTERY_REPORT ?= 1

SUPPORT_HF_INDICATORS ?= 0

SUPPORT_SIRI ?= 1

BES_AUDIO_DEV_Main_Board_9v0 ?= 0

APP_USE_LED_INDICATE_IBRT_STATUS ?= 0

export BT_HOST_REJECT_UNEXCEPT_SCO_PACKET := 1

export BT_PAUSE_A2DP := 1

export BT_PAUSE_A2DP_WHEN_CALL_EXIST := 1

export SBC_RAM_OPT_1306 ?= 1

ifeq ($(SBC_RAM_OPT_1306),1)
KBUILD_CPPFLAGS += -DSBC_RAM_OPT_1306
endif

export NORMAL_TEST_MODE_SWITCH ?= 0

#For ble feture verification
export BLE := 0
export EP_1306_SMALL_CODE ?= 1
export GATT_OVER_BR_EDR ?= 0

export GFPS_ENABLE ?= 0

HAS_BT_SYNC ?= 1

#For free tws pairing feature
FREE_TWS_PAIRING_ENABLED ?= 0

APP_UART_MODULE ?= 0

export OPT_LEVEL ?=s
export PROMPT_IN_FLASH ?= 0

export CALIB_SLOW_TIMER ?= 1
export BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH ?= 1

export TRACE_BUF_SIZE ?= 4*1024
export TRACE_BAUD_RATE ?= 8*115200
# 8*115200 = 921600, matches the boot ROM / dbg_boot_init [CP] baud so the whole
# log stream stays at one rate. (10*115200 = 1152000 was the old value.)
# export BTM_MAX_LINK_NUMS ?= 3
# export BT_DEVICE_NUM ?= 2

# support two bt connections by default
ifndef BT_MULTIPOINT_NUMBER
BT_MULTIPOINT_NUMBER := 2
endif

export BTM_MAX_LINK_NUMS := $(shell echo $$(($(BT_MULTIPOINT_NUMBER) + 1)))
export BT_DEVICE_NUM := $(BT_MULTIPOINT_NUMBER)

init-y :=
ifeq ($(DOOM),1)
# DOOM build: no audio hardware -> drop the multimedia (codec/audio-process) tree.
core-y := platform/ utils/cqueue/ utils/list/ utils/intersyshci/ utils/sha256/
else
core-y := platform/ utils/cqueue/ utils/list/ services/multimedia/ utils/intersyshci/ utils/sha256/
endif

ifneq ($(EXT_SRC_DIR),)
core-y += $(EXT_SRC_DIR)
else
core-y += apps/ services/
endif

KBUILD_CPPFLAGS += \
    -Iplatform/cmsis/inc \
    -Iservices/audioflinger \
    -Iplatform/hal

KBUILD_CPPFLAGS += \
    -DCHARGER_PLUGINOUT_RESET=0

KBUILD_CPPFLAGS += \
    -DBTM_MAX_LINK_NUMS=$(BTM_MAX_LINK_NUMS) \
    -DBT_DEVICE_NUM=$(BT_DEVICE_NUM)

ifeq ($(BES_AUDIO_DEV_Main_Board_9v0),1)
KBUILD_CPPFLAGS += -DBES_AUDIO_DEV_Main_Board_9v0
endif

ifeq ($(TPORTS_KEY_COEXIST),1)
KBUILD_CPPFLAGS += -DTPORTS_KEY_COEXIST
endif


#-DIBRT_LINK_LOWLAYER_MONITOR

#-D_AUTO_SWITCH_POWER_MODE__
#-D__APP_KEY_FN_STYLE_A__
#-D__APP_KEY_FN_STYLE_B__
#-D__EARPHONE_STAY_BOTH_SCAN__
#-D__POWERKEY_CTRL_ONOFF_ONLY__
#-DAUDIO_LINEIN

ifeq ($(CURRENT_TEST),1)
export SMALL_RET_RAM ?= 1
export CORE_SLEEP_POWER_DOWN ?= 1
#INTSRAM_RUN ?= 1
endif
LDS_FILE ?= best1000_1306.lds

export OTA_SUPPORT_SLAVE_BIN ?= 0
export AUDIO_OUTPUT_DC_AUTO_CALIB ?= 1

export AUDIO_ADC_DC_AUTO_CALIB ?= 1
KBUILD_CFLAGS +=

LIB_LDFLAGS += -lstdc++ -lsupc++

DUAL_MIC_RECORDING ?= 0
RECORDING_USE_SCALABLE ?= 0
RECORDING_USE_OPUS ?= 0
RECORDING_USE_OPUS_LOWER_BANDWIDTH ?= 0
STEREO_RECORD_PROCESS ?= 0

# mutex for power on tws pairing and freeman pairing
POWER_ON_ENTER_TWS_PAIRING_ENABLED ?= 0
POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED ?= 0
POWER_ON_ENTER_BOTH_SCAN_MODE ?= 0

OS_THREAD_TIMING_STATISTICS_ENABLE ?= 0

#CFLAGS_IMAGE += -u _printf_float -u _scanf_float

#LDFLAGS_IMAGE += --wrap main


ifeq ($(AI_ENABLE),1)
include $(srctree)/config/$(CHIP)/ai_config.mk
endif

ifeq ($(ANC_ENABLE),1)
include $(srctree)/config/$(CHIP)/anc_config.mk
endif

ifeq ($(CAPSENSOR_ENABLE),1)
include config/$(CHIP)/capsensor_cfg.mk
endif

ifeq ($(LEA_ENABLE),1)
include $(srctree)/config/$(CHIP)/lea_config.mk
endif

include $(srctree)/config/$(CHIP)/speech_config.mk

include $(srctree)/config/$(CHIP)/mem_config.mk

# ---- MiBand9 RM690B0 AMOLED display (bring-up) -------------------------------
# On by default; disable with DISPLAY_RM690B0=0 on the make command line. Pulls in
# the SDK display stack (hal_dsi.c, hal_lcdc.c, graphic fb/lcdc/lcd drivers) + the
# best1503 glue (hal_display_best1503.c) + the RM690B0 panel driver, and wires
# up_fbinitialize() into main() so it is initialised at boot. Bases/clocks are
# placeholders (see DISPLAY_BRINGUP.md) so this COMPILES + runs the init path; it
# won't light the panel until the TODO(vela) values are verified.
export DISPLAY_RM690B0 ?= 1
ifeq ($(DISPLAY_RM690B0),1)
export CHIP_HAS_LCDC := 1
export FBTEST_DRV := 1
KBUILD_CPPFLAGS += -DCONFIG_DISPLAY_DSI_BEST1503 -DDISPLAY_RM690B0 \
                   -DCHIP_HAS_LCDC -DCONFIG_DSIPHY_OUTFMT_RGB565
endif

ifeq ($(HW_SELFTEST),1)
KBUILD_CPPFLAGS += -DHW_SELFTEST
endif

# ---- GBADoom port (apps/doom/) ----------------------------------------------
# Off by default. `make ... DOOM=1` compiles apps/doom/ and defines DOOM so the
# firmware can launch it (doom_launch(), see apps/doom/bes_glue.c).
export DOOM ?= 0
ifeq ($(DOOM),1)
KBUILD_CPPFLAGS += -DDOOM
endif
