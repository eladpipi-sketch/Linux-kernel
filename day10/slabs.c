// SPDX-License-Identifier: GPL-2.0
/* slabs: the kernel's three allocators side by side */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/gfp.h>
#include <linux/mm.h>

struct lab_obj {
	int id;
	char name[200];
};

#define NR_OBJS 1000
#define VSIZE (SZ_1M + PAGE_SIZE)	/* 257 pages */

static struct kmem_cache *cache;
static struct lab_obj *objs[NR_OBJS];
static struct page *pages;
static void *vbuf;

/* How many pages, from the start of buf, sit next to each other in RAM? */
static unsigned long contiguous_run(void *buf, size_t size, bool is_vmalloc)
{
	unsigned long first = 0, i, n = size / PAGE_SIZE;

	for (i = 0; i < n; i++) {
		void *p = buf + i * PAGE_SIZE;
		unsigned long pfn = is_vmalloc ? vmalloc_to_pfn(p)
					       : page_to_pfn(virt_to_page(p));

		if (i == 0)
			first = pfn;
		else if (pfn != first + i)
			break;
	}
	return i;
}

static int __init slabs_init(void)
{
	void *kbuf;
	int i;

	/* 1. A slab cache of our own: one object size, many objects per page */
	cache = kmem_cache_create("lab_obj", sizeof(struct lab_obj), 0, 0, NULL);
	if (!cache)
		return -ENOMEM;
	for (i = 0; i < NR_OBJS; i++)
		objs[i] = kmem_cache_alloc(cache, GFP_KERNEL);
	pr_info("slabs: %d lab_obj objects of %zu bytes, first at %px\n",
		NR_OBJS, sizeof(struct lab_obj), objs[0]);

	/* 2. Straight from the buddy allocator: 2^3 = 8 contiguous pages */
	pages = alloc_pages(GFP_KERNEL, 3);
	if (pages)
		pr_info("slabs: alloc_pages(order 3): pfn %#lx..%#lx\n",
			page_to_pfn(pages), page_to_pfn(pages) + 7);

	/* 3. kmalloc: one physical block. vmalloc: pages from anywhere. */
	kbuf = kmalloc(SZ_1M, GFP_KERNEL);
	vbuf = vmalloc(VSIZE);
	if (kbuf && vbuf)
		pr_info("slabs: kmalloc 1MB at %px: %lu of %lu pages contiguous; vmalloc 1MB+4KB at %px: %lu of %lu\n",
			kbuf, contiguous_run(kbuf, SZ_1M, false), SZ_1M / PAGE_SIZE,
			vbuf, contiguous_run(vbuf, VSIZE, true), VSIZE / PAGE_SIZE);
	kfree(kbuf);
	return 0;
}

static void __exit slabs_exit(void)
{
	int i;

	vfree(vbuf);
	if (pages)
		__free_pages(pages, 3);
	for (i = 0; i < NR_OBJS; i++)
		if (objs[i])
			kmem_cache_free(cache, objs[i]);
	kmem_cache_destroy(cache);
}

module_init(slabs_init);
module_exit(slabs_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 10: slab, buddy and vmalloc allocators");
