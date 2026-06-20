cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=
obj-y += ActivationFunctions/
# obj-y += BasicMathFunctions/
# obj-y += ConcatenationFunctions/
obj-y += ConvolutionFunctions/
obj-y += FullyConnectedFunctions/
obj-y += NNSupportFunctions/
obj-y += PoolingFunctions/
# obj-y += ReshapeFunctions/
# obj-y += SoftmaxFunctions/

subdir-ccflags-y += -Iplatform/cmsis/inc -Iinclude/cmsis_nn
subdir-ccflags-y += -Iinclude/cmsis_dsp

cmsis_nn_lib-y := $(obj-y)
obj-y := cmsis_nn_lib.a
