# ============================================================================
#  best1503 firmware - build + flash wrapper  (DEFAULT TARGET)
#  Run from git-bash:
#
#     make                     build + flash best1503 (DEFAULT, incremental)
#     make all                 incremental build only (no flash)
#     make flash               flash the already-built best1503.bin
#     make flash PORT=COM5     flash to a specific serial port
#     make clean all flash     full rebuild from scratch AND flash
#
#  The default target is an INCREMENTAL `all flash` on purpose: a from-scratch
#  `clean` build is slow, so clean is never run automatically - request it
#  explicitly (`make clean ...`) when you really need a full rebuild.
#
#  GNU make automatically prefers this GNUmakefile over the SDK's "Makefile".
#  The SDK Makefile is left untouched and is still used for the actual build
#  (called below via `-f Makefile`); its recursive sub-makes use an explicit
#  `-f $(CURDIR)/Makefile`, so they are unaffected by this wrapper.
# ============================================================================

# Run this wrapper's own recipes under MSYS2 sh so pipes and the C:/msys64
# make/sh paths resolve identically regardless of the top-level make.
SHELL := C:/msys64/usr/bin/sh.exe

# Toolchain + MSYS2 on PATH.
export PATH := /c/gcc-arm-none-eabi/bin:/c/MinGW/bin:/c/msys64/usr/bin:$(PATH)

# best1503 REQUIRES MSYS2 make 4.4.1 ($(file) response files; mingw32-make 3.82
# truncates the long commands).
MAKE_1503 := C:/msys64/usr/bin/make.exe

# ---- best1503 firmware build -------------------------------------------------
OUT_BIN   := out/best1503/best1503.bin
# Canonical build flags (the one supported configuration):
#   T=best1503 CHIP=best1503 DEBUG=1 MAGIC_NUM=1 LIBC_ROM=0 HW_SELFTEST=1
# LIBC_ROM=0 / HW_SELFTEST=1 are set explicitly here so a bare `make` is identical
# to the full command line regardless of the SDK target.mk defaults.
ARGS_1503 := T=best1503 SHELL=/usr/bin/sh CHIP=best1503 DEBUG=1 MAGIC_NUM=1 \
             LIBC_ROM=0 HW_SELFTEST=1

# Flashing (override on the command line, e.g.  make flash PORT=COM5)
PORT    ?= COM9
FLASHER ?= python flasher/bes2700_flasher.py

.NOTPARALLEL:          # run clean/all/flash strictly in the order given
.PHONY: default all clean flash

# Bare `make` = incremental build then flash (NO clean).
.DEFAULT_GOAL := default
default: all flash

# A from-scratch -j4 build hits a transient kbuild race (the .rsp/.d files are
# written before their out/ subdir exists -> "No such file or directory").  Each
# pass creates more dirs, so an incremental retry completes.  Loop up to 3x.
all:
	@for i in 1 2 3; do \
	  echo "==> best1503 build pass $$i ..."; \
	  $(MAKE_1503) -f Makefile $(ARGS_1503) -j4 && break; \
	  echo "==> pass $$i hit the -j dir race; retrying incrementally..."; \
	done
	@test -f $(OUT_BIN) && echo "==> Built $(OUT_BIN)" \
	  || { echo "==> BUILD FAILED: $(OUT_BIN) not produced"; exit 1; }

clean:
	@echo "==> Cleaning best1503 (full rebuild from scratch)..."
	-$(MAKE_1503) -f Makefile T=best1503 SHELL=/usr/bin/sh clean
	rm -rf out/best1503

flash:
	@echo "==> Flashing $(OUT_BIN) -> $(PORT)"
	$(FLASHER) --port $(PORT) --write $(OUT_BIN)
