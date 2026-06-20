cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += BasicMathFunctions/
obj-y += CommonTables/
obj-y += ComplexMathFunctions/
obj-y += FilteringFunctions/
obj-y += MatrixFunctions/
obj-y += SupportFunctions/
obj-y += StatisticsFunctions/
obj-y += TransformFunctions/
obj-y += FastMathFunctions/

subdir-ccflags-y := -Iinclude/cmsis_dsp -Iinclude/cmsis_dsp/PrivateInclude
subdir-ccflags-y += -DARM_MATH_LOOPUNROLL -O3 -ffast-math

#there two much warnings in the new version dsp lib
subdir-ccflags-y += -flax-vector-conversions -Wno-float-conversion -Wno-double-promotion
ifeq ($(TOOLCHAIN), armclang)
subdir-ccflags-y += -Wno-uninitialized
else
subdir-ccflags-y += -Wno-maybe-uninitialized
endif

cmsis_dsp_lib-y := $(obj-y)
cmsis_dsp-y := cmsis_dsp_lib.a
obj-y := cmsis_dsp.o
