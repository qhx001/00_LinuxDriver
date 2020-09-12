/*********************************************************************************************
 * 头文件包含
 ********************************************************************************************/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/wait.h>
#define DEBUG 1
#include "debug_out.h"
/*********************************************************************************************
 * 宏定义
 ********************************************************************************************/
#define USER_KEY_NUMBER    2           /* 用户按键的个数 */
#define USER_KEY_NAME      "USER_KEY"
/*********************************************************************************************
 * 函数声明
 ********************************************************************************************/
int user_key_probe(struct platform_device *pdev);
int user_key_remove(struct platform_device *pdev);
int user_key_open(struct inode *pinode, struct file *pfile);
int user_key_close(struct inode *pinode, struct file *pfile);
ssize_t user_key_read(struct file *pfile, char __user *buf, size_t len, loff_t *offset);
ssize_t user_key_write(struct file *pfile, const char __user *buf, size_t len, loff_t *offset);

void tasklet_func(unsigned long data);
/*********************************************************************************************
 * 变量声明
 ********************************************************************************************/

/*********************************************************************************************
 * 变量定义
 ********************************************************************************************/
/* 设备树匹配列表 */
const struct of_device_id user_key_device_id[1] = {
	[0] = {
		.compatible = "100ask_imx6ull,user_keys",
	},
};
/* platform_driver结构体定义 */
static struct platform_driver user_key_platform_driver = {
    .probe = user_key_probe,
    .remove = user_key_remove,
    .driver = {
        .name = "user_key",
		.of_match_table = user_key_device_id,
    },
};

/* 描述设备树中GPIO相关信息的结构体 */
struct user_key_info {
	int gpio_num;
	int irq_num;
	enum of_gpio_flags flag;

	int gpio_value;
};
struct user_key_info *user_key_desp = NULL;

/* 描述设备驱动相关信息的结构体 */
struct user_key_drv_info {
	int key_num;                    /* 按键个数 */
	int major;                      /* 主设备号 */
	struct cdev char_dev;           /* 字符设备结构体 */
	struct file_operations fileops; /* 文件操作结构体 */
	struct class *class_ptr;        /* 类指针 */
};
static struct user_key_drv_info user_key_drv = {
	.key_num = 0,
	.major = 0,
	.char_dev = {
		.owner = THIS_MODULE,
	},
	.fileops = {
		.owner   = THIS_MODULE,
		.open    = user_key_open,
		.release = user_key_close,
		.read    = user_key_read,
		.write   = user_key_write,
	}
};
/* 定义一个tasklet结构体 */
DECLARE_TASKLET(user_key_tasklet, tasklet_func, 0);

/*********************************************************************************************
 * 函数定义
 ********************************************************************************************/
/* 用户按键的中断处理函数 */
irqreturn_t user_key_isr(int irq, void *dev_id)
{
	struct user_key_info *pinfo = (struct user_key_info *)dev_id;
	int value = 0;

	/* 获取GPIO电平 */
	value = gpio_get_value(pinfo->gpio_num);
	pinfo->gpio_value = value;

	/* 激活tasklet下半部的执行 */
	tasklet_schedule(&user_key_tasklet);

	return IRQ_HANDLED;
}
/* 中断下半部处理函数，tasklet实现 */
void tasklet_func(unsigned long data)
{
	INF("user key bottom half\n");
}

/* probe函数定义，成功匹配到设备树中的设备节点时内核调用此函数 */
int user_key_probe(struct platform_device *pdev)
{
    int ret = 0;
	int idx = 0;
	int count = 0;
	dev_t dev_num = 0;

	/* 1.获取设备树中对按键设备的相关信息 */
	DBG("get gpio count\r\n");
	count = of_gpio_count(pdev->dev.of_node);
	if (count <= 0) {
		ERR("gpio count <= 0!\r\n");
		goto gpio_failed;
	}
	user_key_drv.key_num = count;

	/* 2.申请内存，保存设备树中的GPIO相关信息 */
	DBG("get memory\r\n");
	user_key_desp = NULL;
	user_key_desp = kzalloc(sizeof(struct user_key_info) * count, GFP_KERNEL);
	if (!user_key_desp){
		ERR("kzalloc failed!\r\n");
		goto memory_error;
	}

	/* 3.获取设备树中GPIO相关信息 */
	for (idx = 0; idx < count; idx++) {
		/* 获取GPIO编号 */
		user_key_desp[idx].gpio_num = of_get_gpio_flags(pdev->dev.of_node, idx, &user_key_desp[idx].flag);

		/* GPIO编号转换为中断号 */
		user_key_desp[idx].irq_num  = gpio_to_irq(user_key_desp[idx].gpio_num);

		/* 注册中断处理函数 */
		DBG("register interrupt!\r\n");
		ret = request_irq(user_key_desp[idx].irq_num, user_key_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "user_keys", (void *)&user_key_desp[idx]);
		if (ret) {
			ERR("request irq(%d) failed\r\n", user_key_desp[idx].irq_num);
			goto irq_error;
		}
	}

	/* 申请设备号 */
	if (user_key_drv.major) {    /* 指定设备号 */
		dev_num = MKDEV(user_key_drv.major, 0);    /* 合成设备号，注册进内核 */
		ret = register_chrdev_region(dev_num, 1, USER_KEY_NAME);
	} else {    /* 未指定设备号，由系统分配一个闲置的设备号 */
		ret = alloc_chrdev_region(&dev_num, 0, 1, USER_KEY_NAME);\
		user_key_drv.major = MAJOR(dev_num);    /* 分离出主设备号 */
		INF("major = %d, minor = %d\n", MAJOR(dev_num), MINOR(dev_num));
	}
	if (ret < 0) {
		ERR("Device number alloc failed!\n");
		goto dev_num_alloc_err;
	} else {
		DBG("Device number alloc successed!\n ");
	}

	/* 创建字符设备 */
	cdev_init(&user_key_drv.char_dev, &user_key_drv.fileops);
	cdev_add(&user_key_drv.char_dev, dev_num, 1);
	INF("cdev add finish!\n");

	/* 创建驱动节点文件 */
	user_key_drv.class_ptr =  class_create(THIS_MODULE, USER_KEY_NAME);
	if (IS_ERR(user_key_drv.class_ptr)) {
		ERR("create device class failed!\n");
		goto create_class_err; 
	} else {
		INF("class create finish!\n");
	}
	device_create(user_key_drv.class_ptr, NULL, MKDEV(user_key_drv.major, 0), NULL, USER_KEY_NAME);

	return 0;

create_class_err:
	cdev_del(&user_key_drv.char_dev);
	unregister_chrdev_region(MKDEV(user_key_drv.major, 0), 1);
dev_num_alloc_err:
	for (idx = 0; idx < user_key_drv.key_num; idx++) {
		free_irq(user_key_desp[idx].irq_num, (void *)&user_key_desp[idx]);
	}
irq_error:
memory_error:
gpio_failed:
	return -1;
}
int user_key_remove(struct platform_device *pdev)
{
    int ret = 0;
	int count = 0;
	int idx = 0;

	device_destroy(user_key_drv.class_ptr, MKDEV(user_key_drv.major, 0));
	class_destroy(user_key_drv.class_ptr);
	cdev_del(&user_key_drv.char_dev);
	unregister_chrdev_region(MKDEV(user_key_drv.major, 0), 1);

	count = user_key_drv.key_num;
	for (idx = 0; idx < count; idx++)
	{
		free_irq(user_key_desp[idx].irq_num, (void *)&user_key_desp[idx]);
	}
	kfree(user_key_desp);
    return ret;
}


int user_key_open(struct inode *pinode, struct file *pfile)
{
	return 0;
}
int user_key_close(struct inode *pinode, struct file *pfile)
{
	return 0;
}
ssize_t user_key_read(struct file *pfile, char __user *buf, size_t len, loff_t *offset)
{
#if 0
	/* 获取次设备号 */
	struct inode *inode_ptr = file_inode(pfile);
	int minor = iminor(inode_ptr);
#endif
	int idx = 0;
	char value = 0;
	int count = 0;
	
	count = user_key_drv.key_num;
	for (idx = 0; idx < count; idx++) {
		user_key_desp[idx].gpio_value ? (value |= (0x1 << idx)) : (value &= ~(0x1 << idx));
	}
#if 0
	len = copy_to_user(buf, &value, 4);
#else
	put_user(value, buf);
#endif

	return 4;
}
ssize_t user_key_write(struct file *pfile, const char __user *buf, size_t len, loff_t *offset)
{
	return 0;
}

/* 驱动程序的入口函数 */
static int __init key_drv_init(void)
{
	int ret = 0;

    DBG("platform driver start register!\r\n");
	ret = platform_driver_register(&user_key_platform_driver);
	if (ret) {
		ERR("platform driver register failed!\r\n");
	}

	return ret;
}

/* 驱动程序的出口函数 */
static void __exit key_drv_exit(void)
{
	DBG("platform driver start unregister!\r\n");
    platform_driver_unregister(&user_key_platform_driver);
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");

