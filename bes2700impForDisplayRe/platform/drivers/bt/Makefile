cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(FPGA_VERSION),1500)
obj-y += best1501simu/
else
obj-y += $(CHIP)/
endif
subdir-ccflags-y += \
    -Iinclude/watchdog \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/lzma \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/nv_section/userdata_section\
    -Iapps/key

ifeq ($(BT_XTAL_SYNC),1)
subdir-ccflags-y += -DBT_XTAL_SYNC
endif

export BT_DRV_UART_BRIDGE_FLAGS :=
ifneq ($(DEBUG_PORT),)
BT_DRV_UART_BRIDGE_FLAGS += -DDEBUG_PORT=$(DEBUG_PORT)
endif

export BT_DRV_FLAGS :=
ifeq ($(NO_SLEEP),1)
BT_DRV_FLAGS += -DNO_SLEEP
endif

export BT_DRV_CONFIG_FLAGS :=
ifeq ($(INTERSYS_NO_THREAD),1)
BT_DRV_CONFIG_FLAGS += -D_INTERSYS_NO_THREAD_
endif

VCO_TEST_TOOL ?= 1
ifeq ($(VCO_TEST_TOOL),1)
subdir-ccflags-y += -DVCO_TEST_TOOL
endif

ifeq ($(LAURENT_ALGORITHM),1)
subdir-ccflags-y += -DLAURENT_ALGORITHM
endif

ifeq ($(FORCE_SCO_MAX_RETX),1)
subdir-ccflags-y += -D__FORCE_SCO_MAX_RETX__
endif

ifeq ($(CONTROLLER_DUMP_ENABLE),1)
subdir-ccflags-y += -DCONTROLLER_DUMP_ENABLE=1
endif

ifeq ($(BT_UART_LOG),1)
subdir-ccflags-y += -DBT_UART_LOG
endif

ifeq ($(BT_UART_LOG_P16),1)
subdir-ccflags-y += -DBT_UART_LOG_P16
endif

ifeq ($(LL_MONITOR),1)
subdir-ccflags-y += -DLL_MONITOR=1
endif

ifeq ($(SOFTBIT_EN),1)
subdir-ccflags-y += -DSOFTBIT_EN=1
endif

ifeq ($(ACL_DATA_CRC_TEST),1)
subdir-ccflags-y += -DACL_DATA_CRC_TEST=1
endif

ifeq ($(FA_RX_GAIN_CTRL),1)
subdir-ccflags-y += -D__FA_RX_GAIN_CTRL__
endif

ifeq ($(BT_SYSTEM_52M),1)
subdir-ccflags-y += -DBT_SYSTEM_52M
endif

ifeq ($(BT_FA_ECC),1)
subdir-ccflags-y += -D__FASTACK_ECC_ENABLE__
endif

ifeq ($(BT_FA_SCO_ECC),1)
subdir-ccflags-y += -D__FASTACK_SCO_ECC_ENABLE__
endif

ifeq ($(BT_FA_ACL_ECC),1)
subdir-ccflags-y += -D__FASTACK_ACL_ECC_ENABLE__
endif

ifeq ($(BT_FAST_LOCK_ENABLE),1)
subdir-ccflags-y += -DBT_FAST_LOCK_ENABLE
endif


ifdef BT_ECC_CONFIG_BLK
ifneq ($(BT_ECC_CONFIG_BLK),0)
subdir-ccflags-y += -D__FASTACK_ECC_CONFIG_BLOCK__=$(BT_ECC_CONFIG_BLK)
endif
endif

ifeq ($(BT_ECC_USER_DEFINE),1)
subdir-ccflags-y += -DBT_ECC_USER_DEFINE
endif

ifdef BT_ECC_USER_DEFINE_LENGTH
ifneq ($(BT_ECC_USER_DEFINE_LENGTH),0)
subdir-ccflags-y += -D__ECC_USER_DEFINE_LEN__=$(BT_ECC_USER_DEFINE_LENGTH)
endif
endif

ifdef BT_BID_ECC_EN
ifneq ($(BT_BID_ECC_EN),0)
subdir-ccflags-y += -D__BT_BID_ECC_EN__=$(BT_BID_ECC_EN)
endif
endif

ifeq ($(BES_FA_MODE),1)
subdir-ccflags-y += -D__BES_FA_MODE__
endif

ifeq ($(BT_FAST_ACK),1)
subdir-ccflags-y += -D__BT_FAST_ACK_EN__
endif

ifeq ($(HW_AGC),1)
subdir-ccflags-y += -D__HW_AGC__
endif

ifeq ($(BT_LOG_POWEROFF),1)
subdir-ccflags-y += -DBT_LOG_POWEROFF
endif

ifeq ($(BT_INFO_CHECKER),1)
subdir-ccflags-y += -DBT_INFO_CHECKER
endif

ifeq ($(NEW_SWAGC_MODE),1)
subdir-ccflags-y += -D__NEW_SWAGC_MODE__
endif

ifeq ($(MHDT_SWAGC),1)
subdir-ccflags-y += -D__MHDT_SWAGC__
endif

ifeq ($(MHDT_HWAGC),1)
subdir-ccflags-y += -D__MHDT_HWAGC__
endif

ifeq ($(BLE_NEW_SWAGC_MODE),1)
subdir-ccflags-y += -D__BLE_NEW_SWAGC_MODE__
endif

ifeq ($(BT_EXT_LNA_PA),1)
KBUILD_CPPFLAGS += -DBT_EXT_LNA_PA
ifeq ($(BT_EXT_LNA),1)
KBUILD_CPPFLAGS += -DBT_EXT_LNA
endif

ifeq ($(BT_EXT_PA),1)
KBUILD_CPPFLAGS += -DBT_EXT_PA
endif
endif

export BT_DEBUG_TPORTS ?= 0
ifneq ($(BT_DEBUG_TPORTS),0)
subdir-ccflags-y += -DTPORT_TYPE=$(BT_DEBUG_TPORTS)
subdir-ccflags-y += -D__BT_DEBUG_TPORTS__
endif

export CONTROLLER_DUMP_ENABLE ?= 1
ifeq ($(CONTROLLER_DUMP_ENABLE),1)
	KBUILD_CPPFLAGS += -DCONTROLLER_DUMP_ENABLE=1
endif

ifeq ($(AFH_ASSESS),1)
KBUILD_CPPFLAGS += -D__AFH_ASSESS__
endif

ifeq ($(CT_ISR_TRACE_EN),1)
KBUILD_CPPFLAGS += -D__CT_ISR_TRACE_EN__
endif

ifeq ($(BT_RAMRUN),1)
$(obj)/bt_ramrun_bin.o : $(cur_dir)$(CHIP)/$(CHIP)_ramrun_fw.bin
AFLAGS_bt_ramrun_bin.o += -DBT_RAMRUN_BIN_NAME=$(cur_dir)$(CHIP)/$(CHIP)_ramrun_fw.bin
subdir-ccflags-y += -D__BT_RAMRUN__
endif

ifeq ($(BT_RAMRUN_NEW),1)
subdir-ccflags-y += -D__BT_RAMRUN_NEW
endif

ifeq ($(HIGH_EFFICIENCY_TX_PWR_CTRL),1)
ccflags-y += -DHIGH_EFFICIENCY_TX_PWR_CTRL
endif

ifeq ($(HIGH_EFFICIENCY_RX_IDX_CTRL),1)
ccflags-y += -DHIGH_EFFICIENCY_RX_IDX_CTRL
endif

ifeq ($(RF_INIT_XTAL_CAP_FROM_NV),1)
subdir-ccflags-y += -D__RF_INIT_XTAL_CAP_FROM_NV__
endif

ifeq ($(RX_IQ_CAL),1)
subdir-ccflags-y += -DRX_IQ_CAL
endif

ifeq ($(DPD_ONCHIP_CAL),1)
subdir-ccflags-y += -DDPD_ONCHIP_CAL
endif

ifneq ($(RXON_IOMUX_INDEX),)
subdir-ccflags-y += -DRXON_IOMUX_INDEX=$(RXON_IOMUX_INDEX)
endif

ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)

ifeq ($(BT_ADDR_PRINT_CONFIG),1)
subdir-ccflags-y += -DBT_ADDR_OUTPUT_PRINT_NUM=2
else
subdir-ccflags-y += -DBT_ADDR_OUTPUT_PRINT_NUM=6
endif

ifeq ($(3WIRE_TPORTS),1)
subdir-ccflags-y += -D__BT_3WIRE_TPORTS__
endif

ifeq ($(2WIRE_TPORTS),1)
subdir-ccflags-y += -D__BT_2WIRE_TPORTS__
endif

ifeq ($(NO_MULTI_HEAP),1)
subdir-ccflags-y += -DNO_MULTI_HEAP=1
endif

ifeq ($(IS_HCI_ISO_PACKET_TRACE_ENABLED),1)
subdir-ccflags-y += -DIS_HCI_ISO_PACKET_TRACE_ENABLED
endif

ifeq ($(BT_RAMRUN_BIN_COMPRESSED),1)
subdir-ccflags-y += -D__BT_RAMRUN_BIN_COMPRESSED__
endif

ifneq ($(ISO_BUF_SIZE),)
subdir-ccflags-y += -DISO_BUF_SIZE=$(ISO_BUF_SIZE)
else
subdir-ccflags-y += -DISO_BUF_SIZE=400
endif

ifneq ($(ISO_BUF_NB),)
subdir-ccflags-y += -DISO_BUF_NB=$(ISO_BUF_NB)
else
subdir-ccflags-y += -DISO_BUF_NB=20
endif

ifeq ($(LE_ADV_TX_LOWPOWER),1)
subdir-ccflags-y += -DLE_ADV_TX_LOWPOWER
endif

ifeq ($(BT_MASTER_SLEEP),1)
subdir-ccflags-y += -DBT_MASTER_SLEEP
endif

ifeq ($(PMU_CLK_XO_INPUT_32K),1)
subdir-ccflags-y += -DPMU_CLK_XO_INPUT_32K
endif

ifeq ($(PMU_CLK_LED1_INPUT_32K),1)
subdir-ccflags-y += -DPMU_CLK_LED1_INPUT_32K
endif

ifneq ($(filter master slave 1, $(BT_PLD_SIMU) $(BT_PLD_SOURCE_SIMU)),)
subdir-ccflags-y += -DBT_PLD_SIMU
endif

ifeq ($(TRANSQ_ENABLE),1)
subdir-ccflags-y += -DTRANSQ_ENABLE
endif

ifeq ($(MCU_WAKEUP_BT_V2),1)
subdir-ccflags-y += -DMCU_WAKEUP_BT_V2
endif