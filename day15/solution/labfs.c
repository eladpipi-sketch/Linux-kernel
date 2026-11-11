// SPDX-License-Identifier: GPL-2.0
/*
 * Day 15 exercise: labfs with a fourth file, whoami, which tells each reader
 * who it is: read() runs in the calling process, so current is the reader.
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#define LABFS_MAGIC 0x4c616246		/* "LabF" */

static char note[64] = "write something here\n";
static DEFINE_MUTEX(note_lock);

static ssize_t hello_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	static const char msg[] = "hello from labfs\n";

	return simple_read_from_buffer(buf, len, ppos, msg, sizeof(msg) - 1);
}

static ssize_t jiffies_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	char tmp[32];
	int n = scnprintf(tmp, sizeof(tmp), "%lu\n", jiffies);	/* made up on every read */

	return simple_read_from_buffer(buf, len, ppos, tmp, n);
}

static ssize_t whoami_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	char tmp[48];
	int n = scnprintf(tmp, sizeof(tmp), "pid %d (%s)\n", current->pid, current->comm);

	return simple_read_from_buffer(buf, len, ppos, tmp, n);
}

static ssize_t note_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	guard(mutex)(&note_lock);
	return simple_read_from_buffer(buf, len, ppos, note, strlen(note));
}

static ssize_t note_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	ssize_t n;

	if (len >= sizeof(note))
		return -ENOSPC;
	guard(mutex)(&note_lock);
	memset(note, 0, sizeof(note));
	n = simple_write_to_buffer(note, sizeof(note) - 1, ppos, buf, len);
	return n;
}

static const struct file_operations hello_fops = {
	.read = hello_read, .llseek = default_llseek,
};
static const struct file_operations jiffies_fops = {
	.read = jiffies_read, .llseek = default_llseek,
};
static const struct file_operations whoami_fops = {
	.read = whoami_read, .llseek = default_llseek,
};
static const struct file_operations note_fops = {
	.read = note_read, .write = note_write, .llseek = default_llseek,
};

static int labfs_fill_super(struct super_block *sb, struct fs_context *fc)
{
	/* index = inode number; 1 is the root directory, so files start at 2 */
	static const struct tree_descr files[] = {
		[2] = { "hello",   &hello_fops,   0444 },
		[3] = { "jiffies", &jiffies_fops, 0444 },
		[4] = { "note",    &note_fops,    0644 },
		[5] = { "whoami",  &whoami_fops,  0444 },
		{ "" }
	};

	return simple_fill_super(sb, LABFS_MAGIC, files);
}

static int labfs_get_tree(struct fs_context *fc)
{
	return get_tree_single(fc, labfs_fill_super);	/* one shared instance */
}

static const struct fs_context_operations labfs_context_ops = {
	.get_tree = labfs_get_tree,
};

static int labfs_init_fs_context(struct fs_context *fc)
{
	fc->ops = &labfs_context_ops;
	return 0;
}

static struct file_system_type labfs_type = {
	.owner			= THIS_MODULE,
	.name			= "labfs",
	.init_fs_context	= labfs_init_fs_context,
	.kill_sb		= kill_anon_super,
};

static int __init labfs_init(void)
{
	return register_filesystem(&labfs_type);
}

static void __exit labfs_exit(void)
{
	unregister_filesystem(&labfs_type);
}

module_init(labfs_init);
module_exit(labfs_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 15 exercise: labfs with whoami");
MODULE_ALIAS_FS("labfs");
