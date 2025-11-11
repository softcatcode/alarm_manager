obj-m += am_timer.o
am_timer-objs := am_timer_main.o unpack_args.o
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

.PHONY: all clean
