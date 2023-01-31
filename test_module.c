#define pr_fmt(fmt) "%s %s:%u " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/cred.h>

void show_context(const char *name)
{
	unsigned int uid = from_kuid(&init_user_ns, current_uid());
	unsigned int euid = from_kuid(&init_user_ns, current_euid());

	pr_info("%s:%s():%d ", name, __func__, __LINE__);
	if (likely(in_task())) {
		pr_info("%s in process context ::\n"
				" PID         : %6d\n"
				" TGID        : %6d\n"
				" UID         : %6d\n"
				" EUID        : %6d (%s root)\n"
				" name        : %s\n"
				" current (ptr to our process context's task_struct) :\n"
				"               0x%pK (0x%px)\n"
				" stack start : 0x%pK (0x%px)\n",
				name,
				task_pid_nr(current), task_tgid_nr(current),
				uid, euid,
				(euid == 0 ? "have" : "don't have"),
				current->comm,
				current, current,
				current->stack, current->stack);
	} else
		pr_alert("%s in interrupt context [Should NOT happen here!]\n", name);

}

static int __init mod_init(void)
{
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

	pr_info(" sizeof(struct task_struct)=%zd\n", sizeof(struct task_struct));
	show_context(KBUILD_MODNAME);
	return 0;
}

static void __exit mod_exit(void)
{
	show_context(KBUILD_MODNAME);
	pr_err("Stop. CPU %d\n", smp_processor_id());
}

MODULE_LICENSE("GPL");

module_init(mod_init);
module_exit(mod_exit);

