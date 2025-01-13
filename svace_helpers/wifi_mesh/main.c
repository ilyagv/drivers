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

#define mult_frac(x, numer, denom)(			\
{							\
	typeof(x) quot = (x) / (denom);			\
	typeof(x) rem  = (x) % (denom);			\
	(quot * (numer)) + ((rem * (numer)) / (denom));	\
}							\
)

int main(void)
{
	void *sta = (void *)0xdeadbeef;
	uint32_t new_metric = 0xffffffff;
	uint32_t mpath_metric = 0xffffffff - 1;

	uint32_t mt32 = mult_frac(new_metric, 10, 9);
	uint64_t mt64 = mult_frac((uint64_t)new_metric, 10, 9);

	printf("u32 %u u64 %lu\n", mt32, mt64);

	if ( (sta ? mult_frac(new_metric, 10, 9) : new_metric) >= mpath_metric) {
		printf("u32 Greater!!!\n");
	} else {
		printf("u32 less !!!\n");
	}

	if ( (sta ? mult_frac((uint64_t)new_metric, 10, 9) : new_metric) >= mpath_metric) {
		printf("u64 Greater!!!\n");
	} else {
		printf("u64 less !!!\n");
	}

	return 0;
}

