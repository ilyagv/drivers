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
#include <linux/if_xdp.h>

#define NUM_DESCS 1024
#define IFNAME "eth1"

static const uint32_t chunk_size = 4096;
//static const uint32_t chunk_count = 1048576;
static const uint32_t chunk_count = 1048590;
static const uint64_t umem_len = (uint64_t)chunk_size * chunk_count;

int main(void)
{
	struct xdp_umem_reg umem_reg = {0};
	struct sockaddr_xdp sxdp = {0};
	unsigned char * umem = NULL;
	int ret = EXIT_FAILURE;
	// ring size
	int ndescs = NUM_DESCS;
	int sfd;

	umem = aligned_alloc(sysconf(_SC_PAGESIZE), umem_len);

	umem_reg.addr = (__u64)(void *)umem;
	umem_reg.len = umem_len;
	umem_reg.chunk_size = chunk_size;
	umem_reg.headroom = 0;
	umem_reg.flags = 0;

	printf("chunk_size %u chunk_count %u umem_len %lu(0x%lx)\n"
		, chunk_size
		, chunk_count
		, umem_len
		, umem_len
		);

	sfd = socket(AF_XDP, SOCK_RAW, 0);
	if (sfd < 0) {
		printf("Failed to create a socket. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	sxdp.sxdp_family = AF_XDP;
	sxdp.sxdp_ifindex = if_nametoindex(IFNAME);

	ret = setsockopt(sfd, SOL_XDP, XDP_RX_RING, &ndescs, sizeof(int));
	if (ret < 0) {
		printf("setsockopt failed. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	ret = setsockopt(sfd, SOL_XDP, XDP_TX_RING, &ndescs, sizeof(int));
	if (ret < 0) {
		printf("setsockopt failed. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	ret = setsockopt(sfd, SOL_XDP, XDP_UMEM_COMPLETION_RING, &ndescs, sizeof(int));
	if (ret < 0) {
		printf("setsockopt failed. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	ret = setsockopt(sfd, SOL_XDP, XDP_UMEM_FILL_RING, &ndescs, sizeof(int));
	if (ret < 0) {
		printf("setsockopt failed. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	ret = setsockopt(sfd, SOL_XDP, XDP_UMEM_REG, &umem_reg, sizeof(struct xdp_umem_reg));
	if (ret < 0) {
		printf("setsockopt failed. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	ret = bind(sfd, (struct sockaddr *)&sxdp, sizeof(sxdp));
	if (ret < 0) {
		printf("bind failed. Error (%d) %s\n", errno, strerror(errno));
		goto exit;
	}

exit:
	if (sfd)
		close(sfd);
	free(umem);

	return ret;
}
