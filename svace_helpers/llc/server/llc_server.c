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

#define USE_PEEK
//#define DGRAM_SERVER

#define LLC_SAP_BSPAN 0x42
#define LLC_SAP_IP 0x06
#define LLC_SAP_NULL 0x0

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

static int set_multicast(int fd, const char *iface, bool enable)
{
	unsigned request = enable ? SIOCADDMULTI : SIOCDELMULTI;
	int ret;

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
	ifr.ifr_hwaddr.sa_family = AF_UNSPEC;
	ifr.ifr_hwaddr.sa_data[0] = 0x01;
	ifr.ifr_hwaddr.sa_data[1] = 0x80;
	ifr.ifr_hwaddr.sa_data[2] = 0xC2;

	ret = ioctl(fd, request, &ifr);
	if (ret < 0) {
		perror("ioctl(SIOCADDMULTI)");
		return false;
	}

	return true;
}

static int print_sock(int fd)
{
	struct sockaddr_llc local_addr = { 0 };
	socklen_t sllc_len = sizeof(local_addr);
	int ret;

	ret = getsockname(fd, (struct sockaddr *)&local_addr, &sllc_len);
	if (ret < 0) {
		perror("getsockname()");
		return false;
	}

	printf("sllc_family : %u\n", local_addr.sllc_family);
	printf("sllc_arphrd : %u\n", local_addr.sllc_arphrd);
	printf("sllc_test : %u\n", local_addr.sllc_test);
	printf("sllc_xid : %u\n", local_addr.sllc_xid);
	printf("sllc_ua : %u\n", local_addr.sllc_ua);
	printf("sllc_sap : %u\n", local_addr.sllc_sap);
	printf("sllc_mac : %02x:%02x:%02x:%02x:%02x:%02x\n",
	       local_addr.sllc_mac[0], local_addr.sllc_mac[1],
	       local_addr.sllc_mac[2], local_addr.sllc_mac[3],
	       local_addr.sllc_mac[4], local_addr.sllc_mac[5]);

	return true;
}

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

static int create_socket(int sock_type, const char *iface)
{
	struct sockaddr_llc local_addr = { 0 };
	struct ether_addr *eth;
	int type;
	int ret;
	int fd;

	// create LLC socket
	fd = socket(AF_LLC, sock_type, 0);
	if (fd < 0) {
		perror("socket(LLC)");
		return fd;
	}

	eth = (struct ether_addr *)local_addr.sllc_mac;
	ret = get_hwaddr(iface, eth, &type);
	if (ret < 0)
		goto error;

	// bind LLC socket to SAP
	local_addr.sllc_family = AF_LLC;
	local_addr.sllc_arphrd = type;
	local_addr.sllc_sap = 2;

	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0) {
		perror("bind()");
		goto error;
	}

	if (!print_sock(fd))
		goto error;

	return fd;
error:
	close(fd);
	return -1;
}

static int client_start(int fd)
{
	unsigned char buffer[1024] = { 0 };
	ssize_t bytes_recv;
	int flags = 0;

#ifdef USE_PEEK
	flags = MSG_PEEK;
#endif
	while (true) {
		bytes_recv = recv(fd, buffer, sizeof(buffer), flags);
		if (bytes_recv < 0) {
			fprintf(stderr, "Recieve failed with error: (%d) %s\n",
				errno, strerror(errno));
			return -1;
		}

		if (!bytes_recv)
			break;

		printf("Client: recieved %zd bytes\n", bytes_recv);
	}

	return 0;
}

static int stream_server(const char *iface)
{
	int client;
	int fd;
	int ret;

	fd = create_socket(SOCK_STREAM, iface);
	if (fd < 0)
		return EXIT_FAILURE;

	ret = listen(fd, 10);
	if (ret < 0) {
		perror("listen()\n");
		goto exit;
	}

	while (1) {
		struct sockaddr_llc remote_addr = { 0 };
		socklen_t remote_addr_size = 0;
		client = accept(fd, (struct sockaddr *)&remote_addr,
				&remote_addr_size);
		if (client < 0) {
			ret = client;
			perror("accept()");
			goto exit;
		}

		printf("accept() peer: %02x:%02x:%02x:%02x:%02x:%02x\n",
		       remote_addr.sllc_mac[0], remote_addr.sllc_mac[1],
		       remote_addr.sllc_mac[2], remote_addr.sllc_mac[3],
		       remote_addr.sllc_mac[4], remote_addr.sllc_mac[5]);

		client_start(client);

		close(client);
	}
exit:
	close(fd);
	return ret;
}

static int dgram_server(const char *iface)
{
	int fd;

	fd = create_socket(SOCK_DGRAM, iface);
	if (fd < 0)
		return EXIT_FAILURE;

	//set_multicast(fd, iface, true);

	do {
		struct sockaddr peer_addr;
		socklen_t peer_addr_len;
		ssize_t bytes_recv;
		u_int8_t buf[1536];
		int flags = 0;

#ifdef USE_PEEK
		flags = MSG_PEEK;
#endif
		/* receive LLC packet */
		memset(&peer_addr, 0, sizeof(peer_addr));
		peer_addr_len = sizeof(peer_addr);
		bytes_recv = recvfrom(fd, buf, sizeof(buf), flags, &peer_addr,
				      &peer_addr_len);
		if (bytes_recv < 0) {
			perror("recvfrom()");
			break;
		}

		if (peer_addr.sa_family == AF_LLC) {
			struct sockaddr_llc *peer_sllc;

			peer_sllc = (struct sockaddr_llc *)&peer_addr;
			/* do job with peer_llc & buf */
			printf("Recieved %zd bytes from: %02x:%02x:%02x:%02x:%02x:%02x\n",
			       bytes_recv, peer_sllc->sllc_mac[0],
			       peer_sllc->sllc_mac[1], peer_sllc->sllc_mac[2],
			       peer_sllc->sllc_mac[3], peer_sllc->sllc_mac[4],
			       peer_sllc->sllc_mac[5]);
		}
	} while (1);

	//set_multicast(fd, iface, false);

	return 0;
}

int main(void)
{
	char iface[IFNAMSIZ] = "eth1";
#ifdef DGRAM_SERVER
	return dgram_server(iface);
#else
	return stream_server(iface);
#endif
}
