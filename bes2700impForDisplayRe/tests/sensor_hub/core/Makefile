cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj-y := $(obj-y:.c=.o)

ccflags-y := -Iplatform/hal \
	-Iplatform/cmsis \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/sensor_hub \
	-Iutils/hwtimer_list \
	-Iservices/audioflinger \
	-Iservices/norflash_api \
	-Iapps/sensorhub \
	-Iapps/main \
	-Iapps/common \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/core \
	-Itests/sensor_hub/vad \
	-Itests/sensor_hub/minima \
	-Itests/sensor_hub/sensor_hub_ai \
	-Itests/sensor_hub/test \
	-Itests/besair_platform \
	-Iservices/mcpp \
	-Itests/besair_platform/cmd_test

ifeq ($(PT_APP),1)
obj-y += ../../../tests/mem_test/pt_app/
ccflags-y += -DPT_APP -Itests/mem_test/pt_app
endif

ifeq ($(AI_VOICE),1)
ccflags-y += -DAI_VOICE
endif

ifeq ($(VD_TEST),1)
ccflags-y += -DVD_TEST -Iapps/voice_detector
endif

ifeq ($(VOICE_DETECTOR_EN),1)
ccflags-y += -DVOICE_DETECTOR_EN
endif

ifeq ($(SENSOR_HUB_MINIMA),1)
ccflags-y += -DSENSOR_HUB_MINIMA
endif

ifeq ($(MINIMA_TEST),1)
ccflags-y += -DMINIMA_TEST -Itests/minima_test
endif

ifeq ($(CAPSENSOR_ENABLE),1)
obj-y += \
    ../../../services/capsensor/

ccflags-y += \
    -Iservices/capsensor/
endif

ifeq ($(SENSOR_TEST),1)
ccflags-y += -DSENSOR_TEST -Itests/sensor_test
endif

ifeq ($(SENSOR_HUB_MISC_TEST),1)
ccflags-y += -DSENSOR_HUB_MISC_TEST
endif

ifeq ($(CORE_BRIDGE_DEMO_MSG),1)
CFLAGS_sensor_hub_core_app.o += -DCORE_BRIDGE_DEMO_MSG
endif

ifeq ($(NOAPP),1)
obj-y := $(filter-out sensor_hub_core_app.o,$(obj-y))
ccflags-y += -DNOAPP
endif

ifeq ($(BT_APP),1)
CFLAGS_sensor_hub_main.o += -D__BT_APP__
endif

ifneq ($(DEBUG_PORT),)
CFLAGS_sensor_hub_main.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(SUBSYS_FLASH_BOOT),1)
CFLAGS_sensor_hub_main.o += -DSUBSYS_FLASH_BOOT
endif

ifeq ($(NO_FLASH_INIT),1)
CFLAGS_sensor_hub_main.o += -DNO_FLASH_INIT
endif

ifeq ($(SENS_TRC_TO_MCU),1)
ifneq ($(BT_APP),1)
obj-y += ../../../platform/drivers/rmt_trace/
endif
CFLAGS_sens_trc_to_mcu.o += -Iplatform/drivers/rmt_trace -DSENS_TRC_TO_MCU
CFLAGS_sensor_hub_main.o += -DSENS_TRC_TO_MCU
endif

ifeq ($(MCU2SENS_MSG_TEST),1)
ccflags-y += -DMCU2SENS_MSG_TEST
endif

ifeq ($(SLEEP_TEST),1)
CFLAGS_sensor_hub_main.o += -DSLEEP_TEST
ifeq ($(NO_TIMER),1)
CFLAGS_sensor_hub_main.o += -DNO_TIMER
endif
endif

ifeq ($(BT_CONN_TEST),1)
obj-y += ../../../tests/hwtest/bt_conn_test.o
ccflags-y += -DBT_CONN_TEST -Itests/hwtest
endif

ifeq ($(FULL_WORKLOAD_MODE_ENABLED),1)
ccflags-y += -DFULL_WORKLOAD_MODE_ENABLED
endif

ifeq ($(VAD_CODEC_TEST),1)
ccflags-y += -DVAD_CODEC_TEST
endif

ifeq ($(I2S_TEST),1)
obj-y += ../../../tests/hwtest/i2s_test.o
ccflags-y += -DI2S_TEST -Itests/hwtest
endif

ifeq ($(SENSOR_ENGINE_TEST),1)
obj-y += ../../../tests/hwtest/sensor_engine_test.o
ccflags-y += -DSENSOR_ENGINE_TEST  -Itests/hwtest

ifeq ($(CHIP_HAS_SPI),1)
ccflags-y += -DCHIP_HAS_SPI
endif

ifeq ($(CHIP_HAS_SPILCD),1)
ccflags-y += -DCHIP_HAS_SPILCD
endif
endif

ifeq ($(MAX_POWER_TEST),1)
obj-y += ../../../tests/hwtest/max_power_test.o
ccflags-y += -DMAX_POWER_TEST  -Itests/hwtest
endif

ifeq ($(FAST_TIMER_COMPENSATE),1)
ccflags-y += -DFAST_TIMER_COMPENSATE
endif

ifeq ($(BSP_TEST_1600),1)
obj-y += ../../../tests/besair_platform/bsp_tester/
ccflags-y += -DBSP_TEST_1600 -Itests/besair_platform/bsp_tester -Itests/besair_platform/bsp_tester/dhrystone2
endif

ifeq ($(SENSOR_HUB_BSP_TEST),1)
ccflags-y += -DSENSOR_HUB_BSP_TEST
endif

ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
CFLAGS_sensor_hub_core_app.o += -DVPU_CFG_ON_SENSOR_HUB
ccflags-y += -Iservices/bone_sensor/vpu_cfg
endif

ifeq ($(BECO_TEST), 1)
obj-y += ../../../platform/beco/
obj-y += ../../hwtest/beco_test/
ccflags-y += -Itests/hwtest
subdir-ccflags-y += -DBECO_TEST
endif

ifeq ($(SPA_AUDIO_ENABLE),1)
subdir-ccflags-y += -Ithirdparty/userapi/spa_dummy_app/inc
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)