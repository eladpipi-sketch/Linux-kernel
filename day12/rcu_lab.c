// SPDX-License-Identifier: GPL-2.0
/* rcu_lab: a reader uses a shared config without locks while init replaces it */
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/rcupdate.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>

struct config {
	int version;
	char name[32];
};

static struct config __rcu *live_config;
static DEFINE_MUTEX(update_lock);		/* only writers take it */
static struct task_struct *reader;

static int reader_fn(void *unused)
{
	int last = 0;

	while (!kthread_should_stop()) {
		struct config *cfg;
		int version;

		rcu_read_lock();		/* costs almost nothing, never waits */
		cfg = rcu_dereference(live_config);
		version = cfg->version;
		rcu_read_unlock();		/* after this, cfg may be freed */

		if (version != last)
			pr_info("rcu_lab: reader now sees version %d\n", version);
		last = version;
		msleep(50);
	}
	return 0;
}

static void publish(int version)
{
	struct config *new, *old;

	new = kzalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return;
	new->version = version;
	snprintf(new->name, sizeof(new->name), "config-%d", version);

	mutex_lock(&update_lock);
	old = rcu_replace_pointer(live_config, new, lockdep_is_held(&update_lock));
	mutex_unlock(&update_lock);

	synchronize_rcu();		/* wait until no reader can still hold old */
	pr_info("rcu_lab: published version %d, freed version %d\n",
		version, old ? old->version : 0);
	kfree(old);
}

static int __init rcu_lab_init(void)
{
	int v;

	publish(1);
	reader = kthread_run(reader_fn, NULL, "lab_rcu_reader");
	if (IS_ERR(reader))
		return PTR_ERR(reader);
	for (v = 2; v <= 4; v++) {
		msleep(500);
		publish(v);
	}
	return 0;
}

static void __exit rcu_lab_exit(void)
{
	kthread_stop(reader);
	kfree(rcu_dereference_protected(live_config, true));
}

module_init(rcu_lab_init);
module_exit(rcu_lab_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 12: read-copy-update");
