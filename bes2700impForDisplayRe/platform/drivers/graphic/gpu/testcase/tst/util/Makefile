
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_c += libpng-1.6.1/

obj_c += zlib-1.2.7/

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/  \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/libpng-1.6.1/ \
           -Iplatform/drivers/graphic/gpu/sfs/ \
           -Iplatform/drivers/graphic/gpu/ \


