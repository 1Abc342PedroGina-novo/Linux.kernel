#ifndef _UAPI_LINUX_MOUNT_H
#define _UAPI_LINUX_MOUNT_H

#include <linux/types.h>
#include "ipc.h"
/*
 * ============================================================================
 * NOVA FILOSOFIA: "TUDO É MENSAGEM" E "TUDO É OBJETO"
 * ============================================================================
 * 
 * AVISO: A filosofia "tudo é arquivo" está DEPRECADA!
 * 
 * Este arquivo foi modificado para gerenciar BINDINGS DE OBJETOS e PORTAS,
 * não mais montagem de filesystems.
 * 
 * Bindings conectam objetos a portas, permitindo comunicação por mensagens.
 * 
 * NÃO EXISTEM MAIS:
 *   - Mount points
 *   - Filesystems
 *   - Superblocks
 *   - VFS
 * 
 * EXISTEM APENAS:
 *   - Objetos (com OIDs)
 *   - Portas (filas de mensagens)
 *   - Bindings (conexões objeto-porta)
 *   - Namespaces de objetos
 * 
 * ============================================================================
 */

/*
 * Flags independentes para bindings de objetos (até 32 flags)
 * 
 * Estes flags controlam como objetos se vinculam a portas e como
 * mensagens são entregues.
 */
#define BIND_RDONLY	     1	/* Binding somente leitura (não pode enviar msgs) */
#define BIND_NOSEND	     2	/* Não pode enviar mensagens */
#define BIND_NORECV	     4	/* Não pode receber mensagens */
#define BIND_NOEXEC	     8	/* Não pode invocar métodos */
#define BIND_SYNCHRONOUS    16	/* Mensagens são processadas sincronamente */
#define BIND_REBIND	     32	/* Alterar flags de um binding existente */
#define BIND_MANDLOCK	     64	/* Permitir locks obrigatórios no binding */
#define BIND_DIRSYNC	    128	/* Diretório de mensagens sincrono (deprecado) */
#define BIND_NOSYMFOLLOW    256	/* Não seguir symlinks de objeto (deprecado) */
#define BIND_NOATIME	   1024	/* Não atualizar timestamps de acesso */
#define BIND_NODIRATIME	   2048	/* Não atualizar timestamps de diretório */
#define BIND_BIND	   4096	/* Binding de objeto para objeto */
#define BIND_MOVE	   8192	/* Mover binding */
#define BIND_REC	  16384	/* Recursivo (bind em hierarquia) */
#define BIND_VERBOSE	  32768	/* Verbosidade é silêncio (deprecado) */
#define BIND_SILENT	  32768
#define BIND_OBJACL	 (1<<16)	/* Sistema de objetos não aplica umask */
#define BIND_UNBINDABLE	 (1<<17)	/* Mudar para unbindable */
#define BIND_PRIVATE	 (1<<18)	/* Mudar para private */
#define BIND_SLAVE	 (1<<19)	/* Mudar para slave */
#define BIND_SHARED	 (1<<20)	/* Mudar para shared */
#define BIND_RELATIME	 (1<<21)	/* Atualizar atime relativo */
#define BIND_KERNBIND	 (1<<22)	/* Este é um kern_bind call */
#define BIND_I_VERSION	 (1<<23)	/* Atualizar campo I_version do objeto */
#define BIND_STRICTATIME (1<<24)	/* Sempre realizar atualizações atime */
#define BIND_LAZYTIME	 (1<<25)	/* Atualizar timestamps lazy */

/* Flags internas do binding */
#define BIND_SUBBIND     (1<<26)	/* Sub-binding */
#define BIND_NOREMOTELOCK (1<<27)	/* Sem lock remoto */
#define BIND_NOSEC	 (1<<28)	/* Sem segurança */
#define BIND_BORN	 (1<<29)	/* Binding nascido */
#define BIND_ACTIVE	 (1<<30)	/* Binding ativo */
#define BIND_NOUSER	 (1<<31)	/* Binding de sistema */

/*
 * Flags do binding que podem ser alteradas por BIND_REBIND
 */
#define BIND_RMT_MASK	(BIND_RDONLY|BIND_SYNCHRONOUS|BIND_MANDLOCK|BIND_I_VERSION|\
			 BIND_LAZYTIME)

/*
 * Valor mágico antigo (mantido para compatibilidade)
 */
#define BIND_MGC_VAL 0xC0ED0000
#define BIND_MGC_MSK 0xffff0000

/* ============================================================================
 * OPERAÇÕES COM BINDINGS DE OBJETOS
 * ============================================================================ */

/*
 * open_tree() flags - Modificado para bind_tree
 */
#define OPEN_TREE_CLONE		(1 << 0)	/* Clonar árvore de bindings */
#define OPEN_TREE_NAMESPACE	(1 << 1)	/* Clonar para novo namespace */
#define OPEN_TREE_CLOEXEC	O_CLOEXEC	/* Fechar em execve */

/*
 * move_bind() flags - Modificado para move_binding
 */
#define MOVE_BIND_F_SYMLINKS		0x00000001 /* Seguir symlinks no caminho from */
#define MOVE_BIND_F_AUTOMOBJ		0x00000002 /* Seguir automounts de objeto */
#define MOVE_BIND_F_EMPTY_PATH		0x00000004 /* Caminho from vazio permitido */
#define MOVE_BIND_T_SYMLINKS		0x00000010 /* Seguir symlinks no caminho to */
#define MOVE_BIND_T_AUTOMOBJ		0x00000020 /* Seguir automounts de objeto */
#define MOVE_BIND_T_EMPTY_PATH		0x00000040 /* Caminho to vazio permitido */
#define MOVE_BIND_SET_GROUP		0x00000100 /* Definir grupo compartilhado */
#define MOVE_BIND_BENEATH		0x00000200 /* Binding abaixo do topo */
#define MOVE_BIND__MASK			0x00000377

/*
 * object_open() flags - Abrir objeto (antigo fsopen)
 */
#define OBJOPEN_CLOEXEC		0x00000001

/*
 * object_pick() flags - Selecionar objeto (antigo fspick)
 */
#define OBJPICK_CLOEXEC		0x00000001
#define OBJPICK_SYMLINK_NOFOLLOW	0x00000002
#define OBJPICK_NO_AUTOMOBJ		0x00000004
#define OBJPICK_EMPTY_PATH		0x00000008

/* ============================================================================
 * COMANDOS DE CONFIGURAÇÃO DE BINDING
 * ============================================================================ */

/*
 * Tipo de chamada de configuração do binding
 */
enum binding_config_command {
	BINDCONFIG_SET_FLAG		= 0,	/* Set parameter, sem valor */
	BINDCONFIG_SET_STRING		= 1,	/* Set parameter, string value */
	BINDCONFIG_SET_BINARY		= 2,	/* Set parameter, binary blob */
	BINDCONFIG_SET_OBJECT_PATH	= 3,	/* Set parameter, objeto por path */
	BINDCONFIG_SET_OBJECT_PATH_EMPTY = 4,	/* Set parameter, empty path */
	BINDCONFIG_SET_FD		= 5,	/* Set parameter, objeto por fd */
	BINDCONFIG_CMD_CREATE		= 6,	/* Criar binding */
	BINDCONFIG_CMD_RECONFIGURE	= 7,	/* Reconfigurar binding */
	BINDCONFIG_CMD_CREATE_EXCL	= 8,	/* Criar binding exclusivo */
};

/*
 * object_bind() flags - Bind de objeto (antigo fsmount)
 */
#define OBJBIND_CLOEXEC		0x00000001
#define OBJBIND_NAMESPACE	0x00000002	/* Criar em novo namespace */

/* ============================================================================
 * ATRIBUTOS DE BINDING
 * ============================================================================ */

#define BIND_ATTR_RDONLY	0x00000001 /* Binding somente leitura */
#define BIND_ATTR_NOSEND	0x00000002 /* Não enviar mensagens */
#define BIND_ATTR_NORECV	0x00000004 /* Não receber mensagens */
#define BIND_ATTR_NOEXEC	0x00000008 /* Não executar métodos */
#define BIND_ATTR__ATIME	0x00000070 /* Atualização de atime */
#define BIND_ATTR_RELATIME	0x00000000 /* Atualizar atime relativo */
#define BIND_ATTR_NOATIME	0x00000010 /* Não atualizar access times */
#define BIND_ATTR_STRICTATIME	0x00000020 /* Sempre atualizar atime */
#define BIND_ATTR_NODIRATIME	0x00000080 /* Não atualizar dir atimes */
#define BIND_ATTR_IDMAP		0x00100000 /* ID mapping */
#define BIND_ATTR_NOSYMFOLLOW	0x00200000 /* Não seguir symlinks */

/* ============================================================================
 * ESTRUTURAS DE BINDING
 * ============================================================================ */

/**
 * struct bind_attr - Atributos de binding (antigo mount_attr)
 * 
 * Define como um objeto se conecta a uma porta.
 */
struct bind_attr {
	__u64 attr_set;		/* Atributos a definir */
	__u64 attr_clr;		/* Atributos a limpar */
	__u64 propagation;	/* Propagação do binding */
	__u64 object_fd;	/* File descriptor do objeto */
};

/* Versões do bind_attr */
#define BIND_ATTR_SIZE_VER0	32

/* ============================================================================
 * INFORMAÇÕES DE BINDING (antigo statmount)
 * ============================================================================ */

/**
 * struct statbind - Estatísticas do binding (antigo statmount)
 * 
 * Retorna informações sobre bindings objeto-porta.
 */
struct statbind {
	__u32 size;		/* Tamanho total, incluindo strings */
	__u32 bind_opts;	/* [str] Opções do binding */
	__u64 mask;		/* Quais resultados foram escritos */
	__u32 obj_dev_major;	/* Major do objeto */
	__u32 obj_dev_minor;	/* Minor do objeto */
	__u64 obj_magic;	/* Magic do objeto */
	__u32 obj_flags;	/* BIND_{RDONLY,SYNCHRONOUS,...} */
	__u32 obj_type;		/* [str] Tipo do objeto */
	__u64 bind_id;		/* ID único do binding */
	__u64 bind_parent_id;	/* ID do binding pai */
	__u32 bind_id_old;	/* IDs reutilizados (proc) */
	__u32 bind_parent_id_old;
	__u64 bind_attr;	/* BIND_ATTR_* */
	__u64 bind_propagation;	/* BIND_{SHARED,SLAVE,PRIVATE,UNBINDABLE} */
	__u64 bind_peer_group;	/* ID do grupo peer compartilhado */
	__u64 bind_master;	/* Binding recebe propagação deste ID */
	__u64 propagate_from;	/* Propagação do namespace atual */
	__u32 bind_root;	/* [str] Root do binding */
	__u32 bind_point;	/* [str] Ponto do binding */
	__u64 bind_ns_id;	/* ID do namespace do binding */
	__u32 obj_subtype;	/* [str] Subtipo do objeto */
	__u32 obj_source;	/* [str] String source do binding */
	__u32 opt_num;		/* Número de opções */
	__u32 opt_array;	/* [str] Array de opções */
	__u32 opt_sec_num;	/* Número de opções de segurança */
	__u32 opt_sec_array;	/* [str] Array de opções de segurança */
	__u64 supported_mask;	/* Máscara suportada pelo kernel */
	__u32 bind_uidmap_num;	/* Número de mapeamentos UID */
	__u32 bind_uidmap;	/* [str] Array de mapeamentos UID */
	__u32 bind_gidmap_num;	/* Número de mapeamentos GID */
	__u32 bind_gidmap;	/* [str] Array de mapeamentos GID */
	__u64 __spare2[43];
	char str[];		/* Parte variável com strings */
};

/* ============================================================================
 * REQUISIÇÕES DE ID DE BINDING
 * ============================================================================ */

/**
 * struct bind_id_req - Requisição de ID de binding (antigo mnt_id_req)
 * 
 * Para statbind: @param representa a máscara de requisição
 * Para listbind: @param representa o último ID listado (ou zero)
 */
struct bind_id_req {
	__u32 size;
	union {
		__u32 bind_ns_fd;	/* FD do namespace do binding */
		__u32 bind_fd;		/* FD do binding */
	};
	__u64 bind_id;		/* ID do binding */
	__u64 param;		/* Parâmetro (máscara ou último ID) */
	__u64 bind_ns_id;	/* ID do namespace */
};

/* Versões do bind_id_req */
#define BIND_ID_REQ_SIZE_VER0	24
#define BIND_ID_REQ_SIZE_VER1	32

/* ============================================================================
 * MÁSCARAS PARA STATBIND
 * ============================================================================ */

#define STATBIND_OBJ_BASIC		0x00000001U	/* Obter obj_... */
#define STATBIND_BIND_BASIC		0x00000002U	/* Obter bind_... */
#define STATBIND_PROPAGATE_FROM		0x00000004U	/* Obter propagate_from */
#define STATBIND_BIND_ROOT		0x00000008U	/* Obter bind_root */
#define STATBIND_BIND_POINT		0x00000010U	/* Obter bind_point */
#define STATBIND_OBJ_TYPE		0x00000020U	/* Obter obj_type */
#define STATBIND_BIND_NS_ID		0x00000040U	/* Obter bind_ns_id */
#define STATBIND_BIND_OPTS		0x00000080U	/* Obter bind_opts */
#define STATBIND_OBJ_SUBTYPE		0x00000100U	/* Obter obj_subtype */
#define STATBIND_OBJ_SOURCE		0x00000200U	/* Obter obj_source */
#define STATBIND_OPT_ARRAY		0x00000400U	/* Obter opt_... */
#define STATBIND_OPT_SEC_ARRAY		0x00000800U	/* Obter opt_sec... */
#define STATBIND_SUPPORTED_MASK		0x00001000U	/* Obter supported_mask */
#define STATBIND_BIND_UIDMAP		0x00002000U	/* Obter uidmap... */
#define STATBIND_BIND_GIDMAP		0x00004000U	/* Obter gidmap... */

/* ============================================================================
 * VALORES ESPECIAIS PARA LISTBIND
 * ============================================================================ */

#define LISTBIND_ROOT		0xffffffffffffffff	/* Root binding */
#define LISTBIND_REVERSE	(1 << 0)		/* Listar reverso */

/* ============================================================================
 * FLAGS PARA STATBIND
 * ============================================================================ */

#define STATBIND_BY_FD		0x00000001U	/* Obter info por FD */

/* ============================================================================
 * ESTRUTURAS ADICIONAIS PARA O SISTEMA DE OBJETOS
 * ============================================================================ */

/**
 * struct object_binding - Binding entre objeto e porta
 * 
 * Representa uma conexão onde um objeto se vincula a uma porta
 * para enviar/receber mensagens.
 */
struct object_binding {
	__u64		binding_id;	/* ID único do binding */
	__u64		object_id;	/* ID do objeto (OID) */
	__u64		port_id;	/* ID da porta */
	__u32		flags;		/* BIND_* flags */
	__u32		ref_count;	/* Contagem de referências */
	__u64		created_at;	/* Timestamp de criação */
	__u64		last_msg_at;	/* Última mensagem */
	__u64		msg_count;	/* Total de mensagens */
	__u64		bytes_tx;	/* Bytes transmitidos */
	__u64		bytes_rx;	/* Bytes recebidos */
};

/**
 * struct port_binding_stats - Estatísticas de binding de porta
 */
struct port_binding_stats {
	__u64		port_id;
	__u64		num_bindings;	/* Número de bindings ativos */
	__u64		active_senders;	/* Senders ativos */
	__u64		active_receivers; /* Receivers ativos */
	__u32		queue_usage;	/* Uso da fila (%) */
	__u32		flags;
};

/* * ============================================================================
 * LETRAS DE UNIDADE (DRIVE LETTERS)
 * ============================================================================
 */

/*
 * struct drive_letter - Representa uma letra de unidade montada
 * 
 * As letras de unidade (C:\, D:\, E:\) são identificadores simbólicos
 * que mapeiam para objetos raiz de sistemas de arquivos.
 */
struct drive_letter {
	__u8		letter;		/* Letra da unidade ('C', 'D', 'E', ...) */
	__u8		reserved[3];
	__u64		root_object_id;	/* OID do objeto raiz */
	__u64		mounted_at;	/* Timestamp de montagem */
	__u32		flags;		/* DRIVE_* flags */
	__u32		volume_serial;	/* Número serial do volume */
	__u64		total_space;
	__u64		free_space;
};

/**
 * Flags para letras de unidade
 */
#define DRIVE_REMOVABLE		(1 << 0)	/* Unidade removível (USB, CD) */
#define DRIVE_FIXED		(1 << 1)	/* Disco fixo */
#define DRIVE_REMOTE		(1 << 2)	/* Unidade de rede */
#define DRIVE_CDROM		(1 << 3)	/* CD-ROM/DVD */
#define DRIVE_RAMDISK		(1 << 4)	/* RAM disk */
#define DRIVE_VIRTUAL		(1 << 5)	/* Unidade virtual */
#define DRIVE_NO_ROOT		(1 << 6)	/* Sem objeto raiz (vazia) */

/*
 * Letras de unidade padrão do sistema
 */
#define DRIVE_LETTER_A		0x41	/* 'A' - Disquete (legado) */
#define DRIVE_LETTER_B		0x42	/* 'B' - Disquete (legado) */
#define DRIVE_LETTER_C		0x43	/* 'C' - Disco de boot principal */
#define DRIVE_LETTER_D		0x44	/* 'D' - Segundo disco ou CD-ROM */
#define DRIVE_LETTER_E		0x45	/* 'E' - Terceiro disco */
#define DRIVE_LETTER_Z		0x5A	/* 'Z' - Última letra disponível */

/*
 * Máximo de letras de unidade suportadas (26: A: até Z:)
 */
#define MAX_DRIVE_LETTERS	26

/**
 * Tipos de objetos do sistema de arquivos
 * 
 * "ARQUIVOS SÃO OBJETOS" - Todo arquivo é um objeto que implementa métodos
 * específicos para manipulação de dados.
 */
enum filesystem_object_type {
	FS_OBJ_TYPE_FILE	= 1,	/* Arquivo regular */
	FS_OBJ_TYPE_DIRECTORY	= 2,	/* Diretório */
	FS_OBJ_TYPE_SYMLINK	= 3,	/* Link simbólico */
	FS_OBJ_TYPE_DEVICE	= 4,	/* Dispositivo (deprecado) */
	FS_OBJ_TYPE_PIPE	= 5,	/* Pipe (deprecado - use portas) */
	FS_OBJ_TYPE_SOCKET	= 6,	/* Socket (deprecado - use portas) */
	FS_OBJ_TYPE_EXECUTABLE	= 7,	/* Executável (objeto com método execute) */
	FS_OBJ_TYPE_LIBRARY	= 8,	/* Biblioteca compartilhada */
	FS_OBJ_TYPE_MOUNT_ROOT	= 9,	/* Raiz de unidade montada (C:\, D:\) */
};

/**
 * struct fs_object - Representa um objeto no sistema de arquivos
 * 
 * "Arquivos são objetos" - Esta estrutura define um arquivo/diretório
 * como um objeto com OID, métodos, propriedades e capacidades.
 */
struct fs_object {
	__u64		oid;		/* Object ID (único no sistema) */
	__u64		parent_oid;	/* OID do diretório pai */
	__u8		drive_letter;	/* Letra da unidade ('C', 'D', etc.) */
	__u32		type;		/* filesystem_object_type */
	__u32		flags;		/* FS_OBJ_FLAG_* */
	__u64		size;		/* Tamanho em bytes */
	__u64		created_at;
	__u64		modified_at;
	__u64		accessed_at;
	__u32		uid;
	__u32		gid;
	__u32		mode;		/* Permissões Unix-like (opcional) */
	__u32		num_links;
	__u64		capability;	/* Capacidade base do objeto */
	char		name[256];	/* Nome do objeto (arquivo/diretório) */
};

/**
 * Flags para objetos do sistema de arquivos
 */
#define FS_OBJ_FLAG_HIDDEN	(1 << 0)	/* Objeto oculto */
#define FS_OBJ_FLAG_SYSTEM	(1 << 1)	/* Objeto de sistema */
#define FS_OBJ_FLAG_READONLY	(1 << 2)	/* Somente leitura */
#define FS_OBJ_FLAG_ARCHIVE	(1 << 3)	/* Arquivo archive */
#define FS_OBJ_FLAG_COMPRESSED	(1 << 4)	/* Comprimido */
#define FS_OBJ_FLAG_ENCRYPTED	(1 << 5)	/* Criptografado */
#define FS_OBJ_FLAG_IMMUTABLE	(1 << 6)	/* Imutável */
#define FS_OBJ_FLAG_APPEND_ONLY	(1 << 7)	/* Append only */

/* ============================================================================
 * MÉTODOS DE OBJETOS DE ARQUIVO
 * ============================================================================ */

/**
 * Métodos padrão que objetos de arquivo implementam
 * 
 * Cada "arquivo" é um objeto que responde a estas mensagens/métodos.
 */
enum file_object_methods {
	FILE_METHOD_READ	= 0x1001,	/* Ler dados */
	FILE_METHOD_WRITE	= 0x1002,	/* Escrever dados */
	FILE_METHOD_SEEK	= 0x1003,	/* Mudar posição */
	FILE_METHOD_CLOSE	= 0x1004,	/* Fechar objeto */
	FILE_METHOD_GETATTR	= 0x1005,	/* Obter atributos */
	FILE_METHOD_SETATTR	= 0x1006,	/* Definir atributos */
	FILE_METHOD_EXECUTE	= 0x1007,	/* Executar (executáveis) */
	FILE_METHOD_MAP		= 0x1008,	/* Mapear memória */
	FILE_METHOD_SYNC	= 0x1009,	/* Sincronizar */
	FILE_METHOD_LOCK	= 0x100A,	/* Lock/unlock */
};

/* ============================================================================
 * OPERAÇÕES DE MONTAGEM COM LETRAS ALEATÓRIAS
 * ============================================================================ */

/**
 * Comandos para gerenciamento de letras de unidade
 */
enum drive_command {
	DRIVE_MOUNT		= 1,	/* Montar disco com letra aleatória */
	DRIVE_MOUNT_FIXED	= 2,	/* Montar disco com letra específica */
	DRIVE_UNMOUNT		= 3,	/* Desmontar unidade */
	DRIVE_GET_LETTER	= 4,	/* Obter letra de uma unidade */
	DRIVE_LIST		= 5,	/* Listar todas as unidades */
	DRIVE_GET_OBJECT	= 6,	/* Obter objeto por caminho (C:\path) */
	DRIVE_CREATE_OBJECT	= 7,	/* Criar objeto (arquivo/diretório) */
	DRIVE_DELETE_OBJECT	= 8,	/* Deletar objeto */
};

/**
 * struct drive_mount_in - Entrada para montagem de disco
 */
struct drive_mount_in {
	__u64		device_id;	/* ID do dispositivo/partição */
	__u32		flags;		/* DRIVE_MOUNT_FLAG_* */
	__u32		fs_type;	/* Tipo do sistema de arquivos */
	__u8		suggested_letter; /* Letra sugerida (0 = aleatória) */
	__u8		reserved[7];
};

/**
 * struct drive_mount_out - Saída da montagem de disco
 */
struct drive_mount_out {
	__u8		assigned_letter;	/* Letra atribuída (C:, D:, etc.) */
	__u8		reserved[7];
	__u64		root_object_id;		/* OID do objeto raiz */
	__u64		mount_id;		/* ID da montagem */
	__u32		status;			/* Status da montagem */
	__u32		reserved2;
};

/**
 * Flags para montagem de disco
 */
#define DRIVE_MOUNT_FLAG_READONLY	(1 << 0)	/* Montar somente leitura */
#define DRIVE_MOUNT_FLAG_NOEXEC		(1 << 1)	/* Não executar */
#define DRIVE_MOUNT_FLAG_SYNCHRONOUS	(1 << 2)	/* I/O síncrono */
#define DRIVE_MOUNT_FLAG_NOATIME	(1 << 3)	/* Não atualizar atime */

/**
 * struct drive_unmount_in - Entrada para desmontagem
 */
struct drive_unmount_in {
	__u8		drive_letter;	/* Letra da unidade a desmontar */
	__u8		flags;		/* UNMOUNT_FLAG_* */
	__u8		reserved[6];
};

#define UNMOUNT_FLAG_FORCE	(1 << 0)	/* Forçar desmontagem */
#define UNMOUNT_FLAG_LAZY	(1 << 1)	/* Desmontagem lazy */

/**
 * struct drive_list_out - Lista de unidades montadas
 */
struct drive_list_out {
	__u32		num_drives;			/* Número de unidades */
	__u32		reserved;
	struct drive_letter drives[MAX_DRIVE_LETTERS];	/* Array de unidades */
};

/**
 * struct drive_get_object_in - Obter objeto por caminho
 * 
 * Exemplo: caminho = "C:\LinuxOS\note.coff"
 */
struct drive_get_object_in {
	__u8		drive_letter;			/* Letra da unidade */
	__u8		reserved[7];
	__u64		path_offset;			/* Offset do caminho */
	__u32		path_len;			/* Comprimento do caminho */
	__u32		flags;
};

struct drive_get_object_out {
	__u64		object_id;			/* OID do objeto */
	struct fs_object object_info;			/* Informações do objeto */
	__u32		status;
	__u32		reserved;
};


#endif /* _UAPI_LINUX_MOUNT_H */
