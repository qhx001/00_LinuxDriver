#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#define GROUP(x) (x>>16)
#define PIN(x)   (x&0xFFFF)
#define GROUP_PIN(g,p) ((g<<16) | (p))

#define LED_DEVICE_NAME    "100ask_led"

static struct resource led_resource[] = {
    [0] = {
        .start = GROUP_PIN(5, 3),
        .flags = IORESOURCE_IRQ,
        },
};

static struct platform_device led_device = {
    .name = LED_DEVICE_NAME,
    .resource = led_resource,
    .num_resources = ARRAY_SIZE(led_resource),
};

static int __init led_device_init(void)
{
    int err = 0;

    err = platform_device_register(&led_device);
    if (err < 0)
    {
        return -1;
    }
    printk("%s %s line:%d", __FILE__, __FUNCTION__, __LINE__);

    return 0;
}

static void __exit led_device_exit(void)
{
    platform_device_unregister(&led_device);
    printk("%s %s line:%d", __FILE__, __FUNCTION__, __LINE__);
}

module_init(led_device_init);
module_exit(led_device_exit);
MODULE_LICENSE("GPL");


