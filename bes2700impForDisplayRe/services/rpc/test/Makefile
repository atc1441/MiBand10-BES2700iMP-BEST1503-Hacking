cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

SBC_CLIENT ?= 0
SBC_SERVER ?= 0

ifeq ($(SBC_CLIENT),1)
obj-y += rpc_sbc_frontend.o
endif

ifeq ($(SBC_SERVER),1)
obj-y += rpc_sbc_backend.o
endif

subdir-ccflags-y += -Iplatform/hal \
					-Iutils/cqueue \
					$(BES_MULTIMEDIA_INCLUDES) \
					-Iservices/bt_if_enhanced/inc \
					-Iservices/rpc/inc

