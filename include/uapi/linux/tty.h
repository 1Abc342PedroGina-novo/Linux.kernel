/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_TTY_H
#define _UAPI_LINUX_TTY_H

/*
 * ============================================================================
 * TTY DRIVER - RING 0 (KERNEL SPACE)
 * ============================================================================
 * 
 * AVISO: O DRIVER TTY OPERA EM RING 0 (KERNEL MODE)
 * 
 * Componentes que operam em Ring 0 (Kernel Space):
 *   ================================================
 *   • HFS+  (Hierarchical File System Plus - Apple)
 *   • NTFS  (New Technology File System - Microsoft)
 *   • NFS   (Network File System - Sun Microsystems)
 *   • TTY   (Teletypewriter - Serial/Console driver)
 *   ================================================
 * 
 * Estes componentes rodam em Ring 0 por razões de:
 *   - Performance crítica
 *   - Acesso direto a hardware
 *   - Latência mínima
 *   - Segurança de baixo nível
 * 
 * No modelo "tudo é objeto", TTYs são OBJETOS ESPECIAIS que:
 *   - Operam em Ring 0 (kernel mode)
 *   - São acessíveis via portas de mensagem
 *   - Podem ser redirecionados para portas de usuário (Ring 3)
 *   - Mantêm compatibilidade com legado
 * 
 * FILOSOFIA:
 *   "TTYs são objetos de comunicação serial em Ring 0"
 * 
 * ============================================================================
 */

/*
 * TTY objects are special kernel objects that provide:
 *   - Serial communication (RS-232, USB serial, etc.)
 *   - Console access (virtual terminals)
 *   - PTY (pseudo-terminals) for terminal emulation
 *   - Legacy compatibility for serial devices
 * 
 * These operate in RING 0 (Kernel Mode) for performance reasons.
 */

/* ============================================================================
 * LINE DISCIPLINES (TTY LINE DISCIPLINES - Ring 0)
 * ============================================================================
 */

/* line disciplines - operam em Ring 0 */
#define N_TTY		0	/* TTY line discipline (padrão - Ring 0) */
#define N_SLIP		1	/* Serial Line IP (Ring 0) */
#define N_MOUSE		2	/* Mouse serial (Ring 0) */
#define N_PPP		3	/* Point-to-Point Protocol (Ring 0) */
#define N_STRIP		4	/* STanford Research IP (Ring 0) */
#define N_AX25		5	/* AX.25 amateur radio (Ring 0) */
#define N_X25		6	/* X.25 async (Ring 0) */
#define N_6PACK		7	/* 6-Pack (Ring 0) */
#define N_MASC		8	/* Reserved for Mobitex (Ring 0) */
#define N_R3964		9	/* Reserved for Simatic R3964 (Ring 0) */
#define N_PROFIBUS_FDL	10	/* Reserved for Profibus (Ring 0) */
#define N_IRDA		11	/* Linux IrDa (Ring 0) */
#define N_SMSBLOCK	12	/* SMS block mode (Ring 0) */
#define N_HDLC		13	/* synchronous HDLC (Ring 0) */
#define N_SYNC_PPP	14	/* synchronous PPP (Ring 0) */
#define N_HCI		15	/* Bluetooth HCI UART (Ring 0) */
#define N_GIGASET_M101	16	/* Siemens Gigaset (Ring 0) */
#define N_SLCAN		17	/* Serial CAN Adaptors (Ring 0) */
#define N_PPS		18	/* Pulse per Second (Ring 0) */
#define N_V253		19	/* Codec control over voice modem (Ring 0) */
#define N_CAIF		20	/* CAIF protocol (Ring 0) */
#define N_GSM0710	21	/* GSM 0710 Mux (Ring 0) */
#define N_TI_WL		22	/* TI WL combo chips (Ring 0) */
#define N_TRACESINK	23	/* Trace data routing (Ring 0) */
#define N_TRACEROUTER	24	/* Trace data routing (Ring 0) */
#define N_NCI		25	/* NFC NCI UART (Ring 0) */
#define N_SPEAKUP	26	/* Speakup communication (Ring 0) */
#define N_NULL		27	/* Null ldisc for error handling (Ring 0) */
#define N_MCTP		28	/* MCTP-over-serial (Ring 0) */
#define N_DEVELOPMENT	29	/* Manual out-of-tree testing (Ring 0) */
#define N_CAN327	30	/* ELM327 based OBD-II (Ring 0) */

/* Always the newest line discipline + 1 */
#define NR_LDISCS	31

/* ============================================================================
 * TTY OBJECTS (Ring 0 Kernel Objects)
 * ============================================================================ */

/*
 * TTY Object Types - objetos especiais que operam em Ring 0
 */
enum tty_object_type {
	TTY_OBJ_TYPE_SERIAL	= 1,	/* Porta serial física (RS-232) */
	TTY_OBJ_TYPE_CONSOLE	= 2,	/* Console do sistema */
	TTY_OBJ_TYPE_PTY_MASTER	= 3,	/* PTY master (pseudo-terminal) */
	TTY_OBJ_TYPE_PTY_SLAVE	= 4,	/* PTY slave */
	TTY_OBJ_TYPE_USB_SERIAL	= 5,	/* USB Serial adapter */
	TTY_OBJ_TYPE_VIRTUAL	= 6,	/* Virtual TTY (VM, container) */
	TTY_OBJ_TYPE_BLUETOOTH	= 7,	/* Bluetooth RFCOMM */
	TTY_OBJ_TYPE_NETWORK	= 8,	/* Network TTY (NFS, etc.) */
};

/*
 * TTY Object ID (OID para objetos TTY em Ring 0)
 */
struct tty_object_id {
	__u64		tty_id;		/* ID único do TTY */
	__u32		type;		/* tty_object_type */
	__u32		ring_level;	/* Sempre 0 (Ring 0) */
	__u64		port_id;	/* Porta associada (se houver) */
};

/*
 * TTY Object Info - informações do objeto TTY
 */
struct tty_object_info {
	struct tty_object_id id;
	__u64		object_id;	/* OID no sistema de objetos */
	__u64		port_id;	/* Porta de mensagem associada */
	__u32		line_discipline;	/* N_TTY, N_PPP, etc. */
	__u32		flags;		/* TTY_FLAG_* */
	__u64		speed;		/* Baud rate (bits/s) */
	__u64		bytes_sent;	/* Total de bytes enviados */
	__u64		bytes_recv;	/* Total de bytes recebidos */
	__u32		open_count;	/* Número de referências */
	__u32		reserved;
	char		name[64];	/* Nome do dispositivo (ttyS0, pts/1) */
};

/* TTY flags */
#define TTY_FLAG_INITIALIZED	(1 << 0)	/* TTY inicializado */
#define TTY_FLAG_OPEN		(1 << 1)	/* TTY aberto */
#define TTY_FLAG_LOW_LATENCY	(1 << 2)	/* Baixa latência (Ring 0) */
#define TTY_FLAG_NO_FCNTL	(1 << 3)	/* Sem fcntl */
#define TTY_FLAG_PTY		(1 << 4)	/* É um PTY */
#define TTY_FLAG_MASTER		(1 << 5)	/* PTY master */
#define TTY_FLAG_SLAVE		(1 << 6)	/* PTY slave */
#define TTY_FLAG_CLOSING	(1 << 7)	/* Fechando */
#define TTY_FLAG_HW_FLOW	(1 << 8)	/* Hardware flow control */
#define TTY_FLAG_SW_FLOW	(1 << 9)	/* Software flow control */
#define TTY_FLAG_ECHO		(1 << 10)	/* Echo habilitado */
#define TTY_FLAG_CANON		(1 << 11)	/* Modo canônico */
#define TTY_FLAG_ISIG		(1 << 12)	/* Signal handling */
#define TTY_FLAG_ICANON		(1 << 13)	/* Canonical input */
#define TTY_FLAG_ECHOE		(1 << 14)	/* Echo erase */
#define TTY_FLAG_ECHOK		(1 << 15)	/* Echo kill */
#define TTY_FLAG_ECHONL		(1 << 16)	/* Echo NL */
#define TTY_FLAG_NOFLSH		(1 << 17)	/* No flush on signal */
#define TTY_FLAG_TOSTOP		(1 << 18)	/* Stop on background output */

/* ============================================================================
 * MENSAGENS PARA TTY OBJECTS (via portas)
 * ============================================================================ */

/*
 * Comandos/métodos para objetos TTY (enviados via portas)
 */
enum tty_method {
	TTY_METHOD_READ		= 0x2001,	/* Ler dados do TTY */
	TTY_METHOD_WRITE	= 0x2002,	/* Escrever dados no TTY */
	TTY_METHOD_SET_SPEED	= 0x2003,	/* Definir baud rate */
	TTY_METHOD_GET_SPEED	= 0x2004,	/* Obter baud rate */
	TTY_METHOD_SET_LINE	= 0x2005,	/* Definir line discipline */
	TTY_METHOD_GET_LINE	= 0x2006,	/* Obter line discipline */
	TTY_METHOD_SET_FLAGS	= 0x2007,	/* Definir flags TTY */
	TTY_METHOD_GET_FLAGS	= 0x2008,	/* Obter flags TTY */
	TTY_METHOD_FLUSH	= 0x2009,	/* Flush buffers */
	TTY_METHOD_DRAIN	= 0x200A,	/* Drenar buffers */
	TTY_METHOD_SEND_BREAK	= 0x200B,	/* Enviar break */
	TTY_METHOD_GET_INFO	= 0x200C,	/* Obter info do TTY */
	TTY_METHOD_SET_BAUD	= 0x200D,	/* Definir baud rate */
	TTY_METHOD_SET_PARITY	= 0x200E,	/* Definir paridade */
	TTY_METHOD_SET_STOPBITS	= 0x200F,	/* Definir stop bits */
	TTY_METHOD_SET_DATABITS	= 0x2010,	/* Definir data bits */
	TTY_METHOD_SET_FLOWCTRL	= 0x2011,	/* Definir flow control */
	TTY_METHOD_SET_RTS	= 0x2012,	/* Set RTS line */
	TTY_METHOD_SET_DTR	= 0x2013,	/* Set DTR line */
	TTY_METHOD_GET_RI	= 0x2014,	/* Get RI line */
	TTY_METHOD_GET_DSR	= 0x2015,	/* Get DSR line */
	TTY_METHOD_GET_CD	= 0x2016,	/* Get CD line */
	TTY_METHOD_GET_CTS	= 0x2017,	/* Get CTS line */
};

/*
 * Estrutura para mensagem de escrita no TTY
 */
struct tty_write_msg {
	__u64		tty_id;		/* ID do TTY (Ring 0) */
	__u64		offset;		/* Offset (ignorado para TTY) */
	__u64		length;		/* Comprimento dos dados */
	__u32		flags;		/* TTY_WRITE_FLAG_* */
	__u32		reserved;
	__u8		data[];		/* Dados a escrever */
};

/*
 * Estrutura para mensagem de leitura do TTY
 */
struct tty_read_msg {
	__u64		tty_id;
	__u64		length;		/* Tamanho do buffer */
	__u32		flags;
	__u32		reserved;
	__u8		data[];		/* Buffer para dados lidos */
};

/*
 * Flags para escrita/leitura TTY
 */
#define TTY_WRITE_FLAG_NOWAIT	(1 << 0)	/* Não bloquear */
#define TTY_WRITE_FLAG_ASYNC	(1 << 1)	/* Escrita assíncrona */
#define TTY_WRITE_FLAG_RAW	(1 << 2)	/* Modo raw (sem processamento) */
#define TTY_WRITE_FLAG_ECHO	(1 << 3)	/* Ecoar dados */

#define TTY_READ_FLAG_NOWAIT	(1 << 0)
#define TTY_READ_FLAG_PEEK	(1 << 1)	/* Visualizar sem consumir */
#define TTY_READ_FLAG_RAW	(1 << 2)

/*
 * Estrutura para configurar TTY
 */
struct tty_config_msg {
	__u64		tty_id;
	__u64		speed;		/* Baud rate (0 = manter) */
	__u32		line_discipline;	/* N_TTY, N_PPP, etc. */
	__u32		flags;		/* TTY_FLAG_* */
	__u8		parity;		/* 0=none, 1=odd, 2=even, 3=mark, 4=space */
	__u8		stop_bits;	/* 1 ou 2 */
	__u8		data_bits;	/* 5, 6, 7, 8 */
	__u8		flow_ctrl;	/* 0=none, 1=hardware, 2=software */
	__u32		reserved;
};

/* ============================================================================
 * PTY (Pseudo-Terminal) Objects - Ring 0
 * ============================================================================ */

/*
 * PTY Object - par master/slave em Ring 0
 */
struct pty_object {
	__u64		master_id;	/* OID do PTY master */
	__u64		slave_id;	/* OID do PTY slave */
	__u64		port_id;	/* Porta de comunicação */
	__u32		flags;		/* PTY_FLAG_* */
	__u32		index;		/* Número do PTY (0, 1, 2, ...) */
	char		name[32];	/* Nome (pts/0, pts/1, etc.) */
};

#define PTY_FLAG_ACTIVE		(1 << 0)	/* PTY ativo */
#define PTY_FLAG_PACKET_MODE	(1 << 1)	/* Modo packet */
#define PTY_FLAG_CLOSED		(1 << 2)	/* Fechado */

/*
 * Mensagem para criar PTY
 */
struct pty_create_msg {
	__u32		flags;
	__u32		reserved;
	__u64		owner_port;	/* Porta do proprietário */
};

struct pty_create_out {
	__u64		master_id;
	__u64		slave_id;
	__u64		port_id;
	__u32		index;
	__u32		reserved;
};

/* ============================================================================
 * IOCTLS LEGADOS (MANTIDOS PARA COMPATIBILIDADE - DEPRECADOS)
 * ============================================================================ */

/*
 * Estes ioctls ainda funcionam, mas são traduzidos internamente para
 * mensagens enviadas aos objetos TTY em Ring 0. Seu uso é DEPRECADO.
 * Use as mensagens TTY_METHOD_* via portas.
 */

#define TTY_IOCTL_BASE		'T'

#define TCGETS			_IOR('T', 0x01, struct termios)
#define TCSETS			_IOW('T', 0x02, struct termios)
#define TCSETSW			_IOW('T', 0x03, struct termios)
#define TCSETSF			_IOW('T', 0x04, struct termios)
#define TCGETA			_IOR('T', 0x05, struct termio)
#define TCSETA			_IOW('T', 0x06, struct termio)
#define TCSETAW			_IOW('T', 0x07, struct termio)
#define TCSETAF			_IOW('T', 0x08, struct termio)
#define TCSBRK			_IO('T', 0x09)
#define TCXONC			_IO('T', 0x0A)
#define TCFLSH			_IO('T', 0x0B)
#define TIOCEXCL		_IO('T', 0x0C)
#define TIOCNXCL		_IO('T', 0x0D)
#define TIOCSCTTY		_IO('T', 0x0E)
#define TIOCGPGRP		_IOR('T', 0x0F, pid_t)
#define TIOCSPGRP		_IOW('T', 0x10, pid_t)
#define TIOCOUTQ		_IOR('T', 0x11, int)
#define TIOCSTI			_IOW('T', 0x12, char)
#define TIOCGWINSZ		_IOR('T', 0x13, struct winsize)
#define TIOCSWINSZ		_IOW('T', 0x14, struct winsize)
#define TIOCMGET		_IOR('T', 0x15, int)
#define TIOCMBIS		_IOW('T', 0x16, int)
#define TIOCMBIC		_IOW('T', 0x17, int)
#define TIOCMSET		_IOW('T', 0x18, int)
#define TIOCGSOFTCAR		_IOR('T', 0x19, int)
#define TIOCSSOFTCAR		_IOW('T', 0x1A, int)
#define FIONREAD		_IOR('T', 0x1B, int)
#define TIOCINQ			FIONREAD
#define TIOCLINUX		_IOW('T', 0x1C, char)
#define TIOCCONS		_IO('T', 0x1D)
#define TIOCGSERIAL		_IOR('T', 0x1E, struct serial_struct)
#define TIOCSSERIAL		_IOW('T', 0x1F, struct serial_struct)
#define TIOCPKT			_IOW('T', 0x20, int)
#define TIOCNOTTY		_IO('T', 0x21)
#define TIOCSETD		_IOW('T', 0x22, int)
#define TIOCGETD		_IOR('T', 0x23, int)
#define TCSBRKP			_IOW('T', 0x24, int)
#define TIOCSBRK		_IO('T', 0x27)
#define TIOCCBRK		_IO('T', 0x28)
#define TIOCGSID		_IOR('T', 0x29, pid_t)
#define TIOCGPTPEER		_IO('T', 0x2A)
#define TIOCGDEV		_IOR('T', 0x2B, __u32)

/* ============================================================================
 * CONSTANTES ADICIONAIS
 * ============================================================================ */

/* Modos de controle de fluxo */
#define TTY_FLOW_NONE		0
#define TTY_FLOW_HARDWARE	1	/* RTS/CTS */
#define TTY_FLOW_SOFTWARE	2	/* XON/XOFF */

/* Paridade */
#define TTY_PARITY_NONE		0
#define TTY_PARITY_ODD		1
#define TTY_PARITY_EVEN		2
#define TTY_PARITY_MARK		3
#define TTY_PARITY_SPACE	4

/* Sinais TTY (para TIOCM_*) */
#define TIOCM_LE	0x001	/* line enable */
#define TIOCM_DTR	0x002	/* data terminal ready */
#define TIOCM_RTS	0x004	/* request to send */
#define TIOCM_ST	0x008	/* secondary transmit */
#define TIOCM_SR	0x010	/* secondary receive */
#define TIOCM_CTS	0x020	/* clear to send */
#define TIOCM_CAR	0x040	/* carrier detect */
#define TIOCM_CD	TIOCM_CAR
#define TIOCM_RNG	0x080	/* ring */
#define TIOCM_RI	TIOCM_RNG
#define TIOCM_DSR	0x100	/* data set ready */
#define TIOCM_OUT1	0x2000	/* output1 */
#define TIOCM_OUT2	0x4000	/* output2 */
#define TIOCM_LOOP	0x8000	/* loopback */


#endif /* _UAPI_LINUX_TTY_H */
