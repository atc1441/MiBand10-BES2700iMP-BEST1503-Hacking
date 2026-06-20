cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))

ifeq ($(OTA_TZ_ENABLE),1)
obj-y += ../../../utils/mbedtls/
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(FLASH_SECURITY_REGISTER),1)
CFLAGS_tz_otp_handler_demo.o += -DFLASH_SECURITY_REGISTER
endif

ccflags-y +=-Iutils/mbedtls/ -Iutils/crc32/ -Iutils/heap -Iutils/mbedtls/include -Iutils/user_secure_boot
