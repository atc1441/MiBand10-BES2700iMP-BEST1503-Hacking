cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ALEXA_LIB_NAME := libpryon_lite-PRL1000

obj-y :=

obj-y += ./models/common/WS_50k.en-US.alexa.cpp


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

LIB_BIN_DIR := lib/thirdparty/kws/alexa_lite
obj-y += $(ALEXA_LIB_NAME).a

C_ONLY_FLAGS += -std=gnu99 -Wimplicit-function-declaration

ccflags-y := \
	-Ithirdparty/kws/alexa_lib/inc \
	-DARM_MATH_CM4 -D__FPU_PRESENT -DFLOATING_POINT -DWEBRTC_ARCH_ARM_V7 -DCONFIG_NEWLIB_LIBC=y \
	-mfpu=fpv4-sp-d16 -mfloat-abi=hard -fsingle-precision-constant \
	-mlittle-endian -mthumb -mcpu=cortex-m4 -Wdouble-promotion \
	-ffunction-sections -fdata-sections -fno-builtin -gdwarf-2 \
	-Os -Wall -fno-strict-aliasing -fno-common -Wall -Werror=uninitialized -Wno-error=maybe-uninitialized -Werror=return-type -DPCFG_OS=2 -D_REENT_SMALL -Wno-error -Wno-switch
	#-DCNN_ATTRIBUTE=1 -DAID_NN_IN_RAM=1 -DAID_PRE_IN_RAM
