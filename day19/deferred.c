// SPDX-License-Identifier: GPL-2.0
/*
 * deferred: one periodic job handled three ways, to see where each one runs.
 * A timer_list callback runs in softirq context, a work item in a kworker
 * thread, an hrtimer callback in a hard interrupt. Each reports its context.
 */
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/ktime.h>

#define TIMER_RUNS	3
#define HRTIMER_RUNS	1000
#define HRTIMER_US	500		/* ask for one hrtimer run every 0.5 ms */

static struct timer_list timer;
static struct work_struct work;
static struct hrtimer hrtimer;
static int timer_runs, hrtimer_runs;
static ktime_t hrtimer_last;
static s64 gap_min = S64_MAX, gap_max;

static void report(const char *who)
{
	pr_info("deferred: %-8s hardirq=%d softirq=%d task=%d cpu=%d current=%s\n",
		who, !!in_hardirq(), !!in_serving_softirq(), !!in_task(),
		raw_smp_processor_id(), current->comm);
}

static void work_fn(struct work_struct *w)
{
	report("work");
	msleep(1);			/* allowed: a work item runs in a kernel thread */
}

static void timer_fn(struct timer_list *t)
{
	report("timer");
	schedule_work(&work);		/* hand anything slow to process context */
	if (++timer_runs < TIMER_RUNS)
		mod_timer(&timer, jiffies + msecs_to_jiffies(100));
}

static enum hrtimer_restart hrtimer_fn(struct hrtimer *t)
{
	ktime_t now = ktime_get();

	if (hrtimer_runs == 0) {
		report("hrtimer");
	} else {
		s64 gap = ktime_us_delta(now, hrtimer_last);

		gap_min = min(gap_min, gap);
		gap_max = max(gap_max, gap);
	}
	hrtimer_last = now;
	if (++hrtimer_runs == HRTIMER_RUNS)
		return HRTIMER_NORESTART;
	hrtimer_forward_now(t, us_to_ktime(HRTIMER_US));
	return HRTIMER_RESTART;
}

static int __init deferred_init(void)
{
	INIT_WORK(&work, work_fn);
	timer_setup(&timer, timer_fn, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(100));
	hrtimer_setup(&hrtimer, hrtimer_fn, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_start(&hrtimer, us_to_ktime(HRTIMER_US), HRTIMER_MODE_REL);
	return 0;
}

static void __exit deferred_exit(void)
{
	timer_shutdown_sync(&timer);	/* wait for it, and refuse any re-arming */
	hrtimer_cancel(&hrtimer);
	cancel_work_sync(&work);
	pr_info("deferred: hrtimer ran %d times; asked for %d us, got gaps of %lld..%lld us\n",
		hrtimer_runs, HRTIMER_US, gap_min, gap_max);
}

module_init(deferred_init);
module_exit(deferred_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 19: timers, work items and hrtimers");
