// SPDX-License-Identifier: GPL-2.0
/*
 * mystery, fixed: the first loop wrote squares[COUNT], one int past the end
 * (KASAN: slab-out-of-bounds), and report was never freed (kmemleak).
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sizes.h>

#define COUNT 1000

static int *squares;

static int __init mystery_init(void)
{
	char *report;
	long sum = 0;
	int i;

	squares = kmalloc_array(COUNT, sizeof(int), GFP_KERNEL);
	report = kmalloc(SZ_16K, GFP_KERNEL);
	if (!squares || !report) {
		kfree(squares);
		kfree(report);
		return -ENOMEM;
	}

	for (i = 0; i < COUNT; i++)		/* was <=: one past the end */
		squares[i] = i * i;
	for (i = 0; i < COUNT; i++)
		sum += squares[i];

	snprintf(report, SZ_16K, "sum of the first %d squares is %ld", COUNT, sum);
	pr_info("mystery: %s\n", report);
	kfree(report);					/* was missing: a leak */
	return 0;
}

static void __exit mystery_exit(void)
{
	kfree(squares);
}

module_init(mystery_init);
module_exit(mystery_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 18 exercise, fixed");
