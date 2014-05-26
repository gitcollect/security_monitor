#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>


#define NETLINK_CMD 29

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main(int argc, char* argv[])
{
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_CMD);
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    int proc_pid = atoi(argv[2]);
    int monitor_time = atoi(argv[1]);
 
    int cmd;
    char cmd_string[10];

    pid_t stop_proc = fork();
    if (stop_proc == 0) {
        sleep(monitor_time);
        cmd = 0x20000000 + proc_pid;
        sprintf(cmd_string, "%d", cmd);
        strcpy(NLMSG_DATA(nlh), cmd_string);
        printf("Stop the performance counting......\n");
        sendmsg(sock_fd, &msg, 0);
        close(sock_fd);
        return 0;
    }

    cmd = 0x10000000 + proc_pid;
    sprintf(cmd_string, "%d", cmd);
    strcpy(NLMSG_DATA(nlh), cmd_string);
    printf("Start the performance counting......\n");
    sendmsg(sock_fd, &msg, 0);
    close(sock_fd);

    return 0;
}
