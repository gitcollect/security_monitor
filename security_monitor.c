#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 32
#define MAX_VM 20

struct vtpm {
    pid_t process_id;
    char trust_evidence[32];
};

extern struct vtpm security_measure[MAX_VM];

struct sock *nl_sk = NULL;

static void security_monitor_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */
    printk(KERN_INFO "pid: %d\n", pid);

    skb_out = nlmsg_new(msg_size, 0);

    if (!skb_out)
    {

        printk(KERN_ERR "Failed to allocate new skb\n");
        return;

    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);

    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

static int __init security_monitor_init(void)
{
    int index;
    struct netlink_kernel_cfg cfg = {
        .input = security_monitor_recv_msg,
    };

    printk("entering security monitor module\n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    for (index = 0; index < MAX_VM; index++) {
        if (security_measure[index].process_id != -1)
            printk(KERN_INFO "%d: %d: %s\n", index, security_measure[index].process_id, security_measure[index].trust_evidence);
    }
    return 0;
}

static void __exit security_monitor_exit(void)
{
    printk(KERN_INFO "exiting secu module\n");
    netlink_kernel_release(nl_sk);
}

module_init(security_monitor_init); 
module_exit(security_monitor_exit);

MODULE_LICENSE("GPL");
