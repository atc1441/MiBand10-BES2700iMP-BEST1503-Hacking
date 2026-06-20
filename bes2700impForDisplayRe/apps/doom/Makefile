cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

# Build every C source in this dir. NOTE: fixeddiv.s is ARM-mode (ARM7TDMI) asm
# and is intentionally NOT built - it is dead code on the Thumb-2 Cortex-M33
# (its udiv*_arm symbols are unreferenced; FixedMul/FixedDiv are C).
obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)

# doom headers (doomdef.h, ...) are included unqualified from this dir.
subdir-ccflags-y += -I$(cur_dir)

# The engine is legacy 1990s C; relax the SDK's strict warnings-as-errors for it.
subdir-ccflags-y += -Wno-error -Wno-implicit-function-declaration \
                    -Wno-unused -Wno-pointer-sign -Wno-format \
                    -Wno-sign-compare -Wno-missing-braces -fgnu89-inline

# The SDK appends C_ONLY_FLAGS (-Werror-implicit-function-declaration,
# -Werror=implicit-int) and KBUILD_CPPFLAGS (-Werror) AFTER subdir-ccflags, so
# -Wno-error above cannot win. _c_flags (scripts/lib.mk) filters out the
# per-object CFLAGS_REMOVE_<obj>.o list from the *final* flags, so strip the
# error-promoting flags there for every doom object instead.
DOOM_REMOVE := -Werror -Werror-implicit-function-declaration -Werror=implicit-int -Werror=date-time
$(foreach o,$(obj-y),$(eval CFLAGS_REMOVE_$(o) += $(DOOM_REMOVE)))
