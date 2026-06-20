
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

# best1503/MiBand9: rm690b0_lcd.c provides board_lcd_* / up_fb* — drop the stock
# dumy_lcd.c so the symbols don't clash (and it references best1306-only GPIOs).
ifeq ($(DISPLAY_RM690B0),1)
obj-y := $(filter-out dumy_lcd.o,$(obj-y))
endif

ccflags-y := \
