################################################################################
##
## This file is part of the coreboot project.
##
## Copyright (C) 2009-2010 coresystems GmbH
## Copyright (C) 2015 Google Inc.
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

######################################################################
# set up payload config and version files for later inclusion
ifeq ($(CONFIG_PAYLOAD_SEABIOS),y)
PAYLOAD_CONFIG=payloads/external/SeaBIOS/seabios/.config
PAYLOAD_VERSION=payloads/external/SeaBIOS/seabios/out/autoversion.h
endif
ifeq ($(CONFIG_PAYLOAD_FILO),y)
PAYLOAD_CONFIG=payloads/external/FILO/filo/.config
PAYLOAD_VERSION=payloads/external/FILO/filo/build/version.h
endif

cbfs-files-$(CONFIG_INCLUDE_CONFIG_FILE) += payload_config
payload_config-file := $(PAYLOAD_CONFIG)
payload_config-type := raw

cbfs-files-$(CONFIG_INCLUDE_CONFIG_FILE) += payload_revision
payload_revision-file := $(PAYLOAD_VERSION)
payload_revision-type := raw

SEABIOS_CC_OFFSET=$(if $(filter %ccache,$(HOSTCC)),2,1)
payloads/external/SeaBIOS/seabios/out/bios.bin.elf seabios: $(top)/$(DOTCONFIG)
	$(MAKE) -C payloads/external/SeaBIOS -f Makefile.inc \
			HOSTCC="$(HOSTCC)" \
			CC=$(word $(SEABIOS_CC_OFFSET),$(CC_x86_32)) \
			CFLAGS="$(patsubst $(word $(SEABIOS_CC_OFFSET),$(CC_x86_32))%,,$(wordlist $(SEABIOS_CC_OFFSET),9999,$(CC_x86_32)))" \
			LD=$(word 1,$(LD_x86_32)) LDFLAGS="$(patsubst $(word 1,$(LD_x86_32))%,,$(LD_x86_32))" \
			OBJDUMP="$(OBJDUMP_x86_32)" \
			OBJCOPY="$(OBJCOPY_x86_32)" STRIP="$(STRIP_x86_32)" \
			AS="$(AS_x86_32)" IASL="$(IASL)" \
			CONFIG_SEABIOS_MASTER=$(CONFIG_SEABIOS_MASTER) \
			CONFIG_SEABIOS_STABLE=$(CONFIG_SEABIOS_STABLE) \
			CONFIG_SEABIOS_REVISION=$(CONFIG_SEABIOS_REVISION) \
			CONFIG_SEABIOS_REVISION_ID=$(CONFIG_SEABIOS_REVISION_ID) \
			CONFIG_PAYLOAD_CONFIGFILE=$(CONFIG_PAYLOAD_CONFIGFILE) \
			CONFIG_SEABIOS_THREAD_OPTIONROMS=$(CONFIG_SEABIOS_THREAD_OPTIONROMS) \
			CONFIG_SEABIOS_VGA_COREBOOT=$(CONFIG_SEABIOS_VGA_COREBOOT) \
			CONFIG_DRIVERS_UART_8250IO=$(CONFIG_DRIVERS_UART_8250IO) \
			CONFIG_CONSOLE_SERIAL=$(CONFIG_CONSOLE_SERIAL) \
			CONFIG_TTYS0_BASE=$(CONFIG_TTYS0_BASE)

payloads/external/SeaBIOS/seabios/out/vgabios.bin: seabios
payloads/external/SeaBIOS/seabios/.config: payloads/external/SeaBIOS/seabios/out/bios.bin.elf
payloads/external/SeaBIOS/seabios/out/autoversion.h: payloads/external/SeaBIOS/seabios/out/bios.bin.elf

filo:
	$(MAKE) -C payloads/external/FILO -f Makefile.inc \
			HOSTCC="$(HOSTCC)" \
			CC="$(CC_x86_32)" LD="$(LD_x86_32)" OBJDUMP="$(OBJDUMP_x86_32)" \
			OBJCOPY="$(OBJCOPY_x86_32)" STRIP="$(STRIP_x86_32)" \
			CONFIG_FILO_MASTER=$(CONFIG_FILO_MASTER) \
			CONFIG_FILO_STABLE=$(CONFIG_FILO_STABLE)

payloads/external/FILO/filo/build/filo.elf: filo
payloads/external/FILO/filo/.config: filo
payloads/external/FILO/filo/build/version.h: filo

grub2:
	$(MAKE) -C payloads/external/GRUB2 -f Makefile.inc \
			HOSTCC="$(HOSTCC)" \
			CC="$(CC_x86_32)" LD="$(LD_x86_32)" OBJDUMP="$(OBJDUMP_x86_32)" \
			OBJCOPY="$(OBJCOPY_x86_32)" STRIP="$(STRIP_x86_32)" \
			CONFIG_GRUB2_MASTER=$(CONFIG_GRUB2_MASTER)

payloads/external/GRUB2/grub2/build/default_payload.elf: grub2

payloads/external/U-Boot/u-boot/u-boot-dtb.bin u-boot: $(top)/$(DOTCONFIG)
	$(MAKE) -C payloads/external/U-Boot -f Makefile.inc \
			CONFIG_UBOOT_MASTER=$(CONFIG_UBOOT_MASTER) \
			CONFIG_UBOOT_STABLE=$(CONFIG_UBOOT_STABLE)

cbfs-files-$(CONFIG_MEMTEST_SECONDARY_PAYLOAD) += img/memtest
img/memtest-file := payloads/external/Memtest86Plus/memtest86plus/memtest
img/memtest-type := payload

ifeq ($(CONFIG_CONSOLE_SERIAL)$(CONFIG_DRIVERS_UART_8250IO),yy)
	MEMTEST_SERIAL_OPTIONS=SERIAL_CONSOLE_DEFAULT=1 \
		SERIAL_TTY=$(CONFIG_UART_FOR_CONSOLE) \
		SERIAL_BAUD_RATE=$(CONFIG_TTYS0_BAUD)
endif

payloads/external/Memtest86Plus/memtest86plus/memtest: $(top)/$(DOTCONFIG)
	$(MAKE) -C payloads/external/Memtest86Plus -f Makefile.inc all \
		CC="$(CC_x86_32)" \
		LD="$(LD_x86_32)" \
		OBJCOPY="$(OBJCOPY_x86_32)" \
		AS="$(AS_x86_32)" \
		$(MEMTEST_SERIAL_OPTIONS) \
		MFLAGS= MAKEFLAGS=
