#include "types.h"
#include "arm_common.h"
#include "cpu_cop.h"
#include "cpu.h"

/*
 * TLB
 * */

void mem_mmu_tlb_invalidate_all(BOOL inst, BOOL data)
{
	uint32_t tmp = 0;
	if (inst && data)
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ALL, tmp);
	else if (inst)
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ALL_INST, tmp);
	else if (data)
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_ALL_DATA, tmp);
}

void mem_mmu_tlb_invalidate_one(BOOL inst, BOOL data, uint32_t virtual_addr)
{
	uint32_t tmp = virtual_addr;

	if (inst && data)
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA, tmp);
	else if (inst)
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA_INST, tmp);
	else if (data)
		COP_WRITE(COP_SYSTEM, COP_TLB_INVALIDATE_MVA_DATA, tmp);
}

/*
 * CACHES 
 */

/* XXX: this can be simplified alot! /AV */
static inline signed int log_2_n_round_up(uint32_t n)
{
	signed int log2n = -1;
	uint32_t temp = n;

	while (temp) {
		log2n++;
		temp >>= 1;
	}

	if (n & (n - 1))
		return log2n + 1;	/* not power of 2 - round up */
	else
		return log2n;	/* power of 2 */
}

static void setway(uint32_t level, uint32_t clean_it)
{
	uint32_t csselr, ccsidr;	// cache size selection/id register
	uint32_t num_sets, num_ways, log2_line_len, log2_num_ways;
	uint32_t way_shift;

	//Write level and type you want to extract from cache size selection register
	csselr = level << 1 | ARMV7_CSSELR_IND_DATA_UNIFIED;
	COP_WRITE2(COP_SYSTEM, "2", COP_ID_CPU, csselr);

	// Get size details from current cache size id register
	COP_READ2(COP_SYSTEM, "1", COP_ID_CPU, ccsidr);

	log2_line_len = (ccsidr & CCSIDR_LINE_SIZE_MASK) + 2;
	/* Converting from words to bytes */
	log2_line_len += 2;

	num_ways = ((ccsidr & CCSIDR_ASSOCIATIVITY_MASK) >>
		    CCSIDR_ASSOCIATIVITY_OFFSET) + 1;
	num_sets = ((ccsidr & CCSIDR_NUM_SETS_MASK) >>
		    CCSIDR_NUM_SETS_OFFSET) + 1;

	log2_num_ways = log_2_n_round_up(num_ways);

	way_shift = (32 - log2_num_ways);

	int way, set, setway;

	for (way = num_ways - 1; way >= 0; way--) {
		for (set = num_sets - 1; set >= 0; set--) {
			setway = (level << 1) | (set << log2_line_len) |
			    (way << way_shift);

			if (clean_it)
				COP_WRITE(COP_SYSTEM,
					  COP_DCACHE_CLEAN_INVALIDATE_SW,
					  setway);
			else
				COP_WRITE(COP_SYSTEM,
					  COP_DCACHE_INVALIDATE_SW, setway);

		}
	}

	/* Data synchronization barrier operation
	 *  to make sure the operation is complete 
	 */
	dsb();
}

// Viktor
static void mem_icache_invalidate(BOOL clean_it)
{
	uint32_t level, cache_type, level_start_bit = 0;
	uint32_t clidr;

	/* Get cache level ID register */
	COP_READ2(COP_SYSTEM, "1", COP_ID_CACHE_CONTROL_ARMv7_CLIDR, clidr);

	for (level = 0; level < 8; level++) {
		cache_type = (clidr >> level_start_bit) & 0x7;

		if ((cache_type == ARMV7_CLIDR_CTYPE_DATA_ONLY) ||
		    (cache_type == ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA) ||
		    (cache_type == ARMV7_CLIDR_CTYPE_UNIFIED)) {
			setway(level, clean_it);
		}

		level_start_bit += 3;
	}
}

void mem_cache_set_enable(BOOL enable)
{
	uint32_t tmp;

	COP_READ(COP_SYSTEM, COP_SYSTEM_CONTROL, tmp);
	if (enable)
		tmp |=
		    (COP_SYSTEM_CONTROL_ICACHE_ENABLE |
		     COP_SYSTEM_CONTROL_DCACHE_ENABLE);
	else
		tmp &=
		    ~(COP_SYSTEM_CONTROL_ICACHE_ENABLE |
		      COP_SYSTEM_CONTROL_DCACHE_ENABLE);
	COP_WRITE(COP_SYSTEM, COP_SYSTEM_CONTROL, tmp);
}

void mem_cache_invalidate(BOOL inst_inv, BOOL data_inv, BOOL data_writeback)
{
	uint32_t tmp = 1;
	/* first, handle the data cache */
	if (data_inv) {
		mem_icache_invalidate(data_writeback);
	}

	/* now, the instruction cache */
	if (inst_inv) {
		COP_WRITE(COP_SYSTEM, COP_ICACHE_INVALIDATE_ALL, tmp);
	}
}

void mem_cache_dcache_area(addr_t va, uint32_t size, uint32_t op)
{
	uint32_t csidr;
	uint32_t cache_size;

	uint32_t end, next;
	COP_READ2(COP_SYSTEM, "1", COP_ID_CPU, csidr);
	csidr &= 7;		// Cache line size encoding
	cache_size = 16;	//Size offset
	cache_size = (cache_size << csidr);

	next = va;
	end = va + size;
	do {
		if (op == FLUSH)
			COP_WRITE(COP_SYSTEM, COP_DCACHE_CLEAN_INVALIDATE_MVA,
				  next);
		else if (op == CLEAN)
			COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA,
				  next);
		next += cache_size;

	} while (next < end);

	dsb();
}

/////////
/*
 *@start: Start virtual address.
 *
 *@end: End virtual address (exclusive).
 *
 *Function: For caches containing data:
 *1.Cleans and invalidates the cache line containing the start address if
 *it is not cache line size aligned. The cleaning is done since the first
 *cache line may contain other data not beloning to the DMA region if it
 *is not cache line size aligned.
 *2.Cleans and invalidates the cache line containing the end address if it
 *is not cache line size aligned. The cleaning is done since the last
 *cache line may contain other data not beloning to the DMA region if it
 *is not cache line size aligned.
 *3.Invalidates all cache lines whose cache line aligned addresses are
 *greater than or equal to the cache line size aligned address
 *corresponding to the start address, and less than the cache line size
 *aligned address corresponding to the end address. This means that the
 *end address is exclusive.
 */
void inv_dcache_region(addr_t start, addr_t end)
{
	uint32_t ctr, min_data_cache_line_size, min_data_cache_line_mask;

	//ctr := CTR, Cache Type Register.
	COP_READ(COP_SYSTEM, COP_ID_CACHE_CONTROL_ARMv7_CLIDR, ctr);

	//ctr := ctr >> 16. This is the bit offset of the DminLine field of CTR.
	ctr >>= 16;

	//ctr := CTR.DminLine;
	//Log2 of the number of words in the smallest cache line of all the data
	//caches and unified caches that are controlled by the processor. Hence the
	//cache line size is a power of two. Minimum cache line size is needed to
	//prevent skipping a cache line in the loop.
	ctr &= 0xF;

	//Cache line size in bytes: 2^ctr * 4 =
	//(1 << ctr) * 4 = (1 << ctr) << 2 = 1 << (ctr + 2) =
	//1 << (2 + ctr) = (1 << 2) << ctr = 4 << ctr
	min_data_cache_line_size = 4 << ctr;

	//Cache line size is a power of two. Gives ones below the bit that is set
	//to denote the size. Hence a sort of a mask is stored in r3 that
	//identifies the byte addresses of a cache line.
	min_data_cache_line_mask = min_data_cache_line_size - 1;

	//If the start address is not cache line size aligned, then it is masked to
	//be cache line size aligned, and the cache line containing the word at
	//address start might contain data not pertaining to the invalidation
	//region, and hence that cache line is cleaned and then invalidated.
	if (start & min_data_cache_line_mask) {
		//Makes the start address cache line size aligned.
		start = start & ~min_data_cache_line_mask;

		//DCCIMVAC, Clean and invalidate data (or unified) cache line by MVA to
		//PoC. Invalidates cache line containing the start address.
		COP_WRITE(COP_SYSTEM, COP_DCACHE_CLEAN_INVALIDATE_MVA, start);
	}
	//If the end address is not cache line size aligned, then it is masked to
	//be cache line size aligned, and the cache line containing the word at
	//address end might contain data not pertaining to the invalidation
	//region, and hence that cache line is cleaned and then invalidated.
	if (end & min_data_cache_line_mask) {
		//Makes the end address cache line size aligned.
		end = end & ~min_data_cache_line_mask;

		//DCCIMVAC, Clean and invalidate data (or unified) cache line by MVA to
		//PoC. Invalidates cache line containing the start address.
		COP_WRITE(COP_SYSTEM, COP_DCACHE_CLEAN_INVALIDATE_MVA, end);
	}
	//Invalidates data cache lines containing words at addresses in the
	//interval [start, end) (for values in start and end at this execution
	//point).
	do {
		//DCIMVAC, Invalidate data (or unified) cache line by MVA to PoC.
		//Invalidates the cache line holding the word at address start.
		//Executes: MCR p15, 0, start, c7, c6, 1;
		COP_WRITE(COP_SYSTEM, COP_DCIMVAC, start);

		//Increments the cache line size aligned address in start with the
		//value of the minimum cache line size.
		start += min_data_cache_line_size;
	} while (start < end);

	//Data Synchronization Barrier acts as a special kind of memory barrier. No
	//instruction in program order after this instruction executes until this
	//instruction completes. This instruction completes when:
	//• All explicit memory accesses before this instruction complete.
	//• All Cache, Branch predictor and TLB maintenance operations before this
	//  instruction complete.
	dsb();
}

////////

////////
/*
 *@start: Start virtual address.
 *
 *@end: End virtual address (exclusive).
 *
 *Function: Cleans cache lines containing data starting at the address @start
 *and ending at @end. @end is exclusive but is cleaned if it is not the first
 *word of a cache line.
 */
void clean_dcache_region(addr_t start, addr_t end)
{
	uint32_t ctr, min_data_cache_line_size, min_data_cache_line_mask;

	//ctr := CTR, Cache Type Register.
	COP_READ(COP_SYSTEM, COP_ID_CACHE_CONTROL_ARMv7_CLIDR, ctr);

	//ctr := ctr >> 16. This is the bit offset of the DminLine field of CTR.
	ctr >>= 16;

	//ctr := CTR.DminLine;
	//Log2 of the number of words in the smallest cache line of all the data
	//caches and unified caches that are controlled by the processor. Hence the
	//cache line size is a power of two. Minimum cache line size is needed to
	//prevent skipping a cache line in the loop.
	ctr &= 0xF;

	//Cache line size in bytes: 2^ctr * 4 =
	//(1 << ctr) * 4 = (1 << ctr) << 2 = 1 << (ctr + 2) =
	//1 << (2 + ctr) = (1 << 2) << ctr = 4 << ctr
	min_data_cache_line_size = 4 << ctr;

	//Cache line size is a power of two. Gives ones below the bit that is set
	//to denote the size. Hence a sort of a mask is stored in r3 that
	//identifies the byte addresses of a cache line.
	min_data_cache_line_mask = min_data_cache_line_size - 1;

	//Makes the start address cache line size aligned.
	start = start & ~min_data_cache_line_mask;

	//Cleans data cache lines containing words at addresses in the
	//interval [start, end) (for values in start and end at this execution
	//point).
	do {
		//DCCMVAC, Clean data (or unified) cache line by MVA to PoC. Can be
		//executed only by software executing at PL1 or higher. Cleans the
		//cache line containing the data word at address start.
		//Executes: MCR p15, 0, start, c7, c10, 1;
		COP_WRITE(COP_SYSTEM, COP_DCACHE_INVALIDATE_MVA, start);

		//Increments the cache line size aligned address in start with the
		//value of the minimum cache line size.
		start += min_data_cache_line_size;
	} while (start < end);

	//Data Synchronization Barrier acts as a special kind of memory barrier. No
	//instruction in program order after this instruction executes until this
	//instruction completes. This instruction completes when:
	//• All explicit memory accesses before this instruction complete.
	//• All Cache, Branch predictor and TLB maintenance operations before this
	//  instruction complete.
	dsb();
}
