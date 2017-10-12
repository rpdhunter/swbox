/********************************************************************
	created:	2015/11/09
	created:	9:11:2015   14:11
	filename: 	D:\cygwin64\home\ibm\prog\rtu\inc\iec104.h
	file path:	D:\cygwin64\home\ibm\prog\rtu\inc
	file base:	iec104
	file ext:	h
	author:		zhouxf
	purpose:	iec104 protocol
*********************************************************************/
#ifndef _IEC104_H_
#define _IEC104_H_

#include "../rdb/rdb.h"

#define IEC104_PORT			2404
#define IEC104_APDU_SIZE	255
#define IEC104_MSG_SIZE		253
#define IEC104_HEAD			0x68
#define K_VALUE				12
#define W_VALUE				8
#define T0_TO				30
#define T1_TO				15
#define T2_TO				10
#define T3_TO				20

#define MAX_CONN_NUM		2
#define MAX_SBUF_NO			100
#define MAX_CACHE_NO		(MAX_SBUF_NO - 20)

#define MAX_YX_EVENT_NUM_PER_FRAME	10
#define MAX_YC_EVENT_NUM_PER_FRAME	10
#define MAX_FILE_NAME_NUM_PER_FRAME	4
#define MAX_FILE_BYTES_PER_FRAME	200

#define I_FRAME				1
#define S_FRAME				2
#define U_FRAME				3

#define FRAME_PRIO_HI		1
#define FRAME_PRIO_LO		2

#define U_START_CONFIRM		1
#define U_STOP_CONFIRM		2
#define U_TEST_ACTIVE		3
#define U_TEST_CONFIRM		4

#define COT_MATCH_LO_BYTE	1
#define COT_MATCH_HI_BYTE	2
#define COT_MATCH_LO_HI		3
#define COT_MATCH_HI_LO		4

#define COT_ERR_IEC			1
#define COT_ERR_DEF			2

#define SUPERVISION_DIR		1
#define CONTROL_DIR			2

/* supervision direction */
#define M_SP_NA_1			1		/* 单点信息 */
#define M_SP_TA_1			2		/* 带时标的单点 */
#define M_DP_NA_1			3		/* 双点信息 */
#define M_DP_TA_1			4		/* 带时标的双点 */
#define M_PS_NA_1			20		/* 带变位检出的成组单点信息 */
#define M_SP_TB_1			30		/* 带CP56Time2a时标的单点 */
#define M_DP_TB_1			31		/* 带CP56Time2a时标的双点 */

#define M_ME_NA_1			9		/* 测量值，归一化，有品质 */
#define M_ME_TA_1			10		/* 测量值，归一化，有品质，带时标 */
#define M_ME_NB_1			11		/* 测量值，标度化，有品质 */
#define M_ME_TB_1			12		/* 测量值，标度化，有品质，带时标 */
#define M_ME_NC_1			13		/* 测量值，短浮点，有品质 */
#define M_ME_TC_1			14		/* 测量值，短浮点，有品质，带时标 */
#define M_ME_ND_1			21		/* 测量值，归一化，无品质 */
#define M_ME_TD_1			34		/* 带CP56Time2a时标的测量值，归一化 */
#define M_ME_TE_1			35		/* 带CP56Time2a时标的测量值，标度化 */
#define M_ME_TF_1			36		/* 带CP56Time2a时标的测量值，短浮点 */

#define M_FT_NA_1			42		/* 故障事件信息 */

#define M_IT_NA_1			15		/* 累计量 */
#define M_IT_TA_1			16		/* 带时标的累计量 */
#define M_IT_TB_1			37		/* 带CP56Time2a时标的累计量 */
#define M_IT_NB_1			206		/* 累计量，短浮点 */
#define M_IT_TC_1			207		/* 带CP56Time2a时标的累计量，短浮点 */

#define M_ST_NA_1			5		/* 步位置信息 */
#define M_ST_TA_1			6		/* 带时标的步位置信息 */
#define M_ST_TB_1			32		/* 带CP56Time2a时标的步位置信息 */

#define M_EI_NA_1			70		/* 初始化结束 */

/* control direction */
#define C_SC_NA_1			45		/* 单点命令 */
#define C_DC_NA_1			46		/* 双点命令 */
#define C_IC_NA_1			100		/* 召唤 */
#define C_CI_NA_1			101		/* 计量召唤 */
#define C_RD_NA_1			102		/* 读命令 */
#define C_CS_NA_1			103		/* 时钟同步 */
#define C_TS_NA_1			104		/* 测试命令 */
#define C_RP_NA_1			105		/* 复位进程命令 */
#define C_SR_NA_1			200		/* 切换定值区 */
#define C_RR_NA_1			201		/* 读定值区号 */
#define C_RS_NA_1			202		/* 读参数和定值 */
#define C_WS_NA_1			203		/* 写参数和定值 */
#define F_FR_NA_1			210		/* 文件传输 */
#define F_SR_NA_1			211		/* 软件升级 */

#define VSQ_SQ					0x80

#define COT_T					0x80
#define COT_P_N					0x40
#define COT_PER					1
#define COT_BACK				2
#define COT_SPONT				3
#define COT_INIT				4
#define COT_REQ					5
#define COT_ACT					6
#define COT_ACTCON				7
#define COT_DEACT				8
#define COT_DEACTCON			9
#define COT_ACTTERM				10
#define COT_FILE				13
#define COT_INTROGEN			20
#define COT_REQCOGEN			37
#define COT_UNKNOWN_ASDU_TYPE	44
#define COT_UNKNOWN_COT			45
#define	COT_UNKNOWN_APP_ADDR	46
#define COT_UNKNOWN_INF_ADDR	47
#define COT_YK_OP_SYP_ERR		48			/* 遥控执行软压板状态错误 */
#define COT_YK_OP_TS_ERR		49			/* 遥控执行时间戳错误 */
#define COT_YK_OP_DS_ERR		50			/* 遥控执行数字签名认证错误 */

#define GI_YX_1FRAME_NUM		100
#define GI_YC_1FRAME_NUM		40
#define GI_YM_1FRAME_NUM		40

#define YK_CO_MASK				0x03
#define YK_SEL_MASK				0x80
#define YK_EXC_MASK				0x00

#define YK_END_FRAME_NO			1
#define YK_END_FRAME_YES		2

#define FT_GET_DIR				1			/* 文件传输，读目录 */
#define FT_GET_DIR_CON			2			/* 文件传输，读目录确认 */
#define FT_RD_FILE_ACT			3			/* 文件传输，读文件激活 */
#define FT_RD_FILE_ACTCON		4			/* 文件传输，读文件激活确认 */
#define FT_RD_FILE_DATA			5			/* 文件传输，读文件数据 */
#define FT_RD_FILE_DATA_RESP	6			/* 文件传输，读文件数据相应 */

typedef struct {
	int server_ip;
	unsigned short server_port;
	unsigned short app_addr;
	int max_conn_num;
	int cache_event;
	int clear_unconfirmed_msg;
	int k;
	int w;
	/*int t0_to;*/
	int t1_to;
	int t2_to;
	int t3_to;
	int cot_bytes;
	int cot_match_rule;
	int cot_err_rule;
	int app_addr_bytes;
	int inf_addr_bytes;
	int inf_obj_addr_yx_base;
	int inf_obj_addr_yc_base;
	int inf_obj_addr_ym_base;
	int inf_obj_addr_yk_base;
	int yc_int_byte_order_hi_lo;
	int yc_float_byte_order_hi_lo;
	int ym_byte_order_hi_lo;
	int yc_period;
	int yc_pq_send;
	int gi_yx_asdu;		/* M_SP_NA_1, M_DP_NA_1 */
	int gi_yc_asdu;		/* M_ME_NA_1, M_ME_NB_1, M_ME_NC_1, M_ME_ND_1 */
	int ci_ym_asdu;		/* M_IT_NA_1, M_IT_TA_1, M_IT_TB_1 */
	int per_yc_asdu;	/* M_ME_NA_1, M_ME_NB_1, M_ME_NC_1, M_ME_ND_1 */
	int chg_yc_asdu;	/* M_ME_NA_1, M_ME_NB_1, M_ME_NC_1, M_ME_ND_1, M_ME_TA_1, M_ME_TB_1, M_ME_TC_1, M_ME_TD_1, M_ME_TE_1, M_ME_TF_1 */
	int ft_yc_asdu;		/* M_ME_ND_1, M_ME_NC_1 */
	int cos_yx_asdu;	/* M_SP_NA_1, M_DP_NA_1 */
	int soe_yx_asdu;	/* M_SP_TA_1, M_DP_TA_1, M_SP_TB_1, M_DP_TB_1 */
	int yk_end_frame_rule;
} iec104_server_config_t;

typedef struct iec104_msg_s {
	unsigned char buf [IEC104_APDU_SIZE];
	int len;
	int send_no;
	int frame_type;
	struct iec104_msg_s * next;
} iec104_msg_t;

typedef struct {
	int send_no;
	int recv_no;
} data_num_t;

typedef struct client_connect_s {
	int sid;
	unsigned int remote_ip;
	unsigned short remote_port;
	struct client_connect_s * next;
	int recv_sync;
	unsigned char apdu_buf [IEC104_APDU_SIZE];
	unsigned int apdu_parse_cnt;
	int apdu_len;
	int apdu_ptr;
	data_num_t send_cnt;
	data_num_t recv_cnt;
	/*int t0_cnt;*/
	int t1_cnt;
	int t2_cnt;
	int t3_cnt;
	int yc_period_cnt;
	int recv_frame_type;
	int user_data_start;
	unsigned char recv_asdu_type;
	unsigned char recv_vsq;
	unsigned short recv_cot;
	unsigned short recv_app_addr;
	unsigned int recv_inf_addr;
	unsigned char recv_cmd;
	unsigned char file_op_flag;		/* file transfer using */
	char dir_name [64];				/* file transfer using */
	char file_name [64];			/* file transfer using */
	unsigned int file_pos;			/* file transfer using */
	iec104_msg_t * send_buf_lst_head;
	iec104_msg_t * send_buf_lst_tail;
	iec104_msg_t * unconfirmed_buf_lst_head;
	iec104_msg_t * unconfirmed_buf_lst_tail;
	int unconfirmed_send_frame_cnt;
	int unconfirmed_recv_frame_cnt;
} client_connect_t;

typedef struct {
	int app_id;
	int init_ok;
	int init_counter;
	int init_time;
	unsigned int ip_addr;
	unsigned short port;
	unsigned short app_addr;
	int udp_server_id;		/* 内部通信用 */
	int udp_client_id;		/* 内部通信用 */
	client_connect_t * clients_head;
	iec104_msg_t * cache_buf_lst_head;
	iec104_msg_t * cache_buf_lst_tail;
	yx_event_t * yx_event_lst;
	int yx_event_num;
	yc_event_t * yc_event_lst;
	int yc_event_num;
	int cache_event;
	int cache_cnt;
	int clear_unconfirmed_msg;
	int k;					/* 未被确认的I帧最大个数，达到应停止发送I帧 */
	int w;					/* 接收到w个I帧后，必须确认 */
	/*int t0_to;*/			/* t0超时值，建立连接超时，子站不需要 */
	int t1_to;				/* t1超时值，发送或测试APDU的超时 */
	int t2_to;				/* t2超时值，无数据报文t2<t1时确认的超时 */
	int t3_to;				/* t3超时值，长期空闲t3>t1状态下发送测试帧的超时 */
	int cot_bytes;
	int cot_match_rule;
	int cot_err_rule;
	int app_addr_bytes;
	int inf_addr_bytes;
	int inf_obj_addr_yx_base;
	int inf_obj_addr_yc_base;
	int inf_obj_addr_ym_base;
	int inf_obj_addr_yk_base;
	int yc_int_byte_order_hi_lo;
	int yc_float_byte_order_hi_lo;
	int ym_byte_order_hi_lo;
	int yc_period;
	int yc_pq_send;
	int gi_yx_asdu;
	int gi_yc_asdu;
	int ci_ym_asdu;
	int per_yc_asdu;
	int chg_yc_asdu;
	int ft_yc_asdu;
	int cos_yx_asdu;
	int soe_yx_asdu;
	int yk_end_frame_rule;
} iec104_server_t;

extern int iec104_comm_stat;

int send_iec104_internal_udp_data (unsigned char * send_buf, int send_len);
int init_iec104_server ();

#endif /* _IEC104_H_ */
