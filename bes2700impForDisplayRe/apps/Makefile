obj-y := common/ main/ key/ pwl/ battery/ factory/ cmd/ bt_sync/ btapp/

# GBADoom port (apps/doom/). Off by default; enable with DOOM=1 on the make line.
ifeq ($(DOOM),1)
obj-y += doom/
else
# audio apps - device has no audio HW in the DOOM build
obj-y += audioplayers/ mic/ app_test/audio_test/
endif

ifeq ($(CAPSENSOR_SPP_SERVER),1)
obj-y += app_spp/
endif

ifeq ($(APP_TEST_AUDIO),1)
obj-y += app_test/apptester/
endif

ifeq ($(BLE), 1)
ifeq ($(BLE_AUDIO_ENABLED),1)
obj-y += app_test/ble_audio_test/
endif
endif

ifeq ($(A2DP_KARAOKE),1)
obj-y +=  karaoke/
endif

ifeq ($(BTUSB_AUDIO_MODE),1)
obj-y += usbaudio/
endif
ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
obj-y += btusbaudio/
obj-y += usbaudio/
endif

ifeq ($(APP_TEST_SDMMC),1)
obj-y += sdmmc/
endif

ifeq ($(MSD_MODE),1)
obj-y += sdmmc_msd/
endif

ifeq ($(ANC_APP),1)
obj-y += anc/
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
obj-y += voice_assist/
endif

ifeq ($(VOICE_DETECTOR_EN),1)
obj-y += voice_detector/
endif

ifeq ($(SENSOR_HUB),1)
obj-y += sensorhub/
endif

ifeq ($(DSP_M55),1)
obj-y += dsp_m55/
endif

ifeq ($(AUDIO_HEARING_COMPSATN),1)
obj-y += hearing_detec/
endif

_APP_BT_WATCH ?= 0
ifeq ($(BT_WATCH_MASTER),1)
_APP_BT_WATCH := 1
endif
ifeq ($(BT_WATCH_SLAVE),1)
_APP_BT_WATCH := 1
endif
ifeq ($(_APP_BT_WATCH),1)
obj-y += btwatch/
endif

ifeq ($(APP_RPC_ENABLE),1)
obj-y += app_rpc/
endif

ifeq ($(APP_SOUND_ENABLE),1)
obj-y += sound/
endif

ifeq ($(APP_BT_SPEAKER),1)
obj-y += btspeaker/
endif

ifeq ($(BES_OTA),1)
obj-y += ota/
endif

# include earbud in default
ifneq ($(BT_WATCH_APP),1)
obj-y += earbuds/
endif

export APP_SPP_DEMO ?= 0
ifeq ($(APP_SPP_DEMO),1)
obj-y += example/app_spp/
endif

ifeq ($(HEAD_TRACK_ENABLE),1)
obj-y += head_track_3dof/
endif

subdir-ccflags-y += -Iapps/app_test/apptester \
					-Iapps/audioplayers \
					-Iapps/anc/inc \
					-Iapps/voice_assist/inc \
					-Iapps/common \
					-Iapps/sdmmc \
					-Iapps/sdmmc_msd \
					-Iapps/main \
					-Iapps/cmd \
					-Iapps/key \
					-Iapps/pwl \
					-Iapps/battery \
					-Iservices/voicepath \
					-Iservices/audio_dump/include \
					$(BLUETOOTH_ADAPTER_INCLUDES) \
					$(BT_SERVICE_UX_INCLUDES) \
					-Iutils/list \
					-Iutils/heap \
					-Iservices/audio_manager \
					-Iservices/multimedia/audio/process/filters/include\
					-Ibthost/service/ble_app_new/inc  

ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
subdir-ccflags-y += -Iapps/btusbaudio
endif

ifeq ($(A2DP_LDAC_ON),1)
subdir-ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifeq ($(HEAD_TRACK_ENABLE),1)
subdir-ccflags-y += -Iservices/head_track
endif
