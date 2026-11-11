// SPDX-License-Identifier: GPL-2.0
/*
 * Day 8 exercise: tasks with pid=N prints that task's ancestors up to PID 0,
 * following task->real_parent, with nice value and CPU time.
 *   insmod tasks.ko pid=$$
 */
#include <linux/module.h>
#include <linux/sched/signal.h>

static int pid;
module_param(pid, int, 0444);

static void print_ancestors(void)
{
	struct task_struct *p;

	rcu_read_lock();			/* tasks can't be freed while we look */
	p = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!p)
		pr_info("tasks: no pid %d\n", pid);
	while (p) {
		pr_info("tasks: %5d %-16s nice %3d  cpu %llu ms\n", p->pid, p->comm,
			task_nice(p), p->se.sum_exec_runtime / NSEC_PER_MSEC);
		if (p->pid == 0)		/* swapper: its parent is itself */
			break;
		p = rcu_dereference(p->real_parent);
	}
	rcu_read_unlock();
}

static int __init tasks_init(void)
{
	struct task_struct *p;

	if (pid) {
		print_ancestors();
		return 0;
	}
	pr_info("tasks:   PID COMM             STATE  PRIO  VRUNTIME(ns)   SWITCHES(vol+invol)\n");
	rcu_read_lock();
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
MODULE_DESCRIPTION("Day 8 exercise: a task's ancestors");
