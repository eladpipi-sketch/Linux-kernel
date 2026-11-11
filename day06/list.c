// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

struct reading {
	int value;
	struct list_head node;	/* links this reading into a list */
};

static LIST_HEAD(readings);

static void free_all(void)
{
	struct reading *r, *next;

	list_for_each_entry_safe(r, next, &readings, node) {
		list_del(&r->node);
		kfree(r);
	}
}

static int __init list_init(void)
{
	struct reading *r;
	int i;

	for (i = 1; i <= 3; i++) {
		r = kmalloc(sizeof(*r), GFP_KERNEL);
		if (!r) {
			free_all();	/* undo the ones already added */
			return -ENOMEM;
		}
		r->value = i * 10;
		list_add_tail(&r->node, &readings);
	}

	list_for_each_entry(r, &readings, node)
		pr_info("list: value %d at %px\n", r->value, r);

	r = container_of(readings.next, struct reading, node);
	pr_info("list: first value via container_of: %d\n", r->value);
	pr_info("list: list head at %px, code at %px\n", &readings, list_init);
	return 0;
}

static void __exit list_exit(void)
{
	free_all();
	pr_info("list: freed everything\n");
}

module_init(list_init);
module_exit(list_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 6: a kernel linked list");
