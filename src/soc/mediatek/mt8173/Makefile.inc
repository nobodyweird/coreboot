##
## This file is part of the coreboot project.
##
## Copyright 2015 MediaTek Inc.
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

ifeq ($(CONFIG_SOC_MEDIATEK_MT8173),y)

bootblock-y += bootblock.c
bootblock-$(CONFIG_SPI_FLASH) += flash_controller.c
bootblock-y += i2c.c
bootblock-y += pll.c
bootblock-y += spi.c
bootblock-y += timer.c

ifeq ($(CONFIG_BOOTBLOCK_CONSOLE),y)
bootblock-$(CONFIG_DRIVERS_UART) += uart.c
endif

bootblock-y += gpio.c gpio_init.c pmic_wrap.c mt6391.c
bootblock-y += wdt.c
bootblock-y += mmu_operations.c

################################################################################

verstage-y += i2c.c
verstage-y += spi.c

verstage-$(CONFIG_DRIVERS_UART) += uart.c

verstage-y += timer.c
verstage-y += wdt.c
verstage-$(CONFIG_SPI_FLASH) += flash_controller.c

################################################################################

romstage-$(CONFIG_SPI_FLASH) += flash_controller.c
romstage-y += pll.c
romstage-y += timer.c

romstage-$(CONFIG_DRIVERS_UART) += uart.c
romstage-y += cbmem.c
romstage-y += spi.c
romstage-y += gpio.c
romstage-y += pmic_wrap.c mt6391.c
romstage-y += memory.c
romstage-y += emi.c dramc_pi_basic_api.c dramc_pi_calibration_api.c
romstage-y += mmu_operations.c
romstage-y += rtc.c

################################################################################

ramstage-y += cbmem.c emi.c
ramstage-y += spi.c
ramstage-$(CONFIG_SPI_FLASH) += flash_controller.c
ramstage-y += soc.c mtcmos.c
ramstage-y += timer.c
ramstage-$(CONFIG_DRIVERS_UART) += uart.c
ramstage-y += pmic_wrap.c mt6391.c i2c.c
ramstage-y += gpio.c
ramstage-y += wdt.c
ramstage-y += pll.c
ramstage-y += rtc.c

ramstage-y += usb.c

ramstage-$(CONFIG_ARM64_USE_ARM_TRUSTED_FIRMWARE) += bl31_plat_params.c

BL31_MAKEARGS += PLAT=mt8173 MEDIATEK_MEM_CONFIG=SRAM_192K MEDIATEK_DA9212=1 \
		 MEDIATEK_MT6311=1

################################################################################

# Generate the actual coreboot bootblock code
$(objcbfs)/bootblock.bin: $(objcbfs)/bootblock.raw.bin
	./util/mtkheader/gen-bl-img.py mt8173 sf $< $@

CPPFLAGS_common += -Isrc/soc/mediatek/mt8173/include

endif
