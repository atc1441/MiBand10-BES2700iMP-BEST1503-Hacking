
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-I$(obj)/../../nv_section/aud_section \
	-I$(obj)/../../nv_section/include \
	-I$(obj)/../../norflash_api \
	-Iutils/crc32
	
ifeq ($(PSAP_APP),1)
CFLAGS_aud_section.o += -DPSAP_APP
endif
