// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>

struct sensor {
	int id;
	int value;
};

static struct sensor *sensor;	/* never allocated, so still NULL */

static int __init crash_init(void)
{
	pr_info("crash: writing through a NULL pointer\n");
	sensor->value = 42;
	return 0;
}

static void __exit crash_exit(void)
{
}

module_init(crash_init);
module_exit(crash_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 24: a deliberate NULL pointer bug");
