// SPDX-License-Identifier: GPL-2.0
/* race: two kernel threads, pinned to two CPUs, each add 200000 to three counters */
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/completion.h>

#define LOOPS 200000

static int plain;
static atomic_t atomic_count = ATOMIC_INIT(0);
static int locked;
static DEFINE_SPINLOCK(lock);
static DECLARE_COMPLETION(done);

static int adder(void *unused)
{
	int i;

	for (i = 0; i < LOOPS; i++) {
		WRITE_ONCE(plain, READ_ONCE(plain) + 1);	/* load, add, store */
		atomic_inc(&atomic_count);			/* one locked instruction */
		spin_lock(&lock);
		locked++;
		spin_unlock(&lock);
	}
	complete(&done);
	return 0;
}

static int __init race_init(void)
{
	struct task_struct *t[2];
	int cpu;

	for (cpu = 0; cpu < 2; cpu++) {
		t[cpu] = kthread_create(adder, NULL, "lab_adder/%d", cpu);
		if (IS_ERR(t[cpu])) {
			if (cpu == 1)
				kthread_stop(t[0]);	/* never started: exits at once */
			return PTR_ERR(t[cpu]);
		}
		kthread_bind(t[cpu], cpu);		/* run only on this CPU */
	}
	wake_up_process(t[0]);
	wake_up_process(t[1]);
	wait_for_completion(&done);
	wait_for_completion(&done);

	pr_info("race: expected %d, plain %d, atomic %d, spinlock %d\n",
		2 * LOOPS, plain, atomic_read(&atomic_count), locked);
	return 0;
}

static void __exit race_exit(void)
{
}

module_init(race_init);
module_exit(race_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 12: a data race, fixed two ways");
