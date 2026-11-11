// SPDX-License-Identifier: GPL-2.0
/* worker: a kernel thread that wakes up once a second */
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *worker;

static int worker_fn(void *data)
{
	int tick = 0;

	while (!kthread_should_stop()) {
		pr_info("lab_worker: tick %d on CPU %d\n", tick++, raw_smp_processor_id());
		msleep_interruptible(1000);	/* sleep: state S, off the run queue */
	}
	return 0;
}

static int __init worker_init(void)
{
	worker = kthread_run(worker_fn, NULL, "lab_worker");
	return PTR_ERR_OR_ZERO(worker);
}

static void __exit worker_exit(void)
{
	kthread_stop(worker);		/* wakes it and waits for worker_fn to return */
}

module_init(worker_init);
module_exit(worker_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 8: a kernel thread");
