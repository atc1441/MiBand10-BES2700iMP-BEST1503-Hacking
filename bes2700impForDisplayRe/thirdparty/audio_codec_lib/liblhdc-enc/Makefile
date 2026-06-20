


VERSION_PREFIX := "BES-LHDC-ENC"
VERSION_NUM	   := "3.1.1"
auth_type	   :=


ifneq ($(GEN_LIB), 1)
auth_type += -DHAVE_TEST_KEY
endif

auth_type += -DLICE_KEY_TYPE_KEYPRO -DLHDC_VERIFY_UUID -DVERSION=\"$(VERSION_PREFIX)$(VERSION_NUM)\"


cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))


ifeq ($(T), best2300p_ep)
DEF := -DBEST2300P_EP  $(auth_type)
endif


Lhdcen := liblhdc-enc-$(T)

$(Lhdcen)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
LIB_BIN_DIR := lib/thirdparty/audio_codec/liblhdc-enc

obj-y += $(Lhdcen).a

ccflags-y +=  -lc -O3 -Wchar-subscripts -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fsingle-precision-constant $(DEF)

subdir-ccflags-y += -Iplatform/hal \
					-Iconfig \
					-Iapps/audioplayers \
					-Imbed/api \
					-Iplatform/cmsis/inc \
					-Iapps/audioplayers \
					-Iutils/heap \
					-Iservices/bt_app \
					-Iservices/lhdcencode/inc \
					-Iservices/bt_if/inc \
					-Iservices/bt_if_enhanced/inc \
					-Iservices/nv_section/factory_section \
					-Ithirdparty/audio_codec_lib/liblhdc-enc/inc \
					-Iinclude/cmsis_dsp


