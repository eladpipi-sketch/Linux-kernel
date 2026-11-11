// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>

static char *whom = "world";
module_param(whom, charp, 0644);
MODULE_PARM_DESC(whom, "Who to greet");

static int __init hello_init(void)
{
	pr_info("hello: hello, %s\n", whom);
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("hello: goodbye, %s\n", whom);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 22: the smallest useful module");
