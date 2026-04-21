/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_IPC_H
#define _UAPI_LINUX_IPC_H

#include <linux/types.h>

#define IPC_PRIVATE ((__kernel_key_t) 0)  

/* Estrutura para permissões de portas/RPC */
struct ipc_perm
{
	__kernel_key_t	key;
	__kernel_uid_t	uid;
	__kernel_gid_t	gid;
	__kernel_uid_t	cuid;
	__kernel_gid_t	cgid;
	__kernel_mode_t	mode; 
	unsigned short	seq;
};

/* Include the definition of ipc64_perm */
#include <asm/ipcbuf.h>

/* Flags para requisição de recursos (Portas/RPC) */
#define IPC_CREAT  00001000   /* criar porta se não existir */
#define IPC_EXCL   00002000   /* falhar se porta já existir */
#define IPC_NOWAIT 00004000   /* retornar erro se não disponível */

/* Campos específicos para RPC distribuído */
#define IPC_RPC    00010000  /* comunicação RPC */
#define IPC_PORT   00020000  /* comunicação via portas */

/* 
 * Comandos de controle para RPC e Portas
 */
#define IPC_RMID 0     /* remover porta/recurso */
#define IPC_SET  1     /* definir opções ipc_perm */
#define IPC_STAT 2     /* obter opções ipc_perm */
#define IPC_INFO 3     /* ver informações do sistema RPC */

/*
 * Version flags para comandos RPC
 */
#define IPC_OLD 0	/* Versão antiga */
#define IPC_64  0x0100  /* Nova versão (suporte a 32-bit UIDs, mensagens maiores) */

/*
 * Estrutura para chamadas RPC (Remote Procedure Call)
 */
struct rpc_message {
	__u32 procedure_id;    /* ID do procedimento remoto */
	__u32 version;         /* Versão da interface */
	void __user *params;   /* Parâmetros da chamada */
	size_t params_size;    /* Tamanho dos parâmetros */
	void __user *results;  /* Resultados da chamada */
	size_t results_size;   /* Tamanho dos resultados */
	__u32 flags;           /* Flags da chamada RPC */
};

/*
 * Estrutura para comunicação por portas (message passing)
 */
struct port_message {
	__u32 port_id;         /* ID da porta destino/origem */
	__u32 msg_type;        /* Tipo da mensagem */
	void __user *data;     /* Dados da mensagem */
	size_t data_size;      /* Tamanho dos dados */
	__u32 priority;        /* Prioridade da mensagem */
	__u32 flags;           /* Flags da mensagem */
};

/*
 * Estrutura auxiliar para chamadas de sistema RPC
 */
struct ipc_kludge {
	union {
		struct rpc_message __user *rpc_msg;
		struct port_message __user *port_msg;
		void __user *raw_msg;
	} msg;
	long msgtyp;
};

/* Comandos para RPC (Remote Procedure Call) */
#define RPC_CALL		101   /* Executar chamada RPC */
#define RPC_BIND		102   /* Vincular procedimento a porta */
#define RPC_UNBIND		103   /* Desvincular procedimento */
#define RPC_REGISTER		104   /* Registrar serviço RPC */
#define RPC_UNREGISTER		105   /* Desregistrar serviço */
#define RPC_GETPORT		106   /* Obter porta de serviço */

/* Comandos para Portas (message passing) */
#define PORT_CREATE		201   /* Criar porta */
#define PORT_DESTROY		202   /* Destruir porta */
#define PORT_SEND		203   /* Enviar mensagem */
#define PORT_RECV		204   /* Receber mensagem */
#define PORT_PEEK		205   /* Visualizar mensagem sem consumir */
#define PORT_BIND		206   /* Vincular processo à porta */
#define PORT_UNBIND		207   /* Desvincular processo da porta */
#define PORT_QUERY		208   /* Consultar status da porta */

/* Comandos antigos mantidos para compatibilidade (descontinuados para RPC) */
#define SEMOP		 1   /* Descontinuado - usar RPC */
#define SEMGET		 2   /* Descontinuado - usar RPC */
#define SEMCTL		 3   /* Descontinuado - usar RPC */
#define SEMTIMEDOP	 4   /* Descontinuado - usar RPC */
#define MSGSND		11   /* Descontinuado - usar PORT_SEND */
#define MSGRCV		12   /* Descontinuado - usar PORT_RECV */
#define MSGGET		13   /* Descontinuado - usar PORT_CREATE */
#define MSGCTL		14   /* Descontinuado - usar PORT_* */
#define SHMAT		21   /* Descontinuado - usar RPC */
#define SHMDT		22   /* Descontinuado - usar RPC */
#define SHMGET		23   /* Descontinuado - usar RPC */
#define SHMCTL		24   /* Descontinuado - usar RPC */

/* Comandos específicos do novo modelo RPC/Portas */
#define RPC_SVC		    301   /* Serviço RPC */
#define PORT_SVC	    302   /* Serviço de portas */
#define RPC_ASYNC_CALL	    303   /* Chamada RPC assíncrona */
#define PORT_BROADCAST	    304   /* Broadcast para múltiplas portas */
#define PORT_SELECT	    305   /* Seleção de portas (similar a select/poll) */

/* Flags para mensagens de porta */
#define PORT_MSG_NOWAIT	    0x0001  /* Não esperar se porta ocupada */
#define PORT_MSG_PRIORITY   0x0002  /* Usar prioridade */
#define PORT_MSG_PEEK	    0x0004  /* Visualizar mensagem */
#define PORT_MSG_DISCARD    0x0008  /* Descartar mensagem após leitura */

/* Flags para chamadas RPC */
#define RPC_WAIT	    0x0001  /* Aguardar resposta */
#define RPC_ASYNC	    0x0002  /* Chamada assíncrona */
#define RPC_BROADCAST	    0x0004  /* Broadcast para todos servidores */
#define RPC_SECURE	    0x0008  /* Usar autenticação segura */

/* Macro para construir chamadas IPCCALL (mantida para compatibilidade) */
#define IPCCALL(version,op)	((version)<<16 | (op))

#endif /* _UAPI_LINUX_IPC_H */
