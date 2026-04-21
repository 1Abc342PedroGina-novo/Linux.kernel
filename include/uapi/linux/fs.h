#ifndef _UAPI_LINUX_FS_H
#define _UAPI_LINUX_FS_H

/*
 * ============================================================================
 * FILESYSTEM NO ESTILO WINDOWS
 * ============================================================================
 * 
 * AVISO: Este arquivo foi modificado para o modelo de filesystem no estilo
 *        Windows, NÃO o modelo tradicional Unix.
 * 
 * DIFERENÇAS PRINCIPAIS:
 * 
 *   UNIX (DEPRECADO)                    WINDOWS (ATUAL)
 *   -------------------------------     ------------------------------
 *   / (root único)                      C:\, D:\, E:\ (múltiplas raízes)
 *   /mnt, /media (mount points)         Letras de unidade (C:\, D:\)
 *   mount /dev/sda1 /mnt/disk           mount D:\ (letra aleatória)
 *   Hierarquia única                    Namespaces independentes
 * 
 * FILOSOFIA: "ARQUIVOS SÃO OBJETOS"
 * 
 * Todo arquivo é um objeto com:
 *   - Object ID (OID) único
 *   - Letra de unidade (C:, D:, etc.)
 *   - Caminho dentro da unidade
 *   - Métodos (read, write, execute)
 * 
 * ============================================================================
 */

#include <linux/limits.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#ifndef __KERNEL__
#include <linux/fscrypt.h>
#endif

/* Use of MS_* flags within the kernel is restricted to core mount(2) code. */
#if !defined(__KERNEL__)
#include <linux/mount.h>
#endif

/* ============================================================================
 * LIMITES DO SISTEMA DE ARQUIVOS
 * ============================================================================ */

#undef NR_OPEN
#define INR_OPEN_CUR 1024	/* Initial setting for nfile rlimits */
#define INR_OPEN_MAX 4096	/* Hard limit for nfile rlimits */

#define BLOCK_SIZE_BITS 10
#define BLOCK_SIZE (1<<BLOCK_SIZE_BITS)

/* ============================================================================
 * LETRAS DE UNIDADE (DRIVE LETTERS)
 * ============================================================================ */

/*
 * Letras de unidade suportadas: C:\, D:\, E:\, ..., Z:\
 * 
 * C:\ - Disco de boot (geralmente fixo)
 * D:\ - Segundo disco ou CD-ROM
 * E:\ - Terceiro disco
 * F:\ - USB drive (montado aleatoriamente)
 * ...
 * Z:\ - Última letra disponível
 */
#define MAX_DRIVE_LETTERS	26
#define DRIVE_C			('C' - 'A')  /* 2 */
#define DRIVE_D			('D' - 'A')  /* 3 */
#define DRIVE_E			('E' - 'A')  /* 4 */
#define DRIVE_F			('F' - 'A')  /* 5 */
#define DRIVE_Z			('Z' - 'A')  /* 25 */

#define DRIVE_LETTER_TO_IDX(c)	(((c) >= 'A' && (c) <= 'Z') ? ((c) - 'A') : \
				 ((c) >= 'a' && (c) <= 'z') ? ((c) - 'a') : -1)

#define DRIVE_IDX_TO_LETTER(i)	('A' + (i))

/*
 * Estrutura para letra de unidade
 */
struct drive_letter_info {
	__u8		drive_letter;	/* 'C', 'D', 'E', etc. */
	__u8		reserved[3];
	__u64		root_object_id;	/* OID do objeto raiz */
	__u64		total_space;	/* Espaço total em bytes */
	__u64		free_space;	/* Espaço livre em bytes */
	__u32		drive_type;	/* DRIVE_TYPE_* */
	__u32		flags;		/* DRIVE_FLAG_* */
	__u8		volume_label[32];	/* Rótulo do volume */
	__u8		file_system[16];	/* "NTFS", "FAT32", "exFAT", etc. */
};

#define DRIVE_TYPE_FIXED		1	/* Disco fixo interno */
#define DRIVE_TYPE_REMOVABLE		2	/* USB, SD Card */
#define DRIVE_TYPE_CDROM		3	/* CD-ROM/DVD/Blu-ray */
#define DRIVE_TYPE_RAMDISK		4	/* RAM disk */
#define DRIVE_TYPE_NETWORK		5	/* Unidade de rede mapeada */
#define DRIVE_TYPE_VIRTUAL		6	/* Unidade virtual (SUBST) */

#define DRIVE_FLAG_READONLY		(1 << 0)	/* Somente leitura */
#define DRIVE_FLAG_COMPRESSED		(1 << 1)	/* Compressão habilitada */
#define DRIVE_FLAG_ENCRYPTED		(1 << 2)	/* BitLocker/criptografia */
#define DRIVE_FLAG_REMOVABLE		(1 << 3)	/* Removível */
#define DRIVE_FLAG_NETWORK		(1 << 4)	/* Unidade de rede */
#define DRIVE_FLAG_NO_ROOT		(1 << 5)	/* Sem objeto raiz (vazia) */

/* ============================================================================
 * CAMINHOS NO ESTILO WINDOWS
 * ============================================================================ */

/*
 * Estrutura para caminho no formato Windows
 * 
 * Exemplos:
 *   - "C:\Windows\System32\notepad.exe"
 *   - "D:\Documentos\arquivo.txt"
 *   - "\\SERVER\Share\file.txt" (UNC path - rede)
 */
struct windows_path {
	__u8		drive_letter;	/* 'C', 'D', etc. (0 = UNC path) */
	__u8		reserved[3];
	__u64		path_offset;	/* Offset do caminho na string */
	__u32		path_length;	/* Comprimento do caminho */
	__u32		flags;		/* PATH_FLAG_* */
};

#define PATH_FLAG_UNC		(1 << 0)	/* Caminho UNC (\\SERVER\Share) */
#define PATH_FLAG_ROOT		(1 << 1)	/* Raiz da unidade (C:\) */
#define PATH_FLAG_RELATIVE	(1 << 2)	/* Caminho relativo */
#define PATH_FLAG_LONG		(1 << 3)	/* Caminho longo (>260 chars) */
#define PATH_FLAG_DEVICE	(1 << 4)	/* Caminho de dispositivo (\\.\C:) */

/*
 * Comprimento máximo de caminho no Windows
 */
#define MAX_PATH_LENGTH		260		/* Caminho tradicional */
#define MAX_LONG_PATH_LENGTH	32767		/* Caminho longo (\\?\) */

/* ============================================================================
 * OBJETOS DO FILESYSTEM (ARQUIVOS SÃO OBJETOS)
 * ============================================================================ */

/*
 * Tipos de objeto do filesystem
 */
enum fs_object_type {
	FS_OBJ_TYPE_NONE	= 0,
	FS_OBJ_TYPE_FILE	= 1,	/* Arquivo regular */
	FS_OBJ_TYPE_DIRECTORY	= 2,	/* Diretório */
	FS_OBJ_TYPE_SYMLINK	= 3,	/* Link simbólico */
	FS_OBJ_TYPE_JUNCTION	= 4,	/* Junção (NTFS) */
	FS_OBJ_TYPE_HARDLINK	= 5,	/* Hard link */
	FS_OBJ_TYPE_PIPE	= 6,	/* Named pipe (deprecado - use portas) */
	FS_OBJ_TYPE_SOCKET	= 7,	/* Socket (deprecado - use portas) */
	FS_OBJ_TYPE_CHAR_DEV	= 8,	/* Dispositivo de caractere */
	FS_OBJ_TYPE_BLOCK_DEV	= 9,	/* Dispositivo de bloco */
	FS_OBJ_TYPE_EXECUTABLE	= 10,	/* Executável (.exe, .dll) */
};

/*
 * Atributos de arquivo no estilo Windows
 */
#define FS_ATTR_READONLY	0x00000001
#define FS_ATTR_HIDDEN		0x00000002
#define FS_ATTR_SYSTEM		0x00000004
#define FS_ATTR_DIRECTORY	0x00000010
#define FS_ATTR_ARCHIVE		0x00000020
#define FS_ATTR_DEVICE		0x00000040
#define FS_ATTR_NORMAL		0x00000080
#define FS_ATTR_TEMPORARY	0x00000100
#define FS_ATTR_SPARSE_FILE	0x00000200
#define FS_ATTR_REPARSE_POINT	0x00000400
#define FS_ATTR_COMPRESSED	0x00000800
#define FS_ATTR_OFFLINE		0x00001000
#define FS_ATTR_NOT_CONTENT_INDEXED	0x00002000
#define FS_ATTR_ENCRYPTED	0x00004000
#define FS_ATTR_INTEGRITY_STREAM 0x00008000
#define FS_ATTR_VIRTUAL		0x00010000
#define FS_ATTR_NO_SCRUB_DATA	0x00020000
#define FS_ATTR_EA		0x00040000
#define FS_ATTR_PINNED		0x00080000
#define FS_ATTR_UNPINNED	0x00100000
#define FS_ATTR_RECALL_ON_OPEN	0x00200000
#define FS_ATTR_RECALL_ON_DATA_ACCESS 0x00400000

/*
 * Estrutura de objeto do filesystem (arquivo/diretório)
 */
struct fs_object {
	__u64		oid;			/* Object ID (único) */
	__u64		parent_oid;		/* OID do diretório pai */
	__u8		drive_letter;		/* Letra da unidade ('C', 'D') */
	__u8		reserved[3];
	__u32		type;			/* fs_object_type */
	__u32		attributes;		/* FS_ATTR_* */
	__u64		size;			/* Tamanho em bytes */
	__u64		allocated_size;		/* Espaço alocado em disco */
	__u64		created_at;		/* Timestamp de criação */
	__u64		modified_at;		/* Timestamp de modificação */
	__u64		accessed_at;		/* Timestamp de acesso */
	__u64		change_at;		/* Timestamp de change (NTFS) */
	__u32		uid;			/* Owner ID */
	__u32		gid;			/* Group ID */
	__u32		security_id;		/* Security ID (SID) */
	__u32		hard_links;		/* Número de hard links */
	__u64		flags;			/* FS_OBJ_FLAG_* */
	__u8		name[256];		/* Nome do objeto */
};

#define FS_OBJ_FLAG_DELETE_PENDING	(1 << 0)	/* Marcação para deleção */
#define FS_OBJ_FLAG_RENAME_PENDING	(1 << 1)	/* Renomeação pendente */
#define FS_OBJ_FLAG_REPARSE		(1 << 2)	/* É um reparse point */
#define FS_OBJ_FLAG_SPARSE		(1 << 3)	/* Arquivo esparso */
#define FS_OBJ_FLAG_COMPRESSED		(1 << 4)	/* Comprimido */
#define FS_OBJ_FLAG_ENCRYPTED		(1 << 5)	/* Criptografado */
#define FS_OBJ_FLAG_NOT_INDEXED		(1 << 6)	/* Não indexado */
#define FS_OBJ_FLAG_OFFLINE		(1 << 7)	/* Offline */
#define FS_OBJ_FLAG_IMMUTABLE		(1 << 8)	/* Imutável */
#define FS_OBJ_FLAG_APPEND_ONLY		(1 << 9)	/* Append only */
#define FS_OBJ_FLAG_NOATIME		(1 << 10)	/* Não atualizar atime */
#define FS_OBJ_FLAG_SYNC		(1 << 11)	/* Escrita síncrona */
#define FS_OBJ_FLAG_DIRSYNC		(1 << 12)	/* Diretório síncrono */

/* ============================================================================
 * MÉTODOS DE OBJETOS DE ARQUIVO
 * ============================================================================ */

/*
 * Métodos que objetos de arquivo implementam
 */
#define FILE_METHOD_READ		0x1001
#define FILE_METHOD_WRITE		0x1002
#define FILE_METHOD_SEEK		0x1003
#define FILE_METHOD_CLOSE		0x1004
#define FILE_METHOD_GETATTR		0x1005
#define FILE_METHOD_SETATTR		0x1006
#define FILE_METHOD_EXECUTE		0x1007	/* Para executáveis */
#define FILE_METHOD_MAP			0x1008	/* Memory-mapped I/O */
#define FILE_METHOD_SYNC		0x1009
#define FILE_METHOD_LOCK		0x100A
#define FILE_METHOD_UNLOCK		0x100B
#define FILE_METHOD_IOCTL		0x100C	/* IOCTL legado */
#define FILE_METHOD_QUERY		0x100D	/* Query info */
#define FILE_METHOD_SET			0x100E	/* Set info */

/* ============================================================================
 * OPERAÇÕES DE FILESYSTEM (ESTILO WINDOWS)
 * ============================================================================ */

/* flags for integrity meta */
#define IO_INTEGRITY_CHK_GUARD		(1U << 0)
#define IO_INTEGRITY_CHK_REFTAG		(1U << 1)
#define IO_INTEGRITY_CHK_APPTAG		(1U << 2)

#define IO_INTEGRITY_VALID_FLAGS (IO_INTEGRITY_CHK_GUARD | \
				  IO_INTEGRITY_CHK_REFTAG | \
				  IO_INTEGRITY_CHK_APPTAG)

#define SEEK_SET	0	/* seek relative to beginning of file */
#define SEEK_CUR	1	/* seek relative to current file position */
#define SEEK_END	2	/* seek relative to end of file */
#define SEEK_DATA	3	/* seek to the next data */
#define SEEK_HOLE	4	/* seek to the next hole */
#define SEEK_MAX	SEEK_HOLE

/*
 * Rename flags (Windows style)
 */
#define RENAME_NOREPLACE	(1 << 0)	/* Don't overwrite target */
#define RENAME_EXCHANGE		(1 << 1)	/* Exchange source and dest */
#define RENAME_WHITEOUT		(1 << 2)	/* Whiteout source */

/* ============================================================================
 * ESTRUTURAS PARA OPERAÇÕES
 * ============================================================================ */

struct file_clone_range {
	__s64 src_fd;
	__u64 src_offset;
	__u64 src_length;
	__u64 dest_offset;
};

struct fstrim_range {
	__u64 start;
	__u64 len;
	__u64 minlen;
};

struct fsuuid2 {
	__u8	len;
	__u8	uuid[16];
};

struct fs_sysfs_path {
	__u8			len;
	__u8			name[128];
};

/* Protection info capability flags */
#define	LBMD_PI_CAP_INTEGRITY		(1 << 0)
#define	LBMD_PI_CAP_REFTAG		(1 << 1)

/* Checksum types for Protection Information */
#define LBMD_PI_CSUM_NONE		0
#define LBMD_PI_CSUM_IP			1
#define LBMD_PI_CSUM_CRC16_T10DIF	2
#define LBMD_PI_CSUM_CRC64_NVME		4

#define LBMD_SIZE_VER0			16

struct logical_block_metadata_cap {
	__u32	lbmd_flags;
	__u16	lbmd_interval;
	__u8	lbmd_size;
	__u8	lbmd_opaque_size;
	__u8	lbmd_opaque_offset;
	__u8	lbmd_pi_size;
	__u8	lbmd_pi_offset;
	__u8	lbmd_guard_tag_type;
	__u8	lbmd_app_tag_size;
	__u8	lbmd_ref_tag_size;
	__u8	lbmd_storage_tag_size;
	__u8	pad;
};

/* ============================================================================
 * DEDUPLICAÇÃO (ESTILO WINDOWS)
 * ============================================================================ */

#define FILE_DEDUPE_RANGE_SAME		0
#define FILE_DEDUPE_RANGE_DIFFERS	1

struct file_dedupe_range_info {
	__s64 dest_fd;
	__u64 dest_offset;
	__u64 bytes_deduped;
	__s32 status;
	__u32 reserved;
};

struct file_dedupe_range {
	__u64 src_offset;
	__u64 src_length;
	__u16 dest_count;
	__u16 reserved1;
	__u32 reserved2;
	struct file_dedupe_range_info info[];
};

/* ============================================================================
 * ESTATÍSTICAS DO SISTEMA
 * ============================================================================ */

struct files_stat_struct {
	unsigned long nr_files;
	unsigned long nr_free_files;
	unsigned long max_files;
};

struct inodes_stat_t {
	long nr_inodes;
	long nr_unused;
	long dummy[5];
};

#define NR_FILE  8192

/* ============================================================================
 * ATRIBUTOS EXTENDIDOS (ESTILO WINDOWS)
 * ============================================================================ */

struct fsxattr {
	__u32		fsx_xflags;
	__u32		fsx_extsize;
	__u32		fsx_nextents;
	__u32		fsx_projid;
	__u32		fsx_cowextsize;
	unsigned char	fsx_pad[8];
};

struct file_attr {
	__u64 fa_xflags;
	__u32 fa_extsize;
	__u32 fa_nextents;
	__u32 fa_projid;
	__u32 fa_cowextsize;
};

#define FILE_ATTR_SIZE_VER0 24
#define FILE_ATTR_SIZE_LATEST FILE_ATTR_SIZE_VER0

/*
 * Flags for the fsx_xflags field
 */
#define FS_XFLAG_REALTIME	0x00000001
#define FS_XFLAG_PREALLOC	0x00000002
#define FS_XFLAG_IMMUTABLE	0x00000008
#define FS_XFLAG_APPEND		0x00000010
#define FS_XFLAG_SYNC		0x00000020
#define FS_XFLAG_NOATIME	0x00000040
#define FS_XFLAG_NODUMP		0x00000080
#define FS_XFLAG_RTINHERIT	0x00000100
#define FS_XFLAG_PROJINHERIT	0x00000200
#define FS_XFLAG_NOSYMLINKS	0x00000400
#define FS_XFLAG_EXTSIZE	0x00000800
#define FS_XFLAG_EXTSZINHERIT	0x00001000
#define FS_XFLAG_NODEFRAG	0x00002000
#define FS_XFLAG_FILESTREAM	0x00004000
#define FS_XFLAG_DAX		0x00008000
#define FS_XFLAG_COWEXTSIZE	0x00010000
#define FS_XFLAG_VERITY		0x00020000
#define FS_XFLAG_HASATTR	0x80000000

/* ============================================================================
 * IOCTLS DE BLOCK DEVICE (ESTILO WINDOWS)
 * ============================================================================ */

#define BLKROSET   _IO(0x12,93)
#define BLKROGET   _IO(0x12,94)
#define BLKRRPART  _IO(0x12,95)
#define BLKGETSIZE _IO(0x12,96)
#define BLKFLSBUF  _IO(0x12,97)
#define BLKRASET   _IO(0x12,98)
#define BLKRAGET   _IO(0x12,99)
#define BLKFRASET  _IO(0x12,100)
#define BLKFRAGET  _IO(0x12,101)
#define BLKSECTSET _IO(0x12,102)
#define BLKSECTGET _IO(0x12,103)
#define BLKSSZGET  _IO(0x12,104)
#define BLKBSZGET  _IOR(0x12,112,size_t)
#define BLKBSZSET  _IOW(0x12,113,size_t)
#define BLKGETSIZE64 _IOR(0x12,114,size_t)
#define BLKTRACESETUP _IOWR(0x12,115,struct blk_user_trace_setup)
#define BLKTRACESTART _IO(0x12,116)
#define BLKTRACESTOP _IO(0x12,117)
#define BLKTRACETEARDOWN _IO(0x12,118)
#define BLKDISCARD _IO(0x12,119)
#define BLKIOMIN _IO(0x12,120)
#define BLKIOOPT _IO(0x12,121)
#define BLKALIGNOFF _IO(0x12,122)
#define BLKPBSZGET _IO(0x12,123)
#define BLKDISCARDZEROES _IO(0x12,124)
#define BLKSECDISCARD _IO(0x12,125)
#define BLKROTATIONAL _IO(0x12,126)
#define BLKZEROOUT _IO(0x12,127)
#define BLKGETDISKSEQ _IOR(0x12,128,__u64)
#define BLKTRACESETUP2 _IOWR(0x12, 142, struct blk_user_trace_setup2)

#define BMAP_IOCTL 1
#define FIBMAP	   _IO(0x00,1)
#define FIGETBSZ   _IO(0x00,2)
#define FIFREEZE	_IOWR('X', 119, int)
#define FITHAW		_IOWR('X', 120, int)
#define FITRIM		_IOWR('X', 121, struct fstrim_range)
#define FICLONE		_IOW(0x94, 9, int)
#define FICLONERANGE	_IOW(0x94, 13, struct file_clone_range)
#define FIDEDUPERANGE	_IOWR(0x94, 54, struct file_dedupe_range)

#define FSLABEL_MAX 256

/* ============================================================================
 * FS IOCTLS (ESTILO WINDOWS)
 * ============================================================================ */

#define	FS_IOC_GETFLAGS			_IOR('f', 1, long)
#define	FS_IOC_SETFLAGS			_IOW('f', 2, long)
#define	FS_IOC_GETVERSION		_IOR('v', 1, long)
#define	FS_IOC_SETVERSION		_IOW('v', 2, long)
#define FS_IOC_FIEMAP			_IOWR('f', 11, struct fiemap)
#define FS_IOC32_GETFLAGS		_IOR('f', 1, int)
#define FS_IOC32_SETFLAGS		_IOW('f', 2, int)
#define FS_IOC32_GETVERSION		_IOR('v', 1, int)
#define FS_IOC32_SETVERSION		_IOW('v', 2, int)
#define FS_IOC_FSGETXATTR		_IOR('X', 31, struct fsxattr)
#define FS_IOC_FSSETXATTR		_IOW('X', 32, struct fsxattr)
#define FS_IOC_GETFSLABEL		_IOR(0x94, 49, char[FSLABEL_MAX])
#define FS_IOC_SETFSLABEL		_IOW(0x94, 50, char[FSLABEL_MAX])
#define FS_IOC_GETFSUUID		_IOR(0x15, 0, struct fsuuid2)
#define FS_IOC_GETFSSYSFSPATH		_IOR(0x15, 1, struct fs_sysfs_path)
#define FS_IOC_GETLBMD_CAP		_IOWR(0x15, 2, struct logical_block_metadata_cap)
#define FS_IOC_GET_DRIVE_INFO		_IOR('F', 100, struct drive_letter_info)  /* Obter info da unidade */
#define FS_IOC_GET_OBJECT_BY_PATH	_IOWR('F', 101, struct windows_path)      /* Obter objeto por caminho */

/* ============================================================================
 * INODE FLAGS (ESTILO WINDOWS)
 * ============================================================================ */

#define	FS_SECRM_FL			0x00000001
#define	FS_UNRM_FL			0x00000002
#define	FS_COMPR_FL			0x00000004
#define FS_SYNC_FL			0x00000008
#define FS_IMMUTABLE_FL			0x00000010
#define FS_APPEND_FL			0x00000020
#define FS_NODUMP_FL			0x00000040
#define FS_NOATIME_FL			0x00000080
#define FS_DIRTY_FL			0x00000100
#define FS_COMPRBLK_FL			0x00000200
#define FS_NOCOMP_FL			0x00000400
#define FS_ENCRYPT_FL			0x00000800
#define FS_BTREE_FL			0x00001000
#define FS_INDEX_FL			0x00001000
#define FS_IMAGIC_FL			0x00002000
#define FS_JOURNAL_DATA_FL		0x00004000
#define FS_NOTAIL_FL			0x00008000
#define FS_DIRSYNC_FL			0x00010000
#define FS_TOPDIR_FL			0x00020000
#define FS_HUGE_FILE_FL			0x00040000
#define FS_EXTENT_FL			0x00080000
#define FS_VERITY_FL			0x00100000
#define FS_EA_INODE_FL			0x00200000
#define FS_EOFBLOCKS_FL			0x00400000
#define FS_NOCOW_FL			0x00800000
#define FS_DAX_FL			0x02000000
#define FS_INLINE_DATA_FL		0x10000000
#define FS_PROJINHERIT_FL		0x20000000
#define FS_CASEFOLD_FL			0x40000000
#define FS_RESERVED_FL			0x80000000

#define FS_FL_USER_VISIBLE		0x0003DFFF
#define FS_FL_USER_MODIFIABLE		0x000380FF

/* ============================================================================
 * SYNC FILE RANGE
 * ============================================================================ */

#define SYNC_FILE_RANGE_WAIT_BEFORE	1
#define SYNC_FILE_RANGE_WRITE		2
#define SYNC_FILE_RANGE_WAIT_AFTER	4
#define SYNC_FILE_RANGE_WRITE_AND_WAIT	(SYNC_FILE_RANGE_WRITE | \
					 SYNC_FILE_RANGE_WAIT_BEFORE | \
					 SYNC_FILE_RANGE_WAIT_AFTER)

/* ============================================================================
 * RWF FLAGS
 * ============================================================================ */

typedef int __bitwise __kernel_rwf_t;

#define RWF_HIPRI	((__force __kernel_rwf_t)0x00000001)
#define RWF_DSYNC	((__force __kernel_rwf_t)0x00000002)
#define RWF_SYNC	((__force __kernel_rwf_t)0x00000004)
#define RWF_NOWAIT	((__force __kernel_rwf_t)0x00000008)
#define RWF_APPEND	((__force __kernel_rwf_t)0x00000010)
#define RWF_NOAPPEND	((__force __kernel_rwf_t)0x00000020)
#define RWF_ATOMIC	((__force __kernel_rwf_t)0x00000040)
#define RWF_DONTCACHE	((__force __kernel_rwf_t)0x00000080)
#define RWF_NOSIGNAL	((__force __kernel_rwf_t)0x00000100)

#define RWF_SUPPORTED	(RWF_HIPRI | RWF_DSYNC | RWF_SYNC | RWF_NOWAIT |\
			 RWF_APPEND | RWF_NOAPPEND | RWF_ATOMIC |\
			 RWF_DONTCACHE | RWF_NOSIGNAL)

/* ============================================================================
 * PROCFS IOCTLS
 * ============================================================================ */

#define PROCFS_IOCTL_MAGIC 'f'

#define PAGEMAP_SCAN	_IOWR(PROCFS_IOCTL_MAGIC, 16, struct pm_scan_arg)

#define PAGE_IS_WPALLOWED	(1 << 0)
#define PAGE_IS_WRITTEN		(1 << 1)
#define PAGE_IS_FILE		(1 << 2)
#define PAGE_IS_PRESENT		(1 << 3)
#define PAGE_IS_SWAPPED		(1 << 4)
#define PAGE_IS_PFNZERO		(1 << 5)
#define PAGE_IS_HUGE		(1 << 6)
#define PAGE_IS_SOFT_DIRTY	(1 << 7)
#define PAGE_IS_GUARD		(1 << 8)

struct page_region {
	__u64 start;
	__u64 end;
	__u64 categories;
};

#define PM_SCAN_WP_MATCHING	(1 << 0)
#define PM_SCAN_CHECK_WPASYNC	(1 << 1)

struct pm_scan_arg {
	__u64 size;
	__u64 flags;
	__u64 start;
	__u64 end;
	__u64 walk_end;
	__u64 vec;
	__u64 vec_len;
	__u64 max_pages;
	__u64 category_inverted;
	__u64 category_mask;
	__u64 category_anyof_mask;
	__u64 return_mask;
};

/* ============================================================================
 * PROCMAP QUERY
 * ============================================================================ */

#define PROCMAP_QUERY	_IOWR(PROCFS_IOCTL_MAGIC, 17, struct procmap_query)

enum procmap_query_flags {
	PROCMAP_QUERY_VMA_READABLE		= 0x01,
	PROCMAP_QUERY_VMA_WRITABLE		= 0x02,
	PROCMAP_QUERY_VMA_EXECUTABLE		= 0x04,
	PROCMAP_QUERY_VMA_SHARED		= 0x08,
	PROCMAP_QUERY_COVERING_OR_NEXT_VMA	= 0x10,
	PROCMAP_QUERY_FILE_BACKED_VMA		= 0x20,
};

struct procmap_query {
	__u64 size;
	__u64 query_flags;
	__u64 query_addr;
	__u64 vma_start;
	__u64 vma_end;
	__u64 vma_flags;
	__u64 vma_page_size;
	__u64 vma_offset;
	__u64 inode;
	__u32 dev_major;
	__u32 dev_minor;
	__u32 vma_name_size;
	__u32 build_id_size;
	__u64 vma_name_addr;
	__u64 build_id_addr;
};

/* ============================================================================
 * SHUTDOWN
 * ============================================================================ */

#define FS_IOC_SHUTDOWN _IOR('X', 125, __u32)

#define FS_SHUTDOWN_FLAGS_DEFAULT	0x0
#define FS_SHUTDOWN_FLAGS_LOGFLUSH	0x1
#define FS_SHUTDOWN_FLAGS_NOLOGFLUSH	0x2
