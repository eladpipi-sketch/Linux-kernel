// SPDX-License-Identifier: GPL-2.0
/* labmap: share one page of kernel memory with user space through mmap() */
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>

static struct page *page;		/* the page both sides will see */

static int labmap_mmap(struct file *file, struct vm_area_struct *vma)
{
	if (vma->vm_end - vma->vm_start != PAGE_SIZE || vma->vm_pgoff != 0)
		return -EINVAL;
	/* put our page into the process's page tables at the address mmap() chose */
	return vm_insert_page(vma, vma->vm_start, page);
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

static int __init labmap_init(void)
{
	int err;

	page = alloc_page(GFP_KERNEL | __GFP_ZERO);
	if (!page)
		return -ENOMEM;
	strscpy(page_address(page), "hello from the kernel", PAGE_SIZE);

	err = misc_register(&labmap_dev);
	if (err)
		__free_page(page);
	return err;
}

static void __exit labmap_exit(void)
{
	misc_deregister(&labmap_dev);
	pr_info("labmap: the page now says: %s\n", (char *)page_address(page));
	__free_page(page);
}

module_init(labmap_init);
module_exit(labmap_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 26: mmap a kernel page into user space");
