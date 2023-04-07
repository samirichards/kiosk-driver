obj-m += kiosk-driver.o
KDIR = /lib/modules/$(shell uname -r)/build

ccflags-y := -D__KERNEL__

all:
	make -C $(KDIR) M=$(shell pwd) modules CCFLAGS_MODULE=-DMODULE

clean:
	make -C $(KDIR) M=$(shell pwd) clean