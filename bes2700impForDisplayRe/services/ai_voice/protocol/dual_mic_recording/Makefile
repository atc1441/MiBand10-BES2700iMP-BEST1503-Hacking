obj-y := voice_manager/ voice_bt/ voice_ble/

ifeq ($(VOC_ENCODE_SCALABLE), 1)
obj-y +=../../../../thirdparty/binaural_lib/
endif

ccflags-y += \
    -Iservices/ibrt/app_ibrt/inc \
    -Iservices/ibrt/ibrt_ui/inc
