KERN_DIR := /home/qhx/work/100ask_imx6ull/kernel/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules
	#$(CROSS_COMPILE)gcc -o led_drv_test led_drv_test.c

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	#rm -f led_drv_test

obj-m += led_device.o led_driver.o
