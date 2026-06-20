cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += GCC/

obj-y += rtx_config/

subdir-ccflags-y += -Irtos/rtx5/rtx_config

CFLAGS_rtx_lib.o += -DOS_IDLE_THREAD_NAME=\"idle\" \
                    -DOS_TIMER_THREAD_NAME=\"timer\"

subdir-ccflags-y += -DOS_CLOCK_NOMINAL=$(OS_CLOCK_NOMINAL)

ifneq ($(OS_IDLESTKSIZE),)
subdir-ccflags-y += -DOS_IDLE_THREAD_STACK_SIZE=$(OS_IDLESTKSIZE)
endif

ifneq ($(OS_FIFOSZ),)
subdir-ccflags-y += -DOS_ISR_FIFO_QUEUE=$(OS_FIFOSZ)
endif

ifneq ($(OS_DYNAMIC_MEM_SIZE),)
subdir-ccflags-y += -DOS_DYNAMIC_MEM_SIZE=$(OS_DYNAMIC_MEM_SIZE)
endif

ifneq ($(OS_SCHEDULERSTKSIZE),)
CFLAGS_rtx_lib.o += -DOS_SCHEDULERSTKSIZE=$(OS_SCHEDULERSTKSIZE)
endif

ifneq ($(OS_TIMER_THREAD_STACK_SIZE),)
CFLAGS_rtx_lib.o += -DOS_TIMER_THREAD_STACK_SIZE=$(OS_TIMER_THREAD_STACK_SIZE)
endif

ifneq ($(DEBUG_SLEEP),)
subdir-ccflags-y += -DDEBUG_SLEEP=$(DEBUG_SLEEP)
endif

ifneq ($(OS_TASKCNT),)
CFLAGS_rtx_thread.o += -DOS_TASKCNT=$(OS_TASKCNT)
endif

ifeq ($(OSTICK_USE_FAST_TIMER),1)
subdir-ccflags-y += -DOSTICK_USE_FAST_TIMER
endif

ifneq ($(SYSTICK_CLKSRC_CPU),)
subdir-ccflags-y += -DSYSTICK_CLKSRC_CPU=$(SYSTICK_CLKSRC_CPU)
endif

ifneq ($(CPU_SYSTICK_CLOCK),)
subdir-ccflags-y += -DCPU_SYSTICK_CLOCK=$(CPU_SYSTICK_CLOCK)
endif

ifeq ($(CALIB_SLOW_TIMER),1)
subdir-ccflags-y += -DCALIB_SLOW_TIMER
endif
