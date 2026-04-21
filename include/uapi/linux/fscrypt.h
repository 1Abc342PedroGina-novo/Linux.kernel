#ifndef _UAPI_LINUX_FSCRYPT_H
#define _UAPI_LINUX_FSCRYPT_H

#include <linux/ioctl.h>
#include <linux/types.h>

/*
 * ============================================================================
 * NOVA FILOSOFIA: CRIPTOGRAFIA DE OBJETOS
 * ============================================================================
 * 
 * Em vez de políticas de criptografia aplicadas a diretórios,
 * agora temos POLÍTICAS DE CRIPTOGRAFIA aplicadas a OBJETOS.
 * 
 * Qualquer objeto pode ser criptografado:
 *   - Objetos arquivo (C:\document.txt)
 *   - Objetos executável (C:\program.exe)
 *   - Objetos porta (porta de comunicação)
 *   - Objetos mensagem (mensagens individuais)
 *   - Objetos volume (discos inteiros C:\, D:\)
 * 
 * ============================================================================
 */

/* ============================================================================
 * FLAGS DE POLÍTICA DE CRIPTOGRAFIA
 * ============================================================================ */

#define FSCRYPT_POLICY_FLAGS_PAD_4		0x00
#define FSCRYPT_POLICY_FLAGS_PAD_8		0x01
#define FSCRYPT_POLICY_FLAGS_PAD_16		0x02
#define FSCRYPT_POLICY_FLAGS_PAD_32		0x03
#define FSCRYPT_POLICY_FLAGS_PAD_MASK		0x03
#define FSCRYPT_POLICY_FLAG_DIRECT_KEY		0x04
#define FSCRYPT_POLICY_FLAG_IV_INO_LBLK_64	0x08
#define FSCRYPT_POLICY_FLAG_IV_INO_LBLK_32	0x10

/* Novos flags para criptografia de objetos */
#define FSCRYPT_POLICY_FLAG_OBJECT_ENCRYPT	0x20  /* Criptografar objeto inteiro */
#define FSCRYPT_POLICY_FLAG_MESSAGE_ENCRYPT	0x40  /* Criptografar mensagens */
#define FSCRYPT_POLICY_FLAG_PORT_ENCRYPT	0x80  /* Criptografar porta */
#define FSCRYPT_POLICY_FLAG_DRIVE_ENCRYPT	0x100 /* Criptografar unidade (C:\) */

/* ============================================================================
 * ALGORITMOS DE CRIPTOGRAFIA
 * ============================================================================ */

#define FSCRYPT_MODE_AES_256_XTS		1
#define FSCRYPT_MODE_AES_256_CTS		4
#define FSCRYPT_MODE_AES_128_CBC		5
#define FSCRYPT_MODE_AES_128_CTS		6
#define FSCRYPT_MODE_SM4_XTS			7
#define FSCRYPT_MODE_SM4_CTS			8
#define FSCRYPT_MODE_ADIANTUM			9
#define FSCRYPT_MODE_AES_256_HCTR2		10

/* Novos algoritmos para criptografia de mensagens */
#define FSCRYPT_MODE_CHACHA20_POLY1305		11  /* Para mensagens */
#define FSCRYPT_MODE_AES_256_GCM		12  /* Para mensagens/portas */
#define FSCRYPT_MODE_AES_256_CCM		13  /* Para objetos pequenos */

/* ============================================================================
 * POLÍTICA DE CRIPTOGRAFIA V1 (LEGADO)
 * ============================================================================ */

#define FSCRYPT_POLICY_V1		0
#define FSCRYPT_KEY_DESCRIPTOR_SIZE	8
struct fscrypt_policy_v1 {
	__u8 version;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 master_key_descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE];
};

/* ============================================================================
 * CHAVE DE CRIPTOGRAFIA (LEGADO)
 * ============================================================================ */

#define FSCRYPT_KEY_DESC_PREFIX		"fscrypt:"
#define FSCRYPT_KEY_DESC_PREFIX_SIZE	8
#define FSCRYPT_MAX_KEY_SIZE		64
struct fscrypt_key {
	__u32 mode;
	__u8 raw[FSCRYPT_MAX_KEY_SIZE];
	__u32 size;
};

/* ============================================================================
 * POLÍTICA DE CRIPTOGRAFIA V2 (RECOMENDADA)
 * ============================================================================ */

#define FSCRYPT_POLICY_V2		2
#define FSCRYPT_KEY_IDENTIFIER_SIZE	16
struct fscrypt_policy_v2 {
	__u8 version;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 log2_data_unit_size;
	__u8 __reserved[3];
	__u8 master_key_identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
};

/* ============================================================================
 * POLÍTICA DE CRIPTOGRAFIA V3 - PARA OBJETOS E MENSAGENS
 * ============================================================================ */

#define FSCRYPT_POLICY_V3		3
#define FSCRYPT_OBJECT_ID_SIZE		16

struct fscrypt_policy_v3 {
	__u8 version;
	__u8 encryption_mode;		/* Modo de criptografia */
	__u8 flags;			/* FSCRYPT_POLICY_FLAG_* */
	__u8 object_type;		/* Tipo de objeto a criptografar */
	__u16 reserved;
	__u8 master_key_identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
	__u8 object_id[FSCRYPT_OBJECT_ID_SIZE];	/* ID do objeto (opcional) */
};

/*
 * Tipos de objeto para criptografia
 */
#define FSCRYPT_OBJ_TYPE_FILE		1	/* Arquivo */
#define FSCRYPT_OBJ_TYPE_DIRECTORY	2	/* Diretório */
#define FSCRYPT_OBJ_TYPE_EXECUTABLE	3	/* Executável */
#define FSCRYPT_OBJ_TYPE_PORT		4	/* Porta de comunicação */
#define FSCRYPT_OBJ_TYPE_MESSAGE		5	/* Mensagem individual */
#define FSCRYPT_OBJ_TYPE_DRIVE		6	/* Unidade de disco (C:\) */
#define FSCRYPT_OBJ_TYPE_SERVICE		7	/* Serviço RPC */

/* ============================================================================
 * ESTRUTURA PARA OBTER POLÍTICA DE CRIPTOGRAFIA
 * ============================================================================ */

struct fscrypt_get_policy_ex_arg {
	__u64 policy_size; /* input/output */
	union {
		__u8 version;
		struct fscrypt_policy_v1 v1;
		struct fscrypt_policy_v2 v2;
		struct fscrypt_policy_v3 v3;
	} policy; /* output */
};

/* ============================================================================
 * ESPECIFICADOR DE CHAVE
 * ============================================================================ */

#define FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR	1
#define FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER	2
#define FSCRYPT_KEY_SPEC_TYPE_OBJECT		3	/* Chave vinculada a objeto */
#define FSCRYPT_KEY_SPEC_TYPE_PORT		4	/* Chave vinculada a porta */
#define FSCRYPT_KEY_SPEC_TYPE_DRIVE		5	/* Chave vinculada a unidade */

struct fscrypt_key_specifier {
	__u32 type;	/* one of FSCRYPT_KEY_SPEC_TYPE_* */
	__u32 __reserved;
	union {
		__u8 __reserved[32];
		__u8 descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE];
		__u8 identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
		struct {
			__u8 drive_letter;	/* 'C', 'D', etc. */
			__u8 reserved[15];
		} drive;
		struct {
			__u64 port_id;
			__u8 reserved[8];
		} port;
	} u;
};

/* ============================================================================
 * PAYLOAD DE CHAVE DE PROVISIONAMENTO
 * ============================================================================ */

struct fscrypt_provisioning_key_payload {
	__u32 type;
	__u32 flags;
	__u8 raw[];
};

/* ============================================================================
 * ESTRUTURA PARA ADICIONAR CHAVE DE CRIPTOGRAFIA
 * ============================================================================ */

struct fscrypt_add_key_arg {
	struct fscrypt_key_specifier key_spec;
	__u32 raw_size;
	__u32 key_id;
#define FSCRYPT_ADD_KEY_FLAG_HW_WRAPPED	0x00000001
#define FSCRYPT_ADD_KEY_FLAG_OBJECT_KEY	0x00000002	/* Chave para objeto específico */
#define FSCRYPT_ADD_KEY_FLAG_PORT_KEY	0x00000004	/* Chave para porta */
#define FSCRYPT_ADD_KEY_FLAG_DRIVE_KEY	0x00000008	/* Chave para unidade (C:\) */
	__u32 flags;
	__u32 __reserved[7];
	__u8 raw[];
};

/* ============================================================================
 * ESTRUTURA PARA REMOVER CHAVE
 * ============================================================================ */

struct fscrypt_remove_key_arg {
	struct fscrypt_key_specifier key_spec;
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY	0x00000001
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS	0x00000002
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OBJECTS_BUSY	0x00000004	/* Objetos ocupados */
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_PORT_ACTIVE	0x00000008	/* Porta ativa */
	__u32 removal_status_flags;	/* output */
	__u32 __reserved[5];
};

/* ============================================================================
 * ESTRUTURA PARA OBTER STATUS DA CHAVE
 * ============================================================================ */

struct fscrypt_get_key_status_arg {
	/* input */
	struct fscrypt_key_specifier key_spec;
	__u32 __reserved[6];

	/* output */
#define FSCRYPT_KEY_STATUS_ABSENT		1
#define FSCRYPT_KEY_STATUS_PRESENT		2
#define FSCRYPT_KEY_STATUS_INCOMPLETELY_REMOVED	3
#define FSCRYPT_KEY_STATUS_OBJECT_BOUND		4	/* Vinculado a objeto */
#define FSCRYPT_KEY_STATUS_PORT_BOUND		5	/* Vinculado a porta */
	__u32 status;
#define FSCRYPT_KEY_STATUS_FLAG_ADDED_BY_SELF   0x00000001
#define FSCRYPT_KEY_STATUS_FLAG_PROTECTS_OBJECT 0x00000002	/* Protege objeto */
#define FSCRYPT_KEY_STATUS_FLAG_PROTECTS_PORT   0x00000004	/* Protege porta */
	__u32 status_flags;
	__u32 user_count;
	__u32 object_count;	/* Número de objetos protegidos */
	__u32 __out_reserved[12];
};

/* ============================================================================
 * NOVAS MENSAGENS PARA CRIPTOGRAFIA DE OBJETOS
 * ============================================================================ */

/*
 * Em vez de ioctls, use MENSAGENS para operações de criptografia:
 * 
 *   object_send(obj_id, METHOD_ENCRYPT, encrypt_msg);
 *   object_send(obj_id, METHOD_DECRYPT, decrypt_msg);
 *   port_send(port_id, PORT_MSG_ENCRYPT, encrypt_msg);
 */

/**
 * struct object_encrypt_msg - Mensagem para criptografar objeto
 */
struct object_encrypt_msg {
	__u64		object_id;	/* ID do objeto a criptografar */
	__u32		algorithm;	/* FSCRYPT_MODE_* */
	__u32		flags;
	__u64		key_id;		/* ID da chave a usar */
	__u64		offset;		/* Offset inicial (para objetos parciais) */
	__u64		length;		/* Comprimento a criptografar */
	__u8		nonce[16];	/* Nonce (opcional) */
	__u8		__reserved[48];
};

/**
 * struct object_decrypt_msg - Mensagem para descriptografar objeto
 */
struct object_decrypt_msg {
	__u64		object_id;
	__u32		algorithm;
	__u32		flags;
	__u64		key_id;
	__u64		offset;
	__u64		length;
	__u8		nonce[16];
	__u8		__reserved[48];
};

/**
 * struct message_encrypt_msg - Mensagem para criptografar outra mensagem
 * 
 * "Tudo é mensagem" - Até mensagens podem ser criptografadas!
 */
struct message_encrypt_msg {
	__u64		msg_id;		/* ID da mensagem a criptografar */
	__u32		algorithm;
	__u32		flags;
	__u64		key_id;
	__u8		nonce[16];
	__u8		__reserved[48];
};

/**
 * struct drive_encrypt_msg - Mensagem para criptografar unidade inteira
 * 
 * Criptografar C:\ inteiro!
 */
struct drive_encrypt_msg {
	__u8		drive_letter;	/* 'C', 'D', etc. */
	__u8		reserved[7];
	__u32		algorithm;
	__u32		flags;
	__u64		key_id;
	__u64		start_sector;	/* Setor inicial (0 = início) */
	__u64		sector_count;	/* Número de setores (0 = todo disco) */
	__u8		__reserved[48];
};

/* ============================================================================
 * IOCTLS LEGADOS (DEPRECADOS - USE MENSAGENS)
 * ============================================================================ */

/*
 * ESTES IOCTLS ESTÃO DEPRECADOS!
 * 
 * Use as mensagens acima em vez de ioctls.
 */

#define FS_IOC_SET_ENCRYPTION_POLICY		_IOR('f', 19, struct fscrypt_policy_v1)
#define FS_IOC_GET_ENCRYPTION_PWSALT		_IOW('f', 20, __u8[16])
#define FS_IOC_GET_ENCRYPTION_POLICY		_IOW('f', 21, struct fscrypt_policy_v1)
#define FS_IOC_GET_ENCRYPTION_POLICY_EX		_IOWR('f', 22, __u8[9])
#define FS_IOC_ADD_ENCRYPTION_KEY		_IOWR('f', 23, struct fscrypt_add_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY		_IOWR('f', 24, struct fscrypt_remove_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY_ALL_USERS	_IOWR('f', 25, struct fscrypt_remove_key_arg)
#define FS_IOC_GET_ENCRYPTION_KEY_STATUS	_IOWR('f', 26, struct fscrypt_get_key_status_arg)
#define FS_IOC_GET_ENCRYPTION_NONCE		_IOR('f', 27, __u8[16])

/* ============================================================================
 * NOVAS MENSAGENS PARA CRIPTOGRAFIA (SUBSTITUTOS DOS IOCTLS)
 * ============================================================================ */

/*
 * Comandos base para mensagens de criptografia
 */
#define FSCRYPT_MSG_BASE		0x46  /* 'F' */

#define FSCRYPT_MSG_SET_POLICY		_IOWR(FSCRYPT_MSG_BASE, 1, struct fscrypt_policy_v3)
#define FSCRYPT_MSG_GET_POLICY		_IOWR(FSCRYPT_MSG_BASE, 2, struct fscrypt_get_policy_ex_arg)
#define FSCRYPT_MSG_ADD_KEY		_IOWR(FSCRYPT_MSG_BASE, 3, struct fscrypt_add_key_arg)
#define FSCRYPT_MSG_REMOVE_KEY		_IOWR(FSCRYPT_MSG_BASE, 4, struct fscrypt_remove_key_arg)
#define FSCRYPT_MSG_GET_KEY_STATUS	_IOWR(FSCRYPT_MSG_BASE, 5, struct fscrypt_get_key_status_arg)
#define FSCRYPT_MSG_ENCRYPT_OBJECT	_IOWR(FSCRYPT_MSG_BASE, 6, struct object_encrypt_msg)
#define FSCRYPT_MSG_DECRYPT_OBJECT	_IOWR(FSCRYPT_MSG_BASE, 7, struct object_decrypt_msg)
#define FSCRYPT_MSG_ENCRYPT_MESSAGE	_IOWR(FSCRYPT_MSG_BASE, 8, struct message_encrypt_msg)
#define FSCRYPT_MSG_ENCRYPT_DRIVE	_IOWR(FSCRYPT_MSG_BASE, 9, struct drive_encrypt_msg)

/* ============================================================================
 * COMPATIBILIDADE COM NOMES ANTIGOS
 * ============================================================================ */

/* old names; don't add anything new here! */
#ifndef __KERNEL__
#define fscrypt_policy			fscrypt_policy_v1
#define FS_KEY_DESCRIPTOR_SIZE		FSCRYPT_KEY_DESCRIPTOR_SIZE
#define FS_POLICY_FLAGS_PAD_4		FSCRYPT_POLICY_FLAGS_PAD_4
#define FS_POLICY_FLAGS_PAD_8		FSCRYPT_POLICY_FLAGS_PAD_8
#define FS_POLICY_FLAGS_PAD_16		FSCRYPT_POLICY_FLAGS_PAD_16
#define FS_POLICY_FLAGS_PAD_32		FSCRYPT_POLICY_FLAGS_PAD_32
#define FS_POLICY_FLAGS_PAD_MASK	FSCRYPT_POLICY_FLAGS_PAD_MASK
#define FS_POLICY_FLAG_DIRECT_KEY	FSCRYPT_POLICY_FLAG_DIRECT_KEY
#define FS_POLICY_FLAGS_VALID		0x07
#define FS_ENCRYPTION_MODE_INVALID	0
#define FS_ENCRYPTION_MODE_AES_256_XTS	FSCRYPT_MODE_AES_256_XTS
#define FS_ENCRYPTION_MODE_AES_256_GCM	2
#define FS_ENCRYPTION_MODE_AES_256_CBC	3
#define FS_ENCRYPTION_MODE_AES_256_CTS	FSCRYPT_MODE_AES_256_CTS
#define FS_ENCRYPTION_MODE_AES_128_CBC	FSCRYPT_MODE_AES_128_CBC
#define FS_ENCRYPTION_MODE_AES_128_CTS	FSCRYPT_MODE_AES_128_CTS
#define FS_ENCRYPTION_MODE_ADIANTUM	FSCRYPT_MODE_ADIANTUM
#define FS_KEY_DESC_PREFIX		FSCRYPT_KEY_DESC_PREFIX
#define FS_KEY_DESC_PREFIX_SIZE		FSCRYPT_KEY_DESC_PREFIX_SIZE
#define FS_MAX_KEY_SIZE			FSCRYPT_MAX_KEY_SIZE
#endif /* !__KERNEL__ */
