ifeq ($(KERNEL),RTX)
rtos_lib-y := rtx/TARGET_CORTEX_M/
else ifeq ($(KERNEL),RTX5)
rtos_lib-y := rtx5/
else ifeq ($(KERNEL),FREERTOS)
rtos_lib-y := freertos/
else ifeq ($(KERNEL),NUTTX)
rtos_lib-y := nuttx/
else ifeq ($(KERNEL),RTTHREAD)
rtos_lib-y := rt_thread/
KBUILD_CFLAGS += -Wno-error=date-time
else ifeq ($(KERNEL),LITEOS_M)
rtos_lib-y := liteos/liteos_m/
endif

ifeq ($(MBED),1)
rtos_lib-y += rtos/
endif

obj-y := rtos_lib.o

subdir-ccflags-y += -Iplatform/cmsis/inc -Iplatform/hal -Iutils/hwtimer_list

