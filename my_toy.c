#include <linux/init.h>
#include <linux/module.h>
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

#define LED_6G_GREEN           0x6

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
	pr_emerg("blink led completed\n");

}

static int hello_init(void)
{
	gpiomem_base = ioremap(0x1000000,0x300000);
	if (!gpiomem_base)
		pr_emerg("Hello,this is a toy.led on %d ms,off %d ms,not mapped gpio registers\n",on_time_ms,off_time_ms);
	else
		pr_emerg("Hello,my toy.led on %d ms,off %d ms,mapped %p,total_cnt is %d\n",
				on_time_ms,off_time_ms,gpiomem_base,total_cnt);
	
	toy_wq = alloc_workqueue("toy_wq", WQ_HIGHPRI | WQ_UNBOUND, 0);
	INIT_WORK(&toy_work, (void *)blink_led);
	queue_work(toy_wq,&toy_work);
	return 0;
}

static void hello_exit(void)
{
	if(gpiomem_base)
		iounmap(gpiomem_base);

	pr_emerg("Goobye,my toy\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Yinzhi.Dong");
MODULE_LICENSE("GPL");
