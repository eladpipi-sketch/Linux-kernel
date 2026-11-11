// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/slab.h>

static int bug;
module_param(bug, int, 0);
MODULE_PARM_DESC(bug, "1 = use after free, 2 = out of bounds");

static int __init buggy_init(void)
{
	char *p = kmalloc(16, GFP_KERNEL);

	if (!p)
		return -ENOMEM;

	if (bug == 1) {
		kfree(p);
		p[0] = 'x';		/* use after free */
		return 0;
	}

	if (bug == 2)
		p[16] = 'x';		/* one byte past the end */

	kfree(p);
	return 0;
}

static void __exit buggy_exit(void)
{
}

module_init(buggy_init);
module_exit(buggy_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 34: two memory bugs for KASAN");
