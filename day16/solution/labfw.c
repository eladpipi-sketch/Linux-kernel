// SPDX-License-Identifier: GPL-2.0
/*
 * Day 16 exercise: labfw on the receiving side. It drops every second ping
 * reply on its way in, so ping sees packet loss instead of an error.
 */
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

static atomic_t seen = ATOMIC_INIT(0);

static unsigned int labfw_hook(void *priv, struct sk_buff *skb,
			       const struct nf_hook_state *state)
{
	struct iphdr *ip = ip_hdr(skb);
	int n;

	if (ip->protocol != IPPROTO_ICMP)
		return NF_ACCEPT;
	n = atomic_inc_return(&seen);
	pr_info("labfw: reply #%d from %pI4 %s\n", n, &ip->saddr,
		n % 2 ? "accepted" : "dropped");
	return n % 2 ? NF_ACCEPT : NF_DROP;
}

static struct nf_hook_ops labfw_ops = {
	.hook		= labfw_hook,
	.pf		= NFPROTO_IPV4,
	.hooknum	= NF_INET_LOCAL_IN,	/* packets delivered to this machine */
	.priority	= NF_IP_PRI_FIRST,
};

static int __init labfw_init(void)
{
	return nf_register_net_hook(&init_net, &labfw_ops);
}

static void __exit labfw_exit(void)
{
	nf_unregister_net_hook(&init_net, &labfw_ops);
}

module_init(labfw_init);
module_exit(labfw_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 16 exercise: drop every second reply");
