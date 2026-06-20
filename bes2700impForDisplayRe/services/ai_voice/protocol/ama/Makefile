cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_manager/*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_bt/*.cpp))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_ble/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_stream/*.c))
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_protobuf/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_protobuf/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_protobuf/*.cpp))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_protobuf/protobuf-c/*.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ama_gatt_v2/*.c))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

AMA_LIB_NAME := lib_ama

$(AMA_LIB_NAME)-y := $(src_obj)

obj-y :=  $(AMA_LIB_NAME).a

AMA_BLE_INCLUDES += \
              -Iservices/ai_voice/protocol/ama/ama_gatt_v2

ccflags-y +=  -Iapps/main \
              -Iapps/app_anker/device_drivers/bes_hal \
              -Iapps/audioplayers \
              -Iapps/key \
              -Iplatform/hal \
              -Iplatform/drivers/bt \
              -Iservices/ai_voice/algorithm \
              -Iservices/ai_voice/protocol/ama/ama_ble \
              -Iservices/ai_voice/protocol/ama/ama_bt \
              $(AMA_BLE_INCLUDES) \
              -Iservices/ai_voice/protocol/ama/ama_manager \
              -Iservices/ai_voice/protocol/ama/ama_protobuf \
              -Iservices/ai_voice/protocol/ama/ama_protobuf/protobuf-c \
              -Iservices/ai_voice/protocol/ama/ama_stream \
              -Iservices/ai_voice/audio \
              -Iservices/ai_voice/codec/compression \
              -Iservices/ai_voice/codec/opus121/include \
              -Iservices/ai_voice/transport \
              -Iservices/ai_voice/manager \
              -Iservices/app_ai/voice_sbc \
              -Iservices/app_ai/inc \
              -Iservices/bt_app \
              -Iservices/bt_app/a2dp_codecs/include \
              -Iservices/resources \
              -Iutils/cqueue \
              -Iutils/crc32 \
              -Iutils/heap
