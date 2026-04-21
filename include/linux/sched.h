#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

/*
 * ============================================================================
 * SCHED.H MODIFICADO: SEPARAÇÃO ENTRE TASK (EXECUÇÃO) E PROC (RECURSOS)
 * ============================================================================
 * 
 * AVISO: ESTRUTURAS DE ESCALONAMENTO FORAM MODIFICADAS!
 * 
 * AGORA EXISTE:
 *   ================================================
 *   ✓ task_struct - Unidade de EXECUÇÃO (AGENDÁVEL)
 *   ✓ proc_struct - Unidade de RECURSOS (NÃO AGENDÁVEL)
 *   ================================================
 * 
 * TASK_STRUCT:
 *   - Representa uma thread/unidade de execução
 *   - Pode ser agendada em CPUs
 *   - Contém estado de execução, contexto, prioridade
 *   - Comunica via portas e mensagens
 * 
 * PROC_STRUCT:
 *   - Representa um processo/coleção de recursos
 *   - NÃO é agendável diretamente
 *   - Contém mm, files, fs, signals, credentials
 *   - Várias tasks podem compartilhar o mesmo proc
 * 
 * FILOSOFIA: "TUDO É OBJETO" e "TUDO É MENSAGEM"
 * 
 * ARQUIVOS SÃO OBJETOS DE MENSAGEM:
 *   - Tasks comunicam via portas
 *   - Proc contém portas e objetos
 * 
 * FILESYSTEMS EM RING 0: NFS, EXFAT, FAT, NTFS, HFS, HFS+
 * 
 * ============================================================================
 */

#include <uapi/linux/sched.h>

#include <asm/current.h>
#include <asm/processor.h>
#include <linux/thread_info.h>
#include <linux/preempt.h>
#include <linux/cpumask_types.h>

#include <linux/cache.h>
#include <linux/irqflags_types.h>
#include <linux/smp_types.h>
#include <linux/pid_types.h>
#include <linux/sem_types.h>
#include <linux/shm.h>
#include <linux/kmsan_types.h>
#include <linux/mutex_types.h>
#include <linux/plist_types.h>
#include <linux/hrtimer_types.h>
#include <linux/timer_types.h>
#include <linux/seccomp_types.h>
#include <linux/nodemask_types.h>
#include <linux/refcount_types.h>
#include <linux/resource.h>
#include <linux/latencytop.h>
#include <linux/sched/prio.h>
#include <linux/sched/types.h>
#include <linux/signal_types.h>
#include <linux/spinlock.h>
#include <linux/syscall_user_dispatch_types.h>
#include <linux/mm_types_task.h>
#include <linux/netdevice_xmit.h>
#include <linux/task_io_accounting.h>
#include <linux/posix-timers_types.h>
#include <linux/restart_block.h>
#include <linux/rseq_types.h>
#include <linux/seqlock_types.h>
#include <linux/kcsan.h>
#include <linux/rv.h>
#include <linux/uidgid_types.h>
#include <linux/tracepoint-defs.h>
#include <linux/unwind_deferred_types.h>
#include <asm/kmap_size.h>
#include <linux/time64.h>

/* ============================================================================
 * CONSTANTES E TIPOS BÁSICOS
 * ============================================================================ */

/* Object ID para tasks e procs */
typedef __u64 task_object_id_t;
typedef __u64 proc_object_id_t;
typedef __u64 port_id_t;

/* ============================================================================
 * PROC_STRUCT - UNIDADE DE RECURSOS (NÃO AGENDÁVEL)
 * ============================================================================ */

/**
 * struct proc_struct - Unidade de recursos do processo
 * 
 * NÃO É AGENDÁVEL! Contém apenas recursos compartilhados.
 * Várias task_struct podem compartilhar o mesmo proc_struct (threads).
 */
struct proc_struct {
	/* Identificação */
	proc_object_id_t	proc_id;	/* Object ID do proc */
	pid_t			pid;		/* Process ID (compatibilidade) */
	pid_t			tgid;		/* Thread group ID */
	
	/* Contagem de referências */
	refcount_t		refcount;	/* Número de tasks usando este proc */
	atomic_t		nr_threads;	/* Número de threads no grupo */
	
	/* Portas de comunicação */
	port_id_t		default_port;	/* Porta padrão do proc */
	port_id_t		signal_port;	/* Porta para sinais */
	port_id_t		ipc_port;	/* Porta para IPC */
	
	/* Tabela de objetos do processo */
	struct xarray		objects;	/* Objetos owned pelo proc */
	
	/* Espaço de endereços (compartilhado entre tasks) */
	struct mm_struct	*mm;
	struct mm_struct	*active_mm;
	
	/* Filesystem information (compartilhado) */
	struct fs_struct	*fs;
	
	/* Open file information (compartilhado) */
	struct files_struct	*files;
	
	/* Namespaces (compartilhado) */
	struct nsproxy		*nsproxy;
	
	/* Signal handlers (compartilhado) */
	struct signal_struct	*signal;
	struct sighand_struct __rcu *sighand;
	sigset_t		blocked;
	struct sigpending	pending;
	
	/* Credenciais (compartilhadas) */
	const struct cred __rcu *real_cred;
	const struct cred __rcu *cred;
	
	/* Limites de recursos */
	struct rlimit		rlim[RLIM_NLIMITS];
	
	/* Estatísticas do processo */
	u64			utime;
	u64			stime;
	u64			cutime;
	u64			cstime;
	
	/* Lista de tasks neste proc */
	struct list_head	tasks;
	spinlock_t		tasks_lock;
	
	/* Lista de procs (global) */
	struct list_head	proc_list;
	
	/* Sincronização */
	spinlock_t		lock;
	struct rcu_head		rcu;
	
	/* Nome do processo */
	char			comm[TASK_COMM_LEN];
	
	/* Capacidades */
	capability_t		capability_mask;
};

/* ============================================================================
 * TASK_STRUCT - UNIDADE DE EXECUÇÃO (AGENDÁVEL)
 * ============================================================================ */

/**
 * struct task_struct - Unidade de execução agendável
 * 
 * REPRESENTA UMA THREAD/UNIDADE DE EXECUÇÃO:
 *   - Pode ser agendada em CPUs
 *   - Tem estado de execução próprio
 *   - Tem contexto de pilha próprio
 *   - Aponta para um proc_struct (recursos compartilhados)
 */
struct task_struct {
#ifdef CONFIG_THREAD_INFO_IN_TASK
	/* Must be first element */
	struct thread_info		thread_info;
#endif
	
	/* Identificação */
	task_object_id_t		task_id;	/* Object ID da task */
	pid_t				pid;		/* Thread ID */
	
	/* Proc associado (recursos compartilhados) */
	struct proc_struct		*proc;		/* NÃO é agendável */
	
	/* Estado de execução */
	unsigned int			__state;	/* TASK_RUNNING, etc */
	unsigned int			saved_state;	/* Para rtlock waiters */
	
	/* Escalonamento */
	int				on_rq;
	int				prio;
	int				static_prio;
	int				normal_prio;
	unsigned int			rt_priority;
	
	struct sched_entity		se;
	struct sched_rt_entity		rt;
	struct sched_dl_entity		dl;
	const struct sched_class	*sched_class;
	
	/* CPU affinity */
	int				nr_cpus_allowed;
	const cpumask_t			*cpus_ptr;
	cpumask_t			cpus_mask;
	
	/* Migration */
	unsigned short			migration_disabled;
	int				migration_flags;
	
	/* Portas de comunicação da task */
	port_id_t			command_port;	/* Porta de comandos */
	port_id_t			message_port;	/* Porta de mensagens */
	port_id_t			notify_port;	/* Porta de notificações */
	
	/* Fila de mensagens pendentes */
	struct list_head		msg_queue;
	spinlock_t			msg_lock;
	
	/* Contexto de pilha */
	void				*stack;
	refcount_t			usage;
	
	/* Flags */
	unsigned int			flags;		/* PF_* flags */
	unsigned int			ptrace;
	
	/* CPU atual */
	int				on_cpu;
	int				wake_cpu;
	unsigned int			cpu;
	
	/* Estatísticas de execução */
	u64				utime;
	u64				stime;
	u64				gtime;
	unsigned long			nvcsw;
	unsigned long			nivcsw;
	u64				start_time;
	u64				start_boottime;
	
	/* Timers */
	struct timer_list		real_timer;
	struct hrtimer			dl_timer;
	
	/* Sinais (por task, mas proc compartilha handlers) */
	int				exit_state;
	int				exit_code;
	int				exit_signal;
	unsigned long			jobctl;
	
	/* Credenciais (cópia ou referência ao proc) */
	const struct cred __rcu		*cred;
	
	/* Sincronização */
	spinlock_t			alloc_lock;
	raw_spinlock_t			pi_lock;
	struct wake_q_node		wake_q;
	
	/* Para bloqueio em mutexes */
	struct mutex			*blocked_on;
	raw_spinlock_t			blocked_lock;
	
	/* RCU */
	struct rcu_head			rcu;
	refcount_t			rcu_users;
	
	/* Thread específico */
	struct task_struct __rcu	*real_parent;
	struct task_struct __rcu	*parent;
	struct list_head		children;
	struct list_head		sibling;
	struct task_struct		*group_leader;
	
	/* Pilha e contexto */
	struct restart_block		restart_block;
	struct thread_struct		thread;
	
	/* Nome da task */
	char				comm[TASK_COMM_LEN];
	
	/* Estatísticas de falhas */
	unsigned long			min_flt;
	unsigned long			maj_flt;
	
	/* Para debugging */
	unsigned long			last_switch_count;
	unsigned long			last_switch_time;
	
#ifdef CONFIG_TRACE_IRQFLAGS
	struct irqtrace_events		irqtrace;
#endif
	
#ifdef CONFIG_LOCKDEP
	u64				curr_chain_key;
	int				lockdep_depth;
	struct held_lock		held_locks[MAX_LOCK_DEPTH];
#endif
	
	/* --- campos randomizáveis --- */
	randomized_struct_fields_start
	
	/* Perf events */
#ifdef CONFIG_PERF_EVENTS
	struct perf_event_context	*perf_event_ctxp;
	struct mutex			perf_event_mutex;
	struct list_head		perf_event_list;
#endif
	
	/* BPF */
#ifdef CONFIG_BPF_SYSCALL
	struct bpf_local_storage __rcu	*bpf_storage;
	struct bpf_run_ctx		*bpf_ctx;
#endif
	
	randomized_struct_fields_end
} __attribute__((aligned(64)));

/* ============================================================================
 * FUNÇÕES PARA PROC_STRUCT (GERENCIAMENTO DE RECURSOS)
 * ============================================================================ */

/* Criar/destruir proc */
extern struct proc_struct *proc_create(void);
extern void proc_destroy(struct proc_struct *proc);
extern struct proc_struct *proc_get(struct proc_struct *proc);
extern void proc_put(struct proc_struct *proc);

/* Adicionar/remover task ao proc */
extern int proc_add_task(struct proc_struct *proc, struct task_struct *task);
extern void proc_remove_task(struct proc_struct *proc, struct task_struct *task);

/* Obter proc atual */
static inline struct proc_struct *current_proc(void)
{
	return current->proc;
}

/* ============================================================================
 * FUNÇÕES PARA TASK_STRUCT (GERENCIAMENTO DE EXECUÇÃO)
 * ============================================================================ */

/* Criar task associada a um proc */
extern struct task_struct *task_create(struct proc_struct *proc);
extern void task_destroy(struct task_struct *task);

/* Funções de escalonamento */
extern void schedule(void);
extern int wake_up_process(struct task_struct *task);
extern int wake_up_state(struct task_struct *task, unsigned int state);

/* Comunicação via portas */
extern int task_send_message(struct task_struct *task, port_id_t port, void *msg, size_t len);
extern int task_recv_message(struct task_struct *task, port_id_t port, void *buf, size_t len);

/* Obter task atual */
static inline struct task_struct *current_task(void)
{
	return current;
}

/* ============================================================================
 * CONSTANTES DE ESTADO DA TASK
 * ============================================================================ */

#define TASK_RUNNING			0x00000000
#define TASK_INTERRUPTIBLE		0x00000001
#define TASK_UNINTERRUPTIBLE		0x00000002
#define __TASK_STOPPED			0x00000004
#define __TASK_TRACED			0x00000008
#define EXIT_DEAD			0x00000010
#define EXIT_ZOMBIE			0x00000020
#define TASK_PARKED			0x00000040
#define TASK_DEAD			0x00000080
#define TASK_WAKEKILL			0x00000100
#define TASK_WAKING			0x00000200
#define TASK_NOLOAD			0x00000400
#define TASK_NEW			0x00000800
#define TASK_FREEZABLE			0x00001000
#define TASK_FROZEN			0x00002000

#define TASK_KILLABLE			(TASK_WAKEKILL | TASK_UNINTERRUPTIBLE)
#define TASK_STOPPED			(TASK_WAKEKILL | __TASK_STOPPED)
#define TASK_TRACED			__TASK_TRACED
#define TASK_IDLE			(TASK_UNINTERRUPTIBLE | TASK_NOLOAD)
#define TASK_NORMAL			(TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE)

/* ============================================================================
 * FLAGS DE PROCESSO (PF_*)
 * ============================================================================ */

#define PF_VCPU			0x00000001
#define PF_IDLE			0x00000002
#define PF_EXITING		0x00000004
#define PF_IO_WORKER		0x00000010
#define PF_WQ_WORKER		0x00000020
#define PF_FORKNOEXEC		0x00000040
#define PF_SUPERPRIV		0x00000100
#define PF_DUMPCORE		0x00000200
#define PF_SIGNALED		0x00000400
#define PF_MEMALLOC		0x00000800
#define PF_USED_MATH		0x00002000
#define PF_USER_WORKER		0x00004000
#define PF_NOFREEZE		0x00008000
#define PF_KTHREAD		0x00200000
#define PF_RANDOMIZE		0x00400000
#define PF_NO_SETAFFINITY	0x04000000
#define PF_BLOCK_TS		0x20000000

/* ============================================================================
 * FUNÇÕES AUXILIARES
 * ============================================================================ */

static inline unsigned int task_cpu(const struct task_struct *p)
{
	return READ_ONCE(p->cpu);
}

static inline int task_curr(const struct task_struct *p)
{
	return p->on_cpu;
}

static inline bool is_idle_task(const struct task_struct *p)
{
	return !!(p->flags & PF_IDLE);
}

static inline bool task_is_running(struct task_struct *p)
{
	return READ_ONCE(p->__state) == TASK_RUNNING;
}

static inline bool task_is_runnable(struct task_struct *p)
{
	return p->on_rq && !p->se.sched_delayed;
}

/* ============================================================================
 * MACROS PARA ACESSO ATUAL
 * ============================================================================ */

#define current_task()		current
#define current_proc()		(current->proc)

#endif /* _LINUX_SCHED_H */
