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

/* 1.ȷ�����豸��
 * 0:��ʾ��ϵͳ����һ�����õ����豸��
 * �豸�ţ�Linux�е�ÿһ���豸����һ���豸�ţ��豸���ɸ�12λ�����豸�ź͵�20λ�Ĵ��豸����ɣ�
 * ���豸�ű�ʾĳһ�������������������led��iic��spi��
 * ���豸�ű�ʾ��������ľ����豸������iic0��iic2��
 */

static int major = 0;
static volatile unsigned int *CCM_CCGR1 = NULL;
static volatile unsigned int *GPIO5_GDIR = NULL;
static volatile unsigned int *GPIO5_DR = NULL;
static volatile unsigned int *IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = NULL;
static volatile unsigned int *IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = NULL;

/*
 * 2.����file_operations�ṹ��
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
 *3.ʵ��led_drv_open,read,write,close����
 */
static ssize_t led_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
    char status = 0;
    int len = 0;

    /* ���ݴ��豸�ź�status����LED */

    len = copy_from_user(&status, buf, 1);
    status ? (*GPIO5_DR |= (0x1 << 3)) : (*GPIO5_DR &= ~(0x1 << 3));

    return 1;
}

static int led_drv_open(struct inode *node, struct file *file)
{
    if (!CCM_CCGR1)
    {
        /* �������ַӳ��Ϊ�����ַ */
        CCM_CCGR1 = ioremap(REG_CCM_CCGR1, 4);
        GPIO5_GDIR = ioremap(REG_GPIO5_GDIR, 4);
        IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_MUX_CTL_PAD_SNVS_TAMPER3, 4);
        IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03 = ioremap(REG_IOMUXC_SNVS_SW_PAD_CTL_PAD_SNVS_TAMPER3, 4);
        GPIO5_DR = ioremap(REG_GPIO5_DR, 4);
        printk("map finish!\n");
    }

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
    printk("gpio init finish!\n");

    return 0;
}

static int led_drv_close(struct inode *node, struct file *file)
{
    return 0;
}

/*
 *4.��file_operation�ṹ������ںˣ�ע����������
 *5.��������ں�����ע����������
 */
static int __init led_drv_init(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    /* ע���豸 */
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
 *6.ж����������
 *7.���ں�����ж����������
 */
static void __exit led_drv_exit(void)
{
    printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

    /* ȡ��ӳ�� */
    iounmap(CCM_CCGR1);
    iounmap(IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03);
    iounmap(IOMUXC_SW_PAD_CTL_PAD_GPIO5_IO03);
    iounmap(GPIO5_GDIR);
    iounmap(GPIO5_DR);

    /* ע���豸 */
    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    unregister_chrdev(major, DEVICE_NAME);
}


module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");

