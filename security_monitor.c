#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 30
#define NETLINK_CMD 29

#define MAX_VM 20

struct vtpm {
    pid_t process_id;
    char trust_evidence[32];
};

struct vtpm security_measure[MAX_VM];

struct sock *nl_sk = NULL;
struct sock *nl_sk_cmd = NULL;

long measure_index;

static void security_monitor_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = security_measure[0].trust_evidence;
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */

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

static void security_monitor_recv_cmd(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    nlh = (struct nlmsghdr *)skb->data;
    kstrtol((char *)nlmsg_data(nlh), 10, &measure_index);
    printk(KERN_INFO "Netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));
    printk(KERN_INFO "Netlink received msg payload: %ld\n", measure_index);

}

static int __init security_monitor_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = security_monitor_recv_msg,
    };
    
    struct netlink_kernel_cfg cfg_cmd = {
        .input = security_monitor_recv_cmd,
    };

    printk("entering security monitor module\n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    nl_sk_cmd = netlink_kernel_create(&init_net, NETLINK_CMD, &cfg_cmd);

    if (!(nl_sk && nl_sk_cmd))
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    strcpy(security_measure[0].trust_evidence, "helloworld");
    strcpy(security_measure[1].trust_evidence, "worldhello");
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
