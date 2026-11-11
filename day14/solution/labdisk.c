// SPDX-License-Identifier: GPL-2.0
/*
 * Day 14 exercise: labdisk as a slow disk. Each request sleeps delay_ms first,
 * which is only allowed because the tag set has BLK_MQ_F_BLOCKING: without
 * it, blk-mq calls queue_rq inside an RCU read-side section.
 */
#include <linux/module.h>
#include <linux/blk-mq.h>
#include <linux/blkdev.h>
#include <linux/highmem.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

#define LABDISK_BYTES	(16 * 1024 * 1024)

static int major;
static u8 *data;				/* the disk's contents */
static struct blk_mq_tag_set tag_set;
static struct gendisk *disk;
static unsigned int delay_ms = 10;
module_param(delay_ms, uint, 0644);	/* change it at /sys/module/labdisk/parameters */

/* The block layer calls this for every request it decides to send us. */
static blk_status_t labdisk_queue_rq(struct blk_mq_hw_ctx *hctx,
				     const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	loff_t pos = (loff_t)blk_rq_pos(rq) << SECTOR_SHIFT;	/* 512-byte sectors */
	struct req_iterator iter;
	struct bio_vec bvec;

	blk_mq_start_request(rq);
	msleep(delay_ms);			/* pretend the disk is slow */
	if (pos + blk_rq_bytes(rq) > LABDISK_BYTES) {
		blk_mq_end_request(rq, BLK_STS_IOERR);
		return BLK_STS_OK;
	}

	rq_for_each_segment(bvec, rq, iter) {	/* each piece of each page */
		void *buf = bvec_kmap_local(&bvec);

		if (rq_data_dir(rq) == WRITE)
			memcpy(data + pos, buf, bvec.bv_len);
		else
			memcpy(buf, data + pos, bvec.bv_len);
		kunmap_local(buf);
		pos += bvec.bv_len;
	}
	blk_mq_end_request(rq, BLK_STS_OK);
	return BLK_STS_OK;
}

static const struct blk_mq_ops labdisk_mq_ops = {
	.queue_rq = labdisk_queue_rq,
};

static const struct block_device_operations labdisk_fops = {
	.owner = THIS_MODULE,
};

static int __init labdisk_init(void)
{
	struct queue_limits lim = { .logical_block_size = 512 };
	int err;

	major = register_blkdev(0, "labdisk");
	if (major < 0)
		return major;

	err = -ENOMEM;
	data = vzalloc(LABDISK_BYTES);
	if (!data)
		goto err_unregister;

	tag_set.ops = &labdisk_mq_ops;
	tag_set.nr_hw_queues = 1;
	tag_set.queue_depth = 16;
	tag_set.numa_node = NUMA_NO_NODE;
	tag_set.flags = BLK_MQ_F_BLOCKING;	/* queue_rq may sleep */
	err = blk_mq_alloc_tag_set(&tag_set);
	if (err)
		goto err_free_data;

	disk = blk_mq_alloc_disk(&tag_set, &lim, NULL);
	if (IS_ERR(disk)) {
		err = PTR_ERR(disk);
		goto err_free_tags;
	}
	disk->major = major;
	disk->first_minor = 0;
	disk->minors = 1;
	disk->fops = &labdisk_fops;
	strscpy(disk->disk_name, "labdisk");
	set_capacity(disk, LABDISK_BYTES >> SECTOR_SHIFT);

	err = add_disk(disk);
	if (err)
		goto err_put_disk;
	return 0;

err_put_disk:
	put_disk(disk);
err_free_tags:
	blk_mq_free_tag_set(&tag_set);
err_free_data:
	vfree(data);
err_unregister:
	unregister_blkdev(major, "labdisk");
	return err;
}

static void __exit labdisk_exit(void)
{
	del_gendisk(disk);
	put_disk(disk);
	blk_mq_free_tag_set(&tag_set);
	vfree(data);
	unregister_blkdev(major, "labdisk");
}

module_init(labdisk_init);
module_exit(labdisk_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 14 exercise: a slow disk");
