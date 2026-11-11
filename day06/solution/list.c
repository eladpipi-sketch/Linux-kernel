// SPDX-License-Identifier: GPL-2.0
/* Day 6 exercise: keep the readings sorted, then drop one by value. */
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

struct reading {
	int value;
	struct list_head node;
};

static LIST_HEAD(readings);

static int values[8] = { 30, 10, 20 };
static int nr_values = 3;
module_param_array(values, int, &nr_values, 0444);
static int drop = 20;
module_param(drop, int, 0444);

/* Insert before the first larger value, so the list stays sorted. */
static void insert_sorted(struct reading *new)
{
	struct reading *pos;

	list_for_each_entry(pos, &readings, node) {
		if (pos->value > new->value) {
			list_add_tail(&new->node, &pos->node);	/* just before pos */
			return;
		}
	}
	list_add_tail(&new->node, &readings);		/* largest so far: at the end */
}

static void print_all(const char *when)
{
	struct reading *r;

	pr_info("list: %s:", when);
	list_for_each_entry(r, &readings, node)
		pr_cont(" %d", r->value);
	pr_cont("\n");
}

/* The _safe walk remembers the next item, so deleting this one is fine. */
static void drop_value(int value)
{
	struct reading *r, *next;

	list_for_each_entry_safe(r, next, &readings, node) {
		if (r->value == value) {
			list_del(&r->node);
			kfree(r);
		}
	}
}

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

	for (i = 0; i < nr_values; i++) {
		r = kmalloc(sizeof(*r), GFP_KERNEL);
		if (!r) {
			free_all();
			return -ENOMEM;
		}
		r->value = values[i];
		insert_sorted(r);
	}
	print_all("sorted");
	drop_value(drop);
	print_all("after dropping");
	return 0;
}

static void __exit list_exit(void)
{
	free_all();
}

module_init(list_init);
module_exit(list_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 6 exercise: a sorted kernel list");
