
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

HEAR_LIB_NAME := lib_hearing
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
$(HEAR_LIB_NAME)-y := $(src_obj)

obj-y := $(HEAR_LIB_NAME).a

ccflags-y := \
	-Iservices/multimedia/speech/inc
