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



#define DEVICE_NAME    "100ask_led"
#define CLASS_NAME     "100ask_led_class"

#define REG_CCM_CCGR1    0x020C406C
#define REG_GPIO5_GDIR   0x020AC004
#define REG_GPIO5_DR     0x020AC000
#define REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3    0x02290014
#define REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3    0x02290058



static int led_drv_open(struct inode *node, struct file *file);
static int led_drv_close(struct inode *node, struct file *file);
static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset);
static ssize_t led_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset);

/* 1.确定主设备号
 * 0:表示由系统分配一个闲置的主设备号
 * 设备号：Linux中的每一个设备都有一个设备号，设备号由高12位的主设备号和低20位的次设备号组成；
 * 主设备号表示某一个具体的驱动，比如是led，iic，spi等
 * 次设备号表示这个驱动的具体设备，比如iic0，iic2等
 */

static int major = 0;
static volatile unsigned int *CCM_CCGR1 = NULL;
static volatile unsigned int *GPIO5_GDIR = NULL;
static volatile unsigned int *GPIO5_DR = NULL;
static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = NULL;
static volatile unsigned int *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = NULL;

/*
 * 2.定义file_operations结构体
 */
static struct file_operations led_drv = {
    .owner   = THIS_MODULE,
    .open    = led_drv_open,
    .read    = led_drv_read,
    .write   = led_drv_write,
    .release = led_drv_close,
};

static struct class *led_class = NULL;

/*
 *3.实现led_drv_open,read,write,close函数
 */
static ssize_t led_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    char status = 0;
    int len = 0;

    /* 根据次设备号和status控制LED */

    len = copy_from_user(&status, buf, 1);
    status ? (*GPIO5_DR |= (0x1 << 3)) : (*GPIO5_DR &= ~(0x1 << 3));

    return 1;
}

static int led_drv_open(struct inode *node, struct file *file)
{
    if (!CCM_CCGR1)
    {
        /* 将物理地址映射为虚拟地址 */
        CCM_CCGR1 = ioremap(REG_CCM_CCGR1, 4);
        GPIO5_GDIR = ioremap(REG_GPIO5_GDIR, 4);
        IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3, 4);
        IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3, 4);
        GPIO5_DR = ioremap(REG_GPIO5_DR, 4);
        printk("map finish!\n");
    }

    /* 使能GPIO1时钟 */
    *CCM_CCGR1 &= ~(0x3 << 30);
    *CCM_CCGR1 |=  (0x3 << 30);

    /* 将GPIO5_IO3设置为GPIO功能 */
    *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 &= ~(0xF << 0);
    *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 |=  (0x5 << 0);

    /* 配置GPIO1_IO3功能 */
    *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 &= ~(0xFFFF << 0);
    *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 |=  (0x10B0 << 0);

    /* 设置GPIO1_IO3的管脚方向为输出 */
    *GPIO5_GDIR &= ~(0x1 << 3);
    *GPIO5_GDIR |=  (0x1 << 3);

    /* 设置GPIO1_IO3管脚输出高电平 */
    *GPIO5_DR &= ~(0x1 << 3);
    *GPIO5_DR |=  (0x1 << 3);
    printk("gpio init finish!\n");

    return 0;
}

static int led_drv_close(struct inode *node, struct file *file)
{
    return 0;
}

/*
 *4.把file_operation结构体告诉内核：注册驱动程序
 *5.驱动的入口函数来注册驱动程序
 */
static int __init led_drv_init(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    /* 注册设备 */
    major = register_chrdev(0, DEVICE_NAME, &led_drv);
    led_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(led_class))
    {
        printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, DEVICE_NAME);
        return -1;
    }
    device_create(led_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    return 0;
}
/*
 *6.卸载驱动程序
 *7.出口函数来卸载驱动程序
 */
static void __exit led_drv_exit(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    /* 取消映射 */
    iounmap(CCM_CCGR1);
    iounmap(IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03);
    iounmap(IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03);
    iounmap(GPIO5_GDIR);
    iounmap(GPIO5_DR);

    /* 注销设备 */
    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    unregister_chrdev(major, DEVICE_NAME);
}


module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");

