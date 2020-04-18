KERN_DIR := /home/qhx/work/Alpha_IMX6/kernel/code

all:
	make -C $(KERN_DIR) M=`pwd` modules
	$(CROSS_COMPILE)gcc -o led_drv_test led_drv_test.c

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f led_drv_test

obj-m += led_drv.o
