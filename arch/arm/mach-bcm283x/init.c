// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <common.h>
#include <dm/device.h>

#define PDATA_BCM2835	0
#define PDATA_BCM2836	1
#define PDATA_BCM2837	2
#define PDATA_BCM2711	3

unsigned long rpi_bcm283x_base = 0x3f000000;

struct bcm283x_pdata {
	unsigned long io_base;
};

struct bcm283x_pdata pdata_bcm283x[] = {
	[PDATA_BCM2835] = {
		.io_base = 0x20000000,
	},
	[PDATA_BCM2836] = {
		.io_base = 0x3f000000,
	},
#ifdef CONFIG_ARM64
	[PDATA_BCM2837] = {
		.io_base = 0x3f000000,
	},
	[PDATA_BCM2711] = {
		.io_base = 0xfe000000,
	},
#endif
};

/*
 * I/O address space varies on different chip versions.
 * We set the base address by inspecting the DTB.
 */
static const struct udevice_id board_ids[] = {
	{ .compatible = "brcm,bcm2835", .data = PDATA_BCM2835},
	{ .compatible = "brcm,bcm2836", .data = PDATA_BCM2836},
	{ .compatible = "brcm,bcm2837", .data = PDATA_BCM2837},
	{ .compatible = "brcm,bcm2838", .data = PDATA_BCM2711},
	{ .compatible = "brcm,bcm2711", .data = PDATA_BCM2711},
	{ },
};

int arch_cpu_init(void)
{
	icache_enable();

	return 0;
}

int mach_cpu_init(void)
{
	const struct udevice_id *of_match = board_ids;
	int ret;

	rpi_bcm283x_base = 0;

	while (of_match->compatible) {
		struct bcm283x_pdata pdat;

		ret = fdt_node_check_compatible(gd->fdt_blob, 0,
						of_match->compatible);
		if (!ret) {
			pdat = pdata_bcm283x[of_match->data];
			rpi_bcm283x_base = pdat.io_base;
			break;
		}

		of_match++;
	}

	return 0;
}

#ifdef CONFIG_ARMV7_LPAE
void enable_caches(void)
{
	dcache_enable();
}
#endif
