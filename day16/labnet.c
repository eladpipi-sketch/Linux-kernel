// SPDX-License-Identifier: GPL-2.0
/*
 * labnet: a network interface with a pretend host behind it. The driver
 * itself answers every IPv4 ping sent out of lab0.
 * Modeled on drivers/net/dummy.c and LKMPG's vnetloop.c.
 */
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <net/checksum.h>

static struct net_device *labnet;

/* Build the echo reply a real host would send, and push it up the stack. */
static void answer_ping(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *reply = skb_copy(skb, GFP_ATOMIC);	/* private, writable */
	struct ethhdr *eth;
	struct iphdr *ip;
	struct icmphdr *icmp;

	if (!reply)
		return;
	eth = (struct ethhdr *)reply->data;
	ip = (struct iphdr *)(reply->data + ETH_HLEN);
	icmp = (struct icmphdr *)((u8 *)ip + ip->ihl * 4);

	ether_addr_copy(eth->h_dest, dev->dev_addr);	/* addressed to us */
	swap(ip->saddr, ip->daddr);			/* IP checksum stays valid */
	icmp->type = ICMP_ECHOREPLY;
	icmp->checksum = 0;
	icmp->checksum = ip_compute_csum(icmp, ntohs(ip->tot_len) - ip->ihl * 4);

	reply->protocol = eth_type_trans(reply, dev);	/* strips the Ethernet header */
	netif_rx(reply);				/* "received" from the wire */
	DEV_STATS_INC(dev, rx_packets);
	DEV_STATS_ADD(dev, rx_bytes, reply->len);
}

/* The stack calls this for every packet routed out of lab0. */
static netdev_tx_t labnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct iphdr *ip = (struct iphdr *)(skb->data + ETH_HLEN);
	struct icmphdr *icmp;

	DEV_STATS_INC(dev, tx_packets);
	DEV_STATS_ADD(dev, tx_bytes, skb->len);
	netdev_info(dev, "xmit %u bytes, ethertype %#06x\n", skb->len, ntohs(skb->protocol));

	if (skb->protocol == htons(ETH_P_IP) && ip->protocol == IPPROTO_ICMP &&
	    skb->len >= ETH_HLEN + ip->ihl * 4 + sizeof(*icmp)) {
		icmp = (struct icmphdr *)((u8 *)ip + ip->ihl * 4);
		if (icmp->type == ICMP_ECHO)
			answer_ping(skb, dev);
	}
	dev_kfree_skb(skb);				/* we own it: free it */
	return NETDEV_TX_OK;
}

static const struct net_device_ops labnet_ops = {
	.ndo_start_xmit = labnet_xmit,
};

static void labnet_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->netdev_ops = &labnet_ops;
	dev->flags |= IFF_NOARP;	/* no ARP: IP packets go straight to xmit */
	eth_hw_addr_random(dev);
}

static int __init labnet_init(void)
{
	int err;

	labnet = alloc_netdev(0, "lab%d", NET_NAME_ENUM, labnet_setup);
	if (!labnet)
		return -ENOMEM;
	err = register_netdev(labnet);
	if (err)
		free_netdev(labnet);
	return err;
}

static void __exit labnet_exit(void)
{
	unregister_netdev(labnet);
	free_netdev(labnet);
}

module_init(labnet_init);
module_exit(labnet_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 16: a network driver that answers pings");
