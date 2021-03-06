/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2015 Intel Corp.
 * (Written by Alexandru Gagniuc <alexandrux.gagniuc@intel.com> for Intel Corp.)
 * (Written by Andrey Petrov <andrey.petrov@intel.com> for Intel Corp.)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <arch/io.h>
#include <arch/symbols.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>
#include <device/pci_def.h>
#include <fsp/api.h>
#include <fsp/util.h>
#include <device/resource.h>
#include <string.h>
#include <soc/iomap.h>
#include <soc/pci_devs.h>
#include <soc/northbridge.h>
#include <soc/romstage.h>
#include <soc/uart.h>

#define FIT_POINTER				(0x100000000ULL - 0x40)

/*
 * Enables several BARs and devices which are needed for memory init
 * - MCH_BASE_ADDR is needed in order to talk to the memory controller
 * - PMC_BAR0 and PMC_BAR1 are used by FSP (with the base address hardcoded)
 *   Once raminit is done, we can safely let the allocator re-assign them
 * - HPET is enabled because FSP wants to store a pointer to global data in the
 *   HPET comparator register
 */
static void soc_early_romstage_init(void)
{
	device_t pmc = PMC_DEV;

	/* Set MCH base address and enable bit */
	pci_write_config32(NB_DEV_ROOT, MCHBAR, MCH_BASE_ADDR | 1);

	/* Set PMC base address */
	pci_write_config32(pmc, PCI_BASE_ADDRESS_0, PMC_BAR0);
	pci_write_config32(pmc, PCI_BASE_ADDRESS_1, 0);	/* 64-bit BAR */
	pci_write_config32(pmc, PCI_BASE_ADDRESS_2, PMC_BAR1);
	pci_write_config32(pmc, PCI_BASE_ADDRESS_3, 0);	/* 64-bit BAR */

	/* PMIO BAR4 was already set earlier, hence the COMMAND_IO below */
	pci_write_config32(pmc, PCI_COMMAND,
				PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
				PCI_COMMAND_MASTER);

	/* Enable decoding for HPET. Needed for FSP global pointer storage */
	pci_write_config32(P2SB_DEV, 0x60, 1<<7);
}

static void disable_watchdog(void)
{
	uint32_t reg;
	device_t dev = PMC_DEV;

	/* Open up an IO window */
	pci_write_config16(dev, PCI_BASE_ADDRESS_4, ACPI_PMIO_BASE);
	pci_write_config32(dev, PCI_COMMAND,
			   PCI_COMMAND_MASTER | PCI_COMMAND_IO);

	/* Stop TCO timer */
	reg = inl(ACPI_PMIO_BASE + 0x68);
	reg |= 1 << 11;
	outl(reg, ACPI_PMIO_BASE + 0x68);
}

asmlinkage void car_stage_entry(void)
{
	void *hob_list_ptr;
	struct range_entry fsp_mem;
	struct range_entry reg_car;

	printk(BIOS_DEBUG, "Starting romstage...\n");

	disable_watchdog();

	soc_early_romstage_init();

	/* Make sure the blob does not override our data in CAR */
	range_entry_init(&reg_car, (uintptr_t)_car_relocatable_data_end,
			(uintptr_t)_car_region_end, 0);

	if (fsp_memory_init(&hob_list_ptr, &reg_car) != FSP_SUCCESS) {
		die("FSP memory init failed. Giving up.");
	}

	fsp_find_reserved_memory(&fsp_mem, hob_list_ptr);

	/* initialize cbmem by adding FSP reserved memory first thing */
	cbmem_initialize_empty_id_size(CBMEM_ID_FSP_RESERVED_MEMORY,
					range_entry_size(&fsp_mem));

	/* make sure FSP memory is reserved in cbmem */
	if (range_entry_base(&fsp_mem) !=
		(uintptr_t)cbmem_find(CBMEM_ID_FSP_RESERVED_MEMORY))
		die("Failed to accommodate FSP reserved memory request");

	/* Now that CBMEM is up, save the list so ramstage can use it */
	fsp_save_hob_list(hob_list_ptr);

	run_ramstage();
}

static void fill_console_params(struct FSPM_UPD *mupd)
{
	if (IS_ENABLED(CONFIG_CONSOLE_SERIAL)) {
		mupd->FspmConfig.SerialDebugPortDevice = CONFIG_UART_FOR_CONSOLE;
		/* use MMIO port type */
		mupd->FspmConfig.SerialDebugPortType = 2;
		/* use 4 byte register stride */
		mupd->FspmConfig.SerialDebugPortStrideSize = 2;
		/* used only for port type set to external */
		mupd->FspmConfig.SerialDebugPortAddress = 0;
	} else {
		mupd->FspmConfig.SerialDebugPortType = 0;
	}
}

void platform_fsp_memory_init_params_cb(struct FSPM_UPD *mupd)
{
	fill_console_params(mupd);
	mainboard_memory_init_params(mupd);

	/* Do NOT let FSP do any GPIO pad configuration */
	mupd->FspmConfig.GpioPadInitTablePtr = NULL;
	/*
	 * At FIT_POINTER there is an address that points to FIT. Even though it
	 * is technically 64bit value we know only 32bit address is used.
	 */
	mupd->FspmConfig.FitTablePtr = read32((void*) FIT_POINTER);
	/* Reserve enough memory under TOLUD to save CBMEM header */
	mupd->FspmArchUpd.BootLoaderTolumSize = cbmem_overhead_size();
	/*
	 * FSPM_UPD passed here is populated with default values provided by
	 * the blob itself. We let FSPM use top of CAR region of the size it
	 * requests.
	 * TODO: add checks to avoid overlap/conflict of CAR usage.
	 */
	mupd->FspmArchUpd.StackBase = _car_region_end -
					mupd->FspmArchUpd.StackSize;
}

__attribute__ ((weak))
void mainboard_memory_init_params(struct FSPM_UPD *mupd)
{
	printk(BIOS_DEBUG, "WEAK: %s/%s called\n", __FILE__, __func__);
}
