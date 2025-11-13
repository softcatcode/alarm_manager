obj-m += alm.o
alm-objs := am_timer_main.o args_parser.o args_vfile.o common_utils.o
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

.PHONY: all clean
