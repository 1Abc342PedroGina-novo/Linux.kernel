#ifndef _UAPI_LINUX_IOCTL_H
#define _UAPI_LINUX_IOCTL_H

#include <asm/ioctl.h>
/*
 * Letras de unidade para ioctls (C:\, D:\, E:\)
 * 
 * No novo modelo, dispositivos são objetos com letras de unidade.
 */
#define _IOC_SIZEBITS	13
#define _IOC_DIRBITS	3

#define _IOC_NRMASK	((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK	((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZEMASK	((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRMASK	((1 << _IOC_DIRBITS)-1)

#define _IOC_NRSHIFT	0
#define _IOC_TYPESHIFT	(_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT	(_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT	(_IOC_SIZESHIFT+_IOC_SIZEBITS)

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 *
 * NOTE: _IOC_WRITE means userland is writing and kernel is reading.
 * _IOC_READ means userland is reading and kernel is writing.
 */
#ifndef _IOC_NONE
# define _IOC_NONE	0U
#endif
#ifndef _IOC_WRITE
# define _IOC_WRITE	1U
#endif
#ifndef _IOC_READ
# define _IOC_READ	2U
#endif

#define _IOC(dir,type,nr,size) \
	(((dir)  << _IOC_DIRSHIFT) | \
	 ((type) << _IOC_TYPESHIFT) | \
	 ((nr)   << _IOC_NRSHIFT) | \
	 ((size) << _IOC_SIZESHIFT))

#ifndef __KERNEL__
#define _IOC_TYPECHECK(t) (sizeof(t))
#endif

/*
 * Used to create numbers.
 *
 * NOTE: _IOW means userland is writing and kernel is reading.
 * _IOR means userland is reading and kernel is writing.
 */
#define _IO(type,nr)		_IOC(_IOC_NONE,(type),(nr),0)
#define _IOR(type,nr,size)	_IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOW(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOR_BAD(type,nr,size)	_IOC(_IOC_READ,(type),(nr),sizeof(size))
#define _IOW_BAD(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define _IOWR_BAD(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))

/*用于编码参数*/
#define _IOC_DIR(nr)		(((nr) >> _IOC_DIRSHIFT) & _IOC_DIRMASK)
#define _IOC_TYPE(nr)		(((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
#define _IOC_NR(nr)		(((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)
#define _IOC_SIZE(nr)		(((nr) >> _IOC_SIZESHIFT) & _IOC_SIZEMASK)

/*
 * ...and for the drivers/sound files...
 */
#define IOC_IN		(_IOC_WRITE << _IOC_DIRSHIFT)
#define IOC_OUT		(_IOC_READ << _IOC_DIRSHIFT)
#define IOC_INOUT	((_IOC_WRITE|_IOC_READ) << _IOC_DIRSHIFT)
#define IOCSIZE_MASK	(_IOC_SIZEMASK << _IOC_SIZESHIFT)
#define IOCSIZE_SHIFT	(_IOC_SIZESHIFT)

/* ============================================================================
 * NOVAS MACROS PARA MENSAGENS E OBJETOS
 * ============================================================================
 */

/**
 * Macro para enviar mensagem a objeto
 * 
 * Em vez de ioctl(), use OBJECT_MSG() para enviar mensagens.
 */
#define OBJECT_MSG(obj_id, method_id, msg_type, data, data_size) \
	_IOC(_IOC_READ|_IOC_WRITE, (obj_id), (method_id), (data_size))

/**
 * Macro para invocar método de objeto
 */
#define OBJECT_INVOKE(obj_id, method_id, data, data_size) \
	_IOC(_IOC_READ|_IOC_WRITE, (obj_id), (method_id), (data_size))

/**
 * Macro para enviar notificação a objeto
 */
#define OBJECT_NOTIFY(obj_id, event_id, data, data_size) \
	_IOC(_IOC_WRITE, (obj_id), (event_id), (data_size))

/**
 * Macro para consultar propriedade de objeto
 */
#define OBJECT_GETPROP(obj_id, prop_id, data, data_size) \
	_IOC(_IOC_READ, (obj_id), (prop_id), (data_size))

/**
 * Macro para definir propriedade de objeto
 */
#define OBJECT_SETPROP(obj_id, prop_id, data, data_size) \
	_IOC(_IOC_WRITE, (obj_id), (prop_id), (data_size))

/* ============================================================================
 * LETRAS DE UNIDADE PARA MENSAGENS (C:\, D:\, E:\)
 * ============================================================================ */

/**
 * Macros para enviar mensagens a objetos em letras de unidade
 * 
 * Exemplo:
 *   - drive_msg('C', 0x1001, data, size) -> envia mensagem para C:\
 *   - drive_msg('D', 0x1002, data, size) -> envia mensagem para D:\
 */
#define DRIVE_BASE(c)		((c) & 0x1F)  /* 'C' -> 3, 'D' -> 4, etc. */
#define DRIVE_MAGIC(c)		(0x44 + DRIVE_BASE(c))  /* 'D' + offset */

#define drive_msg(drive, method, data, size) \
	_IOC(_IOC_READ|_IOC_WRITE, DRIVE_MAGIC(drive), (method), (size))

#define drive_read(drive, method, data, size) \
	_IOC(_IOC_READ, DRIVE_MAGIC(drive), (method), (size))

#define drive_write(drive, method, data, size) \
	_IOC(_IOC_WRITE, DRIVE_MAGIC(drive), (method), (size))

/* Letras de unidade predefinidas */
#define DRIVE_C_MAGIC	DRIVE_MAGIC('C')  /* C:\ - Disco de boot */
#define DRIVE_D_MAGIC	DRIVE_MAGIC('D')  /* D:\ - Segundo disco */
#define DRIVE_E_MAGIC	DRIVE_MAGIC('E')  /* E:\ - Terceiro disco */
#define DRIVE_F_MAGIC	DRIVE_MAGIC('F')  /* F:\ - USB/Removível */
#define DRIVE_Z_MAGIC	DRIVE_MAGIC('Z')  /* Z:\ - Última letra */

/* ============================================================================
 * MÉTODOS PADRÃO DE OBJETOS (via ioctl)
 * ============================================================================ */

/**
 * Estes são os números de método padrão que substituem ioctls tradicionais.
 * 
 * Em vez de ioctl(fd, CMD, arg), use:
 *   object_send(obj_id, METHOD_READ, message);
 */
#define METHOD_READ		0x01	/* Ler dados do objeto */
#define METHOD_WRITE		0x02	/* Escrever dados no objeto */
#define METHOD_OPEN		0x03	/* Abrir objeto */
#define METHOD_CLOSE		0x04	/* Fechar objeto */
#define METHOD_GETATTR		0x05	/* Obter atributos */
#define METHOD_SETATTR		0x06	/* Definir atributos */
#define METHOD_EXECUTE		0x07	/* Executar objeto (executáveis) */
#define METHOD_IOCTL		0x08	/* IOCTL legado (DEPRECADO) */
#define METHOD_MMAP		0x09	/* Mapear objeto em memória */
#define METHOD_SYNC		0x0A	/* Sincronizar objeto */
#define METHOD_LOCK		0x0B	/* Lock/unlock objeto */
#define METHOD_SEEK		0x0C	/* Mudar posição */

/* ============================================================================
 * MENSAGENS DE DISPOSITIVO (antigos ioctls)
 * ============================================================================ */

/**
 * Categorias de mensagens por tipo de objeto
 */
#define OBJ_MSG_FILE		0x10	/* Mensagens para objetos arquivo */
#define OBJ_MSG_DIR		0x20	/* Mensagens para objetos diretório */
#define OBJ_MSG_DEVICE		0x30	/* Mensagens para objetos dispositivo */
#define OBJ_MSG_PORT		0x40	/* Mensagens para objetos porta */
#define OBJ_MSG_SERVICE		0x50	/* Mensagens para objetos serviço */
#define OBJ_MSG_DRIVE		0x60	/* Mensagens para letras de unidade */

/**
 * Mensagens específicas para letras de unidade (C:\, D:\)
 */
#define DRIVE_MSG_MOUNT		0x6001	/* Montar disco */
#define DRIVE_MSG_UNMOUNT	0x6002	/* Desmontar disco */
#define DRIVE_MSG_FORMAT	0x6003	/* Formatar disco */
#define DRIVE_MSG_EJECT		0x6004	/* Ejetar disco */
#define DRIVE_MSG_GETINFO	0x6005	/* Obter informações da unidade */
#define DRIVE_MSG_SETLABEL	0x6006	/* Definir rótulo do volume */

/* ============================================================================
 * ESTRUTURAS PARA MENSAGENS (substitutos dos argumentos ioctl)
 * ============================================================================ */

/**
 * struct object_message - Mensagem genérica para objeto
 * 
 * Substitui os argumentos arbitrários de ioctl por mensagens estruturadas.
 */
struct object_message {
	__u64		object_id;	/* ID do objeto alvo */
	__u32		method;		/* Método a invocar (METHOD_*) */
	__u32		flags;		/* MSG_FLAG_* */
	__u64		offset;		/* Offset (para leitura/escrita) */
	__u64		size;		/* Tamanho dos dados */
	__u64		capability;	/* Token de capacidade */
	void __user	*data;		/* Dados da mensagem */
};

/**
 * struct drive_message - Mensagem para letra de unidade
 * 
 * Exemplo: enviar mensagem para C:\
 */
struct drive_message {
	__u8		drive_letter;	/* 'C', 'D', 'E', etc. */
	__u8		reserved[7];
	__u32		command;	/* Comando (DRIVE_MSG_*) */
	__u32		flags;
	__u64		param1;
	__u64		param2;
	void __user	*data;
	__u64		data_size;
};

/**
 * struct object_ioctl_msg - Mensagem de ioctl legada (DEPRECADA)
 * 
 * Para compatibilidade com código antigo.
 */
struct object_ioctl_msg {
	__u64		object_id;
	__u32		ioctl_cmd;	/* Comando ioctl original */
	__u32		flags;
	void __user	*arg;		/* Argumento original */
	__u64		arg_size;
};

/* ============================================================================
 * FLAGS PARA MENSAGENS
 * ============================================================================ */

#define MSG_FLAG_NOWAIT		(1 << 0)	/* Não bloquear */
#define MSG_FLAG_ASYNC		(1 << 1)	/* Mensagem assíncrona */
#define MSG_FLAG_BROADCAST	(1 << 2)	/* Broadcast */
#define MSG_FLAG_SECURE		(1 << 3)	/* Mensagem segura */
#define MSG_FLAG_NO_REPLY	(1 << 4)	/* Sem resposta */
#define MSG_FLAG_PEEK		(1 << 5)	/* Visualizar sem consumir */

/* ============================================================================
 * IOCTLS ESPECÍFICOS PARA O SISTEMA DE OBJETOS
 * ============================================================================ */

/*
 * Comandos ioctl específicos para o driver do sistema de objetos.
 * Estes são os POUCOS ioctls que ainda existem (para bootstrap).
 */
#define OBJ_IOCTL_BASE		'O'

/* Obter ID do objeto root */
#define OBJ_IOC_GET_ROOT	_IOR(OBJ_IOCTL_BASE, 1, __u64)

/* Criar nova porta */
#define OBJ_IOC_CREATE_PORT	_IOWR(OBJ_IOCTL_BASE, 2, struct port_create_msg)

/* Enviar mensagem (substituto moderno para ioctl) */
#define OBJ_IOC_SEND_MSG	_IOWR(OBJ_IOCTL_BASE, 3, struct object_message)

/* Receber mensagem */
#define OBJ_IOC_RECV_MSG	_IOWR(OBJ_IOCTL_BASE, 4, struct object_message)

/* Vincular objeto a porta */
#define OBJ_IOC_BIND_PORT	_IOW(OBJ_IOCTL_BASE, 5, struct port_bind_msg)

/* Obter letra de unidade para objeto */
#define OBJ_IOC_GET_DRIVE	_IOR(OBJ_IOCTL_BASE, 6, __u8)

/* Montar disco com letra aleatória */
#define OBJ_IOC_MOUNT_DRIVE	_IOWR(OBJ_IOCTL_BASE, 7, struct drive_mount_msg)

/* Desmontar unidade */
#define OBJ_IOC_UNMOUNT_DRIVE	_IOW(OBJ_IOCTL_BASE, 8, __u8)

/* ============================================================================
 * ESTRUTURAS PARA IOCTLS DO SISTEMA DE OBJETOS
 * ============================================================================ */

struct port_create_msg {
	__u64		owner_id;
	__u32		max_queue;
	__u32		flags;
	__u64		port_id;	/* Retornado */
};

struct port_bind_msg {
	__u64		object_id;
	__u64		port_id;
	__u32		flags;
	__u32		reserved;
};

struct drive_mount_msg {
	__u64		device_id;
	__u8		suggested_letter;	/* 0 = aleatória */
	__u8		reserved[7];
	__u32		flags;
	__u32		fs_type;
	__u8		assigned_letter;	/* Retornado */
	__u8		reserved2[7];
};

#endif /* UAPI_IOCTL_H */
