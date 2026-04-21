/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Definitions for drive/volume mounting interface (Windows-style).
 *
 * This describes the linked list of mounted volumes with drive letters.
 * 
 * MODIFIED FOR: Windows-style drive letters (C:\, D:\, E:\)
 * 
 * NÃO EXISTE MAIS:
 *   - Mount points no estilo Unix (/mnt, /media)
 *   - Hierarquia única de montagem
 * 
 * AGORA EXISTE:
 *   - Drive letters (C:\, D:\, E:\)
 *   - Volume objects (objetos de volume)
 *   - Portas de comunicação com volumes *
 * Author:  Marco van Wieringen <mvw@planets.elm.net>
 * Modified for Windows-style drive letters
 */
#ifndef _LINUX_MOUNT_H
#define _LINUX_MOUNT_H

#include <linux/types.h>
#include <asm/barrier.h>

/*
 * ============================================================================
 * FILOSOFIA: MONTAGEM NO ESTILO WINDOWS
 * ============================================================================
 * 
 * NÃO EXISTE MAIS O CONCEITO DE "MOUNT POINT" NO ESTILO UNIX!
 * 
 * UNIX (DEPRECADO):                    WINDOWS (ATUAL)
 * -------------------------------      ------------------------------
 * mount /dev/sda1 /mnt/disk           mount D:\ (letra aleatória)
 * / (root único)                      C:\, D:\, E:\ (múltiplas raízes)
 * Hierarquia única                    Namespaces independentes
 * 
 * ARQUIVOS SÃO OBJETOS DE MENSAGEM:
 *   - Cada volume é um objeto com OID
 *   - Comunicação via portas (IPC/RPC)
 *   - Acesso por letra de unidade
 * 
 * FILESYSTEMS EM RING 0: NFS, EXFAT, FAT, NTFS, HFS, HFS+
 * 
 * ============================================================================
 */

struct super_block;
struct dentry;
struct user_namespace;
struct mnt_idmap;
struct file_system_type;
struct fs_context;
struct file;
struct path;
struct volume_object;
struct drive_letter;

/* ============================================================================
 * LETRAS DE UNIDADE (DRIVE LETTERS)
 * ============================================================================ */

/* Letras de unidade suportadas: C:\, D:\, E:\, ..., Z:\ */
#define MAX_DRIVE_LETTERS		26
#define INVALID_DRIVE_LETTER		0

/* Letras de unidade padrão */
#define DRIVE_LETTER_C			('C')
#define DRIVE_LETTER_D			('D')
#define DRIVE_LETTER_E			('E')
#define DRIVE_LETTER_F			('F')
#define DRIVE_LETTER_Z			('Z')

/* Converter letra para índice (0-25) */
#define DRIVE_LETTER_TO_IDX(c)		(((c) >= 'A' && (c) <= 'Z') ? ((c) - 'A') : \
					 ((c) >= 'a' && (c) <= 'z') ? ((c) - 'a') : -1)

/* Converter índice para letra */
#define DRIVE_IDX_TO_LETTER(i)		('A' + (i))

/* Verificar se letra de unidade é válida */
#define IS_VALID_DRIVE_LETTER(c)	(DRIVE_LETTER_TO_IDX(c) >= 0 && \
					 DRIVE_LETTER_TO_IDX(c) < MAX_DRIVE_LETTERS)

/* ============================================================================
 * FLAGS DE MONTAGEM (ESTILO WINDOWS)
 * ============================================================================ */

enum mount_flags {
	/* Flags de proteção (Windows-style) */
	MNT_NOSUID	= 0x01,		/* Ignorar bits SUID/SGID */
	MNT_NODEV	= 0x02,		/* Desabilitar acesso a dispositivos */
	MNT_NOEXEC	= 0x04,		/* Desabilitar execução */
	MNT_NOATIME	= 0x08,		/* Não atualizar access time */
	MNT_NODIRATIME	= 0x10,		/* Não atualizar dir access time */
	MNT_RELATIME	= 0x20,		/* Atualizar atime relativo */
	MNT_READONLY	= 0x40,		/* Montagem somente leitura */
	MNT_NOSYMFOLLOW	= 0x80,		/* Não seguir symlinks */
	
	/* Flags específicas do Windows */
	MNT_COMPRESSED	= 0x100,	/* Volume comprimido (NTFS) */
	MNT_ENCRYPTED	= 0x200,	/* Volume criptografado (BitLocker) */
	MNT_REMOVABLE	= 0x400,	/* Unidade removível (USB) */
	MNT_NETWORK	= 0x800,	/* Unidade de rede mapeada */
	MNT_CDROM	= 0x1000,	/* CD-ROM/DVD/Blu-ray */
	MNT_RAMDISK	= 0x2000,	/* RAM disk */
	MNT_VIRTUAL	= 0x4000,	/* Unidade virtual (SUBST) */
	
	MNT_INTERNAL	= 0x8000,	/* Montagem interna do kernel */
	
	/* Flags de lock */
	MNT_LOCK_ATIME		= 0x040000,
	MNT_LOCK_NOEXEC		= 0x080000,
	MNT_LOCK_NOSUID		= 0x100000,
	MNT_LOCK_NODEV		= 0x200000,
	MNT_LOCK_READONLY	= 0x400000,
	MNT_LOCKED		= 0x800000,
	MNT_DOOMED		= 0x1000000,
	MNT_SYNC_UMOUNT		= 0x2000000,
	MNT_UMOUNT		= 0x8000000,
	
	/* Máscaras */
	MNT_USER_SETTABLE_MASK  = MNT_NOSUID | MNT_NODEV | MNT_NOEXEC
				  | MNT_NOATIME | MNT_NODIRATIME | MNT_RELATIME
				  | MNT_READONLY | MNT_NOSYMFOLLOW,
	MNT_ATIME_MASK = MNT_NOATIME | MNT_NODIRATIME | MNT_RELATIME,
	MNT_WINDOWS_MASK = MNT_COMPRESSED | MNT_ENCRYPTED | MNT_REMOVABLE |
			   MNT_NETWORK | MNT_CDROM | MNT_RAMDISK | MNT_VIRTUAL,
	
	MNT_INTERNAL_FLAGS = MNT_INTERNAL | MNT_DOOMED |
			     MNT_SYNC_UMOUNT | MNT_LOCKED
};

/* ============================================================================
 * TIPOS DE UNIDADE (DRIVE TYPES)
 * ============================================================================ */

enum drive_type {
	DRIVE_TYPE_UNKNOWN	= 0,
	DRIVE_TYPE_NO_ROOT_DIR	= 1,	/* Raiz não existe */
	DRIVE_TYPE_REMOVABLE	= 2,	/* Disquete, USB, etc. */
	DRIVE_TYPE_FIXED	= 3,	/* Disco fixo */
	DRIVE_TYPE_REMOTE	= 4,	/* Unidade de rede */
	DRIVE_TYPE_CDROM	= 5,	/* CD-ROM/DVD */
	DRIVE_TYPE_RAMDISK	= 6,	/* RAM disk */
};

/* ============================================================================
 * VOLUME OBJECT - ESTRUTURA PRINCIPAL (SUBSTITUI VFSMOUNT)
 * ============================================================================ */

/**
 * struct volume_object - Representa um volume montado (C:\, D:\, etc.)
 * 
 * Substitui struct vfsmount no modelo Windows.
 * Cada volume tem uma letra de unidade e um objeto associado.
 */
struct volume_object {
	/* Letra de unidade (C, D, E, etc.) */
	unsigned char		drive_letter;	/* 'C', 'D', 'E', ... */
	unsigned char		reserved[3];
	
	/* Object ID do volume (sistema de objetos) */
	__u64			volume_oid;	/* Object ID único */
	__u64			root_object_oid;	/* OID do diretório root */
	
	/* Portas de comunicação */
	__u64			command_port;	/* Porta para comandos do volume */
	__u64			data_port;	/* Porta para transferência de dados */
	__u64			notify_port;	/* Porta para notificações */
	
	/* Dados do filesystem */
	struct super_block	*mnt_sb;	/* Superblock do FS */
	struct dentry		*mnt_root;	/* Root dentry (compatibilidade) */
	struct file_system_type	*fs_type;	/* Tipo do filesystem */
	
	/* Flags e permissões */
	int			mnt_flags;	/* MNT_* flags */
	struct mnt_idmap	*mnt_idmap;	/* ID mapping */
	
	/* Tipo da unidade */
	enum drive_type		drive_type;
	
	/* Informações do volume */
	__u64			total_size;	/* Tamanho total em bytes */
	__u64			free_size;	/* Espaço livre em bytes */
	__u32			sector_size;	/* Tamanho do setor */
	__u32			cluster_size;	/* Tamanho do cluster */
	char			volume_label[32];	/* Rótulo do volume */
	char			file_system_name[16];	/* "NTFS", "FAT32", etc. */
	
	/* Para compatibilidade com código legado */
	struct vfsmount		*legacy_mnt;	/* Vfsmount legado (se existir) */
	
	/* Lista de volumes montados */
	struct list_head	volume_list;
	
	/* Sincronização */
	spinlock_t		lock;
	struct rcu_head		rcu;
} __randomize_layout;

/* ============================================================================
 * ESTRUTURA LEGADA VFSMOUNT (MANTIDA PARA COMPATIBILIDADE)
 * ============================================================================ */

/**
 * struct vfsmount - Estrutura legada (NÃO USE PARA NOVOS VOLUMES!)
 * 
 * Mantida apenas para compatibilidade com código antigo.
 * Novos desenvolvimentos devem usar struct volume_object.
 */
struct vfsmount {
	struct dentry *mnt_root;	/* root of the mounted tree */
	struct super_block *mnt_sb;	/* pointer to superblock */
	int mnt_flags;
	struct mnt_idmap *mnt_idmap;
} __randomize_layout;

/* ============================================================================
 * FUNÇÕES AUXILIARES PARA VOLUME_OBJECT
 * ============================================================================ */

static inline struct mnt_idmap *mnt_idmap(const struct vfsmount *mnt)
{
	/* Pairs with smp_store_release() in do_idmap_mount(). */
	return READ_ONCE(mnt->mnt_idmap);
}

/* Obter letra de unidade do volume */
static inline unsigned char volume_drive_letter(const struct volume_object *vol)
{
	return vol->drive_letter;
}

/* Verificar se volume é somente leitura */
static inline bool volume_is_readonly(const struct volume_object *vol)
{
	return vol->mnt_flags & MNT_READONLY;
}

/* Verificar se volume é removível */
static inline bool volume_is_removable(const struct volume_object *vol)
{
	return vol->drive_type == DRIVE_TYPE_REMOVABLE;
}

/* ============================================================================
 * FUNÇÕES PARA GERENCIAMENTO DE VOLUMES (ESTILO WINDOWS)
 * ============================================================================ */

/* Montar volume com letra aleatória (C:\, D:\, E:\) */
extern struct volume_object *volume_mount_auto(struct file_system_type *fs_type,
						int flags, const char *device,
						void *data);

/* Montar volume com letra específica */
extern struct volume_object *volume_mount_fixed(unsigned char drive_letter,
						struct file_system_type *fs_type,
						int flags, const char *device,
						void *data);

/* Desmontar volume */
extern int volume_umount(struct volume_object *vol);
extern int volume_umount_force(struct volume_object *vol);

/* Obter volume por letra de unidade */
extern struct volume_object *volume_get_by_letter(unsigned char drive_letter);
extern struct volume_object *volume_get_by_oid(__u64 volume_oid);

/* Obter objeto raiz do volume (C:\) */
extern __u64 volume_get_root_object(struct volume_object *vol);

/* Enviar mensagem para o volume */
extern int volume_send_message(struct volume_object *vol, __u32 cmd,
			       void *data, size_t data_size);

/* ============================================================================
 * FUNÇÕES LEGADAS (MANTIDAS PARA COMPATIBILIDADE)
 * ============================================================================ */

extern int mnt_want_write(struct vfsmount *mnt);
extern int mnt_want_write_file(struct file *file);
extern void mnt_drop_write(struct vfsmount *mnt);
extern void mnt_drop_write_file(struct file *file);
extern void mntput(struct vfsmount *mnt);
extern struct vfsmount *mntget(struct vfsmount *mnt);
extern void mnt_make_shortterm(struct vfsmount *mnt);
extern struct vfsmount *mnt_clone_internal(const struct path *path);
extern bool __mnt_is_readonly(const struct vfsmount *mnt);
extern bool mnt_may_suid(struct vfsmount *mnt);

extern struct vfsmount *clone_private_mount(const struct path *path);
int mnt_get_write_access(struct vfsmount *mnt);
void mnt_put_write_access(struct vfsmount *mnt);

extern struct vfsmount *fc_mount(struct fs_context *fc);
extern struct vfsmount *fc_mount_longterm(struct fs_context *fc);
extern struct vfsmount *vfs_create_mount(struct fs_context *fc);
extern struct vfsmount *vfs_kern_mount(struct file_system_type *type,
				      int flags, const char *name,
				      void *data);

extern void mnt_set_expiry(struct vfsmount *mnt, struct list_head *expiry_list);
extern void mark_mounts_for_expiry(struct list_head *mounts);

extern bool path_is_mountpoint(const struct path *path);
extern bool our_mnt(struct vfsmount *mnt);
extern struct vfsmount *kern_mount(struct file_system_type *);
extern void kern_unmount(struct vfsmount *mnt);
extern int may_umount_tree(struct vfsmount *);
extern int may_umount(struct vfsmount *);
int do_mount(const char *, const char __user *,
		     const char *, unsigned long, void *);
extern const struct path *collect_paths(const struct path *, struct path *, unsigned);
extern void drop_collected_paths(const struct path *, const struct path *);
extern void kern_unmount_array(struct vfsmount *mnt[], unsigned int num);

extern int cifs_root_data(char **dev, char **opts);

#endif
