// SPDX-License-Identifier: GPL-2.0
/*
 * mystery: loads, prints a correct-looking answer, and hides two bugs.
 * Find them with the tools from today, without reading too closely first.
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

	for (i = 0; i <= COUNT; i++)
		squares[i] = i * i;
	for (i = 0; i < COUNT; i++)
		sum += squares[i];

	snprintf(report, SZ_16K, "sum of the first %d squares is %ld", COUNT, sum);
	pr_info("mystery: %s\n", report);
	return 0;
}

static void __exit mystery_exit(void)
{
	kfree(squares);
}

module_init(mystery_init);
module_exit(mystery_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 18 exercise: find two bugs");
