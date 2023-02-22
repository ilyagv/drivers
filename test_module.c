#define pr_fmt(fmt) "%s %s:%u " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

#include "exec_context.h"
#include "net_device.h"

static int __init mod_init(void)
{
	int err;
	pr_err("Start. CPU %d\n", smp_processor_id());

	pr_emerg("log-level KERN_EMERG [0]\n");
	pr_alert("log-level KERN_ALERT [1]\n");
	pr_crit("log-level KERN_CRIT [2]\n");
	pr_err("log-level KERN_ERR [3]\n");
	pr_warn("log-level KERN_WARNING [4]\n");
	pr_notice("log-level KERN_NOTICE [5]\n");
	pr_info("log-level KERN_INFO [6]\n");
	pr_debug("log-level KERN_DEBUG [7]\n");
	pr_devel("log-level pr_devel [7]\n");

	show_processes();
	show_threads();

	pr_info(" sizeof(struct task_struct)=%zd\n", sizeof(struct task_struct));
	show_context(KBUILD_MODNAME);

	err = ndev_init();
	if (err)
		return err;

	return 0;
}

static void __exit mod_exit(void)
{
	show_context(KBUILD_MODNAME);
	ndev_exit();
	pr_err("Stop. CPU %d\n", smp_processor_id());
}

MODULE_LICENSE("GPL");

module_init(mod_init);
module_exit(mod_exit);

