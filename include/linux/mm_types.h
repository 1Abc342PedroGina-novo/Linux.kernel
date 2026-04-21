#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

/*
 * ============================================================================
 * MM_TYPES MODIFICADO PARA OBJETOS DE MEMÓRIA E PORTAS
 * ============================================================================
 * 
 * AVISO: GESTÃO DE MEMÓRIA MODIFICADA PARA O MODELO "TUDO É OBJETO"
 * 
 * NÃO EXISTE MAIS O CONCEITO TRADICIONAL DE PÁGINAS COMO UNIDADE BÁSICA!
 * 
 * AGORA USAMOS:
 *   - Memory Objects (objetos de memória com OID)
 *   - Data Ports (portas para transferência de dados)
 *   - Zero-copy messaging (mensagens sem cópia)
 *   - Shared Memory Objects (substituto de shm)
 * 
 * ARQUIVOS SÃO OBJETOS DE MENSAGEM:
 *   - Arquivos mapeados em memória são objetos
 *   - Comunicação via portas de mensagem
 * 
 * FILESYSTEMS EM RING 0:
 *   ✓ NFS, EXFAT, FAT, NTFS, HFS, HFS+
 * 
 * ============================================================================
 */

#include <linux/mm_types_task.h>
#include <linux/auxvec.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rbtree.h>
#include <linux/maple_tree.h>
#include <linux/rwsem.h>
#include <linux/completion.h>
#include <linux/cpumask.h>
#include <linux/uprobes.h>
#include <linux/rcupdate.h>
#include <linux/page-flags-layout.h>
#include <linux/workqueue.h>
#include <linux/seqlock.h>
#include <linux/percpu_counter.h>
#include <linux/types.h>
#include <linux/rseq_types.h>
#include <linux/bitmap.h>

#include <asm/mmu.h>

#ifndef AT_VECTOR_SIZE_ARCH
#define AT_VECTOR_SIZE_ARCH 0
#endif
#define AT_VECTOR_SIZE (2*(AT_VECTOR_SIZE_ARCH + AT_VECTOR_SIZE_BASE + 1))

/* ============================================================================
 * OBJETOS DE MEMÓRIA (MEMORY OBJECTS)
 * ============================================================================ */

/**
 * struct memory_object - Representa um objeto de memória
 * 
 * Substitui o conceito de "páginas" como unidade básica.
 * Cada objeto de memória tem:
 *   - Object ID (OID) único
 *   - Porta de dados associada
 *   - Capacidades de acesso
 */
struct memory_object {
	__u64			oid;		/* Object ID */
	__u64			port_id;	/* Porta de dados */
	__u64			size;		/* Tamanho em bytes */
	__u64			offset;		/* Offset no objeto pai */
	__u32			flags;		/* MEM_OBJ_FLAG_* */
	__u32			ref_count;	/* Contagem de referências */
	
	/* Dados do objeto */
	void			*data;		/* Ponteiro para dados (se residente) */
	__u64			data_size;	/* Tamanho dos dados */
	
	/* Para objetos compostos (arquivos mapeados) */
	__u64			file_oid;	/* OID do arquivo associado */
	__u64			file_offset;	/* Offset no arquivo */
	
	/* Capacidades */
	__u64			capability;	/* Capacidade base */
	__u64			owner_oid;	/* OID do proprietário */
	
	/* Sincronização */
	spinlock_t		lock;
	struct rcu_head		rcu;
};

#define MEM_OBJ_FLAG_READONLY	(1 << 0)	/* Somente leitura */
#define MEM_OBJ_FLAG_SHARED	(1 << 1)	/* Compartilhado */
#define MEM_OBJ_FLAG_PRIVATE	(1 << 2)	/* Privado (COW) */
#define MEM_OBJ_FLAG_ANONYMOUS	(1 << 3)	/* Memória anônima */
#define MEM_OBJ_FLAG_LOCKED	(1 << 4)	/* Páginas lockadas */
#define MEM_OBJ_FLAG_HUGEPAGE	(1 << 5)	/* Usa huge pages */
#define MEM_OBJ_FLAG_ZEROCOPY	(1 << 6)	/* Zero-copy habilitado */
#define MEM_OBJ_FLAG_ENCRYPTED	(1 << 7)	/* Criptografado */

/* ============================================================================
 * PORTAS DE DADOS (DATA PORTS)
 * ============================================================================ */

/**
 * struct data_port - Porta para transferência de dados
 * 
 * Substitui mecanismos tradicionais de I/O.
 * Dados trafegam como mensagens entre objetos.
 */
struct data_port {
	__u64			port_id;	/* ID da porta */
	__u64			object_oid;	/* Objeto associado */
	__u64			offset;		/* Offset atual */
	__u64			size;		/* Tamanho da região */
	__u32			flags;		/* DATA_PORT_FLAG_* */
	__u32			state;		/* DATA_PORT_STATE_* */
	
	/* Fila de mensagens */
	struct list_head	msg_queue;
	spinlock_t		msg_lock;
	
	/* Estatísticas */
	__u64			bytes_read;
	__u64			bytes_written;
	__u64			last_access;
};

#define DATA_PORT_FLAG_READ	(1 << 0)	/* Leitura habilitada */
#define DATA_PORT_FLAG_WRITE	(1 << 1)	/* Escrita habilitada */
#define DATA_PORT_FLAG_NONBLOCK	(1 << 2)	/* Modo não-bloqueante */
#define DATA_PORT_FLAG_ASYNC	(1 << 3)	/* I/O assíncrono */

#define DATA_PORT_STATE_CLOSED	0
#define DATA_PORT_STATE_OPEN	1
#define DATA_PORT_STATE_STREAM	2		/* Modo streaming */
#define DATA_PORT_STATE_ERROR	3

/* ============================================================================
 * MENSAGENS DE TRANSFERÊNCIA DE DADOS
 * ============================================================================ */

/**
 * struct data_message - Mensagem para transferência de dados
 */
struct data_message {
	__u64			msg_id;		/* ID da mensagem */
	__u64			src_port;	/* Porta origem */
	__u64			dst_port;	/* Porta destino */
	__u64			offset;		/* Offset no objeto */
	__u64			length;		/* Comprimento dos dados */
	__u32			flags;		/* DATA_MSG_FLAG_* */
	__u32			priority;	/* Prioridade */
	
	/* Dados (podem ser inline ou referência) */
	union {
		__u8		data[64];	/* Dados pequenos (inline) */
		struct {
			__u64	buffer_oid;	/* OID do buffer para dados grandes */
			__u64	buffer_offset;
		};
	};
	
	/* Para zero-copy */
	__u64			src_object;	/* Objeto fonte */
	__u64			dst_object;	/* Objeto destino */
};

#define DATA_MSG_FLAG_ASYNC	(1 << 0)	/* Mensagem assíncrona */
#define DATA_MSG_FLAG_ZEROCOPY	(1 << 1)	/* Transferência zero-copy */
#define DATA_MSG_FLAG_WAITALL	(1 << 2)	/* Esperar todos os dados */
#define DATA_MSG_FLAG_PEEK	(1 << 3)	/* Visualizar sem consumir */

/* ============================================================================
 * MEMORY OBJECT MANAGEMENT (SUBSTITUTO DO PAGE CACHE)
 * ============================================================================ */

/**
 * struct memory_object_cache - Cache de objetos de memória
 * 
 * Substitui o page cache tradicional.
 */
struct memory_object_cache {
	struct xarray		objects;	/* Tabela de objetos */
	__u64			total_size;	/* Tamanho total */
	__u64			used_size;	/* Espaço usado */
	__u32			flags;		/* CACHE_FLAG_* */
	spinlock_t		lock;
	
	/* Estatísticas */
	__u64			hit_count;
	__u64			miss_count;
	__u64			evict_count;
};

#define CACHE_FLAG_WRITEBACK	(1 << 0)	/* Write-back cache */
#define CACHE_FLAG_WRITETHROUGH	(1 << 1)	/* Write-through */
#define CACHE_FLAG_ENCRYPTED	(1 << 2)	/* Cache criptografado */

/* ============================================================================
 * SHARED MEMORY OBJECTS (SUBSTITUTO DO SISTEMA SHM)
 * ============================================================================ */

/**
 * struct shared_memory_object - Objeto de memória compartilhada
 * 
 * Substitui o sistema IPC shmget/shmat tradicional.
 */
struct shared_memory_object {
	struct memory_object	base;		/* Objeto base */
	__u64			key;		/* Chave IPC (compatibilidade) */
	__u32			attach_count;	/* Número de attaches */
	__u32			flags;		/* SHM_FLAG_* */
	
	/* Lista de portas vinculadas */
	struct list_head	ports;
	struct list_head	objects;
};

#define SHM_FLAG_HUGETLB	(1 << 0)	/* Usa huge pages */
#define SHM_FLAG_NORESERVE	(1 << 1)	/* Sem reserva */
#define SHM_FLAG_PERSISTENT	(1 << 2)	/* Persistente */

/* ============================================================================
 * VMA (VIRTUAL MEMORY AREA) MODIFICADO
 * ============================================================================ */

/**
 * struct vm_area_struct - Área de memória virtual modificada
 * 
 * Agora com suporte a objetos de memória e portas.
 */
struct vm_area_struct {
	/* Campos básicos */
	unsigned long		vm_start;
	unsigned long		vm_end;
	struct mm_struct	*vm_mm;
	
	/* Objeto de memória associado */
	struct memory_object	*vm_object;	/* Objeto de memória */
	__u64			vm_object_offset; /* Offset no objeto */
	
	/* Porta de dados associada */
	__u64			vm_port_id;	/* Porta para I/O */
	
	/* Permissões e flags */
	pgprot_t		vm_page_prot;
	union {
		const vm_flags_t vm_flags;
		vma_flags_t flags;
	};
	
	/* Para arquivos mapeados (arquivos são objetos) */
	struct file_object	*vm_file_object;
	__u64			vm_file_oid;
	
	/* Listas e árvores */
	struct list_head	anon_vma_chain;
	struct anon_vma		*anon_vma;
	const struct vm_operations_struct *vm_ops;
	
	/* Para memória compartilhada */
	struct shared_memory_object *vm_shm_object;
	
#ifdef CONFIG_NUMA
	struct mempolicy	*vm_policy;
#endif
#ifdef CONFIG_NUMA_BALANCING
	struct vma_numab_state	*numab_state;
#endif
	
	/* Capacidades */
	__u64			required_capability;
};

/* ============================================================================
 * MM_STRUCT MODIFICADO
 * ============================================================================ */

/**
 * struct mm_struct - Descritor de espaço de endereços modificado
 * 
 * Agora com suporte a objetos de memória e portas.
 */
struct mm_struct {
	/* Contagem de referências */
	atomic_t		mm_count;
	atomic_t		mm_users;
	
	/* Árvore de objetos de memória (substitui maple tree de VMAs) */
	struct maple_tree	mm_mt;		/* Árvore de objetos */
	
	/* Portas padrão do processo */
	__u64			default_data_port;	/* Porta de dados padrão */
	__u64			default_msg_port;	/* Porta de mensagens padrão */
	
	/* Tabela de objetos de memória */
	struct xarray		memory_objects;	/* Objetos de memória */
	
	/* Cache de objetos */
	struct memory_object_cache *object_cache;
	
	/* Estatísticas de memória (mantidas para compatibilidade) */
	unsigned long		total_vm;
	unsigned long		locked_vm;
	atomic64_t		pinned_vm;
	
	/* Contexto do arquitetura */
	mm_context_t		context;
	
	/* Permissões e capacidades */
	mm_flags_t		flags;
	__u64			capability_mask;
	
	/* Para memória compartilhada (IPC) */
	spinlock_t		shm_lock;
	struct list_head	shm_objects;
	
#ifdef CONFIG_MMU
	pgd_t			*pgd;
	atomic_long_t		pgtables_bytes;
#endif
	
	/* Sincronização */
	spinlock_t		page_table_lock;
	struct rw_semaphore	mmap_lock;
	
	/* Outros campos (mantidos para compatibilidade) */
	struct task_struct __rcu *owner;
	struct user_namespace	*user_ns;
	struct file __rcu	*exe_file;
	
	/* Espaço flexível para dados específicos da arquitetura */
	char			flexible_array[] __aligned(__alignof__(unsigned long));
};

/* ============================================================================
 * FUNÇÕES AUXILIARES
 * ============================================================================ */

/* Criar objeto de memória */
extern struct memory_object *memory_object_create(__u64 size, __u32 flags);
extern void memory_object_destroy(struct memory_object *obj);
extern int memory_object_resize(struct memory_object *obj, __u64 new_size);

/* Operações com portas de dados */
extern __u64 data_port_create(__u64 object_oid, __u32 flags);
extern int data_port_destroy(__u64 port_id);
extern ssize_t data_port_read(__u64 port_id, void *buf, size_t count);
extern ssize_t data_port_write(__u64 port_id, const void *buf, size_t count);
extern int data_port_send_message(__u64 port_id, struct data_message *msg);

/* Zero-copy transferência */
extern int zero_copy_transfer(__u64 src_oid, __u64 src_offset,
			      __u64 dst_oid, __u64 dst_offset,
			      __u64 length, __u32 flags);

/* Compartilhamento de memória (substituto de shm) */
extern __u64 shm_object_create(__u64 key, __u64 size, __u32 flags);
extern int shm_object_attach(__u64 shm_oid, __u64 *port_id);
extern int shm_object_detach(__u64 shm_oid, __u64 port_id);

/* ============================================================================
 * ESTRUTURAS LEGADAS (MANTIDAS PARA COMPATIBILIDADE)
 * ============================================================================ */

/*
 * struct page - Mantido apenas para compatibilidade com drivers antigos.
 * NÃO USE EM NOVO CÓDIGO! Use struct memory_object.
 */
struct page {
	unsigned long flags;
	union {
		struct list_head lru;
		struct list_head buddy_list;
	};
	struct address_space *mapping;
	unsigned long private;
	atomic_t _mapcount;
	atomic_t _refcount;
#ifdef CONFIG_MEMCG
	unsigned long memcg_data;
#endif
} __attribute__((aligned(sizeof(unsigned long))));

/*
 * struct folio - Mantido para compatibilidade.
 * Use struct memory_object em vez disso.
 */
struct folio {
	struct page page;
};

#endif
