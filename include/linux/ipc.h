/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_IPC_H
#define _LINUX_IPC_H

#include <linux/spinlock_types.h>
#include <linux/uidgid.h>
#include <linux/rhashtable-types.h>
#include <uapi/linux/ipc.h>
#include <linux/refcount.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/rcupdate.h>

/* ============================================================================
 * CONSTANTES E TIPOS BÁSICOS
 * ============================================================================ */

/* Object ID - Identificador único de objeto (64-bit) */
typedef __u64 object_id_t;

/* Port ID - Identificador de porta */
typedef __u64 port_id_t;

/* Method ID - Identificador de método de objeto */
typedef __u64 method_id_t;

/* Capability - Token de capacidade */
typedef __u64 capability_t;

/* ============================================================================
 * TIPOS DE OBJETO
 * ============================================================================ */

#define OBJ_TYPE_GENERIC	1	/* Objeto genérico */
#define OBJ_TYPE_PORT		2	/* Porta */
#define OBJ_TYPE_SERVICE	3	/* Serviço RPC */
#define OBJ_TYPE_DEVICE		4	/* Dispositivo */
#define OBJ_TYPE_FILE		5	/* Arquivo (objeto filesystem) */
#define OBJ_TYPE_DIRECTORY	6	/* Diretório */
#define OBJ_TYPE_SHM		7	/* Objeto de memória compartilhada */
#define OBJ_TYPE_TTY		8	/* TTY (Ring 0) */
#define OBJ_TYPE_DRIVE		9	/* Unidade de disco (C:\) */
#define OBJ_TYPE_PIPE		10	/* Pipe (deprecado - use portas) */
#define OBJ_TYPE_SOCKET		11	/* Socket (deprecado - use portas) */
#define OBJ_TYPE_MESSAGE	12	/* Mensagem */
#define OBJ_TYPE_BUFFER		13	/* Buffer de dados */
#define OBJ_TYPE_STREAM		14	/* Stream de dados */

/* ============================================================================
 * FLAGS DE OBJETO
 * ============================================================================ */

#define OBJ_FLAG_PERSISTENT	(1 << 0)	/* Objeto persistente */
#define OBJ_FLAG_SYSTEM		(1 << 1)	/* Objeto de sistema */
#define OBJ_FLAG_VOLATILE	(1 << 2)	/* Objeto volátil */
#define OBJ_FLAG_REMOTE		(1 << 3)	/* Objeto remoto */
#define OBJ_FLAG_LOCAL		(1 << 4)	/* Objeto local */
#define OBJ_FLAG_RING0		(1 << 5)	/* Opera em Ring 0 */
#define OBJ_FLAG_SECURE		(1 << 6)	/* Objeto seguro */
#define OBJ_FLAG_ENCRYPTED	(1 << 7)	/* Objeto criptografado */
#define OBJ_FLAG_ZEROCOPY	(1 << 8)	/* Zero-copy habilitado */

/* ============================================================================
 * ESTRUTURA BASE PARA PERMISSÕES DE OBJETOS (KERNEL)
 * ============================================================================ */

/**
 * struct kernel_object_perm - Permissões de objeto no kernel
 * 
 * Substitui struct kern_ipc_perm. Gerencia permissões e metadados
 * de objetos no sistema de objetos.
 */
struct kernel_object_perm {
	spinlock_t		lock;		/* Lock para acesso concorrente */
	bool			deleted;	/* Objeto marcado para deleção */
	int			id;		/* ID interno do objeto */
	object_id_t		oid;		/* Object ID (64-bit) */
	key_t			key;		/* Chave IPC (compatibilidade) */
	kuid_t			uid;		/* UID do proprietário */
	kgid_t			gid;		/* GID do proprietário */
	kuid_t			cuid;		/* UID do criador */
	kgid_t			cgid;		/* GID do criador */
	umode_t			mode;		/* Modo de permissão */
	unsigned long		seq;		/* Número de sequência */
	void			*security;	/* Dados de segurança (LSM) */
	
	/* Campos específicos do modelo de objetos */
	capability_t		base_cap;	/* Capacidade base do objeto */
	__u32			object_type;	/* Tipo do objeto (OBJ_TYPE_*) */
	__u32			flags;		/* OBJ_FLAG_* */
	__u64			parent_oid;	/* OID do objeto pai (0 = none) */
	__u64			port_id;	/* Porta associada (se houver) */
	
	/* Estatísticas */
	__u64			created_at;	/* Timestamp de criação */
	__u64			accessed_at;	/* Último acesso */
	__u64			ops_count;	/* Contador de operações */
	
	struct rhash_head	khtnode;	/* Hash table node */
	struct rcu_head		rcu;		/* RCU para deleção segura */
	refcount_t		refcount;	/* Contagem de referências */
} ____cacheline_aligned_in_smp __randomize_layout;

/* ============================================================================
 * ESTADOS DE PORTA
 * ============================================================================ */

#define PORT_STATE_CLOSED	0	/* Porta fechada */
#define PORT_STATE_OPEN		1	/* Porta aberta */
#define PORT_STATE_LISTENING	2	/* Porta em listening */
#define PORT_STATE_DRAINING	3	/* Drenando mensagens */
#define PORT_STATE_ERROR	4	/* Estado de erro */
#define PORT_STATE_BOUND	5	/* Vinculada a objeto */

/* ============================================================================
 * FLAGS DE PORTA (KERNEL)
 * ============================================================================ */

#define PORT_KERNEL_FLAG_BLOCKING	(1 << 0)	/* Operações bloqueantes */
#define PORT_KERNEL_FLAG_NONBLOCK	(1 << 1)	/* Operações não-bloqueantes */
#define PORT_KERNEL_FLAG_PRIORITY	(1 << 2)	/* Fila com prioridade */
#define PORT_KERNEL_FLAG_SECURE		(1 << 3)	/* Porta segura */
#define PORT_KERNEL_FLAG_AUDIT		(1 << 4)	/* Auditoria habilitada */
#define PORT_KERNEL_FLAG_BROADCAST	(1 << 5)	/* Broadcast habilitado */
#define PORT_KERNEL_FLAG_ZEROCOPY	(1 << 6)	/* Zero-copy habilitado */

/* ============================================================================
 * ESTRUTURA PARA PORTAS (KERNEL)
 * ============================================================================ */

/**
 * struct kernel_port_perm - Permissões de porta no kernel
 * 
 * Portas são objetos especiais que gerenciam filas de mensagens.
 */
struct kernel_port_perm {
	spinlock_t		lock;		/* Lock da porta */
	bool			deleted;	/* Porta marcada para deleção */
	port_id_t		port_id;	/* ID da porta */
	object_id_t		object_oid;	/* Objeto associado (0 = none) */
	key_t			key;		/* Chave IPC (compatibilidade) */
	kuid_t			uid;		/* UID do proprietário */
	kgid_t			gid;		/* GID do proprietário */
	kuid_t			cuid;		/* UID do criador */
	kgid_t			cgid;		/* GID do criador */
	umode_t			mode;		/* Modo de permissão */
	unsigned long		seq;		/* Número de sequência */
	void			*security;	/* Dados de segurança */
	capability_t		required_cap;	/* Capacidade necessária */
	
	/* Fila de mensagens */
	struct list_head	msg_queue;	/* Lista de mensagens na fila */
	__u64			queue_depth;	/* Número de mensagens */
	__u64			max_queue;	/* Tamanho máximo da fila */
	__u64			queue_bytes;	/* Bytes totais na fila */
	__u64			max_bytes;	/* Máximo de bytes na fila */
	
	/* Bindings */
	struct list_head	bindings;	/* Objetos vinculados à porta */
	__u32			num_bindings;	/* Número de bindings */
	
	/* Estatísticas */
	__u64			msg_sent;	/* Mensagens enviadas */
	__u64			msg_recv;	/* Mensagens recebidas */
	__u64			bytes_sent;	/* Bytes enviados */
	__u64			bytes_recv;	/* Bytes recebidos */
	__u64			last_msg_at;	/* Última mensagem */
	
	/* Estado e flags */
	__u32			state;		/* PORT_STATE_* */
	__u32			flags;		/* PORT_KERNEL_FLAG_* */
	
	struct rhash_head	khtnode;
	struct rcu_head		rcu;
	refcount_t		refcount;
} ____cacheline_aligned_in_smp __randomize_layout;

/* ============================================================================
 * FLAGS DE BINDING
 * ============================================================================ */

#define BINDING_FLAG_SEND	(1 << 0)	/* Pode enviar mensagens */
#define BINDING_FLAG_RECV	(1 << 1)	/* Pode receber mensagens */
#define BINDING_FLAG_EXCLUSIVE	(1 << 2)	/* Binding exclusivo */
#define BINDING_FLAG_ASYNC	(1 << 3)	/* Comunicação assíncrona */
#define BINDING_FLAG_SECURE	(1 << 4)	/* Canal seguro */
#define BINDING_FLAG_ZEROCOPY	(1 << 5)	/* Zero-copy habilitado */

/* ============================================================================
 * ESTRUTURA PARA BINDINGS (CONEXÕES OBJETO-PORTA)
 * ============================================================================ */

/**
 * struct kernel_binding - Binding entre objeto e porta
 * 
 * Representa uma conexão onde um objeto se vincula a uma porta
 * para enviar/receber mensagens.
 */
struct kernel_binding {
	spinlock_t		lock;
	object_id_t		object_id;	/* Objeto vinculado */
	port_id_t		port_id;	/* Porta de destino */
	capability_t		capability;	/* Capacidade do binding */
	__u32			flags;		/* BINDING_FLAG_* */
	__u32			ref_count;	/* Referências */
	
	/* Fila de mensagens pendentes para este binding */
	struct list_head	pending_msgs;
	__u64			pending_count;
	
	/* Estatísticas */
	__u64			msgs_sent;
	__u64			msgs_recv;
	__u64			bytes_sent;
	__u64			bytes_recv;
	
	struct list_head	binding_list;	/* Lista no objeto */
	struct list_head	port_list;	/* Lista na porta */
	struct rcu_head		rcu;
};

/* ============================================================================
 * TIPOS DE MENSAGEM
 * ============================================================================ */

#define MSG_TYPE_REQUEST	1	/* Requisição (chamada de método) */
#define MSG_TYPE_RESPONSE	2	/* Resposta */
#define MSG_TYPE_NOTIFICATION	3	/* Notificação (evento) */
#define MSG_TYPE_BROADCAST	4	/* Broadcast */
#define MSG_TYPE_DATA		5	/* Dados puros */
#define MSG_TYPE_CONTROL	6	/* Mensagem de controle */
#define MSG_TYPE_ERROR		7	/* Mensagem de erro */

/* ============================================================================
 * FLAGS DE MENSAGEM
 * ============================================================================ */

#define MSG_FLAG_ASYNC		(1 << 0)	/* Mensagem assíncrona */
#define MSG_FLAG_NO_REPLY	(1 << 1)	/* Não espera resposta */
#define MSG_FLAG_PEEK		(1 << 2)	/* Visualizar sem consumir */
#define MSG_FLAG_DISCARD	(1 << 3)	/* Descartar após leitura */
#define MSG_FLAG_ENCRYPTED	(1 << 4)	/* Payload criptografado */
#define MSG_FLAG_SIGNED		(1 << 5)	/* Payload assinado */
#define MSG_FLAG_ZEROCOPY	(1 << 6)	/* Zero-copy transfer */
#define MSG_FLAG_BROADCAST	(1 << 7)	/* Broadcast para múltiplos */
#define MSG_FLAG_HIGH_PRIORITY	(1 << 8)	/* Alta prioridade */

/* ============================================================================
 * ESTRUTURA PARA MENSAGENS (KERNEL)
 * ============================================================================ */

/**
 * struct kernel_message - Mensagem no kernel
 * 
 * Representa uma mensagem trafegando entre objetos via portas.
 */
struct kernel_message {
	__u64			msg_id;		/* ID único da mensagem */
	object_id_t		sender;		/* Objeto remetente */
	object_id_t		target;		/* Objeto destinatário */
	port_id_t		port_id;	/* Porta de origem/destino */
	method_id_t		method;		/* Método a invocar */
	capability_t		capability;	/* Capacidade da mensagem */
	
	__u32			msg_type;	/* MSG_TYPE_* */
	__u32			priority;	/* Prioridade (0-255) */
	__u32			flags;		/* MSG_FLAG_* */
	__u32			payload_size;	/* Tamanho do payload */
	
	void			*payload;	/* Dados da mensagem */
	
	/* Para zero-copy */
	__u64			buffer_oid;	/* OID do buffer (zero-copy) */
	__u64			buffer_offset;	/* Offset no buffer */
	
	/* Temporização */
	__u64			deadline_ns;	/* Deadline absoluta */
	__u64			created_at;	/* Timestamp de criação */
	__u64			sent_at;	/* Timestamp de envio */
	__u64			delivered_at;	/* Timestamp de entrega */
	
	struct list_head	queue_list;	/* Lista na fila da porta */
	struct list_head	binding_list;	/* Lista no binding */
	struct rcu_head		rcu;
};

/* ============================================================================
 * FUNÇÕES AUXILIARES (MACROS)
 * ============================================================================ */

/* Converter entre IPC perm e Object perm */
#define IPC_TO_OBJECT(ipcp)	container_of(ipcp, struct kernel_object_perm, id)

/* Verificar se objeto tem capacidade */
#define object_has_capability(obj, cap)	((obj)->base_cap & (cap))

/* Verificar se binding tem permissão */
#define binding_can_send(bind)	((bind)->flags & BINDING_FLAG_SEND)
#define binding_can_recv(bind)	((bind)->flags & BINDING_FLAG_RECV)

/* Verificar estado da porta */
#define port_is_open(port)	((port)->state == PORT_STATE_OPEN)
#define port_is_listening(port)	((port)->state == PORT_STATE_LISTENING)
#define port_is_bound(port)	((port)->state == PORT_STATE_BOUND)

/* ============================================================================
 * DECLARAÇÕES DE FUNÇÕES
 * ============================================================================ */

/* Operações com objetos */
extern struct kernel_object_perm *object_create(object_id_t oid, __u32 type);
extern void object_destroy(struct kernel_object_perm *obj);
extern struct kernel_object_perm *object_lookup(object_id_t oid);
extern int object_set_capability(struct kernel_object_perm *obj, capability_t cap);

/* Operações com portas */
extern struct kernel_port_perm *port_create(port_id_t port_id, __u32 flags);
extern void port_destroy(struct kernel_port_perm *port);
extern int port_bind_object(struct kernel_port_perm *port, object_id_t oid);
extern int port_bind_process(struct kernel_port_perm *port, struct task_struct *task);

/* Operações com bindings */
extern struct kernel_binding *binding_create(object_id_t obj_id, port_id_t port_id);
extern void binding_destroy(struct kernel_binding *binding);
extern int binding_set_flags(struct kernel_binding *binding, __u32 flags);

/* Operações com mensagens */
extern struct kernel_message *message_alloc(__u32 payload_size);
extern void message_free(struct kernel_message *msg);
extern int message_send(struct kernel_message *msg, port_id_t port_id);
extern struct kernel_message *message_recv(port_id_t port_id, __u32 flags);
extern int message_broadcast(struct kernel_message *msg, port_id_t *ports, __u32 num_ports);

/* ============================================================================
 * ESTRUTURAS LEGADAS (MANTIDAS PARA COMPATIBILIDADE)
 * ============================================================================ */

/*
 * struct kern_ipc_perm - Estrutura legada (NÃO USE!)
 * 
 * Mantida apenas para compatibilidade com módulos kernel antigos.
 * TODO: Remover após todos os módulos forem portados.
 */
struct kern_ipc_perm {
	spinlock_t	lock;
	bool		deleted;
	int		id;
	key_t		key;
	kuid_t		uid;
	kgid_t		gid;
	kuid_t		cuid;
	kgid_t		cgid;
	umode_t		mode;
	unsigned long	seq;
	void		*security;

	struct rhash_head khtnode;

	struct rcu_head rcu;
	refcount_t refcount;
} ____cacheline_aligned_in_smp __randomize_layout;

#endif /* _LINUX_IPC_H */
