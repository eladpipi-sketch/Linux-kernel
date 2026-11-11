// SPDX-License-Identifier: GPL-2.0
/*
 * Day 10 exercise: labmap shares four pages, and maps each one only when the
 * process first touches it, from a .fault handler: demand paging, done by you.
 */
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define NPAGES 4
static struct page *pages[NPAGES];

static vm_fault_t labmap_fault(struct vm_fault *vmf)
{
	if (vmf->pgoff >= NPAGES)
		return VM_FAULT_SIGBUS;
	pr_info("labmap: fault on page %lu by %s\n", vmf->pgoff, current->comm);
	vmf->page = pages[vmf->pgoff];
	get_page(vmf->page);		/* the mapping holds a reference */
	return 0;
}

static const struct vm_operations_struct labmap_vm_ops = {
	.fault	= labmap_fault,
};

static int labmap_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &labmap_vm_ops;	/* nothing is mapped yet */
	return 0;
}

static const struct file_operations labmap_fops = {
	.owner	= THIS_MODULE,
	.mmap	= labmap_mmap,
};

static struct miscdevice labmap_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "labmap",
	.fops	= &labmap_fops,
};

static void free_pages_array(void)
{
	int i;

	for (i = 0; i < NPAGES; i++)
		if (pages[i])
			__free_page(pages[i]);
}

static int __init labmap_init(void)
{
	int i, err;

	for (i = 0; i < NPAGES; i++) {
		pages[i] = alloc_page(GFP_KERNEL | __GFP_ZERO);
		if (!pages[i]) {
			free_pages_array();
			return -ENOMEM;
		}
		snprintf(page_address(pages[i]), PAGE_SIZE, "kernel page %d", i);
	}
	err = misc_register(&labmap_dev);
	if (err)
		free_pages_array();
	return err;
}

static void __exit labmap_exit(void)
{
	int i;

	misc_deregister(&labmap_dev);
	for (i = 0; i < NPAGES; i++)
		pr_info("labmap: page %d says: %s\n", i, (char *)page_address(pages[i]));
	free_pages_array();
}

module_init(labmap_init);
module_exit(labmap_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 10 exercise: map pages on demand");
