#define _GNU_SOURCE
#include <endian.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>

#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <linux/in6.h>
#include <net/if.h>
#include <arpa/inet.h>

/* Calculate "x * n / d" without unnecessary overflow or loss of precision. */
#define mult_frac(x, n, d)	\
({				\
	typeof(x) x_ = (x);	\
	typeof(n) n_ = (n);	\
	typeof(d) d_ = (d);	\
				\
	typeof(x_) q = x_ / d_;	\
	typeof(x_) r = x_ % d_;	\
	q * n_ + r * n_ / d_;	\
})

#define check_add_overflow(a, b, d) ({		\
	typeof(a) __a = (a);			\
	typeof(b) __b = (b);			\
	typeof(d) __d = (d);			\
	(void) (&__a == &__b);			\
	(void) (&__a == __d);			\
	__builtin_add_overflow(__a, __b, __d);	\
})

static bool is_metric_better_old(uint32_t x, uint32_t y, const  uint32_t persent)
{
	uint32_t a, e;

	if (x >= y)
		return false;

	a = mult_frac(x, persent, 100);

	if (check_add_overflow(x, a, &e)) {
		if (x > y - a)
			return false;
	} else {
		if (e > y)
			return false;
	}

	return true;
}

static bool is_metric_better(uint32_t x, uint32_t y)
{
	return (x < y) && (x < (y - x / 10));
}

struct metrics {
	uint32_t new_metric;
	uint32_t mpath_metric;
};

static inline const char * bstr(bool v)
{
	static const char * strings[2] = {
		"false", "true"
	};
	return strings[!!v];
}

int main(void)
{
	void *sta = (void *)0xdeadbeef;
	void *mpath_next_hop = (void *)0x0000efef;
	uint32_t new_metric = 0xffffffff;
	uint32_t mpath_metric = 0xffffffff - 1;
	unsigned i;
	bool process = true;

	uint32_t mt32 = mult_frac(new_metric, 10, 9);
	uint64_t mt64 = mult_frac((uint64_t)new_metric, 10, 9);

	static struct metrics test_metrics[] = {
		//new_metric	mpath_metric
		{ 0,		0 },
		{ 1,		1 },
		{ 1,		2 },
		{ 1,		3 },
		{ 1,		4 },
		{ 1,		5 },
		{ 1,		6 },
		{ 1,		7 },
		{ 1,		8 },
		{ 1,		9 },
		{ 1,		10 },
		{ 2,            3 },
		{ 3,            4 },
		{ 4,            5 },
		{ 7,            4 },
		{ 6,            4 },
		{ 5,            4 },
		{ 10,		11 },
		{ 10,		12 },
		{ 10,		13 },
		{ 10,		14 },
		{ 10,		15 },
		{ 10,		16 },
		{ 10,		17 },
		{ 10,		18 },
		{ 0xffffffff-1, 0xffffffff},
		{ 1, 0xffffffff},
		{ 0, 0xffffffff},
		{ 0xffffffff,	0xffffffff - 1},
		{ 0xffffffff,	0xffffffff},
		{ 0xffffffff - 0xffffffff/10 - 1,	0xffffffff},
		{ 0xffffffff - 0xffffffff/10,	0xffffffff},
		{ 0x2,	0xffffffff},
		{ 0x20,	0x2},
		{3904515710,	0xffffffff }, 
		{3904515720,	0xffffffff }, 
		{3904515721,	0xffffffff }, 
		{3904515722,	0xffffffff }, 

		{3904515830,	0xffffffff }, 
		{3904515840,	0xffffffff }, 
		{3904515850,	0xffffffff }, 
	};

	printf("u32 %u u64 %lu\n", mt32, mt64);

	if ( (sta ? mult_frac(new_metric, 10, 9) : new_metric) >= mpath_metric) {
		printf("u32 process=false!!!\n");
	} else {
		printf("u32 process=true !!!\n");
	}

	if ( (sta ? mult_frac((uint64_t)new_metric, 10, 9) : new_metric) >= mpath_metric) {
		printf("u64 process=false!!!\n");
	} else {
		printf("u64 process=true !!!\n");
	}

	printf("%3s: %10s(       hex) %10s(       hex) %10s %10s chk %10s %10s   chk %7s %7s %7s %7s xxx %7s xxx\n"
			, "num"
			, "new_metric"
			, "mpath_metr"
			, "10% mult"
			, "10% div"
			, "new_me+10%"
			, "mul_frac64"
			, "procnew"
			, "process"
			, "core_10"
			, "correct"
			//, "u64"
			, "u32"
	      );

	for (i = 0; i < sizeof(test_metrics) / sizeof(test_metrics[0]); ++i) {
		bool correct = true;
		bool correct_ten_percent = true;
		bool u32res = true;
		bool process_new = true;

		process = true;

		new_metric = test_metrics[i].new_metric;
		mpath_metric = test_metrics[i].mpath_metric;

		if ( mpath_next_hop != sta ? !is_metric_better_old(new_metric, mpath_metric, 10) : new_metric >= mpath_metric)
			process = false;

		if ( !is_metric_better(new_metric, mpath_metric) )
			process_new = false;

		if ( (mpath_next_hop != sta ? mult_frac((uint64_t)new_metric, 10, 9) : new_metric) >= mpath_metric)
			correct = false;

//		uint64_t n = (uint64_t)new_metric + new_metric / 10;
//		printf("new_metric %u new_metric + 10%% %lu mpath_metric %u >= %u\n"
//			, new_metric
//			, n
//			, mpath_metric
//			, n >= mpath_metric
//		      );
		if ( ((uint64_t)new_metric + new_metric / 10) >= (uint64_t)mpath_metric)
			correct_ten_percent = false;

		if ( (mpath_next_hop != sta ? mult_frac(new_metric, 10, 9) : new_metric) >= mpath_metric)
			u32res = false;

		uint32_t ten_percent_mult = mult_frac(new_metric, 10, 100);
		uint32_t ten_percent_div = new_metric / 10;

		printf("%3u: %10u(0x%08x) %10u(0x%08x) %10u %10u %-3s %10lu %10lu   %-3s %7s %7s %7s %7s %-3s %7s %-3s\n"
				, i
				, new_metric
				, new_metric
				, mpath_metric
				, mpath_metric
				, ten_percent_mult
				, ten_percent_div
				, ten_percent_div == ten_percent_mult ? "" : "!!!"
				, (uint64_t)new_metric + ten_percent_div
				, mult_frac((uint64_t)new_metric, 10, 9)
				, process == process_new ? "" : "!!!"
				, bstr(process_new)
				, bstr(process)
				, bstr(correct_ten_percent)
				, bstr(correct)
				, process == correct ? "" : "!!!"
				, bstr(u32res)
				, u32res == correct ? "" : "!!!"
				);
	}

	return 0;
}

