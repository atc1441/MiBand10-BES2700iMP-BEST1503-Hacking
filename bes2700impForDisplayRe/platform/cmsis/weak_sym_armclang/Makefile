cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S $(cur_dir)*.cpp))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)
obj-y := $(obj-y:.cpp=.o)

CFLAGS_weak_sym_armclang.o += \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/codec \
	-Iplatform/drivers/norflash

ifeq ($(SPI_ROM_ONLY),1)
CFLAGS_weak_sym_armclang.o += -DSPI_ROM_ONLY
endif
