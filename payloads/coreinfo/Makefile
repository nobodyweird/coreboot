##
## This file is part of the coreinfo project.
##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright (C) 2008 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

src := $(CURDIR)
srctree := $(src)
srck := $(src)/../../util/kconfig
coreinfo_obj := $(src)/build
objk := $(src)/build/util/kconfig

export KERNELVERSION      := 0.1.0
export KCONFIG_AUTOHEADER := $(coreinfo_obj)/config.h
export KCONFIG_AUTOCONFIG := $(coreinfo_obj)/auto.conf
export KCONFIG_DEPENDENCIES := $(coreinfo_obj)/auto.conf.cmd
export KCONFIG_SPLITCONFIG := $(coreinfo_obj)/config
export KCONFIG_TRISTATE := $(coreinfo_obj)/tristate.conf
export KCONFIG_NEGATIVES := 1
export Kconfig := Kconfig

export V := $(V)

CONFIG_SHELL := sh
KBUILD_DEFCONFIG := configs/defconfig
UNAME_RELEASE := $(shell uname -r)
HAVE_DOTCONFIG := $(wildcard .config)
MAKEFLAGS += -rR --no-print-directory

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
.SILENT:
endif

HOSTCC ?= gcc
HOSTCXX ?= g++
HOSTCFLAGS := -I$(srck) -I$(objk)
HOSTCXXFLAGS := -I$(srck) -I$(objk)

LIBCONFIG_PATH := ../libpayload
LIBPAYLOAD_DIR := $(coreinfo_obj)/libpayload
HAVE_LIBPAYLOAD := $(wildcard $(LIBPAYLOAD_DIR)/lib/libpayload.a)
LIB_CONFIG ?= defconfig
OBJCOPY ?= objcopy

INCLUDES = -I$(coreinfo_obj) -include $(LIBPAYLOAD_DIR)/include/kconfig.h
OBJECTS = cpuinfo_module.o cpuid.S.o pci_module.o coreboot_module.o \
	  nvram_module.o bootlog_module.o ramdump_module.o \
	  multiboot_module.o cbfs_module.o coreinfo.o
OBJS    = $(patsubst %,$(coreinfo_obj)/%,$(OBJECTS))
TARGET  = $(coreinfo_obj)/coreinfo.elf

all: real-all

# in addition to the dependency below, create the file if it doesn't exist
# to silence warnings about a file that would be generated anyway.
$(if $(wildcard .xcompile),,$(eval $(shell ../../util/xcompile/xcompile $(XGCCPATH) > .xcompile || rm -f .xcompile)))
.xcompile: ../../util/xcompile/xcompile
	$< $(XGCCPATH) > $@.tmp
	\mv -f $@.tmp $@ 2> /dev/null || rm -f $@.tmp $@

CONFIG_COMPILER_GCC := y
ARCH-y     := x86_32

include .xcompile

CC := $(CC_$(ARCH-y))
AS := $(AS_$(ARCH-y))
OBJCOPY := $(OBJCOPY_$(ARCH-y))

LPCC := CC="$(CC)" $(LIBPAYLOAD_DIR)/bin/lpgcc
LPAS := AS="$(AS)" $(LIBPAYLOAD_DIR)/bin/lpas

CFLAGS += -Wall -Werror -Os -fno-builtin $(CFLAGS_$(ARCH-y)) $(INCLUDES)

ifneq ($(strip $(HAVE_DOTCONFIG)),)
include $(src)/.config
real-all: $(TARGET)

$(TARGET): $(src)/.config $(coreinfo_obj)/config.h $(OBJS) libpayload
	printf "    LPCC       $(subst $(CURDIR)/,,$(@)) (LINK)\n"
	$(LPCC) -o $@ $(OBJS)
	$(OBJCOPY) --only-keep-debug $@ $(TARGET).debug
	$(OBJCOPY) --strip-debug $@
	$(OBJCOPY) --add-gnu-debuglink=$(TARGET).debug $@

$(coreinfo_obj)/%.S.o: $(src)/%.S libpayload
	printf "    LPAS       $(subst $(CURDIR)/,,$(@))\n"
	$(LPAS) -o $@ $<

$(coreinfo_obj)/%.o: $(src)/%.c libpayload
	printf "    LPCC       $(subst $(CURDIR)/,,$(@))\n"
	$(LPCC) $(CFLAGS) -c -o $@ $<

else
real-all: config
endif

defaultbuild:
	$(MAKE) olddefconfig
	$(MAKE) all

ifneq ($(strip $(HAVE_LIBPAYLOAD)),)
libpayload:
	printf "Found Libpayload $(LIBPAYLOAD_DIR).\n"
else
libpayload:
	printf "Building libpayload @ $(LIBCONFIG_PATH).\n"
	$(MAKE) -C $(LIBCONFIG_PATH) distclean coreinfo_obj=$(coreinfo_obj)/libptmp
	$(MAKE) -C $(LIBCONFIG_PATH) $(LIB_CONFIG)
	$(MAKE) -C $(LIBCONFIG_PATH) install DESTDIR=$(coreinfo_obj)
endif

$(coreinfo_obj)/config.h:
	$(MAKE) oldconfig

$(shell mkdir -p $(coreinfo_obj) $(objk)/lxdialog $(KCONFIG_SPLITCONFIG))

clean:
	rm -rf build/*.elf build/*.o .xcompile

distclean: clean
	rm -rf build
	rm -f .config .config.old ..config.tmp .kconfig.d .tmpconfig*

include $(srck)/Makefile

.PHONY: $(PHONY) prepare clean distclean
