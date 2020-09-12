#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>

#define LED_CHR_DEV_NAME    "100ask_led"
#define LED_CLASS_NAME      "100ask_led_class"

static int led_drv_probe(struct platform_device *pdev);
static int led_drv_remove(struct platform_device *pdev);
static int led_open(struct inode *node, struct file *file);
static int led_close(struct inode *node, struct file *file);
static ssize_t led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
static ssize_t led_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);

static int major = 0;
static int minor = 0;
static struct gpio_desc* led_desc = NULL;
static struct class *led_class = NULL;
static struct cdev led_cdev;
static struct file_operations led_opr = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_close,
    .write = led_write,
    .read = led_read,
};
static const struct of_device_id leddev_id[] = {
    {.compatible = "100ask_imx6ull,led"},
};

static struct platform_driver led_platform_drv = {
    .probe = led_drv_probe,
    .remove = led_drv_remove,
    .driver = {
        .name = "user_led",
        .of_match_table = leddev_id,
    },
};


int led_open(struct inode *node, struct file *file)
{
    /* 设置GPIO的方向为输出 */
    gpiod_direction_output(led_desc, 0);
    return 0;
}
/* close函数定义 */
int led_close(struct inode *node, struct file *file)
{
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
    int err = 0;
    int status = 0;

    err = copy_from_user(&status, user_buf, 1);

    /* 设置GPIO输出逻辑电平状态 */
    gpiod_set_value(led_desc, status);
    return 1;
}

static int led_drv_probe(struct platform_device *pdev)
{
    int err = 0;
    dev_t dev_num = 0;

    printk("%s\n", __FUNCTION__);
    printk("pdev->name = %s\n", pdev->name);
    /* 获取led-gpios的GPIO相关描述 */
    led_desc = gpiod_get(&pdev->dev, "led", GPIOD_ASIS);
    if (IS_ERR(led_desc)) {
        printk("%s : gpio get failed\n", __FUNCTION__);
        return -1;
    }

    if (major) {
        /* 设备号已指定 */
        err = register_chrdev_region(MKDEV(major, minor), 1, LED_CHR_DEV_NAME);
    }
    else
    {   /* 动态分配设备号 */
        err =  alloc_chrdev_region(&dev_num, 0, 1, LED_CHR_DEV_NAME);
        major = MAJOR(dev_num);
        minor = MINOR(dev_num);
    }
    
    if (err) {
        printk("%s : device number get failed!\n", __FUNCTION__);
        return -1;
    }

    /* void cdev_init(struct cdev *cdev, const struct file_operations *fops) */
    /* int cdev_add(struct cdev *p, dev_t dev, unsigned count) */
    led_cdev.owner = THIS_MODULE;
    cdev_init(&led_cdev, &led_opr);
    err = cdev_add(&led_cdev, MKDEV(major, minor), 1);
    if (err) {
        printk("%s : cdev add failed!\n", __FUNCTION__);
        return -1;
    }
    
    led_class = class_create(THIS_MODULE, LED_CLASS_NAME);
    if (!led_class)
    {
        cdev_del(&led_cdev);
        unregister_chrdev_region(MKDEV(major, minor), 1);
        printk("%s %s line:%d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    device_create(led_class, NULL, MKDEV(major,minor), NULL, LED_CHR_DEV_NAME"%d", 0);

    return 0;
}

static int led_drv_remove(struct platform_device *pdev)
{
	/* 摧毁设备节点 */
	device_destroy(led_class, MKDEV(major, 0));
	/* 删除class */
	class_destroy(led_class);
	/* 删除cdev */
	cdev_del(&led_cdev);
	/* 注销设备号 */
	unregister_chrdev_region(MKDEV(major, 0), 1);
    gpiod_put(led_desc);

    return 0;
}

static int __init led_drv_init(void)
{
	int err;
    printk("%s1\n", __FUNCTION__);
    
    err = platform_driver_register(&led_platform_drv);
	if (err)
		printk("%s: failed to add driver\n", __FUNCTION__);
    printk("%s2\n", __FUNCTION__);
	return err;
}

static void __exit led_drv_exit(void)
{
    platform_driver_unregister(&led_platform_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
