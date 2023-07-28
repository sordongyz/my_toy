#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

static uint32_t on_time_ms  = 500;
static uint32_t off_time_ms = 500;
static void __iomem *gpiomem_base=NULL;
static uint32_t total_cnt = 10;
module_param(total_cnt,uint,0644);
module_param_named(on,on_time_ms,uint,0644);
module_param_named(off,off_time_ms,uint,0644);

static struct workqueue_struct *toy_wq = NULL;
static struct work_struct toy_work;
//static struct timer_list toy_timer;



#define LED_6G_GREEN           0x15

static void led_out(uint32_t val)
{
	writel(val,(gpiomem_base + 4 + (LED_6G_GREEN * 0x1000)));
}

static void blink_led(void)
{
	uint32_t cnt = 0;

	while (cnt++ < total_cnt)
	{
		led_out(0x2);
		msleep(on_time_ms);
		led_out(0x0);
		msleep(off_time_ms);
	}
	pr_info("blink led completed\n");

}


/*
 * This module shows how to create a simple subdirectory in sysfs called
 * /sys/kernel/kobject-example  In that directory, 3 files are created:
 * "foo", "baz", and "bar".  If an integer is written to these files, it can be
 * later read out of it.
 */

static int foo;
static int baz;
static int bar;

/*
 * The "foo" file where a static variable is read from and written to.
 */
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", foo);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &foo);
	if (ret < 0)
		return ret;

	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0664, foo_show, foo_store);

/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "baz" and "bar" files.
 */
static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "baz") == 0)
		var = baz;
	else
		var = bar;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;

	if (strcmp(attr->attr.name, "baz") == 0)
		baz = var;
	else
		bar = var;
	return count;
}

static struct kobj_attribute baz_attribute =
	__ATTR(baz, 0664, b_show, b_store);
static struct kobj_attribute bar_attribute =
	__ATTR(bar, 0664, b_show, b_store);


/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&foo_attribute.attr,
	&baz_attribute.attr,
	&bar_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *toy_kobj;


static int toy_init(void)
{
	int result = 0;

	gpiomem_base = ioremap(0x1000000,0x300000);
	if (!gpiomem_base)
		pr_emerg("Hello,this is a toy.led on %d ms,off %d ms,not mapped gpio registers\n",on_time_ms,off_time_ms);
	else
		pr_emerg("Hello,my toy.led on %d ms,off %d ms,mapped %p,total_cnt is %d\n",
				on_time_ms,off_time_ms,gpiomem_base,total_cnt);
	
	toy_wq = alloc_workqueue("toy_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	INIT_WORK(&toy_work, (void *)blink_led);
	queue_work(toy_wq,&toy_work);


	/*
	 * Create a simple kobject with the name of "toy",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	toy_kobj = kobject_create_and_add("toy", kernel_kobj);
	if (!toy_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	result = sysfs_create_group(toy_kobj, &attr_group);
	if (result)
		kobject_put(toy_kobj);


	return result;
}

static void toy_exit(void)
{
	if(gpiomem_base)
		iounmap(gpiomem_base);

	kobject_put(toy_kobj);

	pr_emerg("Goobye,my toy\n");
}

module_init(toy_init);
module_exit(toy_exit);

MODULE_AUTHOR("Yinzhi.Dong");
MODULE_LICENSE("GPL");
