// SPDX-License-Identifier: GPL-2.0
/*
 * Day 12 exercise: a fourth counter that is exact with no lock and no atomic.
 * Each thread adds to its own counter; init adds the two up at the end.
 * The kernel counts packets and page faults this way, with per-CPU counters.
 */
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
static int mine[2];			/* one counter per thread */

static int adder(void *data)
{
	int *my_count = data;
	int i;

	for (i = 0; i < LOOPS; i++) {
		WRITE_ONCE(plain, READ_ONCE(plain) + 1);	/* load, add, store */
		atomic_inc(&atomic_count);			/* one locked instruction */
		spin_lock(&lock);
		locked++;
		spin_unlock(&lock);
		(*my_count)++;					/* nobody else writes it */
	}
	complete(&done);
	return 0;
}

static int __init race_init(void)
{
	struct task_struct *t[2];
	int cpu;

	for (cpu = 0; cpu < 2; cpu++) {
		t[cpu] = kthread_create(adder, &mine[cpu], "lab_adder/%d", cpu);
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

	pr_info("race: expected %d, plain %d, atomic %d, spinlock %d, per-thread %d\n",
		2 * LOOPS, plain, atomic_read(&atomic_count), locked, mine[0] + mine[1]);
	return 0;
}

static void __exit race_exit(void)
{
}

module_init(race_init);
module_exit(race_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 12 exercise: per-thread counters");
