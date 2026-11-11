// SPDX-License-Identifier: GPL-2.0
/*
 * pagewalk: translate a virtual address of process <pid> into a physical
 * address by walking its page tables by hand, one level at a time.
 *   insmod pagewalk.ko pid=123 addr=0x7f0000001000
 */
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/pid.h>

static int pid;
static unsigned long addr;
module_param(pid, int, 0);
module_param(addr, ulong, 0);

static void walk(struct mm_struct *mm)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = pgd_offset(mm, addr);
	pr_info("pagewalk: pgd entry %#018lx\n", pgd_val(*pgd));
	if (pgd_none(*pgd))
		return;
	p4d = p4d_offset(pgd, addr);		/* folded into pgd on 4-level paging */
	if (p4d_none(*p4d))
		return;
	pud = pud_offset(p4d, addr);
	pr_info("pagewalk: pud entry %#018lx\n", pud_val(*pud));
	if (pud_none(*pud) || pud_leaf(*pud))
		return;
	pmd = pmd_offset(pud, addr);
	pr_info("pagewalk: pmd entry %#018lx\n", pmd_val(*pmd));
	if (pmd_none(*pmd) || pmd_leaf(*pmd))	/* pmd_leaf: a 2 MB huge page */
		return;
	/*
	 * pte_offset_map() is the safe accessor, but it is not exported to
	 * modules. On x86-64 every page-table page sits in the kernel's direct
	 * map, so pte_offset_kernel() computes the same pointer.
	 */
	pte = pte_offset_kernel(pmd, addr);
	pr_info("pagewalk: pte entry %#018lx\n", pte_val(*pte));
	if (pte_present(*pte))
		pr_info("pagewalk: %#lx -> pfn %#lx -> physical %#lx\n", addr,
			pte_pfn(*pte), (pte_pfn(*pte) << PAGE_SHIFT) | (addr & ~PAGE_MASK));
	else
		pr_info("pagewalk: %#lx is not in RAM (never touched, or swapped out)\n", addr);
}

static int __init pagewalk_init(void)
{
	struct task_struct *task;
	struct mm_struct *mm;

	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!task)
		return -ESRCH;
	mm = get_task_mm(task);
	put_task_struct(task);
	if (!mm)
		return -EINVAL;			/* a kernel thread has no user memory */

	mmap_read_lock(mm);			/* keep the page tables stable */
	walk(mm);
	mmap_read_unlock(mm);
	mmput(mm);
	return 0;
}

static void __exit pagewalk_exit(void)
{
}

module_init(pagewalk_init);
module_exit(pagewalk_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 25: walk a process's page tables");
