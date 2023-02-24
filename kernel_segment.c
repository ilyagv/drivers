#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/highmem.h>
#include <linux/version.h>
#include <asm/fixmap.h>

#define SHOW_DELTA_K(low, hi) ((void *)low), ((void *)hi), (((hi) - (low)) >> 10)
#define SHOW_DELTA_M(low, hi) ((void *)low), ((void *)hi), (((hi) - (low)) >> 20)
#define SHOW_DELTA_MG(low, hi) ((void *)low), ((void *)hi), (((hi) - (low)) >> 20), (((hi) - (low)) >> 30)


#define ELLPS "|                           [ . . . ]                         |\n"


void show_kernel_segment(void)
{
    	pr_info("\nSome Kernel Details [by decreasing address]\n"
            "+-------------------------------------------------------------+\n");
#ifdef CONFIG_ARM
	/* On ARM, the definition of VECTORS_BASE turns up only in kernels >= 4.11 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
        pr_info("|vector table:       "
            " %px - %px | [%4ld KB]\n",
            SHOW_DELTA_K(VECTORS_BASE, VECTORS_BASE + PAGE_SIZE));
#endif
#endif // CONFIG_ARM

	/* kernel fixmap region */
	pr_info(ELLPS
		"|fixmap region:      "
		" %px - %px     | [%4ld MB]\n",
#ifdef CONFIG_ARM
		SHOW_DELTA_M(FIXADDR_START, FIXADDR_END));
#else
		SHOW_DELTA_M(FIXADDR_START, (FIXADDR_START+FIXADDR_SIZE)));
#endif

	/* kernel module region
	 * For the modules region, it's high in the kernel segment on typical 64-bit
	 * systems, but the other way around on many 32-bit systems (particularly
	 * ARM-32); so we rearrange the order in which it's shown depending on the
	 * arch, thus trying to maintain a 'by descending address' ordering.
	 */
#if (BITS_PER_LONG == 64)
	pr_info("|module region:      "
		" %px - %px     | [%ld MB]\n",
		SHOW_DELTA_M(MODULES_VADDR, MODULES_END));
#endif

#ifdef CONFIG_KASAN		// KASAN region: Kernel Address SANitizer
	pr_info("|KASAN shadow:       "
		" %px - %px     | [%2ld GB]\n",
		SHOW_DELTA_G(KASAN_SHADOW_START, KASAN_SHADOW_END));
#endif

	/* vmalloc region */
	pr_info("|vmalloc region:     "
		" %px - %px     | [%4ld MB = %2ld GB]\n",
		SHOW_DELTA_MG(VMALLOC_START, VMALLOC_END));

	/* lowmem region */
	pr_info("|lowmem region:      "
		" %px - %px     | [%4ld MB = %2ld GB]\n"
#if (BITS_PER_LONG == 32)
		"|           (above:PAGE_OFFSET - highmem)                     |\n",
#else
		"|                (above:PAGE_OFFSET    -      highmem)        |\n",
#endif
		SHOW_DELTA_MG((unsigned long)PAGE_OFFSET, (unsigned long)high_memory));

	/* (possible) highmem region;  may be present on some 32-bit systems */
#ifdef CONFIG_HIGHMEM
	pr_info("|HIGHMEM region:     "
		" %px - %px | [%4ld MB]\n",
		SHOW_DELTA_M(PKMAP_BASE, (PKMAP_BASE) + (LAST_PKMAP * PAGE_SIZE)));
#endif

	/*
	 * Symbols for kernel:
	 *   text begin/end (_text/_etext)
	 *   init begin/end (__init_begin, __init_end)
	 *   data begin/end (_sdata, _edata)
	 *   bss begin/end (__bss_start, __bss_stop)
	 * are only defined *within* (in-tree) and aren't available for modules;
	 * thus we don't attempt to print them.
	 */

#if (BITS_PER_LONG == 32)	/* modules region: see the comment above reg this */
	pr_info("|module region:      "
		" %px - %px | [%4ld MB]\n",
		SHOW_DELTA_M(MODULES_VADDR, MODULES_END));
#endif
	pr_info(ELLPS);

}