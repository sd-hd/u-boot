// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <common.h>
#include <dm/device.h>
#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#endif

#define PDATA_BCM2835	0
#define PDATA_BCM2836	1
#define PDATA_BCM2837	2
#define PDATA_BCM2711	3

unsigned long rpi_bcm283x_base = 0x3f000000;

#ifdef CONFIG_ARM64
static struct mm_region bcm283x_mem_map[] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x3f000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x3f000000UL,
		.phys = 0x3f000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

static struct mm_region bcm2711_mem_map[] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0xfe000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xfe000000UL,
		.phys = 0xfe000000UL,
		.size = 0x01800000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = bcm283x_mem_map;

#else
struct mm_region {
	/* dummy struct */
};
#endif

struct bcm283x_pdata {
	unsigned long io_base;
	struct mm_region *m_map;
};

struct bcm283x_pdata pdata_bcm283x[] = {
	[PDATA_BCM2835] = {
		.io_base = 0x20000000,
		.m_map = NULL,
	},
	[PDATA_BCM2836] = {
		.io_base = 0x3f000000,
		.m_map = NULL,
	},
#ifdef CONFIG_ARM64
	[PDATA_BCM2837] = {
		.io_base = 0x3f000000,
		.m_map = bcm283x_mem_map,
	},
	[PDATA_BCM2711] = {
		.io_base = 0xfe000000,
		.m_map = bcm2711_mem_map
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

#ifdef CONFIG_ARM64
static void rpi_updated_mem_map(struct mm_region *pd)
{
	int i;

	for (i = 0; i < 2; i++) {
		mem_map[i].virt = pd[i].virt;
		mem_map[i].phys = pd[i].phys;
		mem_map[i].size = pd[i].size;
		mem_map[i].attrs = pd[i].attrs;
	}
}
#else
static void rpi_updated_mem_map(struct mm_region *pd) {}
#endif

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
			rpi_updated_mem_map(pdat.m_map);
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
