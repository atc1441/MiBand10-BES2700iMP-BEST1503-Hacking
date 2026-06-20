
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))


obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
		   -Iutils/heap/ \
		   -Iplatform/drivers/graphic/gpu/testcase/tst/util/zlib-1.2.7/ \
		   -Wno-double-promotion \
		   -Wno-implicit-fallthrough \
		   -Iplatform/drivers/graphic/gpu/sfs/ \
		   -Iplatform/drivers/graphic/gpu/ \







