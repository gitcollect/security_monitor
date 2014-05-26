KDIR := /lib/modules/$(shell uname -r)/build
PWD     := $(shell pwd)
TESTS_DIR = tests

.PHONY: tests


MODULE_NAME = attestation_service
SRC     := main.c security_monitor/perf_count.c
$(MODULE_NAME)-objs = $(SRC:.c=.o)
obj-m       := $(MODULE_NAME).o

all: kernel tests

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

tests:
	$(MAKE) -C $(TESTS_DIR)

clean:
	rm -rf */*.o *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
	$(MAKE) -C $(TESTS_DIR) clean
