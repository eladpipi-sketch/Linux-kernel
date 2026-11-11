// SPDX-License-Identifier: GPL-2.0
/* leak: allocate in a loop and keep none of it, a classic driver bug */
#include <linux/module.h>
#include <linux/slab.h>

static int __init leak_init(void)
{
	int i;

	for (i = 0; i < 4; i++) {
		char *buf = kmalloc(SZ_16K, GFP_KERNEL);	/* each pass overwrites buf */

		if (buf)
			snprintf(buf, SZ_16K, "lost buffer %d", i);
	}
	pr_info("leak: allocated 4 x 16 KB, kept none\n");
	return 0;
}

static void __exit leak_exit(void)
{
}

module_init(leak_init);
module_exit(leak_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 34: memory leaks for kmemleak");
