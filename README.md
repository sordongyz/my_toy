# my_toy
A linux driver toy, to try some basic linux kernel driver method.

Put this file under kernel/drivers/misc/
then change Makefile with +obj-m += my_toy.o


The new driver mode is based on kobject, linux/kobject.h defines a lots of folder under /sys

Here is some folder in kobject.h

/* The global /sys/kernel/ kobject for people to chain off of */
extern struct kobject *kernel_kobj;
/* The global /sys/kernel/mm/ kobject for people to chain off of */
extern struct kobject *mm_kobj;
/* The global /sys/hypervisor/ kobject for people to chain off of */
extern struct kobject *hypervisor_kobj;
/* The global /sys/power/ kobject for people to chain off of */
extern struct kobject *power_kobj;
/* The global /sys/firmware/ kobject for people to chain off of */
extern struct kobject *firmware_kobj;
