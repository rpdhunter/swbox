/********************************************************************
	created:	2015/11/06
	created:	6:11:2015   20:16
	filename: 	D:\rd\src_code\rtu\code\src\rdb.c
	file path:	D:\rd\src_code\rtu\code\src
	file base:	rdb
	file ext:	c
	author:		zhouxf
	purpose:	rdb source file
*********************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>

#include "thread.h"
#include "ipcs.h"

#include "rdb.h"

#define RDB_APP_ID			5

static yx_t * yx_lst;		/* YX_NUMBER */
static yc_t * yc_lst;		/* YC_NUMBER */
static yk_t * yk_lst;		/* YK_NUMBER */
//static ym_t * ym_lst;		/* YM_NUMBER */
static app_hook_func_t * app_hook_func_lst = NULL;
static int app_reg_num = 0;
static sem_t rdb_event_pool_mutex;
static rdb_event_t * rdb_event_pool = NULL;

static void rdb_task (void *argc);
static int init_rdb_task ();
static int init_rdb_event_pool ();
static int find_valid_rdb_event_buf (rdb_event_t ** ebuf);
static int free_rdb_event_buf (rdb_event_t * ebuf);
static int check_yx_event_list ();
static int check_yc_event_list ();
static int check_yk_list ();

static void rdb_task (void * argc)
{
	while (1) {
		usleep (RDB_CHECK_TIME);	/* 100ms */
#if 1
		/* 任务状态监视 
		task_running_cnt [RDB_TASK_CNT]++;*/
#endif
		check_yx_event_list ();
		check_yc_event_list ();
		check_yk_list ();
	}
	//fdOpenSession(Task_self());
}

static int init_rdb_task ()
{
	pthread_t tid;
	tid = thread_create ("rdb task", 1024 * 8, 1, rdb_task, NULL);//DEF_TASK_STACK_SIZE=1000;PRIO_RDB=1
    if (tid < 0) {
        return -1;
    }

	return 0;
}

int init_rdb_table ()
{
	int i;
	
	for (i = 0; i < YX_NUMBER; i++) {		//yx点表初始化，默认值为DP_OPEN | DIQ_IV;
		memset (&yx_lst [i], 0, sizeof (yx_t));
		yx_lst [i].cur_val = DP_OPEN | DIQ_IV;
		if (sem_init (&yx_lst [i].mutex, 0, 1) < 0) {
			return -1;
		}
	}
	                                        //yc点表初始化，该部分要修改*********************************************************
	for (i = 0; i < YC_NUMBER; i++){
		memset (&yc_lst [i], 0, sizeof (yc_t));
        yc_lst [i].cur_val.f_val = 0;
        yc_lst [i].data_type = YC_FLOAT;        //全按浮点数初始化

        yc_lst [i].qds = QDS_IV;
        if (sem_init (&yc_lst [i].mutex, 0, 1) < 0) {
            return -1;
        }

	/*
	if (i >= Ua1 && i <= U23) {
			yc_lst [i].data_type = YC_INT;
			yc_lst [i].cur_val.i_val = 100;
		}
		else if (i >= Ia1 && i <= COS8) {
			if (i == COSa1 ||
				i == COSb1 ||
				i == COSc1 ||
				i == COS1  ||
				i == COSa2 ||
				i == COSb2 ||
				i == COSc2 ||
				i == COS2  ||
				i == COSa3 ||
				i == COSb3 ||
				i == COSc3 ||
				i == COS3  ||
				i == COSa4 ||
				i == COSb4 ||
				i == COSc4 ||
				i == COS4  ||
				i == COSa5 ||
				i == COSb5 ||
				i == COSc5 ||
				i == COS5  ||
				i == COSa6 ||
				i == COSb6 ||
				i == COSc6 ||
				i == COS6  ||
				i == COSa7 ||
				i == COSb7 ||
				i == COSc7 ||
				i == COS7  ||
				i == COSa8 ||
				i == COSb8 ||
				i == COSc8 ||
				i == COS8) {
				yc_lst [i].data_type = YC_FLOAT;
				yc_lst [i].cur_val.i_val = 1.00;
			}
			else {
				yc_lst [i].data_type = YC_INT;
				yc_lst [i].cur_val.i_val = 5;
			}
		}
		else if (i >= dc_vol_1 && i <= dc_temp_2) {
			yc_lst [i].data_type = YC_INT;
			yc_lst [i].cur_val.i_val = 250;
		}
		else if (i >= Uah21 && i <= Ich138) {
			yc_lst [i].data_type = YC_INT;
			yc_lst [i].cur_val.i_val = 0;
		}
		else if (i >= Ua_THV1 && i <= Q_THV8) {
			if ((i >= Ua_THD1 && i <= Uc_THD1) ||
				(i >= Ua_THD2 && i <= Uc_THD2) ||
				(i >= Ia_THD1 && i <= Ic_THD1) ||
				(i >= Ia_THD2 && i <= Ic_THD2) ||
				(i >= Ia_THD3 && i <= Ic_THD3) ||
				(i >= Ia_THD4 && i <= Ic_THD4) ||
				(i >= Ia_THD5 && i <= Ic_THD5) ||
				(i >= Ia_THD6 && i <= Ic_THD6) ||
				(i >= Ia_THD7 && i <= Ic_THD7) ||
				(i >= Ia_THD8 && i <= Ic_THD8)) {
				yc_lst [i].data_type = YC_FLOAT;
				yc_lst [i].cur_val.f_val = 0.1;
			}
			else {
				yc_lst [i].data_type = YC_INT;
				yc_lst [i].cur_val.i_val = 0;
			}
		}
		else if (i >= U_unbalance && i <= c_flicker) {
			yc_lst [i].data_type = YC_FLOAT;
			yc_lst [i].cur_val.f_val = 20.0;
		}
		else if (i == calc_freq) {
			yc_lst [i].data_type = YC_FLOAT;
			yc_lst [i].cur_val.f_val = 50.0;
		}
		else {
			yc_lst [i].data_type = YC_FLOAT;
			yc_lst [i].cur_val.f_val = 10.0;
		}
		yc_lst [i].qds = QDS_IV;
		if (sem_init (&yc_lst [i].mutex, 0, 1) < 0) {
			return -1;
		}
	*/
		
	}	   
	for (i = 0; i < YK_NUMBER; i++) {       //yx点表初始化
		memset (&yk_lst [i], 0, sizeof (yk_t));
		yk_lst [i].ctl_mode = YK_SBO;
		yk_lst [i].ctl_step = YK_VAL;
		yk_lst [i].sel_val = DP_OPEN;
		yk_lst [i].ctl_val = DP_OPEN;
		if (sem_init (&yk_lst [i].mutex, 0, 1) < 0) {
			return -1;
		}
	}
	/*
	for (i = 0; i < YM_NUMBER; i++) {
		memset (&ym_lst [i], 0, sizeof (ym_t));
		ym_lst [i].cur_val.counter_reading = 0;
		ym_lst [i].cur_val.seq_not = 0;
		if (sem_init (&ym_lst [i].mutex, 0, 1) < 0) {
			return -1;
		}
	}
	*/
	

	return 0;
}

int init_rdb ()
/*
为各点表分配内存并做相应初始化
前向生成链表rdb_event_pool
创建线程rdb_task,该线程不停的调 用函数check_yx_event_list ();check_yc_event_list();check_yk_list();
*/
{
	yx_lst = (yx_t *)malloc (sizeof (yx_t) * YX_NUMBER);		//yx点表
	if (yx_lst == NULL) {
		return -1;
	}
	
	yc_lst = (yc_t *)malloc (sizeof (yc_t) * YC_NUMBER);        //yc点表
	if (yc_lst == NULL) {
		return -1;
	}
	
	yk_lst = (yk_t *)malloc (sizeof (yk_t) * YK_NUMBER);        //yk点表
	if (yk_lst == NULL) {
		return -1;
	}
	
/*
	ym_lst = (ym_t *)malloc (sizeof (ym_t) * YM_NUMBER);        //ym点表
	if (ym_lst == NULL) {
	return -1;
	}
*/
	if (init_rdb_table () < 0){                                 //点表初始化
		return -1;
	}

	if (init_rdb_event_pool () < 0) {                           //前向生成链表rdb_event_pool
		return -1;
	}
    if (init_rdb_task () < 0) {                                 //创建线程check_yx_event_list ();check_yc_event_list();check_yk_list();
        printf("init_rdb_task fail\n");
        return -1;
    }
    printf("init_rdb_task success\n");

	return 0;
}

static int init_rdb_event_pool ()
{
	int i;
	rdb_event_t * p_event;

	if (sem_init (&rdb_event_pool_mutex, 0, 1) < 0) {
		return -1;
	}
	for (i = 0; i < MAX_RDB_EVENT_NO; i++) {
		p_event = (rdb_event_t *)malloc (sizeof (rdb_event_t));
		if (p_event == NULL) {
			return -1;
		}
		p_event->next = rdb_event_pool;
		rdb_event_pool = p_event;
	}

	return 0;
}

static int find_valid_rdb_event_buf (rdb_event_t ** ebuf)
{
	if (ebuf == NULL) {
		return -1;
	}

	sem_timewait (&rdb_event_pool_mutex, NULL);
	
	if (rdb_event_pool == NULL) {
		* ebuf = NULL;

		sem_post (&rdb_event_pool_mutex);
		
		return -1;
	}

	* ebuf = rdb_event_pool;
	rdb_event_pool = rdb_event_pool->next;

	sem_post (&rdb_event_pool_mutex);

	return 0;
}

static int free_rdb_event_buf (rdb_event_t * ebuf)
{
	if (ebuf != NULL) {
		sem_timewait (&rdb_event_pool_mutex, NULL);
		
		ebuf->next = rdb_event_pool;
		rdb_event_pool = ebuf;

		sem_post (&rdb_event_pool_mutex);
		
		return 0;
	}
	else {
		return -1;
	}
}

static int hook_msg_flush (app_hook_func_t * ahook)
{
	/*
	hook_msg_flush的作用是将ahook->msg_buf中的内容通过ahook->hook_func发送出去，然后将内容清空
	*/
	if (ahook == NULL) {
		return -1;
	}

	if (ahook->hook_func != NULL) {
		(ahook->hook_func) (ahook->msg_buf, ahook->msg_len);
	}

	ahook->msg_len = 0;
	
	return 0;
}

static int write_hook_msg (app_hook_func_t * ahook, unsigned char * msg, int len)
{
	/*将msg消息进行相应处理*/
	if (ahook == NULL) {
		return -1;
	}
	/*如果内容过长，先处理掉*/
	if (ahook->msg_len + len > MAX_MSG_LEN) {
		hook_msg_flush (ahook);
	}
	/*内容较短，直接保存到ahook->msg_buf*/
	memcpy (ahook->msg_buf + ahook->msg_len, msg, len);
	ahook->msg_len += len;

	return 0;
}

static int check_yx_event_list ()
{
	int i;
	yx_t * yx_entry  = yx_lst;          //yx点表（数组），每一个点背后有一个rdb_event链表和若干信息
	rdb_event_t * p_event;              //yx事件链表的一个Node
	yx_event_t * yx_e;                  //Node里面的info，13 Byte
	internal_msg_t * p_i_msg;           //内部消息。包含四个部分：type;sub_type;content_len;content[4];
	unsigned char sbuf [MAX_MSG_LEN];
	app_hook_func_t * ahook;            //链表的一个Node
	data_hook_t * dhook;                //链表的一个Node

	p_i_msg = (internal_msg_t *)sbuf;
	p_i_msg->type = MSG_CHK_EVENT;
	p_i_msg->sub_type = MSG_SUB_YX;
	for (i = 0; i < YX_NUMBER; i++) {   //按下标访问yx点表
		while (1) {	                    //访问某一节点链表下的各Node
			GET_EVENT_FROM_HEAD (&yx_entry [i], p_event);
			if (p_event == NULL) {		//该点rdb_event链表为空，处理下一点
				break;
			}
			yx_e = (yx_event_t *)p_i_msg->content;//让yx_e指向p_i_msg->content
			memcpy (yx_e, &p_event->yx_e, sizeof (p_event->yx_e));
			p_i_msg->content_len = sizeof (p_event->yx_e);

			/* 保存SOE事件 */
	//		send_soe_msg (p_event->yx_e.data_no, &p_event->yx_e.time_val, p_event->yx_e.event_val);

			free_rdb_event_buf (p_event);//将内存还给rdb_event_pool
#if 0
			printf ("yx event %d, %d\n", p_event->yx_e.data_no, p_event->yx_e.event_val);
#endif
			/* send to all protocols */
			dhook = yx_entry [i].p_hook;
			while (dhook != NULL) {
				yx_e->data_no = dhook->proto_data_no;	/* convert to protocol data index */
				ahook = app_hook_func_lst;
				while (ahook != NULL) {
					if (dhook->app_id == ahook->app_id) {
						write_hook_msg (ahook, (unsigned char *)p_i_msg, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN);
						_DPRINTF ("hook msg send to %d\n", ahook->app_id);
						break;
					}
					ahook = ahook->next;
				}
				dhook = dhook->next;
			}
		}
	}

	ahook = app_hook_func_lst;
	while (ahook != NULL) {
		hook_msg_flush (ahook);         //执行函数指针
		ahook = ahook->next;
	}

	return 0;
}

static int check_yc_event_list ()
{
	int i;
	yc_t * yc_entry = yc_lst;
	rdb_event_t * p_event;
	yc_event_t * yc_e;
	internal_msg_t * p_i_msg;
	unsigned char sbuf [MAX_MSG_LEN];
	app_hook_func_t * ahook;
	data_hook_t * dhook;

	p_i_msg = (internal_msg_t *)sbuf;
	p_i_msg->type = MSG_CHK_EVENT;
	p_i_msg->sub_type = MSG_SUB_YC;
	for (i = 0; i < YC_NUMBER; i++) {
		while (1) {
			GET_EVENT_FROM_HEAD (&yc_entry [i], p_event);
			if (p_event == NULL) {
				break;
			}
			yc_e = (yc_event_t *)p_i_msg->content;
			memcpy (yc_e, &p_event->yc_e, sizeof (p_event->yc_e));
			p_i_msg->content_len = sizeof (p_event->yc_e);
			free_rdb_event_buf (p_event);
#if 0
			if (p_event->yc_e.data_type == YC_INT)
				printf ("yc event %d, %d\n", p_event->yc_e.data_no, p_event->yc_e.event_val.i_val);
			else
				printf ("yc event %d, %f\n", p_event->yc_e.data_no, p_event->yc_e.event_val.f_val);
#endif
			/* send to all protocols */
			dhook = yc_entry [i].p_hook;
			while (dhook != NULL) {
				yc_e->data_no = dhook->proto_data_no;	/* convert to protocol data index */
				ahook = app_hook_func_lst;
				while (ahook != NULL) {
					if (dhook->app_id == ahook->app_id) {
						write_hook_msg (ahook, (unsigned char *)p_i_msg, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN);
						_DPRINTF ("hook msg send to %d\n", ahook->app_id);
						break;
					}
					ahook = ahook->next;
				}		
				dhook = dhook->next;
			}
		}
	}

	ahook = app_hook_func_lst;
	while (ahook != NULL) {
		hook_msg_flush (ahook);
		ahook = ahook->next;
	}

	return 0;
}

static int check_yk_list ()
{
	int i;
	yk_t * yk_entry = yk_lst;

	for (i = 0; i < YK_NUMBER; i++) {
		/* 获取互斥信号量 */
		sem_timewait (&yk_entry [i].mutex, NULL);

		switch (yk_entry [i].ctl_step) {
		case YK_VAL:
			break;
		case YK_SEL:
			if (yk_entry [i].sel_timer > 0) {
				yk_entry [i].sel_timer -= RDB_CHECK_TIME;
				if (yk_entry [i].sel_timer <= 0) {
					yk_unselect (i, RDB_APP_ID);
					_DPRINTF ("yk(%d) selected(%s) by app(%d) timeout\n", i, yk_entry [i].ctl_val == DP_OPEN ? "OPEN" : "CLOSE", yk_entry [i].app_id);
				}
			}
			break;
		case YK_OPER:
			if (yk_entry [i].oper_timer > 0) {
				yk_entry [i].oper_timer -= RDB_CHECK_TIME;
				if (yk_entry [i].oper_timer <= 0) {
					yk_entry [i].ctl_step = YK_VAL;
					_DPRINTF ("yk(%d) operate(%s) by app(%d) timeout\n", i, yk_entry [i].ctl_val == DP_OPEN ? "OPEN" : "CLOSE", yk_entry [i].app_id);
				}
			}
			break;
		case YK_DONE:
			yk_entry [i].ctl_step = YK_VAL;
			break;
		default:
			break;
		}

		/* 释放互斥信号量 */
		sem_post (&yk_entry [i].mutex);
	}

	return 0;
}

int reg_rdb_hook (
	int app_id,
	hook_func_t func_ptr
	)
{
	/*
		app_id = iec104_server.app_id; 
		func_ptr=send_iec104_internal_udp_data
	*/
	app_hook_func_t * phook;
	
	if (app_id == -1 ||
		func_ptr == NULL) {
		return -1;
	}

	if (app_reg_num >= MAX_RDB_REGED_APP) {
		_DPRINTF ("reg_rdb_hook: max reged app error\n");
		return -1;
	}

	phook = (app_hook_func_t *)malloc (sizeof (app_hook_func_t) * 1);
	if (phook == NULL) {
		_DPRINTF ("reg_rdb_hook: malloc error\n");
		return -1;
	}
	memset (phook, 0, sizeof (app_hook_func_t));

	phook->app_id = app_id;
	phook->hook_func = func_ptr;
	phook->next = NULL;

	if (app_hook_func_lst == NULL) {
		app_hook_func_lst = phook;
	}
	else {
		phook->next = app_hook_func_lst;
		app_hook_func_lst = phook;
	}

	app_reg_num++;

	return 0;
}

int reg_rdb_data (
	int app_id,
	int rdb_type,
	unsigned int rdb_no,
	unsigned int proto_no
	)
	/*
	新建立了一个名为phook的节点，并将该节点加入到原点链表的首部
	app_id = iec104_server.app_id;, RDB_TYPE_YX=0x01, i, i
	*/
{
	data_hook_t * phook;
	yx_t * yx_entry = yx_lst;
	yc_t * yc_entry = yc_lst;
	//ym_t * ym_entry = ym_lst;
	
	if (app_id < 0) {
		return -1;
	}

	if (rdb_type == RDB_TYPE_YX) {
		if (rdb_no >= YX_NUMBER) {
			_DPRINTF ("reg_rdb_data: yx index %d, over max index %d\n", rdb_no, YX_NUMBER);
			return -1;
		}
		else {
			phook = (data_hook_t *)malloc (sizeof (data_hook_t) * 1);
			if (phook == NULL) {
				_DPRINTF ("reg_rdb_data: malloc error\n");
				return -1;
			}
			phook->app_id = app_id;
			phook->proto_data_no = proto_no;
			phook->next = NULL;
			if (yx_entry [rdb_no].p_hook == NULL) {
				yx_entry [rdb_no].p_hook = phook;
			}
			else {
				phook->next = yx_entry [rdb_no].p_hook;
				yx_entry [rdb_no].p_hook = phook;
			}
		}
	}
	else if (rdb_type == RDB_TYPE_YC) {
		if (rdb_no >= YC_NUMBER) {
			_DPRINTF ("reg_rdb_data: yc index %d, over max index %d\n", rdb_no, YC_NUMBER);
			return -1;
		}
		else {
			phook = (data_hook_t *)malloc (sizeof (data_hook_t) * 1);
			if (phook == NULL) {
				_DPRINTF ("reg_rdb_data: malloc error\n");
				return -1;
			}
			phook->app_id = app_id;
			phook->proto_data_no = proto_no;
			phook->next = NULL;
			if (yc_entry [rdb_no].p_hook == NULL) {
				yc_entry [rdb_no].p_hook = phook;
			}
			else {
				phook->next = yc_entry [rdb_no].p_hook;
				yc_entry [rdb_no].p_hook = phook;
			}
		}
	}
	else if (rdb_type == RDB_TYPE_YM) {
		/*
		if (rdb_no >= YM_NUMBER) {
			_DPRINTF ("reg_rdb_data: ym index %d, over max index %d\n", rdb_no, YM_NUMBER);
			return -1;
		}
		else {
			phook = (data_hook_t *)malloc (sizeof (data_hook_t) * 1);
			if (phook == NULL) {
				_DPRINTF ("reg_rdb_data: malloc error\n");
				return -1;
			}
			phook->app_id = app_id;
			phook->proto_data_no = proto_no;
			phook->next = NULL;
			if (ym_entry [rdb_no].p_hook == NULL) {
				ym_entry [rdb_no].p_hook = phook;
			}
			else {
				phook->next = ym_entry [rdb_no].p_hook;
				ym_entry [rdb_no].p_hook = phook;
			}
		}*/
	}
		
		
	else {
		_DPRINTF ("reg_rdb_data: type error %d\n", rdb_type);
		return -1;
	}
	
	return 0;
}

int yx_set_value (
	unsigned int yx_no,		/* 从0到YX_NUMBER-1 */
	unsigned int * val,		/* 双点入库，DP_OPEN or DP_CLOSE */
	time_type * ts,			/* 时标 */
	unsigned char cos_soe_flag
	)
{
	int dpi, ret = -1;
	yx_t * yx_entry;
	rdb_event_t * p_event = NULL;

	if (yx_no >= YX_NUMBER) {
		return -1;
	}
	if (val == NULL/* || ts == NULL*/) {
		return -1;
	}
	dpi = * val;
	if (dpi != DP_OPEN && dpi != DP_CLOSE) {
		return -1;
	}
#if 0
	if ((!cos_soe_flag) &&
		!(cos_soe_flag & YX_COS_FLAG) &&
		!(cos_soe_flag & YX_SOE_FLAG)) {
		return -1;
	}
#endif
	yx_entry = yx_lst + yx_no;

    if ((int)yx_entry->cur_val == dpi) {
		return 0;
	}

	/* 获取互斥信号量 */
	sem_timewait (&yx_entry->mutex, NULL);

	yx_entry->cur_val = dpi;

	if (cos_soe_flag) {
		find_valid_rdb_event_buf (&p_event);//从rdb_event_pool头部取出一个Node给p_event，rdb_event_pool往后挪一个Node
		if (p_event != NULL) {
			p_event->yx_e.data_no = yx_no;
			p_event->yx_e.event_val = dpi;
			memcpy (&p_event->yx_e.time_val, ts, sizeof (time_type));
			p_event->yx_e.cos_soe_flag = cos_soe_flag;
			p_event->next = NULL;
			ADD_EVENT_TO_TAIL(yx_entry, p_event);
			ret = 0;
		}
	}

	/* 释放互斥信号量 */
	sem_post (&yx_entry->mutex);

	return ret;
}

int yx_get_value (
	int app_id,
	unsigned int begin_no,	/* 从0到YX_NUMBER-1 */
	unsigned int num,		/* 从1到YX_NUMBER */
	unsigned int data []	/* 数据缓存数组,返回遥信数据值 */
	)
{
	unsigned int i;
	yx_t * yx_entry = yx_lst;

	if (begin_no + num > YX_NUMBER || num < 1) {
		return -1;
	}
	if (data == NULL) {
		return -1;
	}

	for (i = 0; i < num; i++) {
		data [i] = yx_entry [begin_no + i].cur_val;
	}

	return i;
}

int yx_get_value_proto (
	int app_id,
	unsigned int begin_no,	/* 从0开始，协议点号 */
	unsigned int num,		/* 从1到YX_NUMBER */
	unsigned int data []	/* 数据缓存数组,返回已注册的数据 */
	)
{
	unsigned int i, j, proto_no;
	yx_t * yx_entry = yx_lst;
	data_hook_t * dhook;

	if (begin_no + num > YX_NUMBER || num < 1) {
		return -1;
	}
	if (data == NULL) {
		return -1;
	}

	i = 0;
	j = begin_no;
	proto_no = begin_no;
	while ((i < num) && (j < YX_NUMBER)) {
		dhook = yx_entry [j].p_hook;
		while (dhook != NULL) {
			if (dhook->app_id == app_id) {
				if (dhook->proto_data_no == proto_no) {
					data [i] = yx_entry [j].cur_val;
					
					proto_no++;
					i++;
				}
				break;
			}
			dhook = dhook->next;
		}
		j++;
	}

	return i;
}

int yx_get_total_num (
	int app_id,
	unsigned int * num		/* 返回遥信总数目 */
	)
{
	if (num != NULL) {
		* num = YX_NUMBER;
		return 0;
	}
	else {
		return -1;
	}
}

int yx_get_total_num_proto (
	int app_id,
	unsigned int * num		/* 返回遥信总数目 */
	)
{
	yx_t * yx_entry = yx_lst;
	data_hook_t * dhook;
	int i, yx_num;
	
	if (num != NULL) {
		yx_num = 0;
		for (i = 0; i < YX_NUMBER; i++) {
			dhook = yx_entry [i].p_hook;
			while (dhook != NULL) {
				if (dhook->app_id == app_id) {
					yx_num++;
					break;
				}
				dhook = dhook->next;
			}
		}
		* num = yx_num;
		return 0;
	}
	else {
		return -1;
	}
}

#define YC_I_DB			5

int yc_set_value (
	unsigned int yc_no,		/* 从0到YC_NUMBER-1 */
	yc_data_type * val,		/* 遥测值 */
	unsigned char qds,		/* 品质 */
	time_type * ts,			/* 时标，可以为NULL */
	int	b_event				/* 是否触发事件 */
	)
{
	int ret = -1;
	yc_t * yc_entry;
	rdb_event_t * p_event = NULL;
	yc_data_type old_data;
	Int32 i_delt;

	if (yc_no >= YC_NUMBER) {
		return -1;
	}
	if (val == NULL) {
		return -1;
	}

	yc_entry = yc_lst + yc_no;

	if (yc_entry->data_type == YC_INT &&
		yc_entry->cur_val.i_val == val->i_val) {
		return 0;
	}

	/* 获取互斥信号量 */
	sem_timewait (&yc_entry->mutex, NULL);

	if (yc_entry->data_type == YC_INT) {
		old_data.i_val = yc_entry->cur_val.i_val;
		yc_entry->cur_val.i_val = val->i_val;
	}
	else {
		old_data.f_val = yc_entry->cur_val.f_val;
		yc_entry->cur_val.f_val = val->f_val;
	}
	yc_entry->qds = qds;

	if (b_event) {
		do {
			if (yc_entry->data_type == YC_INT) {
				i_delt = old_data.i_val - val->i_val;
				if (i_delt > -YC_I_DB && i_delt < YC_I_DB) {
					break;
				}
			}
			find_valid_rdb_event_buf (&p_event);
			if (p_event != NULL) {
				p_event->yc_e.data_no = yc_no;
				if (yc_entry->data_type == YC_INT) {
					p_event->yc_e.event_val.i_val = val->i_val;
					p_event->yc_e.data_type = YC_INT;
				}
				else {
					p_event->yc_e.event_val.f_val = val->f_val;
					p_event->yc_e.data_type = YC_FLOAT;
				}
				p_event->yc_e.qds = qds;
				if (ts != NULL) {
					memcpy (&p_event->yc_e.time_val, ts, sizeof (time_type));
				}
				else {
					memset (&p_event->yc_e.time_val, 0, sizeof (time_type));
				}
				p_event->next = NULL;
				ADD_EVENT_TO_TAIL(yc_entry, p_event);
				ret = 0;
			}
		} while (0);
	}

	/* 释放互斥信号量 */
	sem_post (&yc_entry->mutex);

	return ret;
}

int yc_get_value (
	int app_id,
	unsigned int begin_no,	/* 从0到YC_NUMBER-1 */
	unsigned int num,		/* 从1到YC_NUMBER */
	yc_data_type data [],	/* 数据缓存数组,返回数据库值 */
	unsigned char qds [],	/* 品质 */
	unsigned char data_type []/* 返回遥测值数据类型，YC_INT or YC_FLOAT */
	)
{
	unsigned int i;
	yc_t * yc_entry = yc_lst;

	if ((begin_no + num > YC_NUMBER) || (num < 1)) {
		return -1;
	}
	if (data == NULL) {
		return -1;
	}

	for (i = 0; i < num; i++) {
		if (yc_entry [i].data_type == YC_INT) {
			data [i].i_val = yc_entry [begin_no + i].cur_val.i_val;
		}
		else {
			data [i].f_val = yc_entry [begin_no + i].cur_val.f_val;
		}
		qds [i] = yc_entry [begin_no + i].qds;
		data_type [i] = yc_entry [begin_no + i].data_type;
	}

	return i;
}

int yc_get_value_proto (
	int app_id,
	unsigned int begin_no,	/* 从0到YC_NUMBER-1 */
	unsigned int num,		/* 从1到YC_NUMBER */
	yc_data_type data [],	/* 数据缓存数组,返回已注册的数据 */
	unsigned char qds [],	/* 品质 */
	unsigned char data_type []/* 返回遥测值数据类型，YC_INT or YC_FLOAT */
	)
{
	unsigned int i, j, proto_no;
	yc_t * yc_entry = yc_lst;
	data_hook_t * dhook;

	if ((begin_no + num > YC_NUMBER) || (num < 1)) {
		return -1;
	}
	if (data == NULL) {
		return -1;
	}

	i = 0;
	j = begin_no;
	proto_no = begin_no;
	while ((i < num) && (j < YC_NUMBER)) {
		dhook = yc_entry [j].p_hook;
		while (dhook != NULL) {
			if (dhook->app_id == app_id) {
				if (dhook->proto_data_no == proto_no) {
					if (yc_entry [j].data_type == YC_INT) {
						data [i].i_val = yc_entry [j].cur_val.i_val;
					}
					else {
						data [i].f_val = yc_entry [j].cur_val.f_val;
					}
					qds [i] = yc_entry [j].qds;
					data_type [i] = yc_entry [j].data_type;
					
					proto_no++;
					i++;
				}
				break;
			}
			dhook = dhook->next;
		}
		j++;
	}

	return i;
}

int yc_get_total_num (
	int app_id,
	unsigned int * num		/* 返回遥测总数目 */
	)
{
	if (num != NULL) {
		* num = YC_NUMBER;
		return 0;
	}
	else {
		return -1;
	}
}

int yc_get_total_num_proto (
	int app_id,
	unsigned int * num		/* 返回遥测总数目 */
	)
{
	yc_t * yc_entry = yc_lst;
	data_hook_t * dhook;
	int i, yc_num;
	
	if (num != NULL) {
		yc_num = 0;
		for (i = 0; i < YC_NUMBER; i++) {
			dhook = yc_entry [i].p_hook;
			while (dhook != NULL) {
				if (dhook->app_id == app_id) {
					yc_num++;
					break;
				}
				dhook = dhook->next;
			}
		}
		* num = yc_num;
		return 0;
	}
	else {
		return -1;
	}
}

int yk_select (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
	int app_id				/* 遥控发起者的id */
	)
{
	unsigned int ctl_val;
	int ret = -1;
	yk_t * yk_entry;
//	time_type ts;

	if (yk_no >= YK_NUMBER) {
		return -1;
	}

	ctl_val = val;
	if (ctl_val != DP_OPEN && ctl_val != DP_CLOSE) {
		return -1;
	}

	yk_entry = yk_lst + yk_no;

	/* 获取互斥信号量 */
	sem_timewait (&yk_entry->mutex, NULL);

	if (yk_entry->ctl_step == YK_VAL) {
		yk_entry->ctl_step = YK_SEL;
		yk_entry->sel_val = ctl_val;
		yk_entry->sel_timer = YK_SEL_TO * 1000000;
		yk_entry->app_id = app_id;
		ret = 0;
		/* 开放继电器电源 
        if (soft_yp_value->remote_control_soft_yp) {	// 经远方就地软压板
			//bin_out_select (yk_no, ctl_val - 1, 1);
		}*/
		_DPRINTF ("yk(%d) selected(%s) by app(%d)\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
		/* 保存事件 */
	//	get_time_sec_msec (&ts);
	//	send_co_msg (yk_no, &ts, CO_SELECT, ctl_val /* 双点 */);
	}
	else {
		_DPRINTF ("yk(%d) selected(%s) by app(%d) check failed\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
	}

	/* 释放互斥信号量 */
	sem_post (&yk_entry->mutex);

	return ret;
}

int yk_unselect (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	int app_id				/* 遥控发起者的id */
	)
{
	int ret = -1;
	yk_t * yk_entry;
//	time_type ts;

	if (yk_no >= YK_NUMBER) {
		return -1;
	}

	yk_entry = yk_lst + yk_no;

	/* 获取互斥信号量 */
	sem_timewait (&yk_entry->mutex, NULL);

	if (yk_entry->ctl_step == YK_SEL) {
		if ((yk_entry->app_id == app_id) ||
			(app_id == RDB_APP_ID)) {
			yk_entry->ctl_step = YK_VAL;
			yk_entry->sel_timer = 0;
			yk_entry->app_id = -1;
			ret = 0;
			/* 关闭继电器电源 */
	//		if (soft_yp_value->remote_control_soft_yp) {	/* 经远方就地软压板 */
//				bin_out_select (yk_no, 0, 0);
		//	}
			_DPRINTF ("yk(%d) unselected by app(%d)\n", yk_no, app_id);
			/* 保存事件 */
	//		get_time_sec_msec (&ts);
		//	send_co_msg (yk_no, &ts, CO_CANCEL, DP_OPEN /* 单点 */);
		}
	}

	/* 释放互斥信号量 */
	sem_post (&yk_entry->mutex);

	return ret;
}

int yk_operate (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
	int app_id				/* 遥控发起者的id */
	)
{
	unsigned int ctl_val;
	int ret = -1;
	yk_t * yk_entry;
//	time_type ts;

	if (yk_no >= YK_NUMBER) {
		return -1;
	}

	ctl_val = val;
	if (ctl_val != DP_OPEN && ctl_val != DP_CLOSE) {
		return -1;
	}

	yk_entry = yk_lst + yk_no;

	/* 获取互斥信号量 */
	sem_timewait (&yk_entry->mutex, NULL);

	if (yk_entry->ctl_step == YK_SEL &&
		yk_entry->app_id == app_id &&
		yk_entry->sel_val == ctl_val) {
		yk_entry->ctl_val = ctl_val;
		yk_entry->ctl_step = YK_OPER;
		yk_entry->sel_timer = 0;
		yk_entry->oper_timer = YK_OPER_TO * 1000000;
		_DPRINTF ("yk(%d) operate(%s) by app(%d)\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
	}
	else {
		_DPRINTF ("yk(%d) operate(%s) by app(%d) check failed\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
	}

	/* 释放互斥信号量 */
	sem_post (&yk_entry->mutex);

	return ret;
}

int yk_done (
	unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
	unsigned char ret		/* 执行结果 */
	)
{
	yk_t * yk_entry;

	if (yk_no >= YK_NUMBER) {
		return -1;
	}

	yk_entry = yk_lst + yk_no;

	/* 获取互斥信号量 */
	/*sem_timewait (&yk_entry->mutex, NULL);*/

	if (yk_entry->ctl_step == YK_OPER) {
		/* anyway done! */
		yk_entry->ctl_step = YK_DONE;
		yk_entry->oper_timer = 0;
		_DPRINTF ("yk(%d) operate(%s) by app(%d) done, ret %d\n", yk_no, yk_entry->ctl_val == DP_OPEN ? "OPEN" : "CLOSE", yk_entry->app_id, ret);
	}
	else {
		_DPRINTF ("yk(%d) operate(%s) by app(%d) done check failed\n", yk_no, yk_entry->ctl_val == DP_OPEN ? "OPEN" : "CLOSE", yk_entry->app_id);
	}

	/* 释放互斥信号量 */
	/*sem_post (&yk_entry->mutex);*/

	return 0;
}

int yk_get_total_num (
	unsigned int * num		/* 返回遥控总数目 */
	)
{
	if (num != NULL) {
		* num = YK_NUMBER;
		return 0;
	}
	else {
		return -1;
	}
}

int rdb_yx_show ()
{
	int i;
	yx_t * yx_entry = yx_lst;
	data_hook_t * dhook;

	printf ("  RDB YX Data:\n");
	for (i = 0; i < YX_NUMBER; i++) {
		dhook = yx_entry [i].p_hook;
		printf ("    %04d, value 0x%02x, registered by:\n", i, yx_entry [i].cur_val);
		while (dhook != NULL) {
			printf ("                  App ID %04d, Proto No %04d\n", dhook->app_id, dhook->proto_data_no);
			dhook = dhook->next;
		}
	}

	return 0;
}

int rdb_yc_show ()
{
	int i;
	yc_t * yc_entry = yc_lst;
	data_hook_t * dhook;

	printf ("  RDB YC Data:\n");
	for (i = 0; i < YC_NUMBER; i++) {
		dhook = yc_entry [i].p_hook;
		if (yc_entry [i].data_type == YC_INT) {
			printf ("    %04d, value 0x%08x, qds %02x registered by:\n", i, yc_entry [i].cur_val.i_val, yc_entry [i].qds);
		}
		else {
			printf ("    %04d, value %7.2f, qds %02x registered by:\n", i, yc_entry [i].cur_val.f_val, yc_entry [i].qds);
		}
		while (dhook != NULL) {
			printf ("                  App ID %04d, Proto No %04d\n", dhook->app_id, dhook->proto_data_no);
			dhook = dhook->next;
		}
	}
	
	return 0;
}

/*
int rdb_ym_show ()
{
int i;
ym_t * ym_entry = ym_lst;
data_hook_t * dhook;

printf ("  RDB YM Data:\n");
for (i = 0; i < YM_NUMBER; i++) {
dhook = ym_entry [i].p_hook;
printf ("    %04d, value 0x%08x, seq_not %02x registered by:\n", i,
ym_entry [i].cur_val.counter_reading, ym_entry [i].cur_val.seq_not);
while (dhook != NULL) {
printf ("                  App ID %04d, Proto No %04d\n", dhook->app_id, dhook->proto_data_no);
dhook = dhook->next;
}
}

return 0;
}
*/




