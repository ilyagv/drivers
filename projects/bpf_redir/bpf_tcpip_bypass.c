
#include "bpf_sockops.h"


/* extract the key that identifies the destination socket in the sock_ops_map */
static inline
void sk_msg_extract4_key(struct sk_msg_md *msg,
	struct sock_key *key)
{
	key->sip4 = msg->remote_ip4;
	key->dip4 = msg->local_ip4;
	key->family = 1;

	key->dport = (bpf_htonl(msg->local_port) >> 16);
	key->sport = FORCE_READ(msg->remote_port) >> 16;
}

SEC("sk_msg")
int bpf_tcpip_bypass(struct sk_msg_md *msg)
{
#ifdef TEST_BPF_PUSH_DATA
	u32 start = 4;
	u32 end = (u32)(-1);
	long err = 0;
#endif // TEST_BPF_PUSH_DATA

	struct  sock_key key = {};
	sk_msg_extract4_key(msg, &key);

#ifdef TEST_BPF_PUSH_DATA
	err = bpf_msg_push_data(msg, start, end, 0);
	bpf_printk("bpf_msg_push_data() start %u, end %u err %ld\n",
		start, end, err);
#endif // TEST_BPF_PUSH_DATA

	bpf_msg_redirect_hash(msg, &sock_ops_map, &key, BPF_F_INGRESS);
	return SK_PASS;
}

char ____license[] SEC("license") = "GPL";
