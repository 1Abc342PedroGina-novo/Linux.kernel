/* SPDX-License-Identifier: ((GPL-2.0 WITH Linux-syscall-note) OR BSD-2-Clause) */
/*
    This file defines the kernel interface of FUSE
    Copyright (C) 21/04/2026 1Abc342PedroGina-novo
    Copyright (C) 10/12/2025 1Abc342PedroGina-novo
    Copyright (C) 2001-2008  Miklos Szeredi <miklos@szeredi.hu>

    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.

    This -- and only this -- header file may also be distributed under
    the terms of the BSD Licence as follows:

    Copyright (C) 2001-2007 Miklos Szeredi. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#ifndef _UAPI_LINUX_FUSE_H
#define _UAPI_LINUX_FUSE_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/* ============================================================================
 * VERSÃO DO PROTOCOLO
 * ============================================================================ */

#define RPC_OBJECT_VERSION_MAJOR 1
#define RPC_OBJECT_VERSION_MINOR 0

/* ID raiz do sistema de objetos */
#define ROOT_OBJECT_ID 1

/* ============================================================================
 * TIPOS BÁSICOS - OBJETOS E MENSAGENS
 * ============================================================================ */

/**
 * Object ID - Identificador único de um objeto no sistema
 */
typedef uint64_t object_id_t;

/**
 * Method ID - Identificador de um método dentro de um objeto
 */
typedef uint64_t method_id_t;

/**
 * Port ID - Identificador de uma porta de comunicação
 */
typedef uint64_t port_id_t;

/**
 * Capability - Token de capacidade para controle de acesso
 */
typedef uint64_t capability_t;

/**
 * Message Type - Tipos de mensagens
 */
enum message_type {
	MSG_TYPE_REQUEST	= 1,  /* Requisição (chamada de método) */
	MSG_TYPE_RESPONSE	= 2,  /* Resposta (retorno do método) */
	MSG_TYPE_NOTIFICATION	= 3,  /* Notificação (evento) */
	MSG_TYPE_BROADCAST	= 4,  /* Broadcast para múltiplos objetos */
};

/**
 * Message Priority - Prioridade da mensagem
 */
enum message_priority {
	MSG_PRIO_LOW		= 0,
	MSG_PRIO_NORMAL		= 1,
	MSG_PRIO_HIGH		= 2,
	MSG_PRIO_URGENT		= 3,
};

/* ============================================================================
 * ESTRUTURA BASE DE MENSAGEM
 * ============================================================================ */

/**
 * struct message_header - Cabeçalho base para todas as mensagens
 * 
 * Toda comunicação é uma MENSAGEM. Não existem arquivos, pipes ou sinais.
 */
struct message_header {
	uint32_t		msg_type;	/* message_type */
	uint32_t		priority;	/* message_priority */
	uint64_t		msg_id;		/* ID único da mensagem */
	uint64_t		in_reply_to;	/* ID da mensagem original (respostas) */
	
	object_id_t		sender;		/* Objeto que enviou */
	object_id_t		target;		/* Objeto alvo */
	method_id_t		method;		/* Método a invocar */
	
	capability_t		capability;	/* Token de capacidade */
	
	uint32_t		payload_size;	/* Tamanho do payload */
	uint32_t		flags;		/* MSG_FLAG_* */
	uint64_t		deadline_ns;	/* Deadline absoluta em nanosegundos */
};

/**
 * Flags para mensagens
 */
#define MSG_FLAG_ASYNC		(1 << 0)  /* Mensagem assíncrona (sem resposta) */
#define MSG_FLAG_SECURE		(1 << 1)  /* Requer canal seguro */
#define MSG_FLAG_ENCRYPTED	(1 << 2)  /* Payload criptografado */
#define MSG_FLAG_SIGNED		(1 << 3)  /* Payload assinado */
#define MSG_FLAG_BROADCAST	(1 << 4)  /* Broadcast para múltiplos alvos */
#define MSG_FLAG_NO_REPLY	(1 << 5)  /* Não espera resposta */

/* ============================================================================
 * OBJETOS - ESTRUTURAS FUNDAMENTAIS
 * ============================================================================ */

/**
 * struct object_info - Informações de um objeto no sistema
 * 
 * Objetos são as entidades fundamentais. Tudo é um objeto:
 * - Serviços são objetos
 * - Procedimentos são objetos
 * - Portas são objetos
 * - Conexões são objetos
 * - Até o sistema é um objeto (ROOT_OBJECT_ID)
 */
struct object_info {
	object_id_t		oid;		/* Object ID */
	uint64_t		type_id;	/* Tipo do objeto */
	uint64_t		flags;		/* OBJECT_FLAG_* */
	uint32_t		ref_count;	/* Contagem de referências */
	uint32_t		num_methods;	/* Número de métodos exportados */
	capability_t		default_cap;	/* Capacidade padrão */
	uint64_t		created_at;	/* Timestamp de criação */
	uint64_t		parent;		/* Objeto pai (0 = root) */
};

/**
 * Flags para objetos
 */
#define OBJECT_FLAG_PERSISTENT	(1 << 0)  /* Objeto persistente */
#define OBJECT_FLAG_VOLATILE	(1 << 1)  /* Objeto volátil */
#define OBJECT_FLAG_SYSTEM	(1 << 2)  /* Objeto de sistema */
#define OBJECT_FLAG_REMOTE	(1 << 3)  /* Objeto remoto */
#define OBJECT_FLAG_LOCAL	(1 << 4)  /* Objeto local */

/**
 * struct method_info - Informações de um método de objeto
 */
struct method_info {
	method_id_t		method_id;	/* ID do método */
	uint64_t		name_hash;	/* Hash do nome do método */
	uint32_t		input_type;	/* Tipo do parâmetro de entrada */
	uint32_t		output_type;	/* Tipo do valor de retorno */
	uint32_t		flags;		/* METHOD_FLAG_* */
	uint32_t		timeout_ms;	/* Timeout padrão */
};

/**
 * Flags para métodos
 */
#define METHOD_FLAG_IDEMPOTENT	(1 << 0)  /* Método idempotente */
#define METHOD_FLAG_ASYNC	(1 << 1)  /* Método assíncrono por padrão */
#define METHOD_FLAG_READONLY	(1 << 2)  /* Método não modifica estado */

/* ============================================================================
 * PORTAS - PONTOS DE COMUNICAÇÃO
 * ============================================================================ */

/**
 * struct port_info - Informações de uma porta
 * 
 * Portas são objetos especiais que gerenciam filas de mensagens.
 * Objetos se vinculam a portas para enviar/receber mensagens.
 */
struct port_info {
	port_id_t		port_id;	/* ID da porta */
	object_id_t		owner;		/* Objeto dono da porta */
	uint64_t		queue_depth;	/* Mensagens na fila */
	uint64_t		max_queue;	/* Tamanho máximo da fila */
	uint32_t		state;		/* PORT_STATE_* */
	uint32_t		num_bindings;	/* Objetos vinculados */
	capability_t		required_cap;	/* Capacidade necessária */
};

/**
 * Estados da porta
 */
#define PORT_STATE_CLOSED	0
#define PORT_STATE_OPEN		1
#define PORT_STATE_LISTENING	2
#define PORT_STATE_DRAINING	3
#define PORT_STATE_ERROR	4

/**
 * struct port_binding - Vinculação de um objeto a uma porta
 */
struct port_binding {
	object_id_t		object;
	port_id_t		port;
	uint32_t		flags;		/* BINDING_FLAG_* */
	capability_t		capability;
};

/**
 * Flags de vinculação
 */
#define BINDING_FLAG_RECV	(1 << 0)  /* Pode receber mensagens */
#define BINDING_FLAG_SEND	(1 << 1)  /* Pode enviar mensagens */
#define BINDING_FLAG_EXCLUSIVE	(1 << 2)  /* Vinculação exclusiva */

/* ============================================================================
 * OPERAÇÕES DO SISTEMA DE OBJETOS
 * ============================================================================ */

/**
 * Comandos/Opcodes para operações com objetos e mensagens
 */
enum object_opcode {
	/* Operações com objetos */
	OBJ_CREATE		= 1,   /* Criar objeto */
	OBJ_DESTROY		= 2,   /* Destruir objeto */
	OBJ_LOOKUP		= 3,   /* Buscar objeto */
	OBJ_GET_INFO		= 4,   /* Obter informações do objeto */
	OBJ_SET_PROPERTY	= 5,   /* Definir propriedade */
	OBJ_GET_PROPERTY	= 6,   /* Obter propriedade */
	OBJ_INVOKE		= 7,   /* Invocar método (RPC) */
	
	/* Operações com portas */
	PORT_CREATE		= 10,  /* Criar porta */
	PORT_DESTROY		= 11,  /* Destruir porta */
	PORT_OPEN		= 12,  /* Abrir porta */
	PORT_CLOSE		= 13,  /* Fechar porta */
	PORT_BIND		= 14,  /* Vincular objeto à porta */
	PORT_UNBIND		= 15,  /* Desvincular objeto */
	PORT_SEND		= 16,  /* Enviar mensagem */
	PORT_RECV		= 17,  /* Receber mensagem */
	PORT_PEEK		= 18,  /* Visualizar mensagem sem consumir */
	PORT_GET_STATUS		= 19,  /* Obter status da porta */
	PORT_SELECT		= 20,  /* Selecionar múltiplas portas */
	
	/* Operações de sistema */
	SYS_INIT		= 26,  /* Inicializar sistema de objetos */
	SYS_SHUTDOWN		= 27,  /* Finalizar sistema */
	SYS_GET_STATS		= 28,  /* Obter estatísticas do sistema */
	SYS_SYNC		= 29,  /* Sincronizar estado */
};

/* ============================================================================
 * ESTRUTURAS PARA OPERAÇÕES
 * ============================================================================ */

/**
 * OBJ_CREATE - Criar objeto
 */
struct obj_create_in {
	uint64_t		type_id;	/* Tipo do objeto */
	uint64_t		flags;		/* OBJECT_FLAG_* */
	object_id_t		parent;		/* Objeto pai (0 = root) */
	capability_t		initial_cap;	/* Capacidade inicial */
	uint32_t		name_len;	/* Comprimento do nome opcional */
};

struct obj_create_out {
	object_id_t		oid;		/* ID do objeto criado */
	uint64_t		created_at;
};

/**
 * OBJ_LOOKUP - Buscar objeto
 */
struct obj_lookup_in {
	uint64_t		name_hash;	/* Hash do nome do objeto */
	object_id_t		scope;		/* Escopo de busca */
};

struct obj_lookup_out {
	object_id_t		oid;
	struct object_info	info;
};

/**
 * OBJ_INVOKE - Invocar método (chamada RPC)
 */
struct obj_invoke_in {
	object_id_t		target;
	method_id_t		method;
	capability_t		capability;
	uint32_t		flags;		/* MSG_FLAG_* */
	uint32_t		input_size;
	/* Dados de entrada seguem */
};

struct obj_invoke_out {
	int32_t			status;
	uint32_t		output_size;
	uint64_t		invoke_id;
	uint64_t		processing_ns;
	/* Dados de saída seguem */
};

/**
 * PORT_CREATE - Criar porta
 */
struct port_create_in {
	uint64_t		max_queue;	/* Tamanho máximo da fila */
	uint32_t		state;		/* PORT_STATE_* inicial */
	capability_t		required_cap;
	object_id_t		owner;
};

struct port_create_out {
	port_id_t		port_id;
};

/**
 * PORT_SEND - Enviar mensagem
 */
struct port_send_in {
	port_id_t		port_id;
	object_id_t		target;
	method_id_t		method;
	uint32_t		priority;
	uint32_t		flags;
	uint32_t		payload_size;
	capability_t		capability;
	/* Payload segue */
};

struct port_send_out {
	uint64_t		msg_id;
	uint64_t		queue_position;
};

/**
 * PORT_RECV - Receber mensagem
 */
struct port_recv_in {
	port_id_t		port_id;
	uint32_t		max_size;
	uint32_t		flags;		/* PORT_MSG_* */
	uint32_t		timeout_ms;
};

struct port_recv_out {
	uint64_t		msg_id;
	object_id_t		sender;
	uint32_t		priority;
	uint32_t		payload_size;
	int32_t			status;
	/* Payload segue */
};

/**
 * PORT_SELECT - Selecionar múltiplas portas
 */
struct port_select_in {
	port_id_t		port_ids[32];
	uint32_t		num_ports;
	uint32_t		timeout_ms;
	uint32_t		events;		/* PORT_EVENT_* */
};

struct port_select_out {
	uint32_t		ready_count;
	uint32_t		events[32];
};

/**
 * Eventos de porta
 */
#define PORT_EVENT_READABLE	(1 << 0)  /* Mensagem disponível para leitura */
#define PORT_EVENT_WRITABLE	(1 << 1)  /* Porta disponível para escrita */
#define PORT_EVENT_ERROR	(1 << 2)  /* Erro na porta */
#define PORT_EVENT_HANGUP	(1 << 3)  /* Desconexão */
#define PORT_EVENT_PEER_CLOSED	(1 << 4)  /* Parceiro fechou */

/* ============================================================================
 * CABEÇALHOS DAS MENSAGENS DO SISTEMA
 * ============================================================================ */

/**
 * struct object_in_header - Cabeçalho de requisição do sistema
 */
struct object_in_header {
	uint32_t		len;
	uint32_t		opcode;		/* object_opcode */
	uint64_t		unique;		/* ID único da requisição */
	object_id_t		caller;		/* Objeto que fez a chamada */
	object_id_t		session;	/* Sessão (0 = sistema) */
	capability_t		capability;	/* Capacidade do chamador */
	uint32_t		flags;
	uint32_t		reserved;
};

/**
 * struct object_out_header - Cabeçalho de resposta do sistema
 */
struct object_out_header {
	uint32_t		len;
	int32_t			error;		/* 0 = sucesso, erro negativo */
	uint64_t		unique;		/* Corresponde à requisição */
};

/* ============================================================================
 * NOTIFICAÇÕES
 * ============================================================================ */

/**
 * Tipos de notificação
 */
enum object_notify_code {
	NOTIFY_PORT_EVENT	= 1,   /* Evento em porta */
	NOTIFY_OBJECT_CREATED	= 2,   /* Objeto criado */
	NOTIFY_OBJECT_DESTROYED	= 3,   /* Objeto destruído */
	NOTIFY_METHOD_INVOKED	= 4,   /* Método invocado */
	NOTIFY_CAPABILITY_CHANGE = 5,  /* Mudança de capacidade */
};

struct notify_port_event {
	port_id_t		port_id;
	uint32_t		events;
	uint64_t		queue_depth;
};

struct notify_object_created {
	object_id_t		oid;
	uint64_t		type_id;
	object_id_t		parent;
};

/* ============================================================================
 * INICIALIZAÇÃO DO SISTEMA DE OBJETOS
 * ============================================================================ */

/**
 * Flags de inicialização do sistema
 */
#define SYS_FLAG_ASYNC_MSG	(1 << 0)
#define SYS_FLAG_SECURE_PORTS	(1 << 1)
#define SYS_FLAG_AUDIT		(1 << 2)
#define SYS_FLAG_DEBUG		(1 << 3)

struct sys_init_in {
	uint32_t		major;
	uint32_t		minor;
	uint64_t		flags;
	uint32_t		max_ports;
	uint32_t		max_objects;
	uint32_t		max_message_size;
	uint32_t		default_timeout_ms;
};

struct sys_init_out {
	uint32_t		major;
	uint32_t		minor;
	uint64_t		flags;
	uint32_t		max_ports;
	uint32_t		max_objects;
	uint32_t		max_message_size;
	uint32_t		default_timeout_ms;
	object_id_t		root_object;	/* = ROOT_OBJECT_ID */
};

/* ============================================================================
 * DISPOSITIVO E IOCTLS
 * ============================================================================ */

#define OBJECT_DEV_IOC_MAGIC		231
#define OBJECT_DEV_IOC_CLONE		_IOR(OBJECT_DEV_IOC_MAGIC, 0, uint32_t)
#define OBJECT_DEV_IOC_GET_STATS	_IOR(OBJECT_DEV_IOC_MAGIC, 1, uint64_t)
#define OBJECT_DEV_IOC_RESET		_IO(OBJECT_DEV_IOC_MAGIC, 2)

#endif /* UAPI_LINUX_FUSE_H */
