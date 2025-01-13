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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>

//#define DGRAM_CLIENT

#define LLC_SAP_NULL 0x0
#define LLC_SAP_BSPAN 0x42
#define LLC_SAP_IP 0x06

struct sockaddr_llc {
	short sllc_family;
	short sllc_arphrd;
	unsigned char sllc_test;
	unsigned char sllc_xid;
	unsigned char sllc_ua;
	unsigned char sllc_sap;
	unsigned char sllc_mac[6];
	unsigned char __pad[2];
};

static int get_hwaddr(const char *name, struct ether_addr *addr, int *type)
{
	struct ifreq ifr;
	int fd;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Error: Unable to create AF_INET socket");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("Error: ioctl() failed");
		close(fd);
		return -1;
	}

	close(fd);

	memcpy(addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	*type = ifr.ifr_hwaddr.sa_family;

	return 0;
}

#define HOSTNAME "virthost"
#define IFNAME "tap2"

static int create_socket(int sock_type)
{
	struct sockaddr_llc local_addr = { 0 };
	struct ether_addr *eth;
	int type = 0;
	int fd = -1;
	int ret = 0;

	fd = socket(AF_LLC, sock_type, 0);
	if (fd < 0) {
		perror("socket(LLC)");
		return fd;
	}

	eth = (struct ether_addr *)local_addr.sllc_mac;
	ret = get_hwaddr(IFNAME, eth, &type);
	if (ret < 0)
		goto error;

	local_addr.sllc_family = AF_LLC;
	local_addr.sllc_arphrd = type;
	local_addr.sllc_sap = 2;

	ret = bind(fd, (const struct sockaddr *)&local_addr,
		   sizeof(struct sockaddr_llc));
	if (ret < 0) {
		perror("bind()");
		goto error;
	}

	return fd;
error:
	close(fd);
	return -1;
}

static bool get_remote_addr(struct sockaddr_llc *remote)
{
	struct ether_addr *eth = (struct ether_addr *)&remote->sllc_mac;

	if (ether_hostton(HOSTNAME, eth)) {
		fprintf(stderr,
			"Error: unable to get MAC address in ethers \'%s'\n",
			HOSTNAME);
		return false;
	}
	remote->sllc_family = AF_LLC;
	remote->sllc_sap = 2;
	remote->sllc_arphrd = ARPHRD_ETHER;

	printf("Remote MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
	       remote->sllc_mac[0], remote->sllc_mac[1], remote->sllc_mac[2],
	       remote->sllc_mac[3], remote->sllc_mac[4], remote->sllc_mac[5]);

	return true;
}

static int stream_client(void)
{
	unsigned char send_buffer[] = "\x65\x65\x65\x65";
	struct sockaddr_llc remote_addr = { 0 };
	int ret = EXIT_FAILURE;
	int fd = -1;

	const unsigned nr_send = 4;
	unsigned i;

	fd = create_socket(SOCK_STREAM);
	if (fd < 0)
		return ret;

	if (!get_remote_addr(&remote_addr))
		goto exit;

	ret = connect(fd, (const struct sockaddr *)&remote_addr,
		      sizeof(struct sockaddr_llc));
	if (ret < 0) {
		perror("connect()");
		goto exit;
	}

	for (i = 0; i < nr_send; ++i) {
		ret = send(fd, send_buffer, sizeof(send_buffer) - 1, 0);
		if (ret < 0) {
			fprintf(stderr, "Send error(%d) %s\n", errno,
				strerror(errno));
			goto exit;
		}

		printf("%u bytes sent\n", ret);
	}

	ret = 0;
exit:
	close(fd);
	return ret;
}

static int dgram_client(void)
{
	unsigned char send_buffer[] = "\x65\x65\x65\x65";
	struct sockaddr_llc remote_addr = { 0 };
	int ret = EXIT_FAILURE;
	int fd = -1;

	fd = create_socket(SOCK_DGRAM);
	if (fd < 0)
		return ret;

	if (!get_remote_addr(&remote_addr))
		goto exit;

	ret = sendto(fd, send_buffer, sizeof(send_buffer) - 1, 0,
		     (struct sockaddr *)&remote_addr, sizeof(remote_addr));
	if (ret < 0) {
		fprintf(stderr, "Send error(%d) %s\n", errno, strerror(errno));
		goto exit;
	}

	printf("%u bytes sent\n", ret);

	ret = 0;
exit:
	close(fd);
	return ret;
}

int main(void)
{
#ifdef DGRAM_CLIENT
	//return dgram_client();
#else
	return stream_client();
#endif
}
