// SPDX-License-Identifier: GPL-2.0
/* lockorder: take two locks in both orders, one after the other. Nothing hangs. */
#include <linux/module.h>
#include <linux/mutex.h>

static DEFINE_MUTEX(lock_a);
static DEFINE_MUTEX(lock_b);

static int __init lockorder_init(void)
{
	mutex_lock(&lock_a);		/* one code path: a, then b */
	mutex_lock(&lock_b);
	mutex_unlock(&lock_b);
	mutex_unlock(&lock_a);

	mutex_lock(&lock_b);		/* another code path: b, then a */
	mutex_lock(&lock_a);
	mutex_unlock(&lock_a);
	mutex_unlock(&lock_b);
	return 0;
}

static void __exit lockorder_exit(void)
{
}

module_init(lockorder_init);
module_exit(lockorder_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 12: a lock order inversion for lockdep");
