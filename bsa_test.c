// buddy system allocator test
//#include <linux/kernel.h>
//#include <linux/init.h>
#include <linux/mm.h>
#include <asm/io.h> // virt_to_phys(), phys_to_virt(), ...

/*
 * show_phy_pages - show the virtual, physical addresses and PFNs of the memory
 *            range provided on a per-page basis.
 *
 * ! NOTE   NOTE   NOTE !
 * The starting kernel address MUST be a 'linear' address, i.e., an adrress
 * within the 'lowmem' direct-mapped region of the kernel segment, else this
 * will NOT work and can possibly crash the system.
 *
 * @kaddr: the starting kernel virtual address; MUST be a 'lowmem' region addr
 * @len: length of the memory piece (bytes)
 * @contiguity_check: if True, check for physical contiguity of pages
 *
 * 'Walk' the virtually contiguous 'array' of pages one by one (i.e. page by
 * page), printing the virt and physical address (& PFN- page frame number).
 * This way, we can see if the memory really is *physically* contiguous or not.
 */
void show_phy_pages(const void *kaddr, size_t len, bool contiguity_check)
{
	const void *vaddr = kaddr;
#if(BITS_PER_LONG == 64)
	const char *hdr = "-pg#-  -------va-------     --------pa--------   --PFN--\n";
#else             // 32-bit
	const char *hdr = "-pg#-  ----va----   --------pa--------   -PFN-\n";
#endif
	phys_addr_t pa;
	int loops = len/PAGE_SIZE, i;
	long pfn, prev_pfn = 1;

#ifdef CONFIG_X86
	if (!virt_addr_valid(vaddr)) {
		pr_info("%s(): invalid virtual address (0x%px)\n", __func__, vaddr);
		return;
	}
	/* Worry not, the ARM implementation of virt_to_phys() performs an internal
	 * validity check
	 */
#endif

	pr_info("%s(): start kaddr %px, len %zu, contiguity_check is %s\n",
		       __func__, vaddr, len, contiguity_check?"on":"off");
	pr_info("%s", hdr);
	if (len % PAGE_SIZE)
		loops++;
	for (i = 0; i < loops; i++) {
		pa = virt_to_phys(vaddr+(i*PAGE_SIZE));
		pfn = PHYS_PFN(pa);

		if (!!contiguity_check) {
		/* what's with the 'if !!(<cond>) ...' ??
		 * a 'C' trick: ensures that the if condition always evaluates
		 * to a boolean - either 0 or 1
		 */
			if (i && pfn != prev_pfn + 1) {
				pr_notice(" *** physical NON-contiguity detected (i=%d) ***\n", i);
				break;
			}
		}

		/* Below we show the actual virt addr and not a hashed value by
		 * using the 0x%[ll]x format specifier instead of the %pK as we
		 * should for security */
		/* if(!(i%100)) */
		pr_info("%05d  0x%px   %pa   %ld\n",
			i, vaddr+(i*PAGE_SIZE), &pa, pfn);
		if (!!contiguity_check)
			prev_pfn = pfn;
	}
}

/*
 * powerof - a simple 'library' function to calculate and return
 *  @base to-the-power-of @exponent
 * f.e. powerof(2, 5) returns 2^5 = 32.
 * Returns -1UL on failure.
 */
u64 powerof(int base, int exponent)
{
	u64 res = 1;

	if (base == 0)		// 0^e = 0
		return 0;
	if (base <= 0 || exponent < 0)
		return -1UL;
	if (exponent == 0)	// b^0 = 1
		return 1;
	while (exponent--)
		res *= base;
	return res;
}


int bsa_alloc(void)
{
    show_phy_pages((void *)PAGE_OFFSET, 5 * PAGE_SIZE, 1);
    return 0;
}