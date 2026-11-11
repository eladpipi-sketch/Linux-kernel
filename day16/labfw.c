// SPDX-License-Identifier: GPL-2.0
/* labfw: a netfilter hook that sees every outgoing ping, and can drop it */
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

static bool drop;
module_param(drop, bool, 0644);		/* echo Y > /sys/module/labfw/parameters/drop */
static atomic_t seen = ATOMIC_INIT(0);

static unsigned int labfw_hook(void *priv, struct sk_buff *skb,
			       const struct nf_hook_state *state)
{
	struct iphdr *ip = ip_hdr(skb);

	if (ip->protocol != IPPROTO_ICMP)
		return NF_ACCEPT;
	pr_info("labfw: ping #%d to %pI4 %s\n", atomic_inc_return(&seen),
		&ip->daddr, drop ? "dropped" : "accepted");
	return drop ? NF_DROP : NF_ACCEPT;
}

static struct nf_hook_ops labfw_ops = {
	.hook		= labfw_hook,
	.pf		= NFPROTO_IPV4,
	.hooknum	= NF_INET_LOCAL_OUT,	/* packets this machine sends */
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
MODULE_DESCRIPTION("Day 16: a netfilter hook");
