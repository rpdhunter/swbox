/********************************************************************
	created:	2015/11/16
	created:	16:11:2015   11:51
	filename: 	D:\cygwin64\home\ibm\prog\rtu\inc\comm_head.h
	file path:	D:\cygwin64\home\ibm\prog\rtu\inc
	file base:	comm_head
	file ext:	h
	author:		zhouxf
	purpose:	common head file
*********************************************************************/

#ifndef _COMM_HEAD_H_
#define _COMM_HEAD_H_

/* __DBUG__ 1
#ifdef __DBUG__

#define _DPRINTF(fmt, args...) \
do { \
	printf (fmt, ##args); \
} while (0)
#else
#define _DPRINTF(fmt, args...)

#endif  */

#ifndef MSB
#define MSB(X) (((X) >> 8) & 0xff)
#endif /* MSB */

#ifndef LSB
#define LSB(X) ((X) & 0xff)
#endif /* LSB */

#ifndef MSW
#define MSW(X) (((X) >> 16) & 0xffff)
#endif /* MSW */

#ifndef LSW
#define LSW(X) ((X) & 0xffff)
#endif /* LSW */

/* 104 internal port */
#define IEC104_INTERNAL_UDP_PORT	9111
/* 101 internal port */
#define IEC101_INTERNAL_UDP_PORT	9112
/* message type */
#define MSG_CHK_SEND_LST			0x10
#define MSG_CHK_TIME1S_TASK			0x11
#define MSG_CHK_EVENT				0x12
#define		MSG_SUB_YX				0x01
#define		MSG_SUB_YC				0x02

#define SDRAM_MEM_BASE				0x60000000		/* base address */
#define SDRAM_MEM_SIZE				0x800000		/* memory size in byte */
#define SDRAM_MEM_TOP				(SDRAM_MEM_BASE + SDRAM_MEM_SIZE)

/* distribution of SDRAM: reverse of the memory address */
/* rdb */
#define RDB_MEM_SIZE				0x20000			/* 128kB size */
#define RDB_MEM_BASE				(SDRAM_MEM_TOP - RDB_MEM_SIZE)

/* iec104 */
#define COMMU_104_MEM_SIZE			0x10000			/* 64kB size */
#define COMMU_104_MEM_BASE			(RDB_MEM_BASE - COMMU_104_MEM_SIZE)

/* iec101 */
#define COMMU_101_MEM_SIZE			0x20000			/* 128kB size */
#define COMMU_101_MEM_BASE			(COMMU_104_MEM_BASE - COMMU_101_MEM_SIZE)

/* protection settings and device parameters */
#define PROT_SETTING_MEM_SIZE		0x4000			/* 16kB size */
#define PROT_SETTING_MEM_BASE		(COMMU_101_MEM_BASE - PROT_SETTING_MEM_SIZE)

#define SDRAM_MALLOC(PTR, TYPE, NUM, MEM_BASE, MEM_SIZE, MEM_OFFSET) \
	do \
	{ \
		int lsize = NUM * sizeof (TYPE) + 8; \
		if ((MEM_OFFSET + lsize) > MEM_SIZE) { \
			PTR = NULL; \
		} \
		else { \
			PTR = (TYPE *)(MEM_BASE + MEM_OFFSET); \
			MEM_OFFSET += lsize; \
		} \
	} while (0)
#if 1
#define FWRITE(BUF, SIZE, N, FP) fwrite (BUF, 1, SIZE * N, FP)
#define FREAD(BUF, SIZE, N, FP) fread (BUF, 1, SIZE * N, FP)
#else
#define FWRITE(BUF, SIZE, N, FP) fwrite (BUF, SIZE, N, FP)
#define FREAD(BUF, SIZE, N, FP) fread (BUF, SIZE, N, FP)
#endif

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef void * (* THREAD_FUNC_TYPE) (void *);
/*

typedef struct {
unsigned char type;
unsigned char sub_type;
unsigned short content_len;
unsigned char content [4];
} internal_msg_t;
*/
typedef struct {
unsigned int seconds;
unsigned int msec;
} time_type;


//#define MAX_MSG_LEN				80
#define INTERNAL_MSG_HEAD_LEN	(sizeof (internal_msg_t) - 4 /* sizeof (internal_msg_t.content)*/)

extern unsigned int * rdb_mem_base;
extern unsigned int rdb_mem_size;
extern unsigned int rdb_mem_offset;

extern unsigned int * commu_104_mem_base;
extern unsigned int commu_104_mem_size;
extern unsigned int commu_104_mem_offset;

extern unsigned int * commu_101_mem_base;
extern unsigned int commu_101_mem_size;
extern unsigned int commu_101_mem_offset;

extern unsigned int * prot_setting_mem_base;
extern unsigned int prot_setting_mem_size;
extern unsigned int prot_setting_mem_offset;

int init_mem_base ();
int show_sdram_mem ();
int init_rtu ();
int send_iec104_internal_udp_data (unsigned char * send_buf, int send_len);
int send_iec101_internal_udp_data (unsigned char * send_buf, int send_len);
int show_msg (char * prompt, unsigned char buf [], int len);
int time_to_cp56time2a (time_type * tt, unsigned char cp56t []);
int cp56time2a_to_time (unsigned char cp56t [], time_type * tt);

#endif /* _COMM_HEAD_H_ */
