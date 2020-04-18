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

#define MIN(a, b)    (((a) < (b)) ? (a) : (b))

static int hello_drv_open(struct inode *node, struct file *file);
static int hello_drv_close(struct inode *node, struct file *file);
static ssize_t hello_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset);
static ssize_t hello_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset);

/* 1.确定主设备号
 * 0:表示由系统分配一个闲置的主设备号
 */
static int major = 0;

/*
 * 2.定义file_operations结构体
 */
static struct file_operations hello_drv = {
    .owner   = THIS_MODULE,
    .open    = hello_drv_open,
    .read    = hello_drv_read,
    .write   = hello_drv_write,
    .release = hello_drv_close,
};
static char kernel_buff[1024] = {0, };
static struct class *hello_class = NULL;

/*
 *3.实现hello_drv_open,read,write,close函数
 */
static ssize_t hello_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    copy_to_user(buf, kernel_buff, MIN(1024, size));
    return MIN(1024, size);
}

static ssize_t hello_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    copy_from_user(kernel_buff, buf, MIN(1024, size));
    return MIN(1024, size);
}

static int hello_drv_open(struct inode *node, struct file *file)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}

static int hello_drv_close(struct inode *node, struct file *file)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}

/*
 *4.把file_operation结构体告诉内核：注册驱动程序
 *5.驱动的入口函数来注册驱动程序
 */
static int __init hello_drv_init(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    major = register_chrdev(0, "hello", &hello_drv);
    hello_class = class_create(THIS_MODULE, "hello_class");
    if (IS_ERR(hello_class))
    {
        printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        unregister_chrdev(major, "hello");
        return -1;
    }
    device_create(hello_class, NULL, MKDEV(major, 0), NULL, "hello");    /* /dev/hello */
    
    return 0;
}
/*
 *6.卸载驱动程序
 *7.出口函数来卸载驱动程序
 */
static void __exit hello_drv_exit(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    device_destroy(hello_class, MKDEV(major, 0));
    class_destroy(hello_class);
    unregister_chrdev(major, "hello");
}


module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");

