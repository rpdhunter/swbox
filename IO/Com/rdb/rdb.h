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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include "global_define.h"
#include "data_types.h"
#include "point_table.h"

#define RDB_SEND_PORT		8111
#define RDB_RECV_PORT		8222
//#define ENABLE_PRINT
#define RDB_CHECK_TIME	50000	/* 100000us, 100ms */

#define YC_INT			1		/* 遥测整型 */
#define YC_FLOAT		2		/* 遥测浮点 */
#define DZ_INT			0x01
#define DZ_FLOAT		0x02
#define DZ_STRING		0x03
#define DZ_CHAR         0x04

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
#define QDS_FO			0x04	/* 品质，前台 */
#define QDS_BA			0x02	/* 品质，后台 */
//#define QDS_EN          0x01    /* 品质，有效 */

#define BCR_SEQ_NOT_SQ	0x1f	/* 顺序记法，顺序号 */
#define BCR_SEQ_NOT_CY	0x20	/* 顺序记法，进位 */
#define BCR_SEQ_NOT_CA	0x40	/* 顺序记法，计数量被调整 */
#define BCR_SEQ_NOT_IV	0x80	/* 顺序记法，无效 */

#define YX_SOE_FLAG		0x01
#define YX_COS_FLAG		0x02

#define YX_NUMBER		(YX_VALID_NUM + 4)
#define YC_NUMBER		(YC_VALID_NUM + 4)
#define YK_NUMBER		(YK_VALID_NUM + 4)
#define DZ_NUMBER		(DZ_VALID_NUM + 4)

#define RDB_TYPE_YX		0x01
#define RDB_TYPE_YC		0x02
#define RDB_TYPE_YM		0x03
#define RDB_TYPE_YK		0x04
#define RDB_TYPE_DZ   	0x05

#define MAX_RDB_EVENT_NO	600

#define MAX_RDB_REGED_APP	5

#define MAX_MSG_LEN					2048

#define INTERNAL_MSG_HEAD_LEN		(sizeof (internal_msg_t) - 4 /* sizeof (internal_msg_t.content)*/)

/* message type */
#define MSG_NONE_TYPE 				0x00
#define	MSG_SUB_YX					0x01
#define	MSG_SUB_YC					0x02
#define MSG_CHK_SEND_LST			0x10
#define MSG_CHK_TIME1S_TASK			0x11
#define MSG_CHK_EVENT				0x12
#define MSG_YX_EVENT 				0x20
#define MSG_YC_EVENT 				0x21
#define MSG_YK_EVENT 				0x22
#define MSG_YK_SELECT 				0x30
#define MSG_YK_UNSELECT 			0x31
#define MSG_YK_OPERATE 				0x32
#define MSG_YK_DONE 				0x33
#define MSG_SENDPATH_EVENT 			0x40
#define MSG_DZ_EVENT 				0x50


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
#pragma pack(1)

typedef union {
    Int32		i_val;
    float32		f_val;
} yc_data_type;

typedef struct rdb_yc_param_s{
    unsigned int yc_no;		/* 从0到YC_NUMBER-1 */
    yc_data_type val[1];		/* 遥测值 */
    unsigned char qds;		/* 品质 */
    time_type ts[1];			/* 时标，可以为NULL */
    int	b_event;			/* 是否触发事件 */
}rdb_yc_param_t;

typedef struct rdb_yx_param_s{
    unsigned int yx_no;		/* 从0到YX_NUMBER-1 */
    unsigned int val[1];		/* 双点入库，DP_OPEN or DP_CLOSE */
    time_type ts[1];			/* 时标 */
    unsigned char cos_soe_flag;
}rdb_yx_param_t;


typedef struct rdb_dz_param_s{
    unsigned int dz_no; 		/* 从0到DZ_NUMBER-1 */
    unsigned char tag;			/* 数据类型 */
    unsigned char data_len;		/* 数据长度 */
    char data_buf[8];	/* 数据值 */
}rdb_dz_param_t;


typedef struct rdb_yk_select_s{
    unsigned int yk_no;		/* 从0到YK_NUMBER-1 */
    unsigned char val;		/* 双点入库，DP_OPEN or DP_CLOSE */
    int app_id;				/* 遥控发起者的id */
    int yk_result;			/* 记录遥控各个步骤的结果 */
}rdb_yk_select_t;

typedef struct rdb_yk_unselect_s{
    unsigned int yk_no;		/* 从0到YK_NUMBER-1 */
    int app_id;				/* 遥控发起者的id */
    int yk_result;			/* 记录遥控各个步骤的结果 */
}rdb_yk_unselect_t;

typedef struct rdb_yk_operate_s{
    unsigned int yk_no;		/* 从0到YK_NUMBER-1 */
    unsigned char val;		/* 双点入库，DP_OPEN or DP_CLOSE */
    int app_id;				/* 遥控发起者的id */
    int yk_result;			/* 记录遥控各个步骤的结果 */
}rdb_yk_operate_t;

typedef struct rdb_yk_done_s{
    unsigned int yk_no; 	/* 从0到YK_NUMBER-1 */
    unsigned char ret;		/* 执行结果 */
    int app_id;				/* 遥控发起者的id */
    int yk_result;			/* 记录遥控各个步骤的结果 */
}rdb_yk_done_t;


typedef struct rdb_udp_s{
    int rdb_server_id;				/* 外部通信用 */
    int rdb_client_id;				/* 外部通信用 */
    struct sockaddr_in client_addr;

    rdb_yc_param_t yc_param;
    rdb_yx_param_t yx_param;
    rdb_dz_param_t dz_param;
    rdb_yk_select_t yk_select;
    rdb_yk_unselect_t yk_unselect;
    rdb_yk_operate_t yk_operate;
    rdb_yk_done_t yk_done;
}rdb_udp_t;
#pragma pack(0)

/* 二进制计数器读数 */
typedef struct bcr_s {
    Uint32 counter_reading;		/* 计数器读法 */
    unsigned char seq_not;		/* 顺序记法 */
} bcr_t;

typedef struct data_hook_s {
    int	app_id;					/* 接受hook的应用ID */
    Uint32 proto_data_no;		/* 在协议中的点号 */
    struct data_hook_s * next;
} data_hook_t;

typedef struct yx_event_s {
    Uint32 data_no;
    Uint32 event_val;
    time_type time_val;
    unsigned char cos_soe_flag;
} yx_event_t;

typedef struct yc_event_s {
    Uint32 data_no;
    yc_data_type event_val;
    unsigned char data_type;
    unsigned char qds;
    time_type time_val;
} yc_event_t;

typedef struct rdb_event_s {
    union {
        yx_event_t yx_e;
        yc_event_t yc_e;
    };
    struct rdb_event_s * next;
} rdb_event_t;

typedef struct yx_preset_s {
    Uint32 preset_val;			/* 预置值，延时结束后，与cur_val不等，则赋予cur_val */
    Uint8  preset_flag;			/* 预置cos/soe的flag */
    time_type ts;				/* 预置值赋值时间 */
    struct timeval delay_until;
} yx_preset_t;

typedef int (* hook_func_t) (unsigned char *, int);

typedef struct yx_s {
    Uint32 		  cur_val;		/* 数值，存为双点值，带品质，DP_OPEN, DP_CLOSE */
    rdb_event_t * p_event_head;	/* event链头，取事件 */
    rdb_event_t * p_event_tail;	/* event链尾，压事件 */
    data_hook_t * p_hook;		/* hook链 */
    yx_preset_t	  pres_val;		/* 预置值 */
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

typedef struct yk_s {
    unsigned char ctl_val;		/* 数值，存为双点值，DP_OPEN or DP_CLOSE */
    unsigned char ctl_mode;		/* 遥控模式，YK_DO or YK_SBO */
    unsigned char ctl_step;		/* 控制阶段，YK_VAL, YK_SEL, YK_OPER or YK_DONE */
    unsigned char sel_val;		/* 选择值 */
    int 		  sel_timer;	/* 选择超时计时器，us */
    int 		  oper_timer;	/* 控制超时计时器，us */
    int 		  app_id;		/* 控制应用的ID */
    data_hook_t * p_hook;		/* hook链 */
    sem_t 		  mutex;		/* 互斥信号量 */
} yk_t;

typedef struct dz_s {
    unsigned int dz_no;
    unsigned char tag;			/* Tag类型 */
    unsigned char data_len;		/* 数据长度 */
    unsigned char data_buf[256];/* 值 */

//	int 		  app_id;		/* 控制应用的ID */
    data_hook_t * p_hook;		/* hook链 */
//	int sn;						/* 当前区号 */
//	int sn_min;					/* 终端支持的最小区号 */
//	int sn_max;					/* 终端支持的最大区号 */
    sem_t 		  mutex;		/* 互斥信号量 */
} dz_t;

typedef struct app_hook_func_s {
    int app_id;								/**/
    hook_func_t hook_func;					/*函数指针,指向注册的函数*/
    unsigned char msg_buf [MAX_MSG_LEN];	/*传递消息的数组*/
    int msg_len;							/*传递消息的长度*/
    struct app_hook_func_s * next;			/*Next指针*/
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

int proto_rdb_reg_data (int com_no);

int yx_set_value (
    unsigned int yx_no,		/* 从0到YX_NUMBER-1 */
    unsigned int * val,		/* 双点入库，DP_OPEN or DP_CLOSE */
    time_type * ts,			/* 时标 */
    unsigned char cos_soe_flag,
    int flag				/* 是否发送给QT的rdb */
    );
int yx_preset_value (
    unsigned int yx_no,		/* 从0到YX_NUMBER-1 */
    unsigned int * val,		/* 双点入库，DP_OPEN or DP_CLOSE */
    time_type * ts,			/* 时标 */
    time_type * delay,		/* 延时 */
    unsigned char cos_soe_flag
    );
int yx_get_value (
    int app_id,
    unsigned int begin_no,	/* 从0到YX_NUMBER-1 */
    unsigned int num,		/* 从1到YX_NUMBER */
    unsigned int data []	/* 数据缓存数组 */
    );
int yx_get_preset_value (
    int app_id,
    unsigned int begin_no,	/* 从0到YX_NUMBER-1 */
    unsigned int num,		/* 从1到YX_NUMBER */
    unsigned int data []	/* 数据缓存数组,返回遥信数据值 */
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
    int	b_event,				/* 是否触发事件 */
    int flag				/* 是否发送给QT的rdb */
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

int dz_set_value (
    rdb_dz_param_t * dz,
    int flag					/* 是否发送给QT的rdb */
    );
int dz_get_value (
//	int app_id,
    rdb_dz_param_t * dz
    );
int dz_get_value_proto (
    int app_id,
    int begin_no,
    int num,
    rdb_dz_param_t data[]

    );


int dz_get_total_num (
    int app_id,
    unsigned int * num		/* 返回遥测总数目 */
    );
int dz_get_total_num_proto (
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
int yk_select_proto (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
    int app_id,				/* 遥控发起者的id */
    int flag				/* 是否发送给QT的rdb */
    );
int yk_unselect (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    int app_id				/* 遥控发起者的id */
    );
int yk_unselect_proto (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    int app_id,				/* 遥控发起者的id */
    int flag				/* 是否发送给QT的rdb */
    );
int yk_operate (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
    int app_id				/* 遥控发起者的id */
    );
int yk_operate_proto (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
    int app_id,				/* 遥控发起者的id */
    int flag				/* 是否发送给QT的rdb */
    );
int yk_done (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    unsigned char ret,		/* 执行结果 */
    int app_id
    );
int yk_done_proto (
    unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
    unsigned char ret,		/* 执行结果 */
    int app_id,
    int flag				/* 是否发送给QT的rdb */
    );
int yk_get_total_num (
    unsigned int * num		/* 返回遥控总数目 */
    );
int yk_get_total_num_proto (
    unsigned int * num		/* 返回遥控总数目 */
    );

int yx_rdb_index_to_proto_index (
    int app_id,
    unsigned int rdb_index,
    unsigned int * proto_index);

int yc_rdb_index_to_proto_index (
    int app_id,
    unsigned int rdb_index,
    unsigned int * proto_index);

int rdb_yx_show ();

int rdb_yc_show ();


extern rdb_udp_t * rdb_udp;
extern void send_path(unsigned char meas_path[],int len);

#ifdef __cplusplus
}
#endif

#endif /* _RDB_H_ */

