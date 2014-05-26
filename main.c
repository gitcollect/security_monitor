#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#include "security_monitor/perf_count.h"

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

long cmd;


int file_p;

static void attestation_service_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = security_measure[measure_index].trust_evidence;
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

static void attestation_service_recv_cmd(struct sk_buff *skb)
{
    int cmd_type;
    struct nlmsghdr *nlh;
    nlh = (struct nlmsghdr *)skb->data;
    kstrtol((char *)nlmsg_data(nlh), 10, &cmd);
    printk(KERN_INFO "received cmd: %ld\n", cmd);
    cmd_type = cmd >> 28;
    switch(cmd_type) {
        case 0:
            printk(KERN_INFO "startup integrity checking\n");
            break;
        case 1:
            printk(KERN_INFO "runtime start counting\n");
            break;
        case 2:
            printk(KERN_INFO "runtime stop counting\n");
            break;
        default:
            printk(KERN_INFO "Invalid cmmand\n");
    }

}

static int __init attestation_service_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = attestation_service_recv_msg,
    };
    
    struct netlink_kernel_cfg cfg_cmd = {
        .input = attestation_service_recv_cmd,
    };

    pid_t pid;
    int ret;    

    printk("entering security monitor module\n");
    measure_index = 0;
    pid = 10213;
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    nl_sk_cmd = netlink_kernel_create(&init_net, NETLINK_CMD, &cfg_cmd);

    if (!(nl_sk && nl_sk_cmd))
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

//    strcpy(security_measure[0].trust_evidence, "helloworld");
//    strcpy(security_measure[1].trust_evidence, "worldhello");

    ret = perf_count_start(pid, &file_p);
    return 0;
}

static void __exit attestation_service_exit(void)
{
    uint64_t result = 0;
    perf_count_stop(&file_p, &result);
    printk("the result: %lld\n", result);
    printk(KERN_INFO "exiting security monitor module\n");
    netlink_kernel_release(nl_sk);
    netlink_kernel_release(nl_sk_cmd);
}

module_init(attestation_service_init); 
module_exit(attestation_service_exit);

MODULE_LICENSE("GPL");
