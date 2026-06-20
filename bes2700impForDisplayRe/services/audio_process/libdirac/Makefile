## Makefile for compatibility with BES SDK. During build, it will
## be copied along with library files to BES SDK for inclusion
## in the platform's Makefile system.
##
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

# LIB_BIN_DIR is used in BES2600Z.
LIB_BIN_DIR := services/audio_process/libdirac/lib

obj-y += libdirac.a

ifeq ($(BUILD_LIBDIRAC_DATA), ON)
obj-y += libdirac_data.a
endif

CFLAGS_obj-y += -Os
