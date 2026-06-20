cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c :=
#obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

ifeq ($(CHIP_HAS_GPU),1)
obj_c += gpu/
endif

ifeq ($(CHIP_HAS_DMA2D),1)
obj_c += dma2d/
endif

ifeq ($(CHIP_HAS_LCDC),1)
obj_c += lcdc/
endif

ifeq ($(GRAPHIC_TEST),1)
obj_c += test_main.c
endif

ifeq ($(FBTEST_DRV),1)
obj_c += lcd/
obj_c += fb/
endif

ifeq ($(KERNEL),RTX5)
obj_c += osl_rtx5/
subdir-ccflags-y += \
				   -Iplatform/drivers/graphic/osl_rtx5/ \
				   -Irtos/ \
				   -Irtos/rtx5/rtx_config/
endif
ifeq ($(KERNEL),LITEOS_M)
subdir-ccflags-y += \
				   -Iplatform/drivers/graphic/osl_rtx5/ \
				   -Iinclude/liteos/liteos_m/kal/cmsis
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


ifeq ($(GRADRV_TMST), 1)
subdir-ccflags-y += -DGRAPHICDRV_TIME_STATIS
endif

ifeq ($(GRADRV_DEBUG), 1)
subdir-ccflags-y += -DGRAPHIC_DRV_DEBUG
endif

ifeq ($(TEST_WITH_LCD), 1)
subdir-ccflags-y += -DTEST_WITH_LCD
endif

ifeq ($(BAREMETAL), 1)
subdir-ccflags-y += -D_BAREMETAL=1
endif

ifneq ($(BOARD_NAME), )
subdir-ccflags-y += -DBOARD_NAME=$(BOARD_NAME)
endif

subdir-ccflags-y += \
				   -Iutils/heap/ \
				   -Iplatform/drivers/graphic \
				   -Iplatform/drivers/graphic/fb \
				   -Iplatform/drivers/graphic/lcdc \
				   -Iplatform/drivers/graphic/lcd \
				   -Iplatform/drivers/graphic/dma2d \


# best1503 bring-up: the stock graphic/fb/lcdc drivers have many test-only static
# funcs/vars; do not let -Werror block the placeholder display build.
subdir-ccflags-y += -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=pointer-sign -Wno-error=incompatible-pointer-types
