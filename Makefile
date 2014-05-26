KDIR := /lib/modules/$(shell uname -r)/build

PWD     := $(shell pwd)

MODULE_NAME = attestation_service
SRC     := main.c security_monitor/perf_count.c
$(MODULE_NAME)-objs = $(SRC:.c=.o)
obj-m       := $(MODULE_NAME).o

all: kernel user

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user:
	$(CC) netlink_user.c -o netlink_user

clean:
	rm -rf */*.o *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp* netlink_user
