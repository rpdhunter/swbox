/********************************************************************
	created:	2015/11/09
	created:	9:11:2015   14:10
	filename: 	D:\cygwin64\home\ibm\prog\rtu\src_bin\iec104.c
	file path:	D:\cygwin64\home\ibm\prog\rtu\src_bin
	file base:	iec104
	file ext:	c
	author:		zhouxf
	purpose:	iec104 protocol
*********************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#include "../rdb/thread.h"
#include "../rdb/ipcs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>


#include "protocol.h"
#include "iec104.h"
#include "../rdb/point_table.h"
#include "../rdb/rdb.h"

/*
#include "global_define.h"
#include "params_settings.h"
#include "base_functions.h"
#include "bin_out.h"
#include "monitor_status.h"
#include "bin_in.h"
#include "screen_driver.h"
*/
static iec104_msg_t * iec104_msg_pool = NULL;
static iec104_server_config_t * iec104_cfg;
static iec104_server_t * iec104_server;

int iec104_comm_stat = 0;

static int read_iec104_server_cfg (char * cfg_file);
static int init_iec104_server_param ();
static int init_internal_udp_com ();
static int new_client (client_connect_t ** p_clients_head, client_connect_t ** p_new_client);
static int del_client (client_connect_t ** p_clients_head, client_connect_t ** p_del_client);
static int init_client (iec104_server_t * p_server, client_connect_t * p_client, int sfd, unsigned int ip, unsigned short port);
static void accept_task (void * arg);
static void iec104_task (void * arg);
static void timer1s_task (void * arg);
static int init_accept_task ();
static int init_iec104_task ();
static int init_timer1s_task ();
static int init_iec104_reg_rdb ();
static int recv_data_process (iec104_server_t * p_server, client_connect_t * p_client, unsigned char recv_buf [], int len);
static int iec104_msg_proc (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_link_data_proc (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_app_data_proc (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_close_connection (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_com_inf (iec104_server_t * p_server, client_connect_t * p_client, unsigned short * cot);
static int iec104_send_uframe (iec104_server_t * p_server, client_connect_t * p_client, int cmd, int prio);
static int iec104_send_sframe (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_gi (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_gi_confirm (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_gi_data (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_gi_yx (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_gi_yc (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_gi_end_data (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_cs (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_cs_end_data (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_ci (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_ci_confirm (iec104_server_t * p_server, client_connect_t * p_client);
//static int iec104_ci_data (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_ci_end_data (iec104_server_t * p_server, client_connect_t * p_client);
//static int iec104_sr (iec104_server_t * p_server, client_connect_t * p_client);
//static int iec104_sr_confirm (iec104_server_t * p_server, client_connect_t * p_client, int neg_ack);
//static int iec104_rr (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_rr_confirm (iec104_server_t * p_server, client_connect_t * p_client, unsigned int setting_group_no);
static int iec104_yk (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_yk_reply (iec104_server_t * p_server, client_connect_t * p_client, int yk_reply_flag);
static int iec104_ft_get_dir (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_ft_file_act (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_ft_file_transfer (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_file_transfer (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_period_yc (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_save_send_frame (iec104_server_t * p_server, client_connect_t * p_client, unsigned char * send_buf, int len, int frame_type, int prio);
static int iec104_save_unconfirmed_frame (iec104_server_t * p_server, client_connect_t * p_client, iec104_msg_t * sbuf);
static int iec104_save_cache_frame (iec104_server_t * p_server, unsigned char * send_buf, int len, int frame_type);
static int iec104_check_send_lst (iec104_server_t * p_server, int sid);
static int iec104_check_1s_task (iec104_server_t * p_server);
static int iec104_check_t1_timer (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_t2_timer (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_t3_timer (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_yc_period (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_event (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_check_yx_event (iec104_server_t * p_server, yx_event_t * p_event, int event_num);
static int iec104_check_yc_event (iec104_server_t * p_server, yc_event_t * p_event, int event_num);
static int iec104_save_yx_event (iec104_server_t * p_server, yx_event_t * p_event);
static int iec104_free_yx_event (iec104_server_t * p_server, yx_event_t * p_event);
static int iec104_free_yc_event (iec104_server_t * p_server, yc_event_t * p_event);
static int iec104_save_yc_event (iec104_server_t * p_server, yc_event_t * p_event);
static int iec104_send_frame_lst (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_confirm_frame (iec104_server_t * p_server, client_connect_t * p_client);
static int iec104_send_frame (iec104_server_t * p_server, client_connect_t * p_client, iec104_msg_t * sbuf);
static int iec104_check_asdu_direct (unsigned char asdu_type, unsigned char * direct);
static int send_chk_send_lst_msg (iec104_server_t * p_server, int sid);
static int send_chk_time1s_task_msg (iec104_server_t * p_server);
static int get_iec104_frame_recv_no (unsigned char apdu [], int len, int * recv_no);
static int get_iec104_frame_send_no (unsigned char apdu [], int len, int * send_no);
static int init_iec104_msg_pool ();
static int find_valid_iec104_msg_buf (iec104_msg_t ** sbuf);
static int free_iec104_msg_buf (iec104_msg_t * sbuf);
static void iec104_msg_init_header (unsigned char send_buf [], int * send_cnt);

int show_msg (char * prompt, unsigned char buf [], int len);
int set_seconds_1900(time_type * tt_1900);
#if 0
int set_seconds_1900(time_type * tt_1900)
{

	Uint32 t_1970;
	struct tm offset;

	offset.tm_year = 70;
	offset.tm_mon = 0;
	offset.tm_mday = 1;
	offset.tm_hour = 0;
	offset.tm_min = 0;
	offset.tm_sec = 0;

	/*
	* mktime return a seconds value from 1900.1.1
	* so year must minus 70
	*/
	t_1970 = tt_1900->seconds - mktime(&offset);

	Seconds_set(t_1970);				/* seconds from 1970.1.1 */
	set_ms_counter(tt_1900->msec);		/* mseconds clear */

	return 0;
}
#endif
int show_msg(char * prompt, unsigned char buf[], int len)
{
	int i;

	if (buf == NULL ||
		len <= 0) {
		return -1;
	}

	if (prompt != NULL) {
		_DPRINTF("%s :", prompt);
	}
	else {
		_DPRINTF("msg :");
	}

	for (i = 0; i < len; i++) {
		if ((i & 0xf) == 0) {
			_DPRINTF("\n");
		}
		_DPRINTF(" 0x%02x", buf[i]);
	}
	_DPRINTF("\n");

	return 0;
}
int cp56time2a_to_time(unsigned char cp56t[], time_type * tt)
{
	struct tm ltm;
	unsigned short ms;

	if (tt == NULL ||
		cp56t == NULL) {
		return -1;
	}

	ms = (cp56t[1] << 8) + cp56t[0];
	tt->msec = ms % 1000;
	ltm.tm_sec = ms / 1000;
	ltm.tm_min = cp56t[2];
	ltm.tm_hour = cp56t[3] & 0x1f;
	ltm.tm_mday = cp56t[4] & 0x1f;
	ltm.tm_wday = (cp56t[4] >> 5) - 1;
	if (ltm.tm_wday < 0) {
		ltm.tm_wday = 0;
	}
	ltm.tm_mon = cp56t[5] - 1;
	if (ltm.tm_mon < 0) {
		ltm.tm_mon = 0;
	}
	ltm.tm_year = cp56t[6] + 100;
	tt->seconds = mktime(&ltm);

	return 0;
}
int time_to_cp56time2a(time_type * tt, unsigned char cp56t[])
{
	struct tm * ltm;
	unsigned short ms;
	time_t ttt;

	if (tt == NULL ||
		cp56t == NULL) {
		return -1;
	}

	ttt = tt->seconds;
	ltm = localtime(&ttt);
	ms = ltm->tm_sec * 1000 + tt->msec;
	cp56t[0] = LSB(ms);
	cp56t[1] = MSB(ms);
	cp56t[2] = ltm->tm_min;
	cp56t[3] = ltm->tm_hour;
	cp56t[4] = ((ltm->tm_wday + 1) << 5) + ltm->tm_mday;
	cp56t[5] = ltm->tm_mon + 1;
	cp56t[6] = ltm->tm_year - 100;

	return 0;
}
static int init_iec104_data_local_mem ()
{
	iec104_cfg = (iec104_server_config_t *)malloc (sizeof (iec104_server_config_t));
	if (iec104_cfg == NULL) {
		return -1;
	}

	iec104_server = (iec104_server_t *)malloc (sizeof (iec104_server_t));
	if (iec104_server == NULL) {
		return -1;
	}
	
	return 0;
}

static int read_iec104_server_cfg (char * cfg_file)
{	
	iec104_cfg->server_ip = /*0xc0a80064*/ /* 192.168.0.100 */ INADDR_ANY;
	iec104_cfg->server_port = IEC104_PORT;
	iec104_cfg->app_addr = 1;
#if 0
	if (iec104_cfg->server_ip != INADDR_ANY) {
		iec104_cfg->app_addr = iec104_cfg->server_ip & 0xffff;
	}
#endif
	iec104_cfg->max_conn_num = MAX_CONN_NUM;		/* SHOULD ≤ MAX_CONN_NUM */
	iec104_cfg->cache_event = 1;
	iec104_cfg->clear_unconfirmed_msg = 1;
	iec104_cfg->k = K_VALUE;
	iec104_cfg->w = W_VALUE;
	/*iec104_cfg->t0_to = T0_TO;*/
	iec104_cfg->t1_to = T1_TO;
	iec104_cfg->t2_to = T2_TO;
	iec104_cfg->t3_to = T3_TO;
	iec104_cfg->cot_bytes = 2;
	iec104_cfg->cot_match_rule = COT_MATCH_LO_BYTE;
	iec104_cfg->cot_err_rule = COT_ERR_IEC;
	iec104_cfg->app_addr_bytes = 2;
	iec104_cfg->inf_addr_bytes = 3;
	iec104_cfg->inf_obj_addr_yx_base = INF_OBJ_ADDR_YX_BASE;
	iec104_cfg->inf_obj_addr_yc_base = INF_OBJ_ADDR_YC_BASE;
	iec104_cfg->inf_obj_addr_yk_base = INF_OBJ_ADDR_YK_BASE;
	iec104_cfg->inf_obj_addr_ym_base = INF_OBJ_ADDR_YM_BASE;
	iec104_cfg->yc_int_byte_order_hi_lo = 0;
	iec104_cfg->yc_float_byte_order_hi_lo = 0;
	iec104_cfg->ym_byte_order_hi_lo = 0;
	iec104_cfg->yc_period = 10;
	iec104_cfg->yc_pq_send = 0;
	iec104_cfg->gi_yx_asdu = M_SP_NA_1;
	if (iec104_cfg->gi_yx_asdu != M_SP_NA_1 &&
		iec104_cfg->gi_yx_asdu != M_DP_NA_1) {
		iec104_cfg->gi_yx_asdu = M_SP_NA_1;
	}
	iec104_cfg->gi_yc_asdu = M_ME_NC_1;
	if (iec104_cfg->gi_yc_asdu != M_ME_NA_1 &&
		iec104_cfg->gi_yc_asdu != M_ME_NB_1 &&
		iec104_cfg->gi_yc_asdu != M_ME_NC_1 &&
		iec104_cfg->gi_yc_asdu != M_ME_ND_1) {
		iec104_cfg->gi_yc_asdu = M_ME_NC_1;
	}
	iec104_cfg->ci_ym_asdu = M_IT_NA_1;
	if (iec104_cfg->ci_ym_asdu != M_IT_NA_1 &&
		iec104_cfg->ci_ym_asdu != M_IT_TA_1 &&
		iec104_cfg->ci_ym_asdu != M_IT_TB_1) {
		iec104_cfg->ci_ym_asdu = M_IT_NA_1;
	}
	iec104_cfg->per_yc_asdu = M_ME_NC_1;
	if (iec104_cfg->per_yc_asdu != M_ME_NA_1 &&
		iec104_cfg->per_yc_asdu != M_ME_NB_1 &&
		iec104_cfg->per_yc_asdu != M_ME_NC_1 &&
		iec104_cfg->per_yc_asdu != M_ME_ND_1) {
		iec104_cfg->per_yc_asdu = M_ME_NC_1;
	}
	iec104_cfg->chg_yc_asdu = M_ME_NC_1;
	if (iec104_cfg->chg_yc_asdu != M_ME_NA_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_NB_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_NC_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_ND_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_TA_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_TB_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_TC_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_TD_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_TE_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_TF_1) {
		iec104_cfg->chg_yc_asdu = M_ME_NC_1;
	}
	//iec104_cfg->ft_yc_asdu = M_ME_NC_1;
	if (iec104_cfg->chg_yc_asdu != M_ME_NC_1 &&
		iec104_cfg->chg_yc_asdu != M_ME_ND_1) {
		iec104_cfg->chg_yc_asdu = M_ME_NC_1;
	}
	iec104_cfg->cos_yx_asdu = M_SP_NA_1;
	if (iec104_cfg->cos_yx_asdu != M_SP_NA_1 &&
		iec104_cfg->cos_yx_asdu != M_DP_NA_1) {
		iec104_cfg->cos_yx_asdu = M_SP_NA_1;
	}
	iec104_cfg->soe_yx_asdu = M_SP_TB_1;
	if (iec104_cfg->soe_yx_asdu != M_SP_TA_1 &&
		iec104_cfg->soe_yx_asdu != M_DP_TA_1 &&
		iec104_cfg->soe_yx_asdu != M_SP_TB_1 &&
		iec104_cfg->soe_yx_asdu != M_DP_TB_1) {
		iec104_cfg->soe_yx_asdu = M_SP_TA_1;
	}
	iec104_cfg->yk_end_frame_rule = YK_END_FRAME_NO;

	return 0;
}

static int init_iec104_server_param ()
{
	iec104_server->app_id = IEC104_PORT;
	iec104_server->init_ok = 0;
	iec104_server->init_counter = 0;
	iec104_server->init_time = 5;
	iec104_server->ip_addr = iec104_cfg->server_ip;
	iec104_server->port = iec104_cfg->server_port;
	iec104_server->app_addr = iec104_cfg->app_addr;
	iec104_server->clients_head = NULL;
	iec104_server->cache_buf_lst_head = NULL;
	iec104_server->cache_buf_lst_tail = NULL;
	iec104_server->yx_event_lst = (yx_event_t *)malloc (sizeof (yx_event_t) * (MAX_YX_EVENT_NUM_PER_FRAME + 1));
	iec104_server->yx_event_num = 0;
	iec104_server->yc_event_lst = (yc_event_t *)malloc (sizeof (yc_event_t) * (MAX_YC_EVENT_NUM_PER_FRAME + 1));
	iec104_server->yc_event_num = 0;
	iec104_server->cache_event = iec104_cfg->cache_event;
	iec104_server->cache_cnt = 0;
	iec104_server->clear_unconfirmed_msg = iec104_cfg->clear_unconfirmed_msg;
	iec104_server->k = iec104_cfg->k;
	iec104_server->w = iec104_cfg->w;
	/*iec104_server->t0_to = iec104_cfg->t0_to;*/
	iec104_server->t1_to = iec104_cfg->t1_to;
	iec104_server->t2_to = iec104_cfg->t2_to;
	iec104_server->t3_to = iec104_cfg->t3_to;
	iec104_server->cot_bytes = iec104_cfg->cot_bytes;
	iec104_server->cot_match_rule = iec104_cfg->cot_match_rule;
	iec104_server->cot_err_rule = iec104_cfg->cot_err_rule;
	iec104_server->app_addr_bytes = iec104_cfg->app_addr_bytes;
	iec104_server->inf_addr_bytes = iec104_cfg->inf_addr_bytes;
	iec104_server->inf_obj_addr_yx_base = iec104_cfg->inf_obj_addr_yx_base;
	iec104_server->inf_obj_addr_yc_base = iec104_cfg->inf_obj_addr_yc_base;
	iec104_server->inf_obj_addr_yk_base = iec104_cfg->inf_obj_addr_yk_base;
	iec104_server->inf_obj_addr_ym_base = iec104_cfg->inf_obj_addr_ym_base;
	iec104_server->yc_int_byte_order_hi_lo = iec104_cfg->yc_int_byte_order_hi_lo;
	iec104_server->yc_float_byte_order_hi_lo = iec104_cfg->yc_float_byte_order_hi_lo;
	iec104_server->ym_byte_order_hi_lo = iec104_cfg->ym_byte_order_hi_lo;
	iec104_server->yc_period = iec104_cfg->yc_period;
	iec104_server->yc_pq_send = iec104_cfg->yc_pq_send;
	iec104_server->gi_yx_asdu = iec104_cfg->gi_yx_asdu;
	iec104_server->gi_yc_asdu = iec104_cfg->gi_yc_asdu;
	iec104_server->ci_ym_asdu = iec104_cfg->ci_ym_asdu;
	iec104_server->per_yc_asdu = iec104_cfg->per_yc_asdu;
	iec104_server->chg_yc_asdu = iec104_cfg->chg_yc_asdu;
	iec104_server->ft_yc_asdu = iec104_cfg->ft_yc_asdu;
	iec104_server->cos_yx_asdu = iec104_cfg->cos_yx_asdu;
	iec104_server->soe_yx_asdu = iec104_cfg->soe_yx_asdu;
	iec104_server->yk_end_frame_rule = iec104_cfg->yk_end_frame_rule;

	return 0;
}

static int init_internal_udp_com ()
{
	struct sockaddr_in serv_addr;
	int server_sfd, client_sfd;

	
	server_sfd = socket (AF_INET, SOCK_DGRAM, 0);
	if (server_sfd == -1) {
		_DPRINTF ("socket failed\n");
		return -1;
	}
	client_sfd = socket (AF_INET, SOCK_DGRAM, 0);
	printf("client_sfd=%d\n", client_sfd);
	if (client_sfd == -1) {
		_DPRINTF ("socket failed\n");
		return -1;
	}

	memset ((unsigned char *)&serv_addr, 0, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (IEC104_INTERNAL_UDP_PORT);
	serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);

	if (bind (server_sfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0) {
		_DPRINTF ("bind internal udp socket failed\n");
		return -1;
	}

	iec104_server->udp_server_id = server_sfd;
	iec104_server->udp_client_id = client_sfd;

	return 0;
}

int send_iec104_internal_udp_data (unsigned char * send_buf, int send_len)
{
	struct sockaddr_in addr_in;
	int bytes_send;
	int re_send_cnt;

	if (iec104_server->udp_client_id == -1 ||
		send_buf == NULL ||
		send_len <= 0) {
		return -1;
	}

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons (IEC104_INTERNAL_UDP_PORT);
	addr_in.sin_addr.s_addr = inet_addr ("127.0.0.1");

	re_send_cnt = 10;
	while (re_send_cnt-- > 0) {
		
		bytes_send = sendto (iec104_server->udp_client_id, send_buf, send_len, 0, 
			(struct sockaddr *)&addr_in, sizeof (addr_in));
		if (bytes_send == send_len) {
			break;
		}
		usleep (2000);
	}

	return bytes_send;
}

static int new_client (client_connect_t ** p_clients_head, client_connect_t ** p_new_client)
{
	client_connect_t * p_client, * p_head;

	if (p_clients_head == NULL ||
		p_new_client == NULL) {
		return -1;
	}

	p_client = (client_connect_t *)malloc (sizeof (client_connect_t));
	if (p_client == NULL) {
		return -1;
	}
	p_client->next = NULL;
	* p_new_client = p_client;

	p_head = * p_clients_head;
	if (p_head == NULL) {
		* p_clients_head = p_client;
	}
	else {
		while (p_head->next) {
			p_head = p_head->next;
		}
		p_head->next = p_client;
	}

	return 0;
}

static int del_client (client_connect_t ** p_clients_head, client_connect_t ** p_del_client)
{
	int find_client = 0;
	client_connect_t * p_head, * p_client;
	
	if (p_clients_head == NULL ||
		p_del_client == NULL) {
		return -1;
	}

	p_head = * p_clients_head;
	p_client = * p_del_client;

	if (p_head == p_client) {
		* p_clients_head = (* p_clients_head)->next;
		find_client = 1;
	}
	else {
		while (p_head->next) {
			if (p_head->next == p_client) {
				p_head->next = p_client->next;
				find_client = 1;
				break;
			}
			p_head = p_head->next;
		}
	}

	* p_del_client = (* p_del_client)->next;

	if (find_client) {
		free (p_client);
		return 0;
	}
	else {
		_DPRINTF ("del_client can't find client\n");
		return -1;
	}
}

static int init_client (iec104_server_t * p_server, client_connect_t * p_client, int sfd, unsigned int ip, unsigned short port)
{
	p_client->sid = sfd;
	p_client->remote_ip = ip;
	p_client->remote_port = port;
	p_client->recv_sync = 0;
	p_client->user_data_start = 0;
	p_client->send_buf_lst_head = NULL;
	p_client->send_buf_lst_tail = NULL;
	p_client->unconfirmed_buf_lst_head = NULL;
	p_client->unconfirmed_buf_lst_tail = NULL;
	p_client->send_cnt.send_no = 0;
	p_client->send_cnt.recv_no = 0;
	p_client->recv_cnt.send_no = 0;
	p_client->recv_cnt.recv_no = 0;
	p_client->t1_cnt = iec104_server->t1_to + 1;	/* time out */
	p_client->t2_cnt = iec104_server->t2_to + 1;	/* time out */
	p_client->t3_cnt = iec104_server->t3_to + 1; /* time out */
	p_client->yc_period_cnt = 0;
	p_client->unconfirmed_send_frame_cnt = 0;
	p_client->unconfirmed_recv_frame_cnt = 0;
	p_client->file_op_flag = 0;
	p_client->file_pos = 0;
	p_client->send_buf_lst_head = p_server->cache_buf_lst_head;
	p_client->send_buf_lst_tail = p_server->cache_buf_lst_tail;
	p_server->cache_buf_lst_head = p_server->cache_buf_lst_tail = NULL;
	p_server->cache_cnt = 0;

	return 0;
}

static void accept_task (void * arg)
{
	unsigned int local_ip = iec104_server->ip_addr;
	unsigned short local_port = iec104_server->port;
	int optval, socket_id, accept_id;
	socklen_t client_addr_length;
	struct sockaddr_in client_addr;
	struct sockaddr_in serv_addr;
	struct linger op;
	client_connect_t * pclient;

	if (init_internal_udp_com () != 0) {
		exit (0);
	}
	
	client_addr_length = sizeof (struct sockaddr_in);
	memset ((unsigned char *)&serv_addr, 0, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(local_port);
	serv_addr.sin_addr.s_addr = htonl(local_ip);

	socket_id = socket (AF_INET, SOCK_STREAM, 0);
	if (socket_id == -1)  {
		_DPRINTF ("socket failed\n");
		exit (0);
	}
	optval = 1;
	setsockopt (socket_id, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof (optval));
	optval = 1;
	setsockopt (socket_id, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof (optval));
	
	if (bind (socket_id, (struct sockaddr *)&serv_addr,sizeof (serv_addr)) < 0) {
		_DPRINTF ("bind failed, local ip(%d.%d.%d.%d), port(%d),socketid(%d)\n",
			(unsigned char)(local_ip >> 24), (unsigned char)(local_ip >> 16),
			(unsigned char)(local_ip >> 8), (unsigned char)(local_ip),
			local_port, socket_id);
		close (socket_id);
		exit (0);
	}

	if (listen (socket_id, iec104_cfg->max_conn_num) < 0) {
		_DPRINTF ("listen failed, local ip(%d.%d.%d.%d), port(%d), socketid(%d)\n",
			(unsigned char)(local_ip >> 24), (unsigned char)(local_ip >> 16),
			(unsigned char)(local_ip >> 8), (unsigned char)(local_ip),
			local_port, socket_id);
		close (socket_id);
		exit (0);
	}
	
	while (1) {
		if (iec104_server->init_ok) {
			accept_id = accept (socket_id,(struct sockaddr *)&client_addr, &client_addr_length);
			if (accept_id < 0) {
				_DPRINTF ("accept failed, local ip(%d.%d.%d.%d), port(%d), socketid(%d)\n",
					(unsigned char)(local_ip >> 24), (unsigned char)(local_ip >> 16),
					(unsigned char)(local_ip >> 8), (unsigned char)(local_ip),
					local_port, socket_id);
			}
			else  {
				op.l_onoff = 1;
				op.l_linger = 0;
				setsockopt (accept_id, SOL_SOCKET,SO_LINGER, (char *)&op, sizeof (op));
				optval = 1;
				setsockopt (accept_id, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof (optval));
				//optval = 1;
				//ioctl (accept_id, FIONBIO, (int)&optval);
				if (new_client (&iec104_server->clients_head, &pclient) < 0) {
					_DPRINTF ("new client failed, close socketid(%d)\n", accept_id);
					close (accept_id);
				}
				else {
					init_client (iec104_server, pclient, accept_id, htonl (client_addr.sin_addr.s_addr), htons (client_addr.sin_port));
					_DPRINTF ("sfd(%d) ip(%d.%d.%d.%d) port(%d) accepted\n", accept_id, 
						(pclient->remote_ip >> 24) & 0xff,
						(pclient->remote_ip >> 16) & 0xff,
						(pclient->remote_ip >> 8) & 0xff,
						(pclient->remote_ip) & 0xff,
						pclient->remote_port);
				}
			}
		}
		sleep (1);
	}

	//fdCloseSession(Task_self());
}

static void iec104_task (void * arg)
{
	int rv, max_fd, temp_fd;
	client_connect_t * p_clients;
	/*struct*/ fd_set read_fds;
	struct timeval tv;
	struct sockaddr_in cli;
	int read_len, offset;
	socklen_t cli_addr_len;
	unsigned char rbuf [MAX_MSG_LEN];
	internal_msg_t * p_i_msg;
	int i_sfd;

	while (1) {
		/* 任务状态监视 
		task_running_cnt [IEC104_TASK_CNT]++;*/
		
		/* clear */
		FD_ZERO (&read_fds);
		max_fd = 0;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		/* set fds */
		p_clients = iec104_server->clients_head;
		if (p_clients && !iec104_comm_stat) {
			iec104_comm_stat = 1;
			//screen_iec104_light (iec104_comm_stat);
		}
		else if (!p_clients && iec104_comm_stat) {
			iec104_comm_stat = 0;
			//screen_iec104_light (iec104_comm_stat);
		}
		while (p_clients != NULL) {
			temp_fd = p_clients->sid;
			FD_SET (temp_fd, &read_fds);
			max_fd = temp_fd > max_fd ? temp_fd : max_fd;
			p_clients = p_clients->next;
		}
		temp_fd = iec104_server->udp_server_id;
		FD_SET (temp_fd, &read_fds);
		max_fd = temp_fd > max_fd ? temp_fd : max_fd;
		/* select */
		rv = select (max_fd + 1, &read_fds, NULL, NULL, &tv);
		if (rv == -1) {
			_DPRINTF ("iec104_task select error\n");
			sleep (1);
			continue;
		}
		else if (rv == 0) {
			/* time out */
			continue;
		}
		
		/* some fd set */
		/* internal msg */
		if (FD_ISSET (iec104_server->udp_server_id, &read_fds)) {
			//printf("recv internal msg\n");
			cli_addr_len = sizeof (cli);
			read_len = recvfrom (temp_fd, rbuf, sizeof (rbuf), 0, (struct sockaddr *)&cli, &cli_addr_len);
			if (read_len <= 0) {
				_DPRINTF ("recv internal msg error or time out\n");
			}
			else {
				offset = 0;
				while (offset < read_len) {
					p_i_msg = (internal_msg_t *)(rbuf + offset);
					switch (p_i_msg->type) {
					case MSG_CHK_SEND_LST:
						i_sfd = p_i_msg->content [0] | (p_i_msg->content [1] << 8) | 
							(p_i_msg->content [2] << 16) | (p_i_msg->content [3] << 24);
						iec104_check_send_lst (iec104_server, i_sfd);
						offset += sizeof (internal_msg_t);
						break;
					case MSG_CHK_TIME1S_TASK:
						iec104_check_1s_task (iec104_server);
						offset += sizeof (internal_msg_t);
						break;
					case MSG_CHK_EVENT:
						switch (p_i_msg->sub_type) {
						case MSG_SUB_YX:
							if (iec104_save_yx_event (iec104_server, (yx_event_t *)p_i_msg->content) >= 
								MAX_YX_EVENT_NUM_PER_FRAME) {
								iec104_check_yx_event (iec104_server, iec104_server->yx_event_lst, iec104_server->yx_event_num);
								iec104_free_yx_event (iec104_server, NULL);
							}
							break;
						case MSG_SUB_YC:
							if (iec104_save_yc_event (iec104_server, (yc_event_t *)p_i_msg->content) >= 
								MAX_YC_EVENT_NUM_PER_FRAME) {
								iec104_check_yc_event (iec104_server, iec104_server->yc_event_lst, iec104_server->yc_event_num);
								iec104_free_yc_event (iec104_server, NULL);
							}
							break;
						default:
							break;
						}
						offset += p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN;
						break;
					default:
						break;
					}
				}
			}
		}
		/* external msg */
		p_clients = iec104_server->clients_head;
		while (p_clients != NULL) {
			temp_fd = p_clients->sid;
			if (FD_ISSET (temp_fd, &read_fds)) {
				read_len = recv (temp_fd, rbuf, sizeof (rbuf), 0);
				if (read_len <= 0) {
					_DPRINTF ("sfd(%d) recv external msg timeout or error, closed\n", temp_fd);
					iec104_close_connection (iec104_server, p_clients);
					del_client (&iec104_server->clients_head, &p_clients);
					continue;
				}
				else {
#if __DBUG__	/* show receive message */
					char prompt [24];
					sprintf (prompt, "sfd(%d) recv msg", temp_fd);
					show_msg (prompt, rbuf, read_len);
#endif
					if (recv_data_process (iec104_server, p_clients, rbuf, read_len) < 0) {
						_DPRINTF ("sfd(%d) data process error, closed\n", temp_fd);
						iec104_close_connection (iec104_server, p_clients);
						del_client (&iec104_server->clients_head, &p_clients);
						continue;
					}
				}
			}
			p_clients = p_clients->next;
		}
	}

	//fdCloseSession(Task_self());
}

static void timer1s_task (void * arg)
{
	while (1) {
		sleep (1);
		send_chk_time1s_task_msg (iec104_server);
	}

	//fdCloseSession(Task_self());
}

static int init_accept_task ()
{
	pthread_t tid;

	tid = thread_create ("accept task", 1000 * 4, 1, accept_task, NULL);
	if (tid < 0) {
		_DPRINTF ("construct_task accept task error\n");
		return -1;
	}

	return 0;
}

static int init_iec104_task ()
{
	pthread_t tid;

	tid = thread_create ("104 task", 1000 * 10, 1, iec104_task, NULL);
	if (tid < 0) {
		_DPRINTF ("construct_task iec104 task error\n");
		return -1;
	}

	return 0;
}

static int init_timer1s_task ()
{
	pthread_t tid;

	tid = thread_create ("time task", 1000 * 2, 1, timer1s_task, NULL);
	if (tid < 0) {
		_DPRINTF ("construct_task timer 1s task error\n");
		return -1;
	}

	return 0;
}

static int init_iec104_reg_rdb ()
{
	int i, app_id/*, reg_num*/;

	app_id = iec104_server->app_id;

#if 1
	/* reg yx data list */
	for (i = 0; i < num_of_yx; i++) {
		reg_rdb_data (app_id, RDB_TYPE_YX, i, i);
	}
	
	/* reg yc data list 
	int reg_num = 0;
	if (iec104_server->yc_pq_send) {
		reg_num = num_of_yc;
	}
	else {
		reg_num = dc_temp_2 + 1;	 last basic yc 
	}
	for (i = 0; i < reg_num; i++) {*/
	for (i = 0; i < num_of_yc; i++) {
		reg_rdb_data (app_id, RDB_TYPE_YC, i, i);
	}

	/* reg ym data list 
	for (i = 0; i < num_of_ym; i++) {
		reg_rdb_data (app_id, RDB_TYPE_YM, i, i);
	}*/
#else
	/* only for test */
	i = 0;
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_2, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_3, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_4, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_5, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_6, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_7, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_8, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_9, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_10, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_11, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_12, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_13, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_14, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_15, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_16, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_17, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_18, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_19, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_20, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_21, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_22, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_23, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, yx_24, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, switch_pos_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, device_locked, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, device_alarm, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, sample_fault, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, ac_fail, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, bat_act_st, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, bat_fail, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, bat_low_vol, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, pt_breaking_alarm_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, ct_breaking_alarm_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, cur_prot_action_1_1_a, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, cur_prot_action_1_1_b, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, cur_prot_action_1_1_c, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, cur_prot_action_2_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, cur_prot_action_3_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YX, cur_zero_prot_action_1_1, i++);

	i = 0;
	reg_rdb_data (app_id, RDB_TYPE_YC, Ua1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Ub1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Uc1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Uab1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Ubc1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Uca1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, U01, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Ia1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Ib1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Ic1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, I01, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, P1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, Q1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, COS1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, dc_vol_1, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, dc_vol_2, i++);
	reg_rdb_data (app_id, RDB_TYPE_YC, dc_temp_1, i++);
#endif

	/* reg hook function */
	reg_rdb_hook (app_id, send_iec104_internal_udp_data);
	
	return 0;
}

static int recv_data_process (iec104_server_t * p_server, client_connect_t * p_client, unsigned char recv_buf [], int len)
{
	int i, rv = 0;
	unsigned char * msg_buf;

	msg_buf = p_client->apdu_buf;
	for (i = 0; i < len; i++) {
		if (!p_client->recv_sync) {
			p_client->apdu_ptr = 0;
			msg_buf [0] = recv_buf [i];
			if (msg_buf [0] == IEC104_HEAD) {
				p_client->recv_sync = 1;
				p_client->apdu_ptr++;
			}
		}
		else {
			msg_buf [p_client->apdu_ptr] = recv_buf [i];
			if (p_client->apdu_ptr == 1) {
				if (msg_buf [1] > IEC104_MSG_SIZE) {
					_DPRINTF ("msg len %d is invalid\n", msg_buf [1]);
					p_client->recv_sync = 0;
				}
				else {
					p_client->apdu_len = msg_buf [1] + 2;
				}
			}
			p_client->apdu_ptr++;
			if (p_client->apdu_ptr == p_client->apdu_len) {
#if __DBUG__	/* show receive 104 message */
				char prompt [24];
				sprintf (prompt, "sfd(%d) recv 104 msg", p_client->sid);
				show_msg (prompt, p_client->apdu_buf, p_client->apdu_len);
#endif
				/* apdu process */
				if ((rv = iec104_msg_proc (p_server, p_client)) >= 0) {
					/* clear sync flag */
					p_client->recv_sync = 0;
				}
				else {
					break;
				}
			}
		}
	}

	return rv;
}

static int iec104_msg_proc (iec104_server_t * p_server, client_connect_t * p_client)
{
	p_client->t3_cnt = 0;
	if (iec104_link_data_proc (p_server, p_client) < 0) {
		return -1;
	}
	if (iec104_app_data_proc (p_server, p_client) < 0) {
		return -1;
	}

	return 0;
}

static int iec104_link_data_proc (iec104_server_t * p_server, client_connect_t * p_client)
{
	int rv = 0;

	do {
		if ((p_client->apdu_buf [2] & 0x03) == 0x03) {
			p_client->recv_frame_type = U_FRAME;
			_DPRINTF ("recv uframe\n");
			if (p_client->apdu_buf [2] & 0x04) {
				_DPRINTF ("master startdt act\n");
				/* 重新初始化监视方向用户数据链路 */
				p_client->user_data_start = 1;
				/* 初始化操作... */
				
				/* 确认回复 */
				iec104_send_uframe (p_server, p_client, U_START_CONFIRM, FRAME_PRIO_HI);
			}
			else if (p_client->apdu_buf [2] & 0x10) {
				_DPRINTF ("master stopdt act\n");
				/* 停止监视方向用户数据链路 */
				p_client->user_data_start = 0;
				/* 清除操作... */
				
				/* 确认回复 */
				iec104_send_uframe (p_server, p_client, U_STOP_CONFIRM, FRAME_PRIO_HI);
			}
			else if (p_client->apdu_buf [2] & 0x40) {
				/* 测试链路激活 */
				_DPRINTF ("master test dt\n");
				p_client->t1_cnt = 0;
				iec104_send_uframe (p_server, p_client, U_TEST_CONFIRM, FRAME_PRIO_LO);
			}
			else if (p_client->apdu_buf [2] & 0x80) {
				/* 测试链路确认 */
				_DPRINTF ("master test dt confirm\n");
				p_client->t1_cnt = 0;
			}
		}
		else if ((p_client->apdu_buf [2] & 0x1) == 0x01) {
			p_client->recv_frame_type = S_FRAME;
			_DPRINTF ("recv sframe\n");
			if (get_iec104_frame_recv_no (p_client->apdu_buf, p_client->apdu_len, &p_client->recv_cnt.recv_no) < 0) {
				_DPRINTF ("invalid recv no %d\n", p_client->recv_cnt.recv_no);
				rv = -1;
				break;
			}
			if (p_client->recv_cnt.recv_no != -1) {
				if (iec104_confirm_frame (p_server, p_client) != 0) {
					_DPRINTF ("confirm frame error\n");
					rv = -1;
					break;
				}
			}
			else {
				_DPRINTF ("recv no error\n");
				rv = -1;
				break;
			}
			p_client->t1_cnt = 0;
			/* 收到S帧确认后，重新检察发送报文列表 */
			send_chk_send_lst_msg (p_server, p_client->sid);
		}
		else {
			p_client->recv_frame_type = I_FRAME;
			_DPRINTF ("recv iframe\n");
			if (get_iec104_frame_recv_no (p_client->apdu_buf, p_client->apdu_len, &p_client->recv_cnt.recv_no) < 0) {
				_DPRINTF ("invalid recv no %d\n", p_client->recv_cnt.recv_no);
				rv = -1;
				break;
			}
			if (get_iec104_frame_send_no (p_client->apdu_buf, p_client->apdu_len, &p_client->recv_cnt.send_no) < 0) {
				_DPRINTF ("invalid send no %d\n", p_client->recv_cnt.recv_no);
				rv = -1;
				break;
			}
			if (p_client->recv_cnt.recv_no != -1) {
				if (iec104_confirm_frame (p_server, p_client) != 0) {
					_DPRINTF ("confirm frame error\n");
					rv = -1;
					break;
				}
			}
			else {
				_DPRINTF ("recv no error\n");
				rv = -1;
				break;
			}
			if (p_client->recv_cnt.send_no != p_client->send_cnt.recv_no) {
				_DPRINTF ("frame sequence error\n");
				rv = -1;
				break;
			}
			p_client->send_cnt.recv_no += 2;
			if (p_client->send_cnt.recv_no >= 0xffff) {
				p_client->send_cnt.recv_no = 0;
			}
			p_client->t1_cnt = 0;
			p_client->t2_cnt = 0;
		}
	} while (0);

	return rv;
}

static int iec104_app_data_proc (iec104_server_t * p_server, client_connect_t * p_client)
{
	int rv = 0;

	if (p_client->recv_frame_type != I_FRAME) {
		return 0;
	}

	if (p_client->apdu_len < 13) {
		_DPRINTF ("apdu len error\n");
		return -1;
	}

	/* asdu offset 6 */
	p_client->apdu_parse_cnt = 6;

	/* asdu type */
	p_client->recv_asdu_type = p_client->apdu_buf [p_client->apdu_parse_cnt++];

	/* vsq == 1 in control direction */
	p_client->recv_vsq = p_client->apdu_buf [p_client->apdu_parse_cnt++];

	/* cot */
	p_client->recv_cot = p_client->apdu_buf [p_client->apdu_parse_cnt++];
	if (p_server->cot_bytes == 2) {
		switch (p_server->cot_match_rule) {
		case COT_MATCH_LO_BYTE:
			break;
		case COT_MATCH_HI_BYTE:
			p_client->recv_cot = p_client->apdu_buf [p_client->apdu_parse_cnt];
			break;
		case COT_MATCH_LO_HI:
			p_client->recv_cot = p_client->recv_cot + (p_client->apdu_buf [p_client->apdu_parse_cnt] << 8);
			break;
		case COT_MATCH_HI_LO:
			p_client->recv_cot = (p_client->recv_cot << 8) + p_client->apdu_buf [p_client->apdu_parse_cnt];
			break;
		default :
			break;
		}
		p_client->apdu_parse_cnt++;
	}

	/* application service data unit address, device address, in short */
	p_client->recv_app_addr = p_client->apdu_buf [p_client->apdu_parse_cnt++];
	if (p_server->app_addr_bytes == 2) {
		p_client->recv_app_addr += p_client->apdu_buf [p_client->apdu_parse_cnt++] << 8;
	}
	if (p_server->app_addr_bytes == 1 &&
		p_client->recv_app_addr == 0xff) {
		p_client->recv_app_addr = p_server->app_addr;
	}
	else if (p_server->app_addr_bytes == 2 &&
		p_client->recv_app_addr == 0xffff) {
		p_client->recv_app_addr = p_server->app_addr;
	}

	/* information object address */
	p_client->recv_inf_addr = p_client->apdu_buf [p_client->apdu_parse_cnt] + 
		(p_client->apdu_buf [p_client->apdu_parse_cnt + 1] << 8);
	p_client->apdu_parse_cnt += 2;
	if (p_server->inf_addr_bytes == 3) {
		p_client->recv_inf_addr += p_client->apdu_buf [p_client->apdu_parse_cnt++] << 16;
	}

	/* command */
	p_client->recv_cmd = p_client->apdu_buf [p_client->apdu_parse_cnt];

	/* parse cnt stop counting after information object address */

	switch (p_client->recv_asdu_type) {
	case C_SC_NA_1:
		_DPRINTF ("master C_SC_NA_1\n");
		iec104_yk (p_server, p_client);
		break;
	case C_DC_NA_1:
		_DPRINTF ("master C_DC_NA_1\n");
		iec104_yk (p_server, p_client);
		break;
	case C_IC_NA_1:
		_DPRINTF ("master C_IC_NA_1\n");
		if (p_client->recv_cot == COT_ACT && 
			p_client->recv_cmd == 20) { /* 总召唤 */
			iec104_gi (p_server, p_client);
		}
		break;
	case C_CI_NA_1:
		_DPRINTF ("master C_CI_NA_1\n");
		if (p_client->recv_cot == COT_ACT) {
			iec104_ci (p_server, p_client);
		}
		break;
	case C_RD_NA_1:
		break;
	case C_CS_NA_1:
		_DPRINTF ("master C_CS_NA_1\n");
		if (p_client->recv_cot == COT_ACT) {
			iec104_cs (p_server, p_client);
		}
		break;
	case C_TS_NA_1:
		break;
	case C_RP_NA_1:
		break;
		/*
		case C_SR_NA_1:
		_DPRINTF ("master C_SR_NA_1\n");
		if (p_client->recv_cot == COT_ACT) {
		iec104_sr (p_server, p_client);
		}
		break;
		case C_RR_NA_1:
		_DPRINTF ("master C_RR_NA_1\n");
		if (p_client->recv_cot == COT_ACT) {
		iec104_rr (p_server, p_client);
		}
		break;
		*/
	
	case C_RS_NA_1:
		break;
	case C_WS_NA_1:
		break;
	case F_FR_NA_1:
		_DPRINTF ("master F_FR_NA_1\n");
		if (p_client->recv_inf_addr == 0x20000) {	/* 文件传输 */
			if (p_client->recv_cot == COT_REQ &&
				p_client->recv_cmd == FT_GET_DIR) {
				iec104_ft_get_dir (p_server, p_client);
			}
			else if (p_client->recv_cot == COT_ACT &&
				p_client->recv_cmd == FT_RD_FILE_ACT) {
				iec104_ft_file_act (p_server, p_client);
			}
			else if (p_client->recv_cot == COT_REQ &&
				p_client->recv_cmd == FT_RD_FILE_DATA_RESP) {
				/* 客户端文件传输确认，无需处理 */
			}
		}
		break;
	case F_SR_NA_1:
		break;
	default:
		break;
	}

	return rv;
}

static int iec104_close_connection (iec104_server_t * p_server, client_connect_t * p_client)
{
	iec104_msg_t * sbuf, * pbuf;

	if ((p_client->next == NULL) && p_server->cache_event) {
		/*
		 * when only one clients exist,
		 * save unconfirmed and un-send messages
		 */
		if (!p_server->clear_unconfirmed_msg) {
			if (p_client->unconfirmed_buf_lst_head != NULL) {
				p_server->cache_buf_lst_head = p_client->unconfirmed_buf_lst_head;
				p_client->unconfirmed_buf_lst_tail->next = p_client->send_buf_lst_head;
				p_server->cache_buf_lst_tail = p_client->send_buf_lst_tail;
			}
		}
		else {
			if (p_client->send_buf_lst_head != NULL) {
				p_server->cache_buf_lst_head = p_client->send_buf_lst_head;
				p_server->cache_buf_lst_tail = p_client->send_buf_lst_tail;
			}
		}

		if (p_server->clear_unconfirmed_msg) {
			while (p_client->unconfirmed_buf_lst_head) {
				sbuf = p_client->unconfirmed_buf_lst_head;
				p_client->unconfirmed_buf_lst_head = p_client->unconfirmed_buf_lst_head->next;
				free_iec104_msg_buf (sbuf);
			}
		}

		pbuf = p_server->cache_buf_lst_head;
		p_server->cache_cnt = 0;
		while (pbuf != NULL) {
			pbuf = pbuf->next;
			p_server->cache_cnt++;
		}
	}
	else {
		/* clear messages */
		while (p_client->unconfirmed_buf_lst_head) {
			sbuf = p_client->unconfirmed_buf_lst_head;
			p_client->unconfirmed_buf_lst_head = p_client->unconfirmed_buf_lst_head->next;
			free_iec104_msg_buf (sbuf);
		}
		while (p_client->send_buf_lst_head) {
			sbuf = p_client->send_buf_lst_head;
			p_client->send_buf_lst_head = p_client->send_buf_lst_head->next;
			free_iec104_msg_buf (sbuf);
		}
	}

	close (p_client->sid);

	return 0;
}

static int iec104_check_com_inf (iec104_server_t * p_server, client_connect_t * p_client, unsigned short * cot)
{
	* cot = p_client->recv_cot + 1;

	/* 策略优先级依次递减 */
	if (p_client->recv_vsq != 0x01) {
		* cot |= COT_P_N;
	}
	else if (p_client->recv_cot != COT_ACT &&
		p_client->recv_cot != COT_DEACT) {
		* cot = COT_UNKNOWN_COT | COT_P_N;
	}
	else if (p_server->app_addr != p_client->recv_app_addr) {
		* cot = COT_UNKNOWN_APP_ADDR | COT_P_N;
	}

	return 0;
}

static int iec104_send_uframe (iec104_server_t * p_server, client_connect_t * p_client, int cmd, int prio)
{
	unsigned char send_buf [6];

	send_buf [0] = IEC104_HEAD;
	send_buf [1] = 0x04;
	switch (cmd) {
	case U_START_CONFIRM:
		send_buf [2] = 0x0b;
		break;
	case U_STOP_CONFIRM:
		send_buf [2] = 0x23;
		break;
	case U_TEST_ACTIVE:
		send_buf [2] = 0x43;
		break;
	case U_TEST_CONFIRM:
	default :
		send_buf [2] = 0x83;
		break;
	}
	send_buf [3] = 0x00;
	send_buf [4] = 0x00;
	send_buf [5] = 0x00;

	iec104_save_send_frame (p_server, p_client, send_buf, sizeof (send_buf), U_FRAME, prio);

	return 0;
}

static int iec104_send_sframe (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [6];

	send_buf [0] = IEC104_HEAD;
	send_buf [1] = 0x04;
	send_buf [2] = 0x01;
	send_buf [3] = 0x00;
	send_buf [4] = 0x00;
	send_buf [5] = 0x00;

	iec104_save_send_frame (p_server, p_client, send_buf, sizeof (send_buf), S_FRAME, FRAME_PRIO_LO);

	return 0;
}

static void iec104_msg_init_header (unsigned char send_buf [], int * send_cnt)
{
	send_buf [0] = IEC104_HEAD;
	send_buf [1] = 0x00;
	send_buf [2] = 0x00;
	send_buf [3] = 0x00;
	send_buf [4] = 0x00;
	send_buf [5] = 0x00;
	* send_cnt = 6;
}

static int iec104_gi (iec104_server_t * p_server, client_connect_t * p_client)
{
	_DPRINTF ("server gi confirm\n");
	iec104_gi_confirm (p_server, p_client);
	iec104_gi_data (p_server, p_client);
	iec104_gi_end_data (p_server, p_client);

	return 0;
}

static int iec104_gi_confirm (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [100];
	unsigned short cot;
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_IC_NA_1;
	send_buf [send_cnt++] = p_client->recv_vsq;
	iec104_check_com_inf (p_server, p_client, &cot);
	if (cot == p_client->recv_cot + 1) {
		if (p_client->recv_inf_addr != 0x00) {
			cot = COT_UNKNOWN_INF_ADDR | COT_P_N;
		}
	}
	send_buf [send_cnt++] = cot & 0xff;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = cot >> 8;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	send_buf [send_cnt++] = 20;
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}

static int iec104_gi_data (iec104_server_t * p_server, client_connect_t * p_client)
{
	iec104_gi_yx (p_server, p_client);
	iec104_gi_yc (p_server, p_client);

	return 0;
}

static int iec104_gi_yx (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int yx_total_num, frame_no, i, j;
	unsigned int yx_value [GI_YX_1FRAME_NUM], yx_begin_no, yx_num;
	unsigned int inf_addr;
	unsigned char send_buf [250], yx_val;
	int send_cnt;

	yx_get_total_num_proto (p_server->app_id, &yx_total_num);
	frame_no = (yx_total_num / GI_YX_1FRAME_NUM) + 1;
	yx_begin_no = 0;
	yx_num = yx_total_num < GI_YX_1FRAME_NUM ? yx_total_num : GI_YX_1FRAME_NUM;
	for (i = 0; i < frame_no; i += 1) {
		yx_get_value_proto (p_server->app_id, yx_begin_no, yx_num, yx_value);

		iec104_msg_init_header (send_buf, &send_cnt);
		send_buf [send_cnt++] = p_server->gi_yx_asdu;
		send_buf [send_cnt++] = yx_num | VSQ_SQ;
		send_buf [send_cnt++] = COT_INTROGEN;
		if (p_server->cot_bytes == 2) {
			send_buf [send_cnt++] = 0x00;
		}
		send_buf [send_cnt++] = p_server->app_addr;
		if (p_server->app_addr_bytes == 2) {
			send_buf [send_cnt++] = p_server->app_addr >> 8;
		}
		inf_addr = yx_begin_no + p_server->inf_obj_addr_yx_base;
		send_buf [send_cnt++] = inf_addr & 0xff;
		send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
		}
		if (p_server->gi_yx_asdu == M_SP_NA_1) {
			for (j = 0; j < yx_num; j++) {
				switch (yx_value [j]) {
				case DP_OPEN:
					yx_val = SP_OPEN;
					break;
				case DP_CLOSE:
					yx_val = SP_CLOSE;
					break;
				case DP_OPEN | DIQ_IV:
					yx_val = SP_OPEN | SIQ_IV;
					break;
				case DP_CLOSE | DIQ_IV:
					yx_val = SP_CLOSE | SIQ_IV;
					break;
				default:
					yx_val = SP_OPEN | SIQ_IV;
					break;
				}
				send_buf [send_cnt++] = yx_val;
			}
		}
		else { /* M_DP_NA_1 */
			for (j = 0; j < yx_num; j++) {
				switch (yx_value [j]) {
				case DP_OPEN:
					yx_val = DP_OPEN;
					break;
				case DP_CLOSE:
					yx_val = DP_CLOSE;
					break;
				case DP_OPEN | DIQ_IV:
					yx_val = DP_OPEN | DIQ_IV;
					break;
				case DP_CLOSE | DIQ_IV:
					yx_val = DP_CLOSE | DIQ_IV;
					break;
				default:
					yx_val = DP_OPEN | DIQ_IV;
					break;
				}
				send_buf [send_cnt++] = yx_val;
			}
		}
		send_buf [1] = send_cnt - 2;
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
		
		yx_begin_no += yx_num;
		yx_num = (yx_total_num - yx_num) < GI_YX_1FRAME_NUM ? (yx_total_num - yx_num) : GI_YX_1FRAME_NUM;
	}

	return 0;
}

static int iec104_gi_yc (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int yc_total_num, frame_no, i, j;
	unsigned int yc_begin_no, yc_num;
	unsigned int inf_addr;
	unsigned char send_buf [250];
	int send_cnt;
	yc_data_type yc_value [GI_YC_1FRAME_NUM];
	unsigned char yc_qds [GI_YC_1FRAME_NUM], data_type [GI_YC_1FRAME_NUM];
	float f_temp;
	unsigned char * p_f_temp = (unsigned char *)&f_temp;
	short s_temp;

	yc_get_total_num_proto (p_server->app_id, &yc_total_num);
	frame_no = (yc_total_num / GI_YC_1FRAME_NUM) + 1;
	yc_begin_no = 0;
	yc_num = yc_total_num < GI_YC_1FRAME_NUM ? yc_total_num : GI_YC_1FRAME_NUM;
	for (i = 0; i < frame_no; i += 1) {
		yc_get_value_proto (p_server->app_id, yc_begin_no, yc_num, yc_value, yc_qds, data_type);

		iec104_msg_init_header (send_buf, &send_cnt);
		send_buf [send_cnt++] = p_server->gi_yc_asdu;
		send_buf [send_cnt++] = yc_num | VSQ_SQ;
		send_buf [send_cnt++] = COT_INTROGEN;
		if (p_server->cot_bytes == 2) {
			send_buf [send_cnt++] = 0x00;
		}
		send_buf [send_cnt++] = p_server->app_addr;
		if (p_server->app_addr_bytes == 2) {
			send_buf [send_cnt++] = p_server->app_addr >> 8;
		}
		inf_addr = yc_begin_no + p_server->inf_obj_addr_yc_base;
		send_buf [send_cnt++] = inf_addr & 0xff;
		send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
		}

		for (j = 0; j < yc_num; j++) {
			if (p_server->gi_yc_asdu == M_ME_NA_1) {
				if (data_type [j] == YC_INT) {
					s_temp = yc_value [j].i_val;
				}
				else {
					s_temp = yc_value [j].f_val;
				}
				if (p_server->yc_int_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(s_temp);
					send_buf [send_cnt++] = LSB(s_temp);
				}
				else {
					send_buf [send_cnt++] = LSB(s_temp);
					send_buf [send_cnt++] = MSB(s_temp);
				}
				send_buf [send_cnt++] = yc_qds [j];
			}
			else if (p_server->gi_yc_asdu == M_ME_NB_1) {
				if (data_type [j] == YC_INT) {
					s_temp = yc_value [j].i_val;
				}
				else {
					s_temp = yc_value [j].f_val;
				}
				if (p_server->yc_int_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(s_temp);
					send_buf [send_cnt++] = LSB(s_temp);
				}
				else {
					send_buf [send_cnt++] = LSB(s_temp);
					send_buf [send_cnt++] = MSB(s_temp);
				}
				send_buf [send_cnt++] = yc_qds [j];
			} 
			else if (p_server->gi_yc_asdu == M_ME_NC_1) {
				if (data_type [j] == YC_INT) {
					f_temp = yc_value [j].i_val;
				}
				else {
					f_temp = yc_value [j].f_val;
				}
				if (p_server->yc_float_byte_order_hi_lo) {
					send_buf [send_cnt++] = p_f_temp [3];
					send_buf [send_cnt++] = p_f_temp [2];
					send_buf [send_cnt++] = p_f_temp [1];
					send_buf [send_cnt++] = p_f_temp [0];
				}
				else {
					send_buf [send_cnt++] = p_f_temp [0];
					send_buf [send_cnt++] = p_f_temp [1];
					send_buf [send_cnt++] = p_f_temp [2];
					send_buf [send_cnt++] = p_f_temp [3];
				}
				send_buf [send_cnt++] = yc_qds [j];
			}
			else { /* M_ME_ND_1 */
				if (data_type [j] == YC_INT) {
					s_temp = yc_value [j].i_val;
				}
				else {
					s_temp = yc_value [j].f_val;
				}
				if (p_server->yc_int_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(s_temp);
					send_buf [send_cnt++] = LSB(s_temp);
				}
				else {
					send_buf [send_cnt++] = LSB(s_temp);
					send_buf [send_cnt++] = MSB(s_temp);
				}
			}
		}
		
		send_buf [1] = send_cnt - 2;
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
		
		yc_begin_no += yc_num;
		yc_num = (yc_total_num - yc_num) < GI_YC_1FRAME_NUM ? (yc_total_num - yc_num) : GI_YC_1FRAME_NUM;
	}

	return 0;
}

static int iec104_gi_end_data (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [100];
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_IC_NA_1;
	send_buf [send_cnt++] = 0x01;
	send_buf [send_cnt++] = COT_ACTTERM;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	send_buf [send_cnt++] = 20;
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}

static int iec104_cs (iec104_server_t * p_server, client_connect_t * p_client)
{
	time_type tt;
	struct tm * ptm;
	time_t ttt;

	cp56time2a_to_time (&p_client->apdu_buf [p_client->apdu_parse_cnt], &tt);
	ttt = tt.seconds;
	ptm = localtime (&ttt);
	/* set system time */
	set_seconds_1900 (&tt);
//	rtc_write (ptm);
	
	/* show time */
#if 1
	_DPRINTF ("sync time: %d-%d-%d %d, %d:%d:%d %d\n",
		ptm->tm_year + 1900,
		ptm->tm_mon + 1,
		ptm->tm_mday,
		ptm->tm_wday,
		ptm->tm_hour,
		ptm->tm_min,
		ptm->tm_sec,
		tt.msec);
#endif

	_DPRINTF ("server cs confirm\n");
	iec104_cs_end_data (p_server, p_client);

	return 0;
}

static int iec104_cs_end_data (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [100];
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_CS_NA_1;
	send_buf [send_cnt++] = 0x01;
	send_buf [send_cnt++] = COT_ACTCON;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	/* timestamp copied from recv buf to send buf */
	memcpy (&send_buf [send_cnt], &p_client->apdu_buf [p_client->apdu_parse_cnt], 7);
	send_cnt += 7;
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}

static int iec104_ci (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char frz;
#if 0
	unsigned char rqt;
	rqt = p_client->recv_cmd & 0x3f;
# endif
	frz = p_client->recv_cmd >> 6;
	_DPRINTF ("server ci confirm\n");
	iec104_ci_confirm (p_server, p_client);
	if (frz == 1) {
		_DPRINTF ("ci frz and not reset\n");
	}
	else if (frz == 2) {
		_DPRINTF ("ci frz and reset\n");
	}
	else if (frz == 3) {
		_DPRINTF ("ci reset\n");
	}
# if 0
	if (rqt >= 1 && rqt <= 5) {
		iec104_ci_data (p_server, p_client);
	}
#endif
	iec104_ci_end_data (p_server, p_client);
	
	return 0;
}

static int iec104_ci_confirm (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [100];
	unsigned short cot;
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_CI_NA_1;
	send_buf [send_cnt++] = p_client->recv_vsq;
	iec104_check_com_inf (p_server, p_client, &cot);
	if (cot == p_client->recv_cot + 1) {
		if (p_client->recv_inf_addr != 0x00) {
			cot = COT_UNKNOWN_INF_ADDR | COT_P_N;
		}
	}
	send_buf [send_cnt++] = cot & 0xff;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = cot >> 8;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	send_buf [send_cnt++] = p_client->recv_cmd;		/* the same as the recv cmd */
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}
#if 0
static int iec104_ci_data (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int ym_total_num, frame_no, i, j;
	unsigned int ym_begin_no, ym_num;
	unsigned int inf_addr;
	unsigned char send_buf [250];
	int send_cnt;
	bcr_t ym_value [GI_YM_1FRAME_NUM];

	ym_get_total_num_proto (p_server->app_id, &ym_total_num);
	frame_no = (ym_total_num / GI_YM_1FRAME_NUM) + 1;
	ym_begin_no = 0;
	ym_num = ym_total_num < GI_YM_1FRAME_NUM ? ym_total_num : GI_YM_1FRAME_NUM;
	for (i = 0; i < frame_no; i += 1) {
		ym_get_value_proto (p_server->app_id, ym_begin_no, ym_num, ym_value);

		iec104_msg_init_header (send_buf, &send_cnt);
		send_buf [send_cnt++] = p_server->ci_ym_asdu;
		send_buf [send_cnt++] = ym_num | VSQ_SQ;
		send_buf [send_cnt++] = COT_REQCOGEN;
		if (p_server->cot_bytes == 2) {
			send_buf [send_cnt++] = 0x00;
		}
		send_buf [send_cnt++] = p_server->app_addr;
		if (p_server->app_addr_bytes == 2) {
			send_buf [send_cnt++] = p_server->app_addr >> 8;
		}
		inf_addr = ym_begin_no + p_server->inf_obj_addr_ym_base;
		send_buf [send_cnt++] = inf_addr & 0xff;
		send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
		}

		for (j = 0; j < ym_num; j++) {
			if (p_server->ci_ym_asdu == M_IT_NA_1) {
				if (p_server->ym_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(MSW(ym_value [j].counter_reading));
					send_buf [send_cnt++] = LSB(MSW(ym_value [j].counter_reading));
					send_buf [send_cnt++] = MSB(LSW(ym_value [j].counter_reading));
					send_buf [send_cnt++] = LSB(LSW(ym_value [j].counter_reading));
				}
				else {
					send_buf [send_cnt++] = LSB(LSW(ym_value [j].counter_reading));
					send_buf [send_cnt++] = MSB(LSW(ym_value [j].counter_reading));
					send_buf [send_cnt++] = LSB(MSW(ym_value [j].counter_reading));
					send_buf [send_cnt++] = MSB(MSW(ym_value [j].counter_reading));
				}
				send_buf [send_cnt++] = ym_value [j].seq_not;
			}
			else /* p_server->gi_yc_asdu == M_IT_TA_1 || p_server->gi_yc_asdu == M_IT_TB_1 */ {
				_DPRINTF ("ci type not support\n");
				return -1;
			}
		}
		
		send_buf [1] = send_cnt - 2;
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
		
		ym_begin_no += ym_num;
		ym_num = (ym_total_num - ym_num) < GI_YM_1FRAME_NUM ? (ym_total_num - ym_num) : GI_YM_1FRAME_NUM;
	}

	return 0;
}
#endif
static int iec104_ci_end_data (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [100];
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_CI_NA_1;
	send_buf [send_cnt++] = 0x01;
	send_buf [send_cnt++] = COT_ACTTERM;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	send_buf [send_cnt++] = p_client->recv_cmd;		/* the same as the recv cmd */
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}
#if 0
static int iec104_sr (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned short setting_group_no;
	int neg_ack = 0;

	setting_group_no = p_client->recv_cmd;	/* 低字节 */
	setting_group_no |= p_client->apdu_buf [++p_client->apdu_parse_cnt] << 8;	/* 高字节 */

	if (switch_setting_group (setting_group_no, p_server->app_id) != 0) {
		neg_ack = 1;
	}

	iec104_sr_confirm (p_server, p_client, neg_ack);
	
	return 0;
}

static int iec104_sr_confirm (iec104_server_t * p_server, client_connect_t * p_client, int neg_ack)
{
	unsigned char send_buf [100];
	unsigned short cot;
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_SR_NA_1;
	send_buf [send_cnt++] = 0x01;
	iec104_check_com_inf (p_server, p_client, &cot);
	if (cot == p_client->recv_cot + 1) {
		if (p_client->recv_inf_addr != 0x00) {
			cot = COT_UNKNOWN_INF_ADDR | COT_P_N;
		}
	}
	if (neg_ack) {
		cot |= COT_P_N;
	}
	send_buf [send_cnt++] = cot & 0xff;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = cot >> 8;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	send_buf [send_cnt++] = p_client->recv_cmd;								/* 低字节*/
	send_buf [send_cnt++] = p_client->apdu_buf [p_client->apdu_parse_cnt];	/* 高字节*/
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}

static int iec104_rr (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int setting_group_no;

//	get_setting_group (&setting_group_no);

	iec104_rr_confirm (p_server, p_client, setting_group_no);
	
	return 0;
}

static int iec104_rr_confirm (iec104_server_t * p_server, client_connect_t * p_client, unsigned int setting_group_no)
{
	unsigned char send_buf [100];
	unsigned short cot;
	int send_cnt;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = C_RR_NA_1;
	send_buf [send_cnt++] = 0x01;
	iec104_check_com_inf (p_server, p_client, &cot);
	if (cot == p_client->recv_cot + 1) {
		if (p_client->recv_inf_addr != 0x00) {
			cot = COT_UNKNOWN_INF_ADDR | COT_P_N;
		}
	}
	send_buf [send_cnt++] = cot & 0xff;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = cot >> 8;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = 0;
	}
	send_buf [send_cnt++] = LSB(setting_group_no);
	send_buf [send_cnt++] = MSB(setting_group_no);
	send_buf [send_cnt++] = LSB(1);
	send_buf [send_cnt++] = MSB(1);
	send_buf[send_cnt++] = LSB(1);
	send_buf[send_cnt++] = MSB(1);
	/*
	send_buf [send_cnt++] = LSB(SETTING_GROUP_NO);
	send_buf [send_cnt++] = MSB(SETTING_GROUP_NO);
	*/
	
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	send_buf [send_cnt++] = 0;
	send_buf [1] = send_cnt - 2;

	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}
#endif
#define YK_REPLY_ACK				0x01
#define YK_REPLY_NACK				0x02
#define YK_REPLY_NACK_COT_ERR		0x03
#define YK_REPLY_NACK_APP_ADDR_ERR	0x04
#define YK_REPLY_NACK_INF_ADDR_ERR	0x05
#define YK_REPLY_END				0x10

static int iec104_yk (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int yk_addr;
	unsigned char sel, cmd;
	int rv = -1;
	unsigned int b_remote = 0;

	yk_addr = p_client->recv_inf_addr - INF_OBJ_ADDR_YK_BASE;
	cmd = p_client->recv_cmd & YK_CO_MASK;
	sel = (p_client->recv_cmd & YK_SEL_MASK) ? 1 : 0;

	if (p_client->recv_vsq != 0x01) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}
	else if (p_client->recv_cot != COT_ACT && p_client->recv_cot != COT_DEACT) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK_COT_ERR);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}
	else if (p_client->recv_app_addr != p_server->app_addr) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK_APP_ADDR_ERR);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}
	else if (/*yk_addr < 0 || */yk_addr >= YK_VALID_NUM) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK_INF_ADDR_ERR);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}
	else if (p_client->recv_asdu_type == C_SC_NA_1 && ((cmd != SP_OPEN) && (cmd != SP_CLOSE))) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}
	else if (p_client->recv_asdu_type == C_DC_NA_1 && ((cmd != DP_OPEN) && (cmd != DP_CLOSE))) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}

	if (p_client->recv_asdu_type == C_SC_NA_1) {
		if (cmd == SP_OPEN) {
			cmd = DP_OPEN;
		}
		else {
			cmd = DP_CLOSE;
		}
	}
	else if (p_client->recv_asdu_type == C_DC_NA_1) {
		if (cmd == DP_OPEN) {
			cmd = DP_OPEN;
		}
		else {
			cmd = DP_CLOSE;
		}
	}
	else {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
		return -1;
	}

	if (sel && (p_client->recv_cot == COT_ACT)) {
		//if (yx_get_value (p_client->sid, get_yx_remote_local_index (), 1, &b_remote)(b_remote = 1) < 0) {
		if ((b_remote = 1) < 0) {
			rv = -1;
		}
		else {
			if (b_remote == DP_CLOSE) {
				/* 远方 */
				rv = yk_select (yk_addr, cmd, p_client->sid);
			}
			else {
				/* 就地 */
				rv = -1;
			}
		}
	}
	else if (sel && (p_client->recv_cot == COT_DEACT)) {
		rv = yk_unselect (yk_addr, p_client->sid);
	}
	else if (!sel && (p_client->recv_cot == COT_ACT)) {
		//if (yx_get_value (p_client->sid, get_yx_remote_local_index (), 1, &b_remote)(b_remote = 1) < 0) {
		if ((b_remote = 1) < 0) {
			rv = -1;
		}
		else {
			if (b_remote == DP_CLOSE) {
				/* 远方 */
				rv = yk_operate (yk_addr, cmd, p_client->sid);
			}
			else {
				/* 就地 */
				rv = -1;
			}
		}
	}
	else if (!sel && (p_client->recv_cot == COT_DEACT)) {
		rv = yk_unselect (yk_addr, p_client->sid);
	}
	else {
		rv = -1;
	}

	if (rv < 0) {
		iec104_yk_reply (p_server, p_client, YK_REPLY_NACK);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
	}
	else {
		iec104_yk_reply (p_server, p_client, YK_REPLY_ACK);
		iec104_yk_reply (p_server, p_client, YK_REPLY_END);
	}

	return rv;
}

static int iec104_yk_reply (iec104_server_t * p_server, client_connect_t * p_client, int yk_reply_flag)
{
	unsigned char send_buf [250];
	int send_cnt;
	unsigned int inf_addr;

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = p_client->recv_asdu_type;
	send_buf [send_cnt++] = 0x01;
	switch (yk_reply_flag) {
	case YK_REPLY_ACK:
		send_buf [send_cnt++] = p_client->recv_cot + 1;
		break;
	case YK_REPLY_NACK:
		send_buf [send_cnt++] = (p_client->recv_cot + 1) | COT_P_N;
		break;
	case YK_REPLY_NACK_COT_ERR:
		if (p_server->cot_err_rule == COT_ERR_IEC) {
			send_buf [send_cnt++] = COT_UNKNOWN_COT | COT_P_N;
		}
		else {
			send_buf [send_cnt++] = (p_client->recv_cot + 1) | COT_P_N;
		}
		break;
	case YK_REPLY_NACK_APP_ADDR_ERR:
		if (p_server->cot_err_rule == COT_ERR_IEC) {
			send_buf [send_cnt++] = COT_UNKNOWN_APP_ADDR | COT_P_N;
		}
		else {
			send_buf [send_cnt++] = (p_client->recv_cot + 1) | COT_P_N;
		}
		break;
	case YK_REPLY_NACK_INF_ADDR_ERR:
		if (p_server->cot_err_rule == COT_ERR_IEC) {
			send_buf [send_cnt++] = COT_UNKNOWN_INF_ADDR | COT_P_N;
		}
		else {
			send_buf [send_cnt++] = (p_client->recv_cot + 1) | COT_P_N;
		}
		break;
	case YK_REPLY_END:
		if (p_server->yk_end_frame_rule == YK_END_FRAME_NO) {
			return 0;
		}
		send_buf [send_cnt++] = COT_ACTTERM;
		break;
	default:
		send_buf [send_cnt++] = (p_client->recv_cot + 1) | COT_P_N;
		break;
	}
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	inf_addr = p_client->recv_inf_addr;
	send_buf [send_cnt++] = inf_addr & 0xff;
	send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
	}
	send_buf [send_cnt++] = p_client->recv_cmd;

	send_buf [1] = send_cnt - 2;
	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_HI);

	return 0;
}

static int iec104_ft_get_dir (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int dir_id;
	unsigned char dir_name_len;
	unsigned char call_flag;
	unsigned char * query_start_time;
	unsigned char * query_end_time;
	time_type tt_start, tt_end, tt_tmp;

	DIR * dp;                      // 定义子目录流指针
	struct dirent *entry;         // 定义dirent结构指针保存后续目录
	struct stat statbuf;          // 定义statbuf结构保存文件属性
	char cwd [200];

	unsigned char send_buf [250];
	int send_cnt, continue_pos, file_num_pos;
	unsigned int inf_addr, file_num = 0;
	int file_name_len;

	p_client->apdu_parse_cnt++;
	/* 目录ID */
	dir_id = p_client->apdu_buf [p_client->apdu_parse_cnt] | 
		(p_client->apdu_buf [p_client->apdu_parse_cnt + 1] << 8) | 
		(p_client->apdu_buf [p_client->apdu_parse_cnt + 2] << 16) | 
		(p_client->apdu_buf [p_client->apdu_parse_cnt + 3] << 24);
	p_client->apdu_parse_cnt += 4;
	/* 目录名长度 */
	dir_name_len = p_client->apdu_buf [p_client->apdu_parse_cnt++];
	/* 目录名 */
	if (dir_name_len > 0 && dir_name_len < sizeof (p_client->dir_name)) {
		memcpy (p_client->dir_name, p_client->apdu_buf + p_client->apdu_parse_cnt, dir_name_len);
		p_client->dir_name [dir_name_len] = 0;
		p_client->apdu_parse_cnt += dir_name_len;
	}
	else {
		strcpy (p_client->dir_name, "comtrade");
	}
	/* 召唤标识 */
	call_flag = p_client->apdu_buf [p_client->apdu_parse_cnt++];
	/* 查询起始时间 */
	if ((p_client->apdu_parse_cnt + 7) <= p_client->apdu_len) {
		query_start_time = p_client->apdu_buf + p_client->apdu_parse_cnt;
		p_client->apdu_parse_cnt += 7;
		cp56time2a_to_time (query_start_time, &tt_start);
	}
	/* 查询终止时间 */
	if ((p_client->apdu_parse_cnt + 7) <= p_client->apdu_len) {
		query_end_time = p_client->apdu_buf + p_client->apdu_parse_cnt;
		cp56time2a_to_time (query_end_time, &tt_end);
	}

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = p_client->recv_asdu_type;
	send_buf [send_cnt++] = 0x01;
	send_buf [send_cnt++] = p_client->recv_cot;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	inf_addr = p_client->recv_inf_addr;
	send_buf [send_cnt++] = inf_addr & 0xff;
	send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
	}
	/* 读目录确认 */
	send_buf [send_cnt++] = FT_GET_DIR_CON;
	
	dp = opendir (p_client->dir_name);
	if (dp == NULL) {
		/* 结果描述 */
		send_buf [send_cnt++] = 1;
		/* 目录ID */
		send_buf [send_cnt++] = LSB(LSW(dir_id));
		send_buf [send_cnt++] = MSB(LSW(dir_id));
		send_buf [send_cnt++] = LSB(MSW(dir_id));
		send_buf [send_cnt++] = MSB(MSW(dir_id));
		/* 后续标识 */
		send_buf [send_cnt++] = 0;
		/* 文件数量 */
		send_buf [send_cnt++] = 0;

		send_buf [1] = send_cnt - 2;
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
	}
	else {
		/* 结果描述 */
		send_buf [send_cnt++] = 0;
		/* 目录ID */
		send_buf [send_cnt++] = LSB(LSW(dir_id));
		send_buf [send_cnt++] = MSB(LSW(dir_id));
		send_buf [send_cnt++] = LSB(MSW(dir_id));
		send_buf [send_cnt++] = MSB(MSW(dir_id));
		/* 后续标识 */
		continue_pos = send_cnt;
		send_buf [send_cnt++] = 0;
		/* 文件数量 */
		file_num_pos = send_cnt;
		send_buf [send_cnt++] = 0;

		file_num = 0;

		getcwd (cwd, sizeof (cwd));
		chdir (p_client->dir_name);
		
		for (;;) {
			entry = readdir (dp);
			if ((entry == NULL) || (entry->d_name [0] == 0)) break;
            if ((strcmp (".", entry->d_name) == 0) || 
				(strcmp ("..", entry->d_name) == 0)) {
				continue;
			}
			lstat (entry->d_name, &statbuf); // 获取下一级成员属性
            if (!S_ISDIR(statbuf.st_mode)) {
				tt_tmp.seconds = statbuf.st_mtim.tv_sec;
				tt_tmp.msec = statbuf.st_mtim.tv_nsec / 1000000;
				if (call_flag == 1) {
					if (tt_tmp.seconds < tt_start.seconds || tt_tmp.seconds > tt_end.seconds) {
						continue;
					}
				}

				if (file_num >= MAX_FILE_NAME_NUM_PER_FRAME) {
					send_buf [continue_pos] = 1;
					send_buf [file_num_pos] = file_num;
					send_buf [1] = send_cnt - 2;
					iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
					file_num = 0;
				}
				
				/* 文件名长度 */
				file_name_len = strlen (entry->d_name);
				send_buf [send_cnt++] = file_name_len;
				/* 文件名称 */
				memcpy (send_buf + send_cnt, entry->d_name, file_name_len);
				send_cnt += file_name_len;
				/* 文件属性 */
				send_buf [send_cnt++] = 0;
				/* 文件大小 */
				file_name_len = statbuf.st_size;
				send_buf [send_cnt++] = LSB(LSW(file_name_len));
				send_buf [send_cnt++] = MSB(LSW(file_name_len));
				send_buf [send_cnt++] = LSB(MSW(file_name_len));
				send_buf [send_cnt++] = MSB(MSW(file_name_len));
				/* 文件时间 */
				time_to_cp56time2a (&tt_tmp, send_buf + send_cnt);
				send_cnt += 7;

				file_num++;
			}
		}
		chdir (cwd);
		closedir (dp);
	}

	if (file_num > 0) {
		send_buf [continue_pos] = 0;
		send_buf [file_num_pos] = file_num;
		send_buf [1] = send_cnt - 2;
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
	}

	return 0;
}

static int iec104_ft_file_act (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [250];
	int send_cnt;
	unsigned int inf_addr;
	unsigned char file_name_len;
	FILE * fp;
	struct stat statbuf;          // 定义statbuf结构保存文件属性
	char path [100];

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = p_client->recv_asdu_type;
	send_buf [send_cnt++] = 0x01;
	send_buf [send_cnt++] = p_client->recv_cot + 1;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	inf_addr = p_client->recv_inf_addr;
	send_buf [send_cnt++] = inf_addr & 0xff;
	send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
	}
	/* 读文件确认 */
	send_buf [send_cnt++] = FT_RD_FILE_ACTCON;

	p_client->apdu_parse_cnt++;

	file_name_len = p_client->apdu_buf [p_client->apdu_parse_cnt++];

	if (file_name_len > 0 && file_name_len < sizeof (p_client->file_name)) {
		memcpy (p_client->file_name, p_client->apdu_buf + p_client->apdu_parse_cnt, file_name_len);
		p_client->file_name [file_name_len] = 0;
	}
	else {
		p_client->file_name [0] = 0;
	}

	sprintf (path, "%s/%s", p_client->dir_name, p_client->file_name);
	fp = fopen (path, "r");
	if (fp != NULL) {
		lstat (path, &statbuf); // 获取下一级成员属性
		/* 设置文件传输初始标识 */
		p_client->file_op_flag = FT_RD_FILE_DATA;
		p_client->file_pos = 0;
		/* 结果描述 */
		send_buf [send_cnt++] = 0;
		/* 文件名长度 */
		send_buf [send_cnt++] = file_name_len;
		/* 文件名 */
		memcpy (send_buf + send_cnt, p_client->file_name, file_name_len);
		send_cnt += file_name_len;
		/* 文件标识 */
		send_buf [send_cnt++] = LSB((int)fp);
		send_buf [send_cnt++] = MSB((int)fp);
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
		/* 文件大小 */
		send_buf [send_cnt++] = LSB(LSW(statbuf.st_size));
		send_buf [send_cnt++] = MSB(LSW(statbuf.st_size));
		send_buf [send_cnt++] = LSB(MSW(statbuf.st_size));
		send_buf [send_cnt++] = MSB(MSW(statbuf.st_size));

		fclose (fp);
	}
	else {
		/* 结果描述 */
		send_buf [send_cnt++] = 1;
		/* 文件名长度 */
		send_buf [send_cnt++] = file_name_len;
		/* 文件名 */
		memcpy (send_buf + send_cnt, p_client->file_name, file_name_len);
		send_cnt += file_name_len;
		/* 文件标识 */
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
		/* 文件大小 */
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
		send_buf [send_cnt++] = 0;
	}

	send_buf [1] = send_cnt - 2;
	iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);

	return 0;
}

static int iec104_ft_file_transfer (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned char send_buf [250], * data, check_sum;
	int send_cnt, i, ret = 0;
	unsigned int inf_addr, bytes_to_read, bytes_read;
	FILE * fp;
	struct stat statbuf;          // 定义statbuf结构保存文件属性
	char path [64];

	if (p_client->file_op_flag != FT_RD_FILE_DATA) {
		return -1;
	}

	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = F_FR_NA_1;
	send_buf [send_cnt++] = 0x01;
	send_buf [send_cnt++] = COT_REQ;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}
	inf_addr = 0x20000;
	send_buf [send_cnt++] = inf_addr & 0xff;
	send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
	}
	/* 读文件数据 */
	send_buf [send_cnt++] = FT_RD_FILE_DATA;

	sprintf (path, "%s/%s", p_client->dir_name, p_client->file_name);
	fp = fopen (path, "r");
	if (fp != NULL) {
		lstat (path, &statbuf); // 获取下一级成员属性
		do {
			fseek (fp, p_client->file_pos, SEEK_SET);
			/* 文件标识 */
			send_buf [send_cnt++] = LSB((int)fp);
			send_buf [send_cnt++] = MSB((int)fp);
			send_buf [send_cnt++] = 0;
			send_buf [send_cnt++] = 0;
			/* 数据段号 */
			send_buf [send_cnt++] = LSB(LSW(p_client->file_pos));
			send_buf [send_cnt++] = MSB(LSW(p_client->file_pos));
			send_buf [send_cnt++] = LSB(MSW(p_client->file_pos));
			send_buf [send_cnt++] = MSB(MSW(p_client->file_pos));
			/* 后续标识 */
			if ((p_client->file_pos + MAX_FILE_BYTES_PER_FRAME) < statbuf.st_size) {
				send_buf [send_cnt++] = 1;
				bytes_to_read = MAX_FILE_BYTES_PER_FRAME;
			}
			else {
				send_buf [send_cnt++] = 0;
				bytes_to_read = statbuf.st_size - p_client->file_pos;
			}
			/* 文件数据 */
			data = send_buf + send_cnt;
			bytes_read = FREAD (data, bytes_to_read, 1, fp);
			if (bytes_read != bytes_to_read) {
				ret = -1;
				break;
			}
			send_cnt += MAX_FILE_BYTES_PER_FRAME;
			/* 效验和 */
			check_sum = 0;
			for (i = 0; i < bytes_read; i++) {
				check_sum += data [i];
			}
			send_buf [send_cnt++] = check_sum;

			/* 移动文件指针 */
			p_client->file_pos += bytes_read;
			if (p_client->file_pos >= statbuf.st_size) {
				p_client->file_op_flag = 0;
			}
		} while (0);

		fclose (fp);

		if (ret == 0) {
			send_buf [1] = send_cnt - 2;
			iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
		}

		return ret;
	}
	else {
		return -1;
	}
}

static int iec104_check_file_transfer (iec104_server_t * p_server, client_connect_t * p_client)
{
	int i;

	if (p_client->file_op_flag == FT_RD_FILE_DATA) {
		/* 一次传20帧 */
		for (i = 0; i < 20; i++) {
			if (iec104_ft_file_transfer (p_server, p_client) != 0) {
				break;
			}
		}
	}

	return 0;
}

static int iec104_period_yc (iec104_server_t * p_server, client_connect_t * p_client)
{
	unsigned int yc_total_num, frame_no, i, j;
	unsigned int yc_begin_no, yc_num;
	unsigned int inf_addr;
	unsigned char send_buf [250];
	int send_cnt;
	yc_data_type yc_value [GI_YC_1FRAME_NUM];
	unsigned char yc_qds [GI_YC_1FRAME_NUM], data_type [GI_YC_1FRAME_NUM];
	float f_temp;
	unsigned char * p_f_temp = (unsigned char *)&f_temp;
	short s_temp;

	yc_get_total_num_proto (p_server->app_id, &yc_total_num);
	frame_no = (yc_total_num / GI_YC_1FRAME_NUM) + 1;
	yc_begin_no = 0;
	yc_num = yc_total_num < GI_YC_1FRAME_NUM ? yc_total_num : GI_YC_1FRAME_NUM;
	for (i = 0; i < frame_no; i += 1) {
		yc_get_value_proto (p_server->app_id, yc_begin_no, yc_num, yc_value, yc_qds, data_type);

		iec104_msg_init_header (send_buf, &send_cnt);
		send_buf [send_cnt++] = p_server->per_yc_asdu;
		send_buf [send_cnt++] = yc_num | VSQ_SQ;
		send_buf [send_cnt++] = COT_PER;
		if (p_server->cot_bytes == 2) {
			send_buf [send_cnt++] = 0x00;
		}
		send_buf [send_cnt++] = p_server->app_addr;
		if (p_server->app_addr_bytes == 2) {
			send_buf [send_cnt++] = p_server->app_addr >> 8;
		}
		inf_addr = yc_begin_no + p_server->inf_obj_addr_yc_base;
		send_buf [send_cnt++] = inf_addr & 0xff;
		send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
		}

		for (j = 0; j < yc_num; j++) {
			if (p_server->per_yc_asdu == M_ME_NA_1) {
				if (data_type [j] == YC_INT) {
					s_temp = yc_value [j].i_val;
				}
				else {
					s_temp = yc_value [j].f_val;
				}
				if (p_server->yc_int_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(s_temp);
					send_buf [send_cnt++] = LSB(s_temp);
				}
				else {
					send_buf [send_cnt++] = LSB(s_temp);
					send_buf [send_cnt++] = MSB(s_temp);
				}
				send_buf [send_cnt++] = yc_qds [j];
			}
			else if (p_server->per_yc_asdu == M_ME_NB_1) {
				if (data_type [j] == YC_INT) {
					s_temp = yc_value [j].i_val;
				}
				else {
					s_temp = yc_value [j].f_val;
				}
				if (p_server->yc_int_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(s_temp);
					send_buf [send_cnt++] = LSB(s_temp);
				}
				else {
					send_buf [send_cnt++] = LSB(s_temp);
					send_buf [send_cnt++] = MSB(s_temp);
				}
				send_buf [send_cnt++] = yc_qds [j];
			} 
			else if (p_server->per_yc_asdu == M_ME_NC_1) {
				if (data_type [j] == YC_INT) {
					f_temp = yc_value [j].i_val;
				}
				else {
					f_temp = yc_value [j].f_val;
				}
				if (p_server->yc_float_byte_order_hi_lo) {
					send_buf [send_cnt++] = p_f_temp [3];
					send_buf [send_cnt++] = p_f_temp [2];
					send_buf [send_cnt++] = p_f_temp [1];
					send_buf [send_cnt++] = p_f_temp [0];
				}
				else {
					send_buf [send_cnt++] = p_f_temp [0];
					send_buf [send_cnt++] = p_f_temp [1];
					send_buf [send_cnt++] = p_f_temp [2];
					send_buf [send_cnt++] = p_f_temp [3];
				}
				send_buf [send_cnt++] = yc_qds [j];
			}
			else { /* M_ME_ND_1 */
				if (data_type [j] == YC_INT) {
					s_temp = yc_value [j].i_val;
				}
				else {
					s_temp = yc_value [j].f_val;
				}
				if (p_server->yc_int_byte_order_hi_lo) {
					send_buf [send_cnt++] = MSB(s_temp);
					send_buf [send_cnt++] = LSB(s_temp);
				}
				else {
					send_buf [send_cnt++] = LSB(s_temp);
					send_buf [send_cnt++] = MSB(s_temp);
				}
			}
		}
		
		send_buf [1] = send_cnt - 2;
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
		
		yc_begin_no += yc_num;
		yc_num = (yc_total_num - yc_num) < GI_YC_1FRAME_NUM ? (yc_total_num - yc_num) : GI_YC_1FRAME_NUM;
	}

	return 0;
}

static int iec104_save_send_frame (iec104_server_t * p_server, client_connect_t * p_client, unsigned char * send_buf, int len, int frame_type, int prio)
{
	iec104_msg_t * sbuf;

	if (find_valid_iec104_msg_buf (&sbuf) < 0) {
		_DPRINTF ("send buf full\n");
		return -1;
	}

	memcpy (sbuf->buf, send_buf, len);
	sbuf->len = len;
	sbuf->send_no = 0;
	sbuf->frame_type = frame_type;
	sbuf->next = NULL;

	if (prio == FRAME_PRIO_LO) {
		if (p_client->send_buf_lst_tail == NULL) {
			p_client->send_buf_lst_head = sbuf;
			p_client->send_buf_lst_tail = sbuf;
		}
		else {
			p_client->send_buf_lst_tail->next = sbuf;
			p_client->send_buf_lst_tail = sbuf;
		}
#if __DBUG__
		if (frame_type == U_FRAME) {
			_DPRINTF ("save uframe in send list tail\n");
		}
		else if (frame_type == S_FRAME) {
			_DPRINTF ("save sframe in send list tail\n");
		}
		else {
			_DPRINTF ("save iframe in send list tail\n");
		}
#endif
	}
	else {
		if (p_client->send_buf_lst_head == NULL) {
			p_client->send_buf_lst_head = sbuf;
			p_client->send_buf_lst_tail = sbuf;
		}
		else {
			sbuf->next = p_client->send_buf_lst_head;
			p_client->send_buf_lst_head = sbuf;
		}
#if __DBUG__
		if (frame_type == U_FRAME) {
			_DPRINTF ("save uframe in send list head\n");
		}
		else if (frame_type == S_FRAME) {
			_DPRINTF ("save sframe in send list head\n");
		}
		else {
			_DPRINTF ("save iframe in send list head\n");
		}
#endif
	}

	send_chk_send_lst_msg (p_server, p_client->sid);

	return 0;
}

static int iec104_save_unconfirmed_frame (iec104_server_t * p_server, client_connect_t * p_client, iec104_msg_t * sbuf)
{
	_DPRINTF ("save send no(%d) iframe in unconfirm list\n", sbuf->send_no);
	sbuf->next = NULL;
	if (p_client->unconfirmed_buf_lst_tail == NULL) {
		p_client->unconfirmed_buf_lst_head = sbuf;
		p_client->unconfirmed_buf_lst_tail = sbuf;
	}
	else {
		p_client->unconfirmed_buf_lst_tail->next = sbuf;
		p_client->unconfirmed_buf_lst_tail = sbuf;
	}

	p_client->unconfirmed_send_frame_cnt++;

	return 0;
}

static int iec104_save_cache_frame (iec104_server_t * p_server, unsigned char * send_buf, int len, int frame_type)
{
	iec104_msg_t * sbuf;

	if (!p_server->cache_event) {
		return -1;
	}

	if (p_server->cache_cnt >= MAX_CACHE_NO) {
		sbuf = p_server->cache_buf_lst_head;
		p_server->cache_buf_lst_head = p_server->cache_buf_lst_head->next;
	}
	else {
		if (find_valid_iec104_msg_buf (&sbuf) < 0) {
			return -1;
		}
		p_server->cache_cnt++;
	}

	memcpy (sbuf->buf, send_buf, len);
	sbuf->len = len;
	sbuf->send_no = 0;
	sbuf->frame_type = frame_type;
	sbuf->next = NULL;

	if (p_server->cache_buf_lst_tail == NULL) {
		p_server->cache_buf_lst_head = sbuf;
		p_server->cache_buf_lst_tail = sbuf;
	}
	else {
		p_server->cache_buf_lst_tail->next = sbuf;
		p_server->cache_buf_lst_tail = sbuf;
	}

	_DPRINTF ("cache iframe in send list\n");

	return 0;
}

static int iec104_check_send_lst (iec104_server_t * p_server, int sid)
{
	client_connect_t * p_client;

	p_client = iec104_server->clients_head;
	while (p_client != NULL) {
		if (sid == -1) {
			iec104_send_frame_lst (iec104_server, p_client);
		}
		else if (p_client->sid == sid) {
			iec104_send_frame_lst (iec104_server, p_client);
			break;
		}
		p_client = p_client->next;
	}

	return 0;
}

static int iec104_check_1s_task (iec104_server_t * p_server)
{
	client_connect_t * p_client;

	if (!p_server->init_ok) {

		p_server->init_counter++;
		if (p_server->init_counter >= p_server->init_time) {
			p_server->init_ok = 1;
		}
		else {
			return 0;
		}
	}

	p_client = p_server->clients_head;
	while (p_client != NULL) {
		iec104_check_t1_timer (p_server, p_client);
		iec104_check_t2_timer (p_server, p_client);
		iec104_check_t3_timer (p_server, p_client);
		iec104_check_yc_period (p_server, p_client);
		iec104_check_event (p_server, p_client);
		iec104_check_file_transfer (p_server, p_client);

		p_client = p_client->next;
	}

	return 0;
}

static int iec104_check_t1_timer (iec104_server_t * p_server, client_connect_t * p_client)
{
	if (p_client->t1_cnt < p_server->t1_to) {
		p_client->t1_cnt++;
	}
	else if (p_client->t1_cnt == p_server->t1_to) {
		/* time out */

		/* continue count up 1 */
		p_client->t1_cnt++;
	}
	else { /* p_client->t1_cnt > p_server->t1_to */
		/* do nothing */
	}

	return 0;
}

static int iec104_check_t2_timer (iec104_server_t * p_server, client_connect_t * p_client)
{
	if (p_client->t2_cnt < p_server->t2_to) {
		p_client->t2_cnt++;
	}
	else if (p_client->t2_cnt == p_server->t2_to) {
		/* time out */
		/* send s frame to confirm i frame */
		iec104_send_sframe (p_server, p_client);
		/* continue count up 1 */
		p_client->t2_cnt++;
	}
	else { /* p_client->t2_cnt > p_server->t2_to */
		/* do nothing */
	}

	return 0;
}

static int iec104_check_t3_timer (iec104_server_t * p_server, client_connect_t * p_client)
{
	if (p_client->t3_cnt < p_server->t3_to) {
		p_client->t3_cnt++;
	}
	else if (p_client->t3_cnt == p_server->t3_to) {
		/* time out */
		/* send test frame */
		iec104_send_uframe (p_server, p_client, U_TEST_ACTIVE, FRAME_PRIO_LO);
		p_client->t3_cnt = 0;
	}
	else { /* p_client->t3_cnt > p_server->t3_to */
		/* do nothing */
	}

	return 0;
}

static int iec104_check_yc_period (iec104_server_t * p_server, client_connect_t * p_client)
{
	if (p_client->yc_period_cnt < p_server->yc_period) {
		p_client->yc_period_cnt++;
	}
	else {
		iec104_period_yc (p_server, p_client);
		p_client->yc_period_cnt = 0;
	}

	return 0;
}

static int iec104_check_event (iec104_server_t * p_server, client_connect_t * p_client)
{
	if (p_server->yx_event_num > 0) {
		iec104_check_yx_event (p_server, p_server->yx_event_lst, p_server->yx_event_num);
		iec104_free_yx_event (p_server, NULL);
	}
	if (p_server->yc_event_num > 0) {
		iec104_check_yc_event (p_server, p_server->yc_event_lst, p_server->yc_event_num);
		iec104_free_yc_event (p_server, NULL);
	}

	return 0;
}

static int iec104_check_yx_event (iec104_server_t * p_server, yx_event_t * p_event, int event_num)
{
	client_connect_t * p_client;
	unsigned int inf_addr;
	unsigned char soe_buf [250], cos_buf [250], yx_val;
	int soe_cnt = 0, cos_cnt = 0, j;
	unsigned char cp56t [7];
	unsigned char soe_num, cos_num, header_cnt, event_num_pos;

	if (event_num <= 0) {
		return -1;
	}

	p_client = p_server->clients_head;
	if (p_client == NULL) {
		return 0;
	}

	/* SOC and COS header */
	iec104_msg_init_header (soe_buf, &soe_cnt);
	iec104_msg_init_header (cos_buf, &cos_cnt);

	soe_buf [soe_cnt++] = p_server->soe_yx_asdu;
	event_num_pos = soe_cnt;
	soe_buf [soe_cnt++] = 0;
	soe_buf [soe_cnt++] = COT_SPONT;
	if (p_server->cot_bytes == 2) {
		soe_buf [soe_cnt++] = 0x00;
	}
	soe_buf [soe_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		soe_buf [soe_cnt++] = p_server->app_addr >> 8;
	}

	cos_buf [cos_cnt++] = p_server->cos_yx_asdu;
	cos_buf [cos_cnt++] = 0;
	cos_buf [cos_cnt++] = COT_SPONT;
	if (p_server->cot_bytes == 2) {
		cos_buf [cos_cnt++] = 0x00;
	}
	cos_buf [cos_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		cos_buf [cos_cnt++] = p_server->app_addr >> 8;
	}

	header_cnt = cos_cnt;
	soe_num = 0;
	cos_num = 0;

	for (j = 0; j < event_num; j++) {
		soe_num++;
		inf_addr = p_event [j].data_no + p_server->inf_obj_addr_yx_base;
		soe_buf [soe_cnt++] = inf_addr & 0xff;
		soe_buf [soe_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			soe_buf [soe_cnt++] = (inf_addr >> 16) & 0xff;
		}
		if (p_server->soe_yx_asdu == M_SP_TA_1 || 
			p_server->soe_yx_asdu == M_SP_TB_1) {
			switch (p_event [j].event_val) {
			case DP_OPEN:
				yx_val = SP_OPEN;
				break;
			case DP_CLOSE:
				yx_val = SP_CLOSE;
				break;
			case DP_OPEN | DIQ_IV:
				yx_val = SP_OPEN | SIQ_IV;
				break;
			case DP_CLOSE | DIQ_IV:
				yx_val = SP_CLOSE | SIQ_IV;
				break;
			default:
				yx_val = SP_OPEN | SIQ_IV;
				break;
			}
		}
		else /*if (p_server->soe_yx_asdu == M_DP_TA_1 || 
			p_server->soe_yx_asdu == M_DP_TB_1) */{
			switch (p_event [j].event_val) {
				case DP_OPEN:
					yx_val = DP_OPEN;
					break;
				case DP_CLOSE:
					yx_val = DP_CLOSE;
					break;
				case DP_OPEN | DIQ_IV:
					yx_val = DP_OPEN | DIQ_IV;
					break;
				case DP_CLOSE | DIQ_IV:
					yx_val = DP_CLOSE | DIQ_IV;
					break;
				default:
					yx_val = DP_OPEN | DIQ_IV;
					break;
			}
		}
		
		/* value */
		soe_buf [soe_cnt++] = yx_val;
		/* time stamp */
		time_to_cp56time2a (&p_event [j].time_val, cp56t);
		soe_buf [soe_cnt++] = cp56t [0];
		soe_buf [soe_cnt++] = cp56t [1];
		soe_buf [soe_cnt++] = cp56t [2];
		if (p_server->soe_yx_asdu == M_SP_TB_1 ||
			p_server->soe_yx_asdu == M_DP_TB_1) {
			soe_buf [soe_cnt++] = cp56t [3];
			soe_buf [soe_cnt++] = cp56t [4];
			soe_buf [soe_cnt++] = cp56t [5];
			soe_buf [soe_cnt++] = cp56t [6];
		}
		
		if (p_event [j].cos_soe_flag & YX_COS_FLAG) {
			cos_num++;
			inf_addr = p_event [j].data_no + p_server->inf_obj_addr_yx_base;
			cos_buf [cos_cnt++] = inf_addr & 0xff;
			cos_buf [cos_cnt++] = (inf_addr >> 8) & 0xff;
			if (p_server->inf_addr_bytes == 3) {
				cos_buf [cos_cnt++] = (inf_addr >> 16) & 0xff;
			}
			if (p_server->cos_yx_asdu == M_SP_NA_1) {
				switch (p_event [j].event_val) {
				case DP_OPEN:
					yx_val = SP_OPEN;
					break;
				case DP_CLOSE:
					yx_val = SP_CLOSE;
					break;
				case DP_OPEN | DIQ_IV:
					yx_val = SP_OPEN | SIQ_IV;
					break;
				case DP_CLOSE | DIQ_IV:
					yx_val = SP_CLOSE | SIQ_IV;
					break;
				default:
					yx_val = SP_OPEN | SIQ_IV;
					break;
				}
			}
			else /*if (p_server->cos_yx_asdu == M_DP_NA_1) */{
				switch (p_event [j].event_val) {
				case DP_OPEN:
					yx_val = DP_OPEN;
					break;
				case DP_CLOSE:
					yx_val = DP_CLOSE;
					break;
				case DP_OPEN | DIQ_IV:
					yx_val = DP_OPEN | DIQ_IV;
					break;
				case DP_CLOSE | DIQ_IV:
					yx_val = DP_CLOSE | DIQ_IV;
					break;
				default:
					yx_val = DP_OPEN | DIQ_IV;
					break;
				}
			}
			
			/* value */
			cos_buf [cos_cnt++] = yx_val;
		}

		if (soe_cnt > 200) {
			soe_buf [1] = soe_cnt - 2;
			soe_buf [event_num_pos] = soe_num;
			p_client = p_server->clients_head;
			if (p_client != NULL) {
				while (p_client != NULL) {
					iec104_save_send_frame (p_server, p_client, soe_buf, soe_cnt, I_FRAME, FRAME_PRIO_HI);
					p_client = p_client->next;
				}
			}
			else if (p_server->cache_event) {
				iec104_save_cache_frame (p_server, soe_buf, soe_cnt, I_FRAME);
			}
			
			soe_cnt = header_cnt;
			soe_num = 0;
		}

		if (cos_cnt > 200) {
			cos_buf [1] = cos_cnt - 2;
			cos_buf [event_num_pos] = cos_num;
			p_client = p_server->clients_head;
			if (p_client != NULL) {
				while (p_client != NULL) {
					iec104_save_send_frame (p_server, p_client, cos_buf, cos_cnt, I_FRAME, FRAME_PRIO_HI);
					p_client = p_client->next;
				}
			}

			cos_cnt = header_cnt;
			cos_num = 0;
		}
	}

	p_client = p_server->clients_head;
	if (p_client != NULL) {
		while (p_client != NULL) {
			if (soe_num > 0) {
				soe_buf [1] = soe_cnt - 2;
				soe_buf [event_num_pos] = soe_num;
				iec104_save_send_frame (p_server, p_client, soe_buf, soe_cnt, I_FRAME, FRAME_PRIO_HI);
			}
			if (cos_num > 0) {
				cos_buf [1] = cos_cnt - 2;
				cos_buf [event_num_pos] = cos_num;
				iec104_save_send_frame (p_server, p_client, cos_buf, cos_cnt, I_FRAME, FRAME_PRIO_HI);
			}
			p_client = p_client->next;
		}
	}
	else if (p_server->cache_event) {
		if (soe_num > 0) {
			soe_buf [1] = soe_cnt - 2;
			soe_buf [event_num_pos] = soe_num;
			iec104_save_cache_frame (p_server, soe_buf, soe_cnt, I_FRAME);
		}
	}

	return 0;
}

static int iec104_check_yc_event (iec104_server_t * p_server, yc_event_t * p_event, int event_num)
{
	client_connect_t * p_client;
	unsigned int inf_addr;
	unsigned char send_buf [250];
	int send_cnt, j;
	unsigned char cp56t [7];
	short s_temp;
	float f_temp;
	unsigned char * p_f_temp = (unsigned char *)&f_temp;
	unsigned char yc_num, header_cnt, event_num_pos;

	if (event_num <= 0) {
		return -1;
	}

	p_client = p_server->clients_head;
	if (p_client == NULL) {
		return 0;
	}
	
	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = p_server->chg_yc_asdu;
	event_num_pos = send_cnt;
	send_buf [send_cnt++] = event_num;
	send_buf [send_cnt++] = COT_SPONT;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}

	header_cnt = send_cnt;
	yc_num = 0;
	
	for (j = 0; j < event_num; j++) {
		yc_num++;
		inf_addr = p_event [j].data_no + p_server->inf_obj_addr_yc_base;
		send_buf [send_cnt++] = inf_addr & 0xff;
		send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
		}
		
		if (p_server->chg_yc_asdu == M_ME_NA_1 ||
			p_server->chg_yc_asdu == M_ME_TA_1 ||
			p_server->chg_yc_asdu == M_ME_TD_1) {
			if (p_event [j].data_type == YC_INT) {
				s_temp = p_event [j].event_val.i_val;
			}
			else {
				s_temp = p_event [j].event_val.f_val;
			}
			if (p_server->yc_int_byte_order_hi_lo) {
				send_buf [send_cnt++] = MSB(s_temp);
				send_buf [send_cnt++] = LSB(s_temp);
			}
			else {
				send_buf [send_cnt++] = LSB(s_temp);
				send_buf [send_cnt++] = MSB(s_temp);
			}
			send_buf [send_cnt++] = p_event [j].qds;
			if (p_server->chg_yc_asdu == M_ME_TA_1 ||
				p_server->chg_yc_asdu == M_ME_TD_1) {
				/* time stamp */
				time_to_cp56time2a (&p_event [j].time_val, cp56t);
				send_buf [send_cnt++] = cp56t [0];
				send_buf [send_cnt++] = cp56t [1];
				send_buf [send_cnt++] = cp56t [2];
				if (p_server->chg_yc_asdu == M_ME_TD_1) {
					send_buf [send_cnt++] = cp56t [3];
					send_buf [send_cnt++] = cp56t [4];
					send_buf [send_cnt++] = cp56t [5];
					send_buf [send_cnt++] = cp56t [6];
				}
			}
		}
		else if (p_server->chg_yc_asdu == M_ME_NB_1 ||
			p_server->chg_yc_asdu == M_ME_TB_1 ||
			p_server->chg_yc_asdu == M_ME_TE_1) {
			if (p_event [j].data_type == YC_INT) {
				s_temp = p_event [j].event_val.i_val;
			}
			else {
				s_temp = p_event [j].event_val.f_val;
			}
			if (p_server->yc_int_byte_order_hi_lo) {
				send_buf [send_cnt++] = MSB(s_temp);
				send_buf [send_cnt++] = LSB(s_temp);
			}
			else {
				send_buf [send_cnt++] = LSB(s_temp);
				send_buf [send_cnt++] = MSB(s_temp);
			}
			send_buf [send_cnt++] = p_event [j].qds;
			if (p_server->chg_yc_asdu == M_ME_TB_1 ||
				p_server->chg_yc_asdu == M_ME_TE_1) {
				/* time stamp */
				time_to_cp56time2a (&p_event [j].time_val, cp56t);
				send_buf [send_cnt++] = cp56t [0];
				send_buf [send_cnt++] = cp56t [1];
				send_buf [send_cnt++] = cp56t [2];
				if (p_server->chg_yc_asdu == M_ME_TE_1) {
					send_buf [send_cnt++] = cp56t [3];
					send_buf [send_cnt++] = cp56t [4];
					send_buf [send_cnt++] = cp56t [5];
					send_buf [send_cnt++] = cp56t [6];
				}
			}
		} 
		else if (p_server->chg_yc_asdu == M_ME_NC_1 ||
			p_server->chg_yc_asdu == M_ME_TC_1 ||
			p_server->chg_yc_asdu == M_ME_TF_1) {
			if (p_event [j].data_type == YC_INT) {
				f_temp = p_event [j].event_val.i_val;
			}
			else {
				f_temp = p_event [j].event_val.f_val;
			}
			if (p_server->yc_float_byte_order_hi_lo) {
				send_buf [send_cnt++] = p_f_temp [3];
				send_buf [send_cnt++] = p_f_temp [2];
				send_buf [send_cnt++] = p_f_temp [1];
				send_buf [send_cnt++] = p_f_temp [0];
			}
			else {
				send_buf [send_cnt++] = p_f_temp [0];
				send_buf [send_cnt++] = p_f_temp [1];
				send_buf [send_cnt++] = p_f_temp [2];
				send_buf [send_cnt++] = p_f_temp [3];
			}
			send_buf [send_cnt++] = p_event [j].qds;
			if (p_server->chg_yc_asdu == M_ME_TC_1 ||
				p_server->chg_yc_asdu == M_ME_TF_1) {
				/* time stamp */
				time_to_cp56time2a (&p_event [j].time_val, cp56t);
				send_buf [send_cnt++] = cp56t [0];
				send_buf [send_cnt++] = cp56t [1];
				send_buf [send_cnt++] = cp56t [2];
				if (p_server->chg_yc_asdu == M_ME_TF_1) {
					send_buf [send_cnt++] = cp56t [3];
					send_buf [send_cnt++] = cp56t [4];
					send_buf [send_cnt++] = cp56t [5];
					send_buf [send_cnt++] = cp56t [6];
				}
			}
		}
		else /*if (p_server->chg_yc_asdu == M_ME_ND_1)*/ {
			if (p_event [j].data_type == YC_INT) {
				s_temp = p_event [j].event_val.i_val;
			}
			else {
				s_temp = p_event [j].event_val.f_val;
			}
			if (p_server->yc_int_byte_order_hi_lo) {
				send_buf [send_cnt++] = MSB(s_temp);
				send_buf [send_cnt++] = LSB(s_temp);
			}
			else {
				send_buf [send_cnt++] = LSB(s_temp);
				send_buf [send_cnt++] = MSB(s_temp);
			}
		}

		if (send_cnt > 200) {
			send_buf [1] = send_cnt - 2;
			send_buf [event_num_pos] = yc_num;
			while (p_client != NULL) {
				iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
				p_client = p_client->next;
			}

			send_cnt = header_cnt;
			yc_num = 0;
		}
	}

	if (yc_num > 0) {
		send_buf [1] = send_cnt - 2;
		send_buf [event_num_pos] = yc_num;
		while (p_client != NULL) {
			iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_LO);
			p_client = p_client->next;
		}
	}

	return 0;
}
# if 0
int iec104_fault_event (unsigned int bus, unsigned int bay, unsigned int yx_item, unsigned int yx_value, Uint16 u4 [], Uint16 i4 [])
{
	iec104_server_t * p_server = iec104_server;
	client_connect_t * p_client;
	unsigned int inf_addr;
	unsigned char send_buf [250], yx_val;
	int send_cnt, j;
	unsigned char cp56t [7];
	short s_temp;
	float f_temp;
	unsigned char * p_f_temp = (unsigned char *)&f_temp;
	time_type tt;
	unsigned int yc_item, yc_num;

	p_client = p_server->clients_head;
	if (p_client == NULL) {
		return 0;
	}

	/* get timestamp */
	get_time_sec_msec (&tt);
	
	iec104_msg_init_header (send_buf, &send_cnt);
	send_buf [send_cnt++] = M_FT_NA_1;
	send_buf [send_cnt++] = 1;
	send_buf [send_cnt++] = COT_SPONT;
	if (p_server->cot_bytes == 2) {
		send_buf [send_cnt++] = 0x00;
	}
	send_buf [send_cnt++] = p_server->app_addr;
	if (p_server->app_addr_bytes == 2) {
		send_buf [send_cnt++] = p_server->app_addr >> 8;
	}

	/* 带时标遥信个数 */
	send_buf [send_cnt++] = 1;
	/* 遥信类型 */
	send_buf [send_cnt++] = p_server->soe_yx_asdu;
	/* 故障遥信点号 */
	inf_addr = yx_item + p_server->inf_obj_addr_yx_base;
	send_buf [send_cnt++] = inf_addr & 0xff;
	send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
	if (p_server->inf_addr_bytes == 3) {
		send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
	}
	/* 遥信值 */
	if (p_server->soe_yx_asdu == M_SP_TA_1 || 
		p_server->soe_yx_asdu == M_SP_TB_1) {
		switch (yx_value) {
		case DP_OPEN:
			yx_val = SP_OPEN;
			break;
		case DP_CLOSE:
			yx_val = SP_CLOSE;
			break;
		case DP_OPEN | DIQ_IV:
			yx_val = SP_OPEN | SIQ_IV;
			break;
		case DP_CLOSE | DIQ_IV:
			yx_val = SP_CLOSE | SIQ_IV;
			break;
		default:
			yx_val = SP_OPEN | SIQ_IV;
			break;
		}
	}
	else /*if (p_server->soe_yx_asdu == M_DP_TA_1 || 
		p_server->soe_yx_asdu == M_DP_TB_1) */{
		switch (yx_value) {
			case DP_OPEN:
				yx_val = DP_OPEN;
				break;
			case DP_CLOSE:
				yx_val = DP_CLOSE;
				break;
			case DP_OPEN | DIQ_IV:
				yx_val = DP_OPEN | DIQ_IV;
				break;
			case DP_CLOSE | DIQ_IV:
				yx_val = DP_CLOSE | DIQ_IV;
				break;
			default:
				yx_val = DP_OPEN | DIQ_IV;
				break;
		}
	}
	send_buf [send_cnt++] = yx_val;
	/* 时标 */
	time_to_cp56time2a (&tt, cp56t);
	send_buf [send_cnt++] = cp56t [0];
	send_buf [send_cnt++] = cp56t [1];
	send_buf [send_cnt++] = cp56t [2];
	if (p_server->soe_yx_asdu == M_SP_TB_1 ||
		p_server->soe_yx_asdu == M_DP_TB_1) {
		send_buf [send_cnt++] = cp56t [3];
		send_buf [send_cnt++] = cp56t [4];
		send_buf [send_cnt++] = cp56t [5];
		send_buf [send_cnt++] = cp56t [6];
	}

	/* 遥测个数 */
	if (bus) {
		send_buf [send_cnt++] = 4;
		yc_num = 4;
	}
	else {
		send_buf [send_cnt++] = 8;
		yc_num = 8;
		if (bay > 0 && bay <= MAX_BAY_NUM) {
			bus = system_setting_value->bay_vol_sel [bay - BAY_VOL_SEL_1];
		}
		else {
			bus = 1;
		}
		if (bus < BAY_VOL_SEL_1 && bus >= BAY_VOL_SEL_BKUP) {
			bus = BAY_VOL_SEL_1;
		}
	}
	/* 变化遥测类型 */
	send_buf [send_cnt++] = p_server->ft_yc_asdu;
	for (j = 0; j < yc_num; j++) {
		if (j >= 0 && j < 4) {
			s_temp = u4 [j];
		}
		else {
			s_temp = i4 [j - 4];
		}
		switch (j) {
		case 0:
		case 1:
		case 2:
			inf_addr = Ua1 + j;
			get_point_no_of_bus_bay (bus, 0, inf_addr, &yc_item);
			break;
		case 3:
			inf_addr = U01;
			get_point_no_of_bus_bay (bus, 0, inf_addr, &yc_item);
			break;
		case 4:
		case 5:
		case 6:
			inf_addr = Ia1 + j;
			get_point_no_of_bus_bay (0, bay, inf_addr, &yc_item);
			break;
		case 7:
			inf_addr = I01;
			get_point_no_of_bus_bay (0, bay, inf_addr, &yc_item);
			break;
		}
		inf_addr = yc_item + p_server->inf_obj_addr_yc_base;
		send_buf [send_cnt++] = inf_addr & 0xff;
		send_buf [send_cnt++] = (inf_addr >> 8) & 0xff;
		if (p_server->inf_addr_bytes == 3) {
			send_buf [send_cnt++] = (inf_addr >> 16) & 0xff;
		}
		
		if (p_server->chg_yc_asdu == M_ME_NC_1) {
			f_temp = s_temp;
			if (p_server->yc_float_byte_order_hi_lo) {
				send_buf [send_cnt++] = p_f_temp [3];
				send_buf [send_cnt++] = p_f_temp [2];
				send_buf [send_cnt++] = p_f_temp [1];
				send_buf [send_cnt++] = p_f_temp [0];
			}
			else {
				send_buf [send_cnt++] = p_f_temp [0];
				send_buf [send_cnt++] = p_f_temp [1];
				send_buf [send_cnt++] = p_f_temp [2];
				send_buf [send_cnt++] = p_f_temp [3];
			}
			//send_buf [send_cnt++] = 0;
		}
		else /*if (p_server->chg_yc_asdu == M_ME_ND_1)*/ {
			if (p_server->yc_int_byte_order_hi_lo) {
				send_buf [send_cnt++] = MSB(s_temp);
				send_buf [send_cnt++] = LSB(s_temp);
			}
			else {
				send_buf [send_cnt++] = LSB(s_temp);
				send_buf [send_cnt++] = MSB(s_temp);
			}
		}
	}

	send_buf [1] = send_cnt - 2;
	while (p_client != NULL) {
		iec104_save_send_frame (p_server, p_client, send_buf, send_cnt, I_FRAME, FRAME_PRIO_HI);
		p_client = p_client->next;
	}

	return 0;
}
# endif
static int iec104_save_yx_event (iec104_server_t * p_server, yx_event_t * p_event)
{
	memcpy (&p_server->yx_event_lst [p_server->yx_event_num], p_event, sizeof (* p_event));
	p_server->yx_event_num++;
	
	return p_server->yx_event_num;
}

static int iec104_save_yc_event (iec104_server_t * p_server, yc_event_t * p_event)
{
	memcpy (&p_server->yc_event_lst [p_server->yc_event_num], p_event, sizeof (* p_event));
	p_server->yc_event_num++;
	
	return p_server->yc_event_num;
}

static int iec104_free_yx_event (iec104_server_t * p_server, yx_event_t * p_event)
{
	p_server->yx_event_num = 0;
	return 0;
}

static int iec104_free_yc_event (iec104_server_t * p_server, yc_event_t * p_event)
{
	p_server->yc_event_num = 0;
	return 0;
}

static int iec104_send_frame_lst (iec104_server_t * p_server, client_connect_t * p_client)
{
	iec104_msg_t * sbuf, * prev;
	unsigned char asdu_type, dir;
#if __DBUG__
	char prompt [24];
#endif

	if (!p_client->user_data_start) {
		prev = p_client->send_buf_lst_head;
		sbuf = p_client->send_buf_lst_head;
		while (sbuf) {
			if (sbuf->frame_type == I_FRAME) {
				asdu_type = sbuf->buf [6];
				iec104_check_asdu_direct (asdu_type, &dir);
				if (dir == SUPERVISION_DIR) {
					prev = sbuf;
					sbuf = sbuf->next;
				}
				else {
					if (iec104_send_frame (p_server, p_client, sbuf) < 0) {
						/* send failed, 1s later retry */
						break;
					}
#if __DBUG__
					sprintf (prompt, "sfd(%d) send 104 msg", p_client->sid);
					show_msg (prompt, sbuf->buf, sbuf->len);
#endif
					if (sbuf == p_client->send_buf_lst_head) {
						p_client->send_buf_lst_head = p_client->send_buf_lst_head->next;
						if (p_client->send_buf_lst_head == NULL) {
							p_client->send_buf_lst_tail = NULL;
						}
						iec104_save_unconfirmed_frame (p_server, p_client, sbuf);
						sbuf = p_client->send_buf_lst_head;
					}
					else {
						prev->next = sbuf->next;
						iec104_save_unconfirmed_frame (p_server, p_client, sbuf);
						sbuf = prev->next;
						if (sbuf == NULL) {
							p_client->send_buf_lst_tail = NULL;
						}
					}
				}
			}
			else {
				if (iec104_send_frame (p_server, p_client, sbuf) < 0) {
					/* send failed, 1s later retry */
					break;
				}
#if __DBUG__
				sprintf (prompt, "sfd(%d) send 104 msg", p_client->sid);
				show_msg (prompt, sbuf->buf, sbuf->len);
#endif
				if (sbuf == p_client->send_buf_lst_head) {
					p_client->send_buf_lst_head = p_client->send_buf_lst_head->next;
					if (p_client->send_buf_lst_head == NULL) {
						p_client->send_buf_lst_tail = NULL;
					}
					free_iec104_msg_buf (sbuf);
					sbuf = p_client->send_buf_lst_head;
				}
				else {
					prev->next = sbuf->next;
					free_iec104_msg_buf (sbuf);
					sbuf = prev->next;
					if (sbuf == NULL) {
						p_client->send_buf_lst_tail = NULL;
					}
				}
			}
		}
		return 0;
	}
	else {
		while (p_client->send_buf_lst_head) {
			sbuf = p_client->send_buf_lst_head;
			if (iec104_send_frame (p_server, p_client, sbuf) < 0) {
				/* send failed, 1s later retry */
				break;
			}
#if __DBUG__
			sprintf (prompt, "sfd(%d) send 104 msg", p_client->sid);
			show_msg (prompt, sbuf->buf, sbuf->len);
#endif
			
			p_client->send_buf_lst_head = p_client->send_buf_lst_head->next;
			if (p_client->send_buf_lst_head == NULL) {
				p_client->send_buf_lst_tail = NULL;
			}
			
			if (sbuf->frame_type == I_FRAME) {
				iec104_save_unconfirmed_frame (p_server, p_client, sbuf);
			}
			else {
				free_iec104_msg_buf (sbuf);
			}
		}
		return 0;
	}
}

static int iec104_confirm_frame (iec104_server_t * p_server, client_connect_t * p_client)
{
	iec104_msg_t * sbuf;
	int confirmed_cnt = 0;

	while (1) {
		sbuf = p_client->unconfirmed_buf_lst_head;
		if (sbuf == NULL) {
			/* e.g. when first received frame come, the list is empty */
			p_client->unconfirmed_buf_lst_tail = NULL;
			confirmed_cnt++;
			break;
		}
		if (sbuf->send_no < p_client->recv_cnt.recv_no) {
			p_client->unconfirmed_buf_lst_head = p_client->unconfirmed_buf_lst_head->next;
			_DPRINTF ("confirm send no (%d) i frame\n", sbuf->send_no);
			free_iec104_msg_buf (sbuf);
			confirmed_cnt++;
			p_client->unconfirmed_send_frame_cnt--;
		}
		else {
			break;
		}
	}

	if (!confirmed_cnt) {
		return -1;
	}
	else {
		return 0;
	}
}

static int iec104_send_frame (iec104_server_t * p_server, client_connect_t * p_client, iec104_msg_t * sbuf)
{
	int send_len;

	if (p_client->unconfirmed_send_frame_cnt >= p_server->k) {
		return -1;
	}
	
	switch (sbuf->frame_type) {
	case U_FRAME:
		break;
	case S_FRAME:
		sbuf->buf [4] = p_client->send_cnt.recv_no & 0xff;
		sbuf->buf [5] = (p_client->send_cnt.recv_no >> 8) & 0xff;
		/*p_client->t2_cnt = 0;*/
		break;
	case I_FRAME:
		sbuf->buf [2] = p_client->send_cnt.send_no & 0xff;
		sbuf->buf [3] = (p_client->send_cnt.send_no >> 8) & 0xff;
		sbuf->buf [4] = p_client->send_cnt.recv_no & 0xff;
		sbuf->buf [5] = (p_client->send_cnt.recv_no >> 8) & 0xff;
		sbuf->send_no = p_client->send_cnt.send_no;
		/*p_client->t2_cnt = 0;*/
		break;
	default:
		_DPRINTF ("Invalid frame type\n");
		return -1;
	}

	send_len = send (p_client->sid, sbuf->buf, sbuf->len, 0);
	if (send_len <= 0) {
		_DPRINTF ("send iec104 msg error\n");
		return -1;
	}
	else {
		if (sbuf->frame_type == I_FRAME) {
			_DPRINTF ("send i frame, send no(%d) recv no(%d)\n",
				p_client->send_cnt.send_no,
				p_client->send_cnt.recv_no);
		}
		else if (sbuf->frame_type == S_FRAME) {
			_DPRINTF ("send s frame, recv no(%d)\n",
				p_client->send_cnt.recv_no);
		}
		else {
			_DPRINTF ("send u frame\n");
		}
		if (sbuf->frame_type == I_FRAME) {
			p_client->send_cnt.send_no += 2;
			if (p_client->send_cnt.send_no >= 0xffff) {
				p_client->send_cnt.send_no = 0;
			}
		}
		if (sbuf->frame_type == S_FRAME ||
			sbuf->frame_type == I_FRAME) {
			/*
			 * 接收序号通过s帧和i帧确认了
			 * t2直接timeout掉
			 */
			if (p_client->t2_cnt <= p_server->t2_to) {
				p_client->t2_cnt = p_server->t2_to + 1;
			}
		}
	}

	return 0;
}

static int iec104_check_asdu_direct (unsigned char asdu_type, unsigned char * direct)
{
	switch (asdu_type) {
	case C_SC_NA_1:
	case C_DC_NA_1:
	case C_IC_NA_1:
	case C_CI_NA_1:
	case C_RD_NA_1:
	case C_CS_NA_1:
	case C_TS_NA_1:
	case C_RP_NA_1:
	case C_SR_NA_1:
	case C_RR_NA_1:
	case C_RS_NA_1:
	case C_WS_NA_1:
	case F_FR_NA_1:
	case F_SR_NA_1:
		* direct = CONTROL_DIR;
		break;
	default:
		* direct = SUPERVISION_DIR;
		break;
	}

	return 0;
}

static int send_chk_send_lst_msg (iec104_server_t * p_server, int sid)
{
	internal_msg_t in_msg;

	in_msg.type = MSG_CHK_SEND_LST;
	in_msg.sub_type = 0;
	in_msg.content [0] = sid & 0xff;
	in_msg.content [1] = (sid >> 8) & 0xff;
	in_msg.content [2] = (sid >> 16) & 0xff;
	in_msg.content [3] = (sid >> 24) & 0xff;

	return send_iec104_internal_udp_data ((unsigned char *)&in_msg, sizeof (internal_msg_t));
}

static int send_chk_time1s_task_msg (iec104_server_t * p_server)
{
	internal_msg_t in_msg;

	in_msg.type = MSG_CHK_TIME1S_TASK;

	return send_iec104_internal_udp_data ((unsigned char *)&in_msg, sizeof (internal_msg_t));
}

static int get_iec104_frame_recv_no (unsigned char apdu [], int len, int * recv_no)
{
	if (apdu == NULL ||
		len < 6) {
		* recv_no = -1;
		return -1;
	}

	* recv_no = (apdu [5] << 8) | apdu [4];

	if (* recv_no < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

static int get_iec104_frame_send_no (unsigned char apdu [], int len, int * send_no)
{
	if (apdu == NULL ||
		len < 6) {
		* send_no = -1;
		return -1;
	}

	* send_no = (apdu [3] << 8) | apdu [2];

	if (* send_no < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

static int init_iec104_msg_pool ()
{
	int i;
	iec104_msg_t * p_msg;

	for (i = 0; i < MAX_SBUF_NO; i++) {
		p_msg = (iec104_msg_t *)malloc (sizeof (iec104_msg_t));
		if (p_msg == NULL) {
			_DPRINTF ("init iec104 message pool malloc fail\n");
			return -1;
		}
		p_msg->next = iec104_msg_pool;
		iec104_msg_pool = p_msg;
	}

	return 0;
}

static int find_valid_iec104_msg_buf (iec104_msg_t ** sbuf)
{
	if (sbuf == NULL) {
		return -1;
	}

	if (iec104_msg_pool == NULL) {
		* sbuf = NULL;
		return -1;
	}

	* sbuf = iec104_msg_pool;
	iec104_msg_pool = iec104_msg_pool->next;

	return 0;
}

static int free_iec104_msg_buf (iec104_msg_t * sbuf)
{
	if (sbuf != NULL) {
		sbuf->next = iec104_msg_pool;
		iec104_msg_pool = sbuf;
		return 0;
	}
	else {
		return -1;
	}
}

int init_iec104_server ()
{
	if (init_iec104_data_local_mem () != 0) {
		return -1;
	}
	
	/* init config */
	if (read_iec104_server_cfg (NULL) != 0) {
		return -1;
	}

	/* init server */
	
	if (init_iec104_server_param () != 0) {
		return -1;
	}
	if (init_iec104_msg_pool () != 0) {
		return -1;
	}
	if (init_accept_task () != 0) {
		return -1;
	}
	if (init_iec104_task () != 0) {
		return -1;
	}
	if (init_timer1s_task () != 0) {
		return -1;
	}
	if (init_iec104_reg_rdb () != 0) {
		return -1;
	}

	return 0;
}

