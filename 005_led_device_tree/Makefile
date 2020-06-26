KERN_DIR := /home/qhx/work/100ask_imx6ull/pro_code/kernel/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += led_drv.o
