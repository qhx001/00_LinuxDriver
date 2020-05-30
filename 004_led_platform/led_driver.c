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

#define REG_CCM_CCGR1    0x020C406C
#define REG_GPIO5_GDIR   0x020AC004
#define REG_GPIO5_DR     0x020AC000
#define REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3    0x02290014
#define REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3    0x02290058


#define LED_DEVICE_NAME    "100ask_led"
#define LED_CLASS_NAME     "100ask_led_class"


static int led_probe(struct platform_device *dev);
static int led_remove(struct platform_device *dev);
static int led_open(struct inode *node, struct file *file);
static int led_close(struct inode *node, struct file *file);
static ssize_t led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
static ssize_t led_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);


static int major = 0;
static int minor = 0;
static int gpio_pin[8] = {0, };
static int resource_total = 0;

static volatile unsigned int *CCM_CCGR1 = NULL;
static volatile unsigned int *GPIO5_GDIR = NULL;
static volatile unsigned int *GPIO5_DR = NULL;
static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = NULL;
static volatile unsigned int *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = NULL;


static struct class *led_class = NULL;
/* ����platform_driver�ṹ�� */
static struct platform_driver led_driver = {
    .probe = led_probe,                    /* ƥ��ɹ������led_probe����                   */
    .remove = led_remove,                  /* ע���豸���������led_remove����                */
    .driver = {
        .name = LED_DEVICE_NAME,           /* �豸������������֣�������deviceƥ�� */
        },
};

static struct file_operations led_file_opr = {
    .owner   = THIS_MODULE,
    .open    = led_open,
    .release = led_close,
    .write   = led_write,
    .read    = led_read,
    };

static struct cdev led_cdev = {
    .owner = THIS_MODULE,
    };

/* ����led_probe����
 * ���ݴ����platform_device�ṹ������ȡ�豸��Ϣ
 * �����豸��Ϣ��ע���豸�����������豸�ڵ�
 */
static int led_probe(struct platform_device *dev)
{
    int ret = 0;
    int i = 0;
    int resource_index = 0;
    struct resource *p_resource = NULL;

    /* 1.ȷ����Դ������������Դ����ȷ��Ҫ������豸�Ÿ��� */
    while (1)
    {
        p_resource = platform_get_resource(dev, IORESOURCE_IRQ, resource_index);
        if (!p_resource)
        {
            break;
        }
        gpio_pin[resource_index] = p_resource->start;
        resource_index++;
    }
    resource_total = resource_index;

    /* 2.�����豸�� */
    if (major != 0)
    {   /* ʹ��Ԥ�ȶ�����豸�� */
        ret = register_chrdev_region(MKDEV(major, 0), resource_total, LED_DEVICE_NAME);
        if (ret < 0)
        {
            printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }
    else
    {   /* ��ϵͳ�Զ������豸�� */
        int dev_num = 0;
        ret = alloc_chrdev_region(&dev_num, 0, resource_total, LED_DEVICE_NAME);
        if (ret < 0)
        {
            printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        major = MAJOR(dev_num);
        minor = MINOR(dev_num);
    }

    /* 3.��ʼ�������cdev�ṹ��. */
    cdev_init(&led_cdev, &led_file_opr);
    cdev_add(&led_cdev, MKDEV(major, 0), resource_total);

    /* 4.�Զ������豸�ڵ� */
    led_class = class_create(THIS_MODULE, LED_CLASS_NAME);
    if (!led_class)
    {
        cdev_del(&led_cdev);
        unregister_chrdev_region(MKDEV(major, 0), resource_total);
        printk("%s %s line:%d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    for (i = 0; i < resource_total; i++)
    {
        device_create(led_class, NULL, MKDEV(major, i), NULL, LED_DEVICE_NAME"%d", i);
    }

    return 0;
}
/* ����led_remove����
 * ע���豸����
 */
static int led_remove(struct platform_device *dev)
{
    int i = 0;
    for (i = 0; i < resource_total; i++)
    {
        device_destroy(led_class, MKDEV(major, i));
    }
    class_destroy(led_class);
    cdev_del(&led_cdev);
    unregister_chrdev_region(MKDEV(major, 0), resource_total);

    return 0;
}

int led_open(struct inode *node, struct file *file)
{
#if 0
    int which = iminor(node);
#endif

    /* �������ַӳ��Ϊ�����ַ */
    CCM_CCGR1 = ioremap(REG_CCM_CCGR1, 4);
    GPIO5_GDIR = ioremap(REG_GPIO5_GDIR, 4);
    IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3, 4);
    IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3, 4);
    GPIO5_DR = ioremap(REG_GPIO5_DR, 4);
    printk("map finish!\n");
    printk("CCM_CCGR1 = 0x%X\n", CCM_CCGR1);

    /* ʹ��GPIO1ʱ�� */
    *CCM_CCGR1 &= ~(0x3 << 30);
    *CCM_CCGR1 |=  (0x3 << 30);

    /* ��GPIO5_IO3����ΪGPIO���� */
    *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 &= ~(0xF << 0);
    *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 |=  (0x5 << 0);

    /* ����GPIO1_IO3���� */
    *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 &= ~(0xFFFF << 0);
    *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 |=  (0x10B0 << 0);

    /* ����GPIO1_IO3�Ĺܽŷ���Ϊ��� */
    *GPIO5_GDIR &= ~(0x1 << 3);
    *GPIO5_GDIR |=  (0x1 << 3);

    /* ����GPIO1_IO3�ܽ�����ߵ�ƽ */
    *GPIO5_DR &= ~(0x1 << 3);
    *GPIO5_DR |=  (0x1 << 3);
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    return 0;

}
int led_close(struct inode *node, struct file *file)
{
    /* ȡ��ӳ�� */
    iounmap(CCM_CCGR1);
    iounmap(IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03);
    iounmap(IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03);
    iounmap(GPIO5_GDIR);
    iounmap(GPIO5_DR);
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    printk("CCM_CCGR1 = 0x%X\n", CCM_CCGR1);

    return 0;
}
ssize_t led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    return 0;
}
ssize_t led_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    char status = 0;
    int len = 0;
    #if 0
    struct inode *inode = file_inode(file);
    int which = iminor(inode);
    #endif
    /* ���ݴ��豸�ź�status����LED */

    len = copy_from_user(&status, user_buf, 1);
    status ? (*GPIO5_DR |= (0x1 << 3)) : (*GPIO5_DR &= ~(0x1 << 3));

    return 1;
}

/* ��ں���
 * ��platform����ע��platform_drvier�ṹ��
 */
static int __init led_driver_init(void)
{
    int err = 0;

    err = platform_driver_register(&led_driver);
    if (err < 0)
    {
        printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

/* ���ں���
 * ��platform������ɾ��platform_driver�ṹ��
 */
static void __exit led_driver_exit(void)
{
    platform_driver_unregister(&led_driver);
}


module_init(led_driver_init);
module_exit(led_driver_exit);
MODULE_LICENSE("GPL");


