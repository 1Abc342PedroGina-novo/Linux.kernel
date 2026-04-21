/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FS_STRUCT_H
#define _LINUX_FS_STRUCT_H

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/seqlock.h>
#include <linux/ipc.h>
#include <linux/types.h>

/* ============================================================================
 * ESTRUTURA DO SISTEMA DE ARQUIVOS (ESTILO WINDOWS)
 * ============================================================================ */

/**
 * struct fs_object_context - Contexto de objeto do filesystem
 * 
 * Substitui o conceito Unix de "root" e "pwd" por:
 *   - Drive letter atual (C:\, D:\, etc.)
 *   - Object ID do diretório atual
 *   - Porta de comunicação com o FS
 */
struct fs_object_context {
	__u8		current_drive;		/* Letra da unidade atual ('C', 'D') */
	__u8		reserved[7];
	__u64		current_dir_oid;	/* OID do diretório atual */
	__u64		root_dir_oid;		/* OID do diretório root da unidade */
	__u64		fs_port_id;		/* Porta IPC do filesystem */
	__u64		object_port_id;		/* Porta do objeto atual */
	
	/* Para compatibilidade com código legado (paths Unix) */
	__u64		legacy_root_oid;	/* OID do root legado (se houver) */
	__u64		legacy_pwd_oid;		/* OID do pwd legado */
	
	/* Capacidades e permissões */
	__u64		capability;		/* Capacidade base */
	__u32		umask;			/* Umask (estilo Unix, deprecado) */
	__u32		flags;			/* FS_CTX_FLAG_* */
};

/* Flags para contexto do filesystem */
#define FS_CTX_FLAG_NO_UNIX_PATHS	(1 << 0)	/* Não usar paths Unix */
#define FS_CTX_FLAG_USE_OBJECT_IDS	(1 << 1)	/* Usar OIDs diretamente */
#define FS_CTX_FLAG_DRIVE_LETTERS	(1 << 2)	/* Usar letras de unidade */
#define FS_CTX_FLAG_LEGACY_COMPAT	(1 << 3)	/* Compatibilidade legada */

/**
 * struct windows_path_context - Contexto de caminho Windows
 * 
 * Representa um caminho no estilo Windows: C:\Windows\System32
 */
struct windows_path_context {
	__u8		drive_letter;		/* 'C', 'D', 'E', etc. */
	__u8		reserved[7];
	__u64		start_oid;		/* OID inicial para lookup */
	__u64		target_oid;		/* OID alvo (após resolução) */
	__u64		port_id;		/* Porta para comunicação */
	char		path[260];		/* Caminho (MAX_PATH) */
	__u32		path_len;
	__u32		flags;			/* WINDOWS_PATH_FLAG_* */
};

#define WINDOWS_PATH_FLAG_ABSOLUTE	(1 << 0)	/* Caminho absoluto (C:\) */
#define WINDOWS_PATH_FLAG_RELATIVE	(1 << 1)	/* Caminho relativo */
#define WINDOWS_PATH_FLAG_UNC		(1 << 2)	/* UNC path (\\SERVER\Share) */
#define WINDOWS_PATH_FLAG_LONG		(1 << 3)	/* Caminho longo (\\?\) */

/* ============================================================================
 * ESTRUTURA FS_STRUCT MODIFICADA
 * ============================================================================ */

/**
 * struct fs_struct - Estrutura do sistema de arquivos por processo
 * 
 * AGORA COM:
 *   - Drive letter atual (C:\, D:\, E:\)
 *   - Object IDs em vez de paths Unix
 *   - Portas para comunicação RPC
 * 
 * NÃO EXISTE MAIS:
 *   - struct path root (substituído por root_dir_oid)
 *   - struct path pwd (substituído por current_dir_oid)
 *   - Caminhos Unix (/usr/bin)
 */
struct fs_struct {
	int			users;		/* Número de usuários */
	seqlock_t		seq;		/* Lock para acesso concorrente */
	
	/* Novo modelo (Windows-style) */
	__u8			current_drive;	/* Letra da unidade atual */
	__u8			reserved[3];
	int			umask;		/* Umask (deprecado - use capabilities) */
	int			in_exec;	/* Em execução */
	
	/* Object IDs em vez de paths */
	__u64			current_dir_oid;	/* OID do diretório atual */
	__u64			root_dir_oid;		/* OID do root da unidade */
	__u64			fs_port_id;		/* Porta do FS */
	
	/* Para compatibilidade com código legado (paths Unix) */
	struct {
		__u64		legacy_root_oid;	/* Root legado */
		__u64		legacy_pwd_oid;		/* PWD legado */
		__u32		legacy_flags;		/* Flags legadas */
	} legacy;
	
	/* Capacidades */
	__u64			capability_mask;	/* Máscara de capacidades */
	
	/* Estatísticas */
	__u64			ops_count;		/* Contador de operações */
	__u64			last_access;		/* Último acesso */
} __randomize_layout;

/* ============================================================================
 * DECLARAÇÕES DE FUNÇÕES (MODIFICADAS)
 * ============================================================================ */

extern struct kmem_cache *fs_cachep;

/* Funções principais */
extern void exit_fs(struct task_struct *);
extern struct fs_struct *copy_fs_struct(struct fs_struct *);
extern void free_fs_struct(struct fs_struct *);
extern int unshare_fs_struct(void);

/* Novas funções (Windows-style) */
extern int set_current_drive(__u8 drive_letter);
extern int set_current_directory_oid(__u64 oid);
extern __u64 get_current_directory_oid(void);
extern __u8 get_current_drive(void);
extern int set_root_directory_oid(__u64 oid);
extern __u64 get_root_directory_oid(void);

/* Funções de resolução de caminho Windows */
extern int resolve_windows_path(struct windows_path_context *ctx);
extern int get_object_by_path(__u8 drive_letter, const char *path, __u64 *oid);
extern int get_path_by_object(__u64 oid, char *path, size_t path_len);

/* Funções legadas (deprecadas - mantidas para compatibilidade) */
extern void set_fs_root(struct fs_struct *, const struct path *);
extern void set_fs_pwd(struct fs_struct *, const struct path *);
static inline void get_fs_root(struct fs_struct *fs, struct path *root)
{
	/* DEPRECADO - Use get_root_directory_oid() em vez disso */
	read_seqlock_excl(&fs->seq);
	/* Compatibilidade: retorna um path vazio */
	memset(root, 0, sizeof(*root));
	read_sequnlock_excl(&fs->seq);
}

static inline void get_fs_pwd(struct fs_struct *fs, struct path *pwd)
{
	/* DEPRECADO - Use get_current_directory_oid() em vez disso */
	read_seqlock_excl(&fs->seq);
	/* Compatibilidade: retorna um path vazio */
	memset(pwd, 0, sizeof(*pwd));
	read_sequnlock_excl(&fs->seq);
}

/* Funções auxiliares */
extern bool current_chrooted(void);	/* DEPRECADO - Sempre retorna false */
static inline int current_umask(void)
{
	/* Umask é deprecado - use capabilities */
	return current->fs->umask;
}

/* ============================================================================
 * FUNÇÕES PARA COMUNICAÇÃO VIA MENSAGENS
 * ============================================================================ */

/**
 * Envia mensagem para o FS atual
 */
static inline int fs_send_message(__u32 cmd, void *data, size_t data_size)
{
	struct fs_struct *fs = current->fs;
	
	if (!fs->fs_port_id)
		return -ENODEV;
	
	/* Aqui seria chamado o sistema de portas/mensagens */
	/* return port_send(fs->fs_port_id, cmd, data, data_size); */
	return 0;
}

/**
 * Envia mensagem para um objeto específico
 */
static inline int object_send_message(__u64 oid, __u32 cmd, void *data, size_t data_size)
{
	struct fs_struct *fs = current->fs;
	__u64 port_id;
	
	/* Obter porta do objeto pelo OID */
	/* port_id = object_get_port(oid); */
	port_id = 0; /* Placeholder */
	
	if (!port_id)
		return -ENOENT;
	
	/* return port_send(port_id, cmd, data, data_size); */
	return 0;
}

/* ============================================================================
 * MACROS PARA ACESSO RÁPIDO
 * ============================================================================ */

/* Obter drive atual */
#define current_drive()		(current->fs->current_drive)

/* Obter OID do diretório atual */
#define current_dir_oid()	(current->fs->current_dir_oid)

/* Obter OID do root da unidade */
#define root_dir_oid()		(current->fs->root_dir_oid)

/* Obter porta do FS */
#define current_fs_port()	(current->fs->fs_port_id)

/* Verificar se está usando novo modelo */
#define fs_using_windows_model(fs)	((fs)->current_drive != 0)

/* ============================================================================
 * ESTRUTURAS LEGADAS (MANTIDAS PARA COMPATIBILIDADE)
 * ============================================================================ */

/*
 * struct fs_struct_legacy - Versão legada (NÃO USE)
 * 
 * Mantida apenas para compatibilidade binária com módulos antigos.
 */
struct fs_struct_legacy {
	int users;
	seqlock_t seq;
	int umask;
	int in_exec;
	struct path root, pwd;
} __randomize_layout;


#endif /* _LINUX_FS_STRUCT_H */
