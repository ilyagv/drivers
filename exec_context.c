#define pr_fmt(fmt) "%s %s:%u " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/cred.h> // current_uid(); current_euid(); 
#include <linux/sched/signal.h> // for_each_process()
#include "exec_context.h"

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

#define MAX_INFO_LEN 128

void show_processes(void)
{
	struct task_struct *p;
	char info[MAX_INFO_LEN];
	unsigned total = 0;
	int n = 0;
	char hdr[] = "     Name       |  TGID  |   PID  |  RUID |  EUID";
	pr_info("%s\n", &hdr[0]);

	rcu_read_lock();
	for_each_process(p) {
		memset(info, 0, sizeof(info));
		n = snprintf(info, MAX_INFO_LEN, "%-16s|%8d|%8d|%7u|%7u\n",
			p->comm,
			p->tgid,
			p->pid,
			__kuid_val(p->cred->uid),
			__kuid_val(p->cred->euid));

		pr_info("%s", info);
		cond_resched();
		total++;
	}

	rcu_read_unlock();
}

