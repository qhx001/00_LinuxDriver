
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define REG_CCM_CCGR1    0x020C406C
#define REG_GPIO5_GDIR   0x020AC004
#define REG_GPIO5_DR     0x020AC000
#define REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3    0x02290014
#define REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3    0x02290058
#define DEVICE_NAME    "user_led"
#define CLASS_NAME     "user_led_class"

static int led_probe(struct platform_device *pdev);
static int led_remove(struct platform_device *pdev);
static int led_open(struct inode *node, struct file *file);
static int led_close(struct inode *node, struct file *file);
static ssize_t led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
static ssize_t led_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);


static struct of_device_id	led_match_table[] = {
    {.compatible = "qhx_imx6ull, user_led",},
};

static struct platform_driver led_platform_drv = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "user_led",
        .of_match_table = led_match_table,
    },
};

static int major = 0;
static struct cdev led_cdev;
static struct file_operations led_oprs = {
    .owner   = THIS_MODULE,
    .open    = led_open,
    .release = led_close,
    .write   = led_write,
    .read    = led_read,
    };
static struct class *led_class = NULL;
static struct device *led_device = NULL;
static int led_pin = 0;
static volatile unsigned int *CCM_CCGR1 = NULL;
static volatile unsigned int *GPIO5_GDIR = NULL;
static volatile unsigned int *GPIO5_DR = NULL;
static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = NULL;
static volatile unsigned int *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = NULL;
/* 设备与驱动匹配成功后，就会调用probe函数 */
static int led_probe(struct platform_device *pdev)
{
	/* 1. 获取设备信息 */
	struct device_node *np = NULL;
	int pin_num = 0;
	int err = 0;
	int devid = 0;

	np = pdev->dev.of_node;
	/* 判断设备信息是否来自设备树 */
	if (!np)
	{   /* 设备信息不是来自设备树 */
		return -1;
	}
	/* 设备信息来自设备树 */
	/* 获取pin属性信息 */
	err = of_property_read_u32(np, "pin", &pin_num);
	if (err != 0)
	{
		printk("%s %s line:%d\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	printk ("pin_num = %x\n", pin_num);
	/* 记录设备的属性信息 */
	led_pin = pin_num;
	
	
	/* 2.注册驱动 */
	/* 2.1.申请设备号 */
	if (major)
	{   /* 申请制定好的设备号 */
		err = register_chrdev_region(MKDEV(major, 0), 1, DEVICE_NAME);
	}
	else
	{   /* 内核分配一个闲置设备号 */
		err = alloc_chrdev_region(&devid, 0, 1, DEVICE_NAME);
		major = MAJOR(devid);
	}

	if (err < 0)
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		goto err_devid;
	}
	printk("major = %d\n", major);

	/* 初始化cdev结构体，将cdev结构体注册进内核 */
	led_cdev.owner = THIS_MODULE;
	cdev_init(&led_cdev, &led_oprs);
	err = cdev_add(&led_cdev, MKDEV(major, 0), 1);
	if (err != 0)
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		goto err_cdev;
	}

	/* 自动创建设备节点 */
	led_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(led_class))
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		goto err_class;
	}

	led_device = device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(led_device))
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		goto err_dev_create;
	}
	return 0;
err_dev_create:
	class_destroy(led_class);
err_class:
	cdev_del(&led_cdev);
err_cdev:
err_devid:
	return -1;
}

/* 驱动卸载后，就会调用remove函数 */
static int led_remove(struct platform_device *pdev)
{
	/* 摧毁设备节点 */
	device_destroy(led_class, MKDEV(major, 0));
	/* 删除class */
	class_destroy(led_class);
	/* 删除cdev */
	cdev_del(&led_cdev);
	/* 注销设备号 */
	unregister_chrdev_region(MKDEV(major, 0), 1);

	return 0;
}

/* open函数定义 */
int led_open(struct inode *node, struct file *file)
{
	/* 映射地址空间 */
    CCM_CCGR1                        = ioremap(REG_CCM_CCGR1, 4);
    GPIO5_GDIR                       = ioremap(REG_GPIO5_GDIR, 4);
    GPIO5_DR                         = ioremap(REG_GPIO5_DR, 4);
	IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3, 4);
    IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3, 4);
    
	/* GPIO管脚初始化 */
    *CCM_CCGR1 &= ~(0x3 << 30);
    *CCM_CCGR1 |=  (0x3 << 30);

    *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 &= ~(0xF << 0);
    *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 |=  (0x5 << 0);

    *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 &= ~(0xFFFF << 0);
    *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 |=  (0x10B0 << 0);

    *GPIO5_GDIR &= ~(0x1 << 3);
    *GPIO5_GDIR |=  (0x1 << 3);

    *GPIO5_DR &= ~(0x1 << 3);
    *GPIO5_DR |=  (0x1 << 3);

    return 0;

}
/* close函数定义 */
int led_close(struct inode *node, struct file *file)
{
	/* 取消映射地址空间 */
    iounmap(CCM_CCGR1);
	iounmap(GPIO5_GDIR);
	iounmap(GPIO5_DR);
    iounmap(IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03);
    iounmap(IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03);

    return 0;
}

/* read函数定义 */
ssize_t led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    return 0;
}
/* write函数定义 */
ssize_t led_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    char status = 0;
    int len = 0;

	/* 拷贝来自应用层的数据 */
    len = copy_from_user(&status, user_buf, 1);

	/* 控制管脚输出电平 */
    status ? (*GPIO5_DR |= (0x1 << 3)) : (*GPIO5_DR &= ~(0x1 << 3));

    return 1;
}

/* 入口函数 */
static int __init led_drv_init(void)
{
	int err = 0;

    err = platform_driver_register(&led_platform_drv);
	if (err != 0)
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		goto err_platform;
	}

    return 0;
err_platform:
	return -1;
}

/* 出口函数 */
static void __exit led_drv_exit(void)
{
    platform_driver_unregister(&led_platform_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
