cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := rpc.o rpc_ping.o

RPC_SAMPLE ?= 0
ifeq ($(RPC_SAMPLE),1)
obj-y += test/
endif

subdir-ccflags-y += -Iplatform/hal \
					-Iutils/cqueue \
					-Iservices/rpc/inc

