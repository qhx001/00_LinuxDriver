KERN_DIR := /home/qhx/Work/100ask_imx6ull/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m += driver.o
