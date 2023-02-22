#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

static struct net_device *dev;

static int ndev_open(struct net_device *dev)
{
    pr_info("Net device open()\n");
    netif_start_queue(dev);
    return 0;
}

static int ndev_close(struct net_device *dev)
{
    pr_info("Net device close()\n");
    netif_stop_queue(dev);
    return 0;
}

static int ndev_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    dev_kfree_skb(skb);
    return 0;
}

static struct net_device_ops ndo = {
    .ndo_open = ndev_open,
    .ndo_stop = ndev_close,
    .ndo_start_xmit = ndev_start_xmit,
};

static void ndev_setup(struct net_device *dev)
{
    u8 addr[ETH_ALEN];

    // Fill in the MAC address
    eth_random_addr(addr);
    __dev_addr_set(dev, addr, ETH_ALEN);

    ether_setup(dev);
    dev->netdev_ops = &ndo;
}

int ndev_init(void)
{
    int err;

    pr_info("Init net device\n");

    dev = alloc_netdev(0, "fict%d", NET_NAME_ENUM, ndev_setup);
    if (!dev)
        return -ENOMEM;

    err = register_netdev(dev);
    if (err) {
        pr_warn("Failed to register net device\n");
        free_netdev(dev);
        return err;
    }
    pr_info("Succeeded in loading %s\n", dev_name(&dev->dev));
    return 0;
}

void ndev_exit(void)
{
    pr_info("Unloading net device\n");
    unregister_netdev(dev);
    free_netdev(dev);
}
