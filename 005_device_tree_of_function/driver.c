#include <linux/module.h>
#include <linux/fs.h>
#include <linux/of.h>


#if 0
device tree information.
backlight {
    compatible = "pwm-backlight";
    pwms = <&pwm1 0 1000>;
    brightness-levels = <0 1 2 3 4 5 6 8 10>;
    default-brightness-level = <8>;
    status = "okay";
};

Run result.
[root@imx6ull:/mnt]# insmod driver.ko 
[  328.734809] driver: loading out-of-tree module taints kernel.
[  328.743774] compatile = pwm-backlight[14 14]
[  328.754579] status = okay
[  328.757337] default-brightness-level = 8
[  328.761368] elem_size = 9
[  328.764094] elem_value = 0
[  328.769176] elem_value = 1
[  328.772009] elem_value = 2
[  328.775983] elem_value = 3
[  328.778817] elem_value = 4
[  328.781632] elem_value = 5
[  328.785686] elem_value = 6
[  328.788514] elem_value = 8
[  328.791330] elem_value = 10
[  328.794229] driver_init success!
[  328.798994] #address-cell = 1, #size-cell = 1

#endif

static int __init driver_init(void)
{
    int ret = 0;
    int i = 0;
    struct device_node *backlight_node_ptr = NULL;
    struct property *compatible_ptr = NULL;
    const char *status_ptr = NULL;
    unsigned int level = 0;
    unsigned int len = 0;
    unsigned int addr_cell = 0;
    unsigned int size_cell = 0;
    int elem_size = 0;
    int elem_value = 0;
    

    /* 1. Get device node. */
    #if 1
    /*
     * Get device node info by path. 
     * static inline struct device_node *of_find_node_by_path(const char *path)
     */
    backlight_node_ptr = of_find_node_by_path("/backlight");
    #else
    /*
     * Get device node info by node name.
     * struct device_node *of_find_node_by_name(struct device_node *from, const char *name)
     */
    backlight_node_ptr = of_find_node_by_name(NULL, "backlight");
    #endif
    if (!backlight_node_ptr) {
        ret = -1;
        goto err_find_node;
    }

    /* 2.Get Property info. */
    /* struct property *of_find_property(const struct device_node *np,
					                     const char *name,
					                     int *lenp);
    */
    compatible_ptr = of_find_property(backlight_node_ptr, "compatible", &len);
    if (!compatible_ptr) {
        ret = -1;
        goto err_find_property;
    }
    printk("compatile = %s[%d %d]\r\n", (char *)compatible_ptr->value, compatible_ptr->length, len);

    /* extern int of_property_read_string(const struct device_node *np,
				                          const char *propname,
				                          const char **out_string);
    */
    ret = of_property_read_string(backlight_node_ptr, "status", &status_ptr);
    if (ret) {
        ret = -1;
        goto err_read_string;
    }
    printk("status = %s\r\n", status_ptr);

    /* static inline int of_property_read_u32(const struct device_node *np,
				                              const char *propname,
				                              u32 *out_value) 
    */
    ret = of_property_read_u32(backlight_node_ptr, "default-brightness-level", &level);
    if (ret) {
        ret = -1;
        goto err_read_u32;
    }
    printk("default-brightness-level = %d\r\n", level);


    /* int of_property_count_elems_of_size(const struct device_node *np,
				                           const char *propname, int elem_size);
       int of_property_read_u32_index(const struct device_node *np,
				                      const char *propname,
				                      u32 index,
                                      u32 *out_value);
    */
    ret = of_property_count_elems_of_size(backlight_node_ptr, "brightness-levels", sizeof(u32));
    if (ret <= 0) {
        ret = -1;
        goto err_count_elems;
    }
    elem_size = ret;
    printk("elem_size = %d\r\n", elem_size);
    for (i = 0; i < elem_size; i++) {
        ret = of_property_read_u32_index(backlight_node_ptr,
                                        "brightness-levels",
                                         i, &elem_value);
        if (ret) {
            ret = -1;
            goto err_read_u32_index;
        }
        printk("elem_value = %d\r\n", elem_value);
    }

    printk("%s success!\r\n", __FUNCTION__);

    addr_cell = of_n_addr_cells(backlight_node_ptr);
    size_cell = of_n_size_cells(backlight_node_ptr);
    printk("#address-cell = %d, #size-cell = %d\n", addr_cell, size_cell);

    return ret;

err_read_u32_index:
    printk("read u32 index failed!\r\n");
    return ret;
err_count_elems:
    printk("count elems failed!\r\n");
    return ret;
err_read_u32:
    printk("read u32 failed!\r\n");
    return ret;
err_read_string:
    printk("read string failed!\r\n");
    return ret;
err_find_property:
    printk("find property failed!\r\n");
    return ret;
err_find_node:
    printk("find node failed!\r\n");
    return ret;
}

static void __exit driver_exit(void)
{
    printk("%s success!\r\n", __FUNCTION__);
}

module_init(driver_init);
module_exit(driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("QHX");
