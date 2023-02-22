//#define pr_fmt(fmt) "%s %s:%u " fmt, KBUILD_MODNAME, __func__, __LINE__

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

static inline void disp_idle_thread(void)
{
	struct task_struct *t = &init_task;

	/* We know that the swapper is a kernel thread */
	pr_info("%8d %8d   0x%px   0x%px  [%16s]\n",
		t->pid, t->pid, t, t->stack, t->comm);
}

void show_threads(void)
{
	struct task_struct *g = NULL; // process ptr
	struct task_struct *t = NULL; // thread ptr
	int nr_threads = 0;
	int total = 0;

#define BUF_MAX 256
#define TMP_MAX 128
	char buf[BUF_MAX];
	char tmp[TMP_MAX];
	const char hdr[] =
"------------------------------------------------------------------------------------------\n"
"    TGID     PID         current           stack-start         Thread Name     MT? # thrds\n"
"------------------------------------------------------------------------------------------\n";

	pr_info("%s", hdr);
	disp_idle_thread();

	rcu_read_lock();
	do_each_thread(g, t) {
		task_lock(t);
		
		snprintf(buf, BUF_MAX - 1, "%8d %8d ", g->tgid, t->pid);
		snprintf(tmp, TMP_MAX - 1, "  0x%px ", t);
		snprintf(buf, BUF_MAX - 1, "%s%s  0x%px ", buf, tmp, t->stack);
		if (g->mm) {
			snprintf(tmp, TMP_MAX - 1, " [%16s]", t->comm);
		} else {
			snprintf(tmp, TMP_MAX - 1, "  %16s ", t->comm);
		}

		snprintf(buf, BUF_MAX - 1, "%s%s", buf, tmp);

		nr_threads = get_nr_threads(g);
		if (g->mm && (g->tgid == t->pid) && (nr_threads > 1)) {
			snprintf(tmp, TMP_MAX - 1, " %3d", nr_threads);
			snprintf(buf, BUF_MAX - 1, "%s%s", buf, tmp);
		}
		snprintf(buf, BUF_MAX - 1, "%s\n", buf);
		pr_info("%s", buf);
		total++;
		memset(buf, 0, sizeof(buf));
		memset(tmp, 0, sizeof(tmp));
		task_unlock(t);
	} while_each_thread(g, t);

	rcu_read_unlock();
}
