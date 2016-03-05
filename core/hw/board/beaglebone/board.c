#include <hw.h>
#include "board.h"

extern uint32_t *flpt_va;

void board_init()
{
	/*Add special mapping of INTC
	 *Interrupt controller and DMTIMER have other virtual offset compared to other IO registers in board_mem in Linux
	 *We use the same in the hypervisor for simplicity*/
	pt_create_coarse(flpt_va, IO_VA_OMAP2_L4_ADDRESS(0x48200000), 0x48200000, 0x1000, MLT_IO_RO_REG);	/*INTC */
	pt_create_coarse(flpt_va, IO_VA_OMAP2_L4_ADDRESS(0x48040000), 0x48040000, 0x1000, MLT_IO_RW_REG);	/* DMTIMER2 4KB  */
	////
	/*
	 *  Maps the following memory regions that are related to the CPSW driver
	 *  (their names can be found in AM335x Sitara™ Processors Technical
	 *  Reference Manual):
	 *  *CPSW_SS: Starts at 0x4A10_0000. Size before CPSW_PORT: 0x100 = 256
	 *   Bytes. The region has read/write access in PL0 since the Linug guest
	 *   owns the physical memory area.
	 *
	 *  *CPSW_PORT: Starts at 0x4A10_0100. Size before CPSW_CPDMA:
	 *   0x700 = 4Ki - 256 Bytes. The region has read/write access in PL0 since
	 *   the Linux guest owns the physical memory area.
	 *
	 *  *CPSW_CPDMA: Starts at 0x4A10_0800. Size before CPSW_STATS:
	 *   0x100 = 256 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_STATS: Starts at 0x4A10_0900. Size before CPSW_STATERAM:
	 *   0x100 = 256 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_STATERAM: Starts at 0x4A10_0A00. Size before CPSW_CPTS:
	 *   0x200 = 512 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_CPTS: Starts at 0x4A10_0C00. Size before CPSW_ALE:
	 *   0x100 = 256 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_ALE: Starts at 0x4A10_0D00. Size before CPSW_SL1:
	 *   0x80 = 128 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_SL1: Starts at 0x4A10_0D80. Size before CPSW_SL2:
	 *   0x40 = 64 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_SL2: Starts at 0x4A10_0DC0. Size before RESERVED:
	 *   0x40 = 64 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *MDIO: Starts at 0x4A10_1000. Size before RESERVED:
	 *   0x100 = 256 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  *CPSW_WR: Starts at 0x4A10_1200. Size before CPPI_RAM:
	 *   0xE00 = 3584 Bytes. The region has read/write access in PL0 since the
	 *   Linux guest owns the physical memory area.
	 *
	 *  From, and including, CPSW_SW to, and including, CPSW_SL2 is a memory
	 *  region consisting of the following physical addresses:
	 *  [0x4A10_0000, 0x4A10_1000) which is a size of 0x1000 = 4KiB = 1 page.
	 */
	pt_create_coarse(flpt_va, 0xFA400000, 0x4A100000, 0x1000, MLT_IO_RO_REG);	/* CPSW_SS except MDIO, CPSW_WR, CPPI_RAM 4KB  */

	/*
	 *  MDIO and CPSW_WR does not affect DMA transfers from/to memory and hence
	 *  they can be read/write. Size is 0x1000 = 4KiB which is one page.
	 */
//  pt_create_coarse(flpt_va, 0xFA401000, 0x4A101000, 0x1000, MLT_IO_RW_REG); /* MDIO, CPSW_WR which is 4KB.  */
	pt_create_coarse(flpt_va, 0xFA401000, 0x4A101000, 0x1000, MLT_IO_RO_REG);	/* MDIO, CPSW_WR which is 4KB.  */

	/*
	 *  Maps the following memory region that is related to the CPSW driver
	 *  (their names can be found in AM335x Sitara™ Processors Technical
	 *  Reference Manual):
	 *  *CPPI_RAM: Starts at 0x4A10_2000. Size is: 0x2000 = 8KiB = 2 pages.
	 *   The region has read-only access since it controls DMA transfers.
	 */
	pt_create_coarse(flpt_va, 0xFA402000, 0x4A102000, 0x2000, MLT_IO_RO_REG);	/* CPPI_RAM 8KB  */
	////
	/*Flush the addresses as we are going to use it soon */
	mem_mmu_tlb_invalidate_all(TRUE, TRUE);
	mem_cache_invalidate(TRUE, TRUE, TRUE);	//instr, data, writeback
}
