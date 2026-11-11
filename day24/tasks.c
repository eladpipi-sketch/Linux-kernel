// SPDX-License-Identifier: GPL-2.0
/* tasks: print every process the way the kernel sees it (struct task_struct) */
#include <linux/module.h>
#include <linux/sched/signal.h>

static int __init tasks_init(void)
{
	struct task_struct *p;

	pr_info("tasks:   PID COMM             STATE  PRIO  VRUNTIME(ns)   SWITCHES(vol+invol)\n");
	rcu_read_lock();			/* the task list is protected by RCU */
	for_each_process(p)
		pr_info("tasks: %5d %-16s %#6x %5d %13llu   %lu+%lu\n",
			p->pid, p->comm, READ_ONCE(p->__state), p->prio,
			p->se.vruntime, p->nvcsw, p->nivcsw);
	rcu_read_unlock();
	return 0;
}

static void __exit tasks_exit(void)
{
}

module_init(tasks_init);
module_exit(tasks_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 24: walk the task list");
