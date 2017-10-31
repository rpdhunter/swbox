/********************************************************************
	created:	2015/11/06
	created:	6:11:2015   16:12
	filename: 	D:\rd\src_code\rtu\code\inc\rdb.h
	file path:	D:\rd\src_code\rtu\code\inc
	file base:	rdb
	file ext:	h
	author:		zhouxf
	purpose:	rtu rdb header
*********************************************************************/
#ifndef _RDB_H_
#define _RDB_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <semaphore.h>

//#include "global_define.h"
#include "point_table.h"
#include "data_types.h"
#include "comm_head.h"

#define RDB_CHECK_TIME	100000	/* 100000us, 100ms */

#define YC_INT			1		/* 遥测整型 */
#define YC_FLOAT		2		/* 遥测浮点 */

#define YK_SP			1		/* 单点遥控 */
#define YK_DP			2		/* 双点遥控 */

#define YK_DO			1		/* 直接控制 */
#define YK_SBO			2		/* 选择控制 */

#define YK_VAL			1		/* 控点可用 */
#define YK_SEL			2		/* 控点被选择 */
#define YK_OPER			3		/* 控制中 */
#define YK_DONE			4		/* 控制完成 */

#define YK_SEL_TO		30		/* 选择超时 */
#define YK_OPER_TO		2		/* 控制超时 */

#define DP_OPEN			1		/* 双点分位 */
#define DP_CLOSE		2		/* 双点合位 */

#define SP_OPEN			0		/* 单点分位 */
#define SP_CLOSE		1		/* 单点合位 */

#define DIQ_IV			0x80	/* 品质，无效 */
#define DIQ_NT			0x40	/* 品质，非当前值 */
#define DIQ_SB			0x20	/* 品质，被取代 */
#define DIQ_BL			0x10	/* 品质，被闭锁 */

#define SIQ_IV			0x80	/* 品质，无效 */
#define SIQ_NT			0x40	/* 品质，非当前值 */
#define SIQ_SB			0x20	/* 品质，被取代 */
#define SIQ_BL			0x10	/* 品质，被闭锁 */

#define QDS_IV			0x80	/* 品质，无效 */
#define QDS_NT			0x40	/* 品质，非当前值 */
#define QDS_SB			0x20	/* 品质，被取代 */
#define QDS_BL			0x10	/* 品质，被闭锁 */
#define QDS_EI			0x08	/* 品质，动作时间无效 */

#define BCR_SEQ_NOT_SQ	0x1f	/* 顺序记法，顺序号 */
#define BCR_SEQ_NOT_CY	0x20	/* 顺序记法，进位 */
#define BCR_SEQ_NOT_CA	0x40	/* 顺序记法，计数量被调整 */
#define BCR_SEQ_NOT_IV	0x80	/* 顺序记法，无效 */

#define YX_SOE_FLAG		0x01
#define YX_COS_FLAG		0x02

#define YX_NUMBER		(YX_VALID_NUM + 4)
#define YC_NUMBER		(YC_VALID_NUM + 4)
#define YK_NUMBER		(YK_VALID_NUM + 4)
//#define YM_NUMBER		(YM_VALID_NUM + 4)

#define RDB_TYPE_YX		0x01
#define RDB_TYPE_YC		0x02
#define RDB_TYPE_YM		0x03

#define MAX_RDB_EVENT_NO	600		/*最大事件数*/

#define MAX_RDB_REGED_APP	5

#define MAX_MSG_LEN					512 		/*消息最大长度*/

#define INTERNAL_MSG_HEAD_LEN		(sizeof (internal_msg_t) - 4 /* sizeof (internal_msg_t.content)*/)

/* message type */
#define MSG_CHK_SEND_LST			0x10
#define MSG_CHK_TIME1S_TASK			0x11
#define MSG_CHK_EVENT				0x12
#define		MSG_SUB_YX				0x01
#define		MSG_SUB_YC				0x02
/*事件链表添加*/
#define ADD_EVENT_TO_TAIL(PENTRY,PEVENT) \
{ \
	if ((PENTRY)->p_event_tail != NULL) { \
		(PENTRY)->p_event_tail->next = (PEVENT); \
		(PENTRY)->p_event_tail = (PEVENT); \
	} \
	else { \
		(PENTRY)->p_event_tail = (PEVENT); \
		(PENTRY)->p_event_head = (PEVENT); \
	} \
}
/*事件链表移除*/
#define GET_EVENT_FROM_HEAD(PENTRY,PEVENT) \
{ \
	if ((PENTRY)->p_event_head != NULL) { \
		(PEVENT) = (PENTRY)->p_event_head; \
		(PENTRY)->p_event_head = (PENTRY)->p_event_head->next; \
		if ((PENTRY)->p_event_head == NULL) { \
			(PENTRY)->p_event_tail = NULL; \
		} \
	} \
	else { \
		(PEVENT) = NULL; \
	} \
}

#define __DBUG__ 1
#if __DBUG__

#define _DPRINTF(fmt, args...) \
do { \
	printf (fmt, ##args); \
} while (0)
#else
#define _DPRINTF(fmt, args...)

#endif /* __DBUG__ */
/* 遥测数据类型*/
typedef union {
	Int32		i_val;
	float32		f_val;
} yc_data_type;

/* 二进制计数器读数 */
typedef struct bcr_s {
	Uint32 counter_reading;		/* 计数器读法 */
	unsigned char seq_not;		/* 顺序记法 */
} bcr_t;

typedef struct data_hook_s {	//本质为一个Node
	int	app_id;					/* 接受hook的应用ID */
	Uint32 proto_data_no;		/* 在协议中的点号，即yx_lst的下标 */
	struct data_hook_s * next;
} data_hook_t;

typedef struct yx_event_s {
	Uint32 data_no;			//存入几号点
	Uint32 event_val;       //存入开还是关
	time_type time_val;
	unsigned char cos_soe_flag;
} yx_event_t;

typedef struct yc_event_s {
	Uint32 data_no;
	yc_data_type event_val;
	/*
typedef union {
	Int32		i_val;
	float32		f_val;
} yc_data_type;
	*/
	unsigned char data_type;
	unsigned char qds;
	time_type time_val;
} yc_event_t;

typedef struct rdb_event_s {
	/*
	本质为一个单项链表的Node，info域为yx_event_t或yc_event_t其一。
	*/
	union {
		yx_event_t yx_e;
		yc_event_t yc_e;
	};
	struct rdb_event_s * next;
} rdb_event_t;

typedef int (* hook_func_t) (unsigned char *, int);

typedef struct yx_s {
	Uint32 		  cur_val;		/* 数值，存为双点值，带品质，DP_OPEN, DP_CLOSE */
	rdb_event_t * p_event_head;	/* event链头，取事件 */
	rdb_event_t * p_event_tail;	/* event链尾，压事件 */
	data_hook_t * p_hook;		/* hook链 */
	sem_t 		  mutex;		/* 互斥信号量 */
} yx_t;

typedef struct yc_s {
	yc_data_type cur_val;		/* 数值，支持整型，浮点 */
	unsigned char qds;			/* 品质 */
	unsigned char data_type;	/* 数据类型，YC_INT or YC_FLOAT */
	rdb_event_t * p_event_head;	/* event链头，取事件 */
	rdb_event_t * p_event_tail;	/* event链尾，压事件 */
	data_hook_t * p_hook;		/* hook链 */
	sem_t 		  mutex;		/* 互斥信号量 */
} yc_t;

typedef struct ym_s {
	bcr_t cur_val;				/* 数值，BCR类型 */
	data_hook_t * p_hook;		/* hook链 */
	sem_t 		  mutex;		/* 互斥信号量 */
} ym_t;

typedef struct yk_s {
	unsigned char ctl_val;		/* 数值，存为双点值，DP_OPEN or DP_CLOSE */
	unsigned char ctl_mode;		/* 遥控模式，YK_DO or YK_SBO */
	unsigned char ctl_step;		/* 控制阶段，YK_VAL, YK_SEL, YK_OPER or YK_DONE */
	unsigned char sel_val;		/* 选择值 */
	int 		  sel_timer;	/* 选择超时计时器，us */
	int 		  oper_timer;	/* 控制超时计时器，us */
	int app_id;					/* 控制应用的ID */
	sem_t 		 mutex;		/* 互斥信号量 */
} yk_t;

typedef struct app_hook_func_s {//本质为Node
	int app_id;
	//int (* hook_func) (unsigned char *, int);
	//hook_func为一函数指针，返回值为int，函数参数为(unsigned char *, int)
	hook_func_t hook_func;
	unsigned char msg_buf [MAX_MSG_LEN];
	int msg_len;
	struct app_hook_func_s * next;
} app_hook_func_t;

typedef struct {
	unsigned char type;
	unsigned char sub_type;
	unsigned short content_len;
	unsigned char content [4];
} internal_msg_t;

int init_rdb ();

int reg_rdb_hook (
	int app_id,
	hook_func_t func_ptr
	);

int reg_rdb_data (
	int app_id,
	int rdb_type,
	unsigned int rdb_no,
	unsigned int proto_no
	);

int yx_set_value (
	unsigned int yx_no,		/* 从0到YX_NUMBER-1 */
	unsigned int * val,		/* 双点入库，DP_OPEN or DP_CLOSE */
	time_type * ts,			/* 时标 */
	unsigned char cos_soe_flag
	);
int yx_get_value (
	int app_id,
	unsigned int begin_no,	/* 从0到YX_NUMBER-1 */
	unsigned int num,		/* 从1到YX_NUMBER */
	unsigned int data []	/* 数据缓存数组 */
	);
int yx_get_value_proto (
	int app_id,
	unsigned int begin_no,	/* 从0开始，协议点号 */
	unsigned int num,		/* 从1到YX_NUMBER */
	unsigned int data []	/* 数据缓存数组 */
	);
int yx_get_total_num (
	int app_id,
	unsigned int * num		/* 返回遥信总数目 */
	);
int yx_get_total_num_proto (
	int app_id,
	unsigned int * num		/* 返回遥信总数目 */
	);
int yc_set_value (
	unsigned int yc_no,		/* 从0到YC_NUMBER-1 */
	yc_data_type * val,		/* 遥测值 */
	unsigned char qds,		/* 品质 */
	time_type * ts,			/* 时标，可以为NULL */
	int	b_event				/* 是否触发事件 */
	);
int yc_get_value (
	int app_id,
	unsigned int begin_no,	/* 从0到YC_NUMBER-1 */
	unsigned int num,		/* 从1到YC_NUMBER */
	yc_data_type data [],	/* 数据缓存数组 */
	unsigned char qds [],	/* 品质 */
	unsigned char data_type []/* 返回遥测值数据类型，YC_INT or YC_FLOAT */
	);
int yc_get_value_proto (
	int app_id,
	unsigned int begin_no,	/* 从0开始，协议点号 */
	unsigned int num,		/* 从1到YC_NUMBER */
	yc_data_type data [],	/* 数据缓存数组 */
	unsigned char qds [],	/* 品质 */
	unsigned char data_type []/* 返回遥测值数据类型，YC_INT or YC_FLOAT */
	);
int yc_get_total_num (
	int app_id,
	unsigned int * num		/* 返回遥测总数目 */
	);
int yc_get_total_num_proto (
	int app_id,
	unsigned int * num		/* 返回遥测总数目 */
	);
int ym_set_value (
	unsigned int ym_no,		/* 从0到YM_NUMBER-1 */
	bcr_t * val				/* 二进制计数量 */
	);
int ym_get_value (
	int app_id,
	unsigned int begin_no,	/* 从0到YM_NUMBER-1 */
	unsigned int num,		/* 从1到YM_NUMBER */
	bcr_t data []			/* 数据缓存数组,返回已注册的数据 */	
	);
int ym_get_value_proto (
	int app_id,
	unsigned int begin_no,	/* 从0到YM_NUMBER-1 */
	unsigned int num,		/* 从1到YM_NUMBER */
	bcr_t data []			/* 数据缓存数组,返回已注册的数据 */	
	);
int ym_get_total_num (
	int app_id,
	unsigned int * num		/* 返回遥脉总数目 */
	);
int ym_get_total_num_proto (
	int app_id,
	unsigned int * num		/* 返回遥脉总数目 */
	);
int yk_select (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
	int app_id				/* 遥控发起者的id */
	);
int yk_unselect (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	int app_id				/* 遥控发起者的id */
	);
int yk_operate (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
	int app_id				/* 遥控发起者的id */
	);
int yk_done (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	unsigned char ret		/* 执行结果 */
	);
int yk_get_total_num (
	unsigned int * num		/* 返回遥控总数目 */
	);

int rdb_yx_show ();

int rdb_yc_show ();

int rdb_ym_show ();

#ifdef __cplusplus
}
#endif

#endif /* _RDB_H_ */

