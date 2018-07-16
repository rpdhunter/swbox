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

//#include "global_define.h"
//#include "base_functions.h"
#include "rdb.h"
//#include "params_settings.h"
//#include "bin_out.h"
//#include "monitor_status.h"
//#include "record_log.h"

#define RDB_APP_ID			5

static yx_t * yx_lst;		/* YX_NUMBER */
static yc_t * yc_lst;		/* YC_NUMBER */
static yk_t * yk_lst;		/* YK_NUMBER */
static dz_t * dz_lst;		/* DZ_NUMBER */

static app_hook_func_t * app_hook_func_lst = NULL;
static int app_reg_num = 0;
static sem_t rdb_event_pool_mutex;
static rdb_event_t * rdb_event_pool = NULL;
rdb_udp_t * rdb_udp = NULL;


static void rdb_task (void *argc);
static int init_rdb_task ();
static int init_rdb_event_pool ();
static int find_valid_rdb_event_buf (rdb_event_t ** ebuf);
static int free_rdb_event_buf (rdb_event_t * ebuf);
static int check_yx_event_list ();
static int check_yc_event_list ();
static int check_yk_list ();
static int init_external_com ();
static int init_rdb_recv_task();
static void rdb_recv_task (void * arg);

static void rdb_task (void * arg)
{
	while (1) {
		usleep (RDB_CHECK_TIME);	/* 100ms */
		
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
	float init_dz_val;

    for (i = 0; i < YX_NUMBER; i++) {		//yx点表初始化，默认值为DP_OPEN | DIQ_IV;
        memset (&yx_lst [i], 0, sizeof (yx_t));		//内存初始化
        yx_lst [i].cur_val = DP_OPEN | DIQ_IV;		//设置cur_val
        if (sem_init (&yx_lst [i].mutex, 0, 1) < 0) {	//信号量初始化
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

	 for (i = 0; i < DZ_NUMBER; i++) {       //dz点表初始化
        memset (&dz_lst [i], 0, sizeof (dz_t));
        dz_lst [i].tag = DZ_FLOAT;
        dz_lst [i].data_len = sizeof(float);
		init_dz_val = 0.01;
		memcpy(dz_lst [i].data_buf,&init_dz_val,sizeof(init_dz_val));
//		dz_lst [i].sn = 1;
//		dz_lst [i].sn_min = 1;
//		dz_lst [i].sn_max = 1;
        if (sem_init (&dz_lst [i].mutex, 0, 1) < 0) {
            return -1;
        }
    }
  
    return 0;
}

int init_rdb ()
/*
为各点表分配内存并做相应初始化
前向生成链表rdb_event_pool
创建线程rdb_task,该线程不停的调 用函数check_yx_event_list ();check_yc_event_list();check_yk_list();
*/
{
    /*动态分配四遥点表*/
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

	dz_lst = (dz_t *)malloc (sizeof (dz_t) * DZ_NUMBER);        //dz点表
    if (dz_lst == NULL) {
        return -1;
    }

    rdb_udp = (rdb_udp_t *)malloc(sizeof (rdb_udp_t));
    if (init_rdb_table () < 0){                                 //点表初始化
        return -1;
    }

    if (init_rdb_event_pool () < 0) {                           //前向生成链表rdb_event_pool
        return -1;
    }

    if(proto_rdb_reg_data (2404) != 0){                         //初始化注册rdb
        return -1;
    }

    if (init_external_com () < 0) {
        return -1;
    }
    if (init_rdb_task () < 0) {
        return -1;
    }

    if(init_rdb_recv_task() < 0){
        return -1;
    }

    return 0;
}

static int init_rdb_recv_task()
{
    pthread_t tid;

    tid = thread_create ("rdb recv task", DEF_TASK_STACK_SIZE * 8, PRIO_SYNC_TASK, rdb_recv_task, NULL);
    if (tid == -1) {
        return -1;
    }

    return 0;
}

static int init_rdb_event_pool ()
{
    int i;
    rdb_event_t * p_event;

    if (sem_init (&rdb_event_pool_mutex, 0, 1) < 0) {
        return -1;
    }
    for (i = 0; i < MAX_RDB_EVENT_NO; i++) {	//循环加入单链表Node
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
	yk_t * yk_entry = yk_lst;
	dz_t * dz_entry = dz_lst;
	
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
    else if (rdb_type == RDB_TYPE_YK) {
        if (rdb_no >= YK_NUMBER) {
            _DPRINTF ("reg_rdb_data: yk index %d, over max index %d\n", rdb_no, YK_NUMBER);
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
            if (yk_entry [rdb_no].p_hook == NULL) {
                yk_entry [rdb_no].p_hook = phook;
            }
            else {
                phook->next = yk_entry [rdb_no].p_hook;
                yk_entry [rdb_no].p_hook = phook;
            }
        }
    }

	
	else if (rdb_type == RDB_TYPE_DZ) {
		if (rdb_no >= DZ_NUMBER) {
		    _DPRINTF ("reg_rdb_data: dz index %d, over max index %d\n", rdb_no, DZ_NUMBER);
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
		    if (dz_entry [rdb_no].p_hook == NULL) {
		        dz_entry [rdb_no].p_hook = phook;
		    }
		    else {
		        phook->next = dz_entry [rdb_no].p_hook;
		        dz_entry [rdb_no].p_hook = phook;
		    }
		}
	}
    else {
        _DPRINTF ("reg_rdb_data: type error %d\n", rdb_type);
        return -1;
    }

    return 0;
}
int proto_rdb_reg_data (int com_no)
{
	int i, rdb_no, proto_no;
	int protect_num;

/* YX */
	proto_no = 0;

	/* YX board 1 */
	for (
		 rdb_no = device_status; 
		 rdb_no <= num_of_yx; 
		 rdb_no++, proto_no++
		) {
		reg_rdb_data (com_no, RDB_TYPE_YX, rdb_no, proto_no);
	}

/* YC */
	proto_no = 0;

	/* voltage */

	for (
		 rdb_no = TEV1_amplitude; 
		 rdb_no <= AE2_biased_adv; 
		 rdb_no++, proto_no++
		) {
		reg_rdb_data (com_no, RDB_TYPE_YC, rdb_no, proto_no);
	}

	for (
	 rdb_no = TEV1_gain; 
	 rdb_no <= AE2_biased; 
	 rdb_no++, proto_no++
	) {
		reg_rdb_data (com_no, RDB_TYPE_YC, rdb_no, proto_no);
	}

	for (
	 rdb_no = version; 
	 rdb_no <= num_of_yc; 
	 rdb_no++, proto_no++
	) {
		reg_rdb_data (com_no, RDB_TYPE_YC, rdb_no, proto_no);
	}
	
/* YK */
	proto_no = 0;
	for(rdb_no = start;rdb_no < num_of_yk;rdb_no++,proto_no++)
	{
		reg_rdb_data (com_no, RDB_TYPE_YK, rdb_no, proto_no);
	}
/* DZ */
	proto_no = 0;
	for(rdb_no = TEV1_gain_sdz;rdb_no < num_of_sdz;rdb_no++,proto_no++)
	{
		reg_rdb_data (com_no, RDB_TYPE_DZ, rdb_no, proto_no);
	}
	return 0;
}

int yx_set_value (
        unsigned int yx_no,		/* 从0到YX_NUMBER-1 */
        unsigned int * val,		/* 双点入库，DP_OPEN or DP_CLOSE */
        time_type * ts,			/* 时标 */
        unsigned char cos_soe_flag,
        int flag                /* 是否发送给QT的rdb */
        )
{
	Uint32 dpi;
	int ret = -1;
	yx_t * yx_entry;
	rdb_event_t * p_event = NULL;
	char s_msg [100];
	internal_msg_t * p_i_msg;
	unsigned char sbuf [MAX_MSG_LEN];
	rdb_yx_param_t * yx_temp;
	int i;
	
	if (yx_no >= YX_NUMBER) {
		return -1;
	}
	if (val == NULL/* || ts == NULL*/) {
		return -1;
	}
#if 0
    dpi = * val & 0x03;
#else
    dpi = * val;			/*获取要设置的遥信值*/
#endif
#if 0
    if (dpi != DP_OPEN && dpi != DP_CLOSE) {
        dpi |= DIQ_IV;
    }
#endif
#if 0
    if ((!cos_soe_flag) &&
            !(cos_soe_flag & YX_COS_FLAG) &&
            !(cos_soe_flag & YX_SOE_FLAG)) {
        return -1;
    }
#endif
    yx_entry = yx_lst + yx_no;

    if (yx_entry->cur_val == dpi) {
        return 0;
    }

	/* 初始化时不打印变位信息 */
	if ((yx_entry->cur_val & (DIQ_IV | DIQ_NT)) != (DIQ_IV | DIQ_NT)) {
//		printf("yx_no %d\n",yx_no);
	
	}
	if(flag){
		/*将参数发送给QT的rdb*/
		p_i_msg = (internal_msg_t *)sbuf;
		p_i_msg->type = MSG_YX_EVENT;
		p_i_msg->sub_type = MSG_NONE_TYPE;
		yx_temp = (rdb_yx_param_t *)p_i_msg->content;
		rdb_udp->yx_param.yx_no = yx_no;
		rdb_udp->yx_param.cos_soe_flag = cos_soe_flag;
		memcpy (rdb_udp->yx_param.val, val, sizeof (unsigned int));
		memcpy (rdb_udp->yx_param.ts, ts, sizeof (time_type));
		memcpy (yx_temp,&rdb_udp->yx_param,sizeof(rdb_yx_param_t));
		p_i_msg->content_len = sizeof(rdb_yx_param_t);
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		printf("Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");		
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
        int	b_event,			/* 是否触发事件 */
        int flag				/* 是否发送给QT的rdb */
        )
{
	int ret = -1;
	yc_t * yc_entry;
	rdb_event_t * p_event = NULL;
	yc_data_type old_data;
	Int32 i_delt;			/*整形变量增量*/
	internal_msg_t * p_i_msg;
	unsigned char sbuf [MAX_MSG_LEN];
	rdb_yc_param_t * yc_temp;
	int i;

    if (yc_no >= YC_NUMBER) {
        return -1;
    }
    if (val == NULL) {
        return -1;
    }
    if(flag){
        p_i_msg = (internal_msg_t *)sbuf;
        p_i_msg->type = MSG_YC_EVENT;
        p_i_msg->sub_type = MSG_NONE_TYPE;
        yc_temp = (rdb_yc_param_t *)p_i_msg->content;

		/*将参数发送给QT的rdb*/
		rdb_udp->yc_param.yc_no = yc_no;
		rdb_udp->yc_param.qds = qds;
		rdb_udp->yc_param.b_event = b_event;
		memcpy (rdb_udp->yc_param.val, val, sizeof (yc_data_type));
		memcpy (rdb_udp->yc_param.ts, ts, sizeof (time_type));
		memcpy (yc_temp,&rdb_udp->yx_param,sizeof(rdb_yc_param_t));
		p_i_msg->content_len = sizeof(rdb_yc_param_t);
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		printf("Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");	
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
int dz_set_value (
	rdb_dz_param_t * dz,
	int flag					/* 是否发送给QT的rdb */
	)
{
	dz_t * dz_entry;
	int ret = -1, proto_no = -1;
	internal_msg_t * p_i_msg;
	unsigned char sbuf [MAX_MSG_LEN];
	rdb_yc_param_t * dz_temp;
	data_hook_t * dhook;
	int i;
	
	if (dz->dz_no >= DZ_NUMBER) {
		return -1;
	}
	if (dz->data_buf == NULL && dz->data_len == 0) {
		return -1;
	}
	
	if(flag){
		p_i_msg = (internal_msg_t *)sbuf;
		p_i_msg->type = MSG_DZ_EVENT;
		p_i_msg->sub_type = MSG_NONE_TYPE;
		dz_temp = (rdb_yc_param_t *)p_i_msg->content;

		/*将参数发送给QT的rdb*/
		rdb_udp->dz_param.dz_no = dz->dz_no;
		rdb_udp->dz_param.tag = dz->tag;
		rdb_udp->dz_param.data_len = dz->data_len;
		memcpy(rdb_udp->dz_param.data_buf,dz->data_buf,dz->data_len);
		memcpy (dz_temp,&rdb_udp->dz_param,sizeof(rdb_dz_param_t));
		p_i_msg->content_len = sizeof(rdb_dz_param_t);
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		printf("IEC104 Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");	
	}

	/* 获取互斥信号量 */
	sem_timewait (&dz_entry->mutex, NULL);
	dz_entry = dz_lst + dz->dz_no;
	memcpy(dz_entry->data_buf,dz->data_buf,dz->data_len);
	dz_entry->tag = dz->tag;
	dz_entry->data_len = dz->data_len;
	/* 释放互斥信号量 */
	sem_post (&dz_entry->mutex);
	return ret;
}
	
int dz_get_value (
//	int app_id,
	rdb_dz_param_t * dz
	)
{
	unsigned int i, j, proto_no;
	
	dz_t * dz_entry;
	if ((dz->dz_no > DZ_NUMBER) || (dz->dz_no < 1)) {
		return -1;
	}
	
	/* 获取互斥信号量 */
	sem_timewait (&dz_entry->mutex, NULL);
	proto_no = dz->dz_no;
	dz_entry = dz_lst + proto_no;
	dz->tag = dz_entry->tag;
	dz->data_len = dz_entry->data_len;
	memcpy(dz->data_buf,dz_entry->data_buf,dz->data_len);
	
	/* 释放互斥信号量 */
	sem_post (&dz_entry->mutex);
	
	return 0;
}
int dz_get_value_proto (
	int app_id,
	rdb_dz_param_t * dz
	)
{
	unsigned int i,proto_no;
	dz_t * dz_entry = dz_lst;
	data_hook_t * dhook;

	i = proto_no = dz->dz_no;
	if ((proto_no > DZ_NUMBER) || (proto_no < 1)) {
		return -1;
	}
	
	dhook = dz_entry [i].p_hook;
	while (dhook != NULL) {
		if (dhook->app_id == app_id) {
			if (dhook->proto_data_no == proto_no) {
				/* 获取互斥信号量 */
				sem_timewait (&dz_entry->mutex, NULL);
				dz->tag = dz_entry [i].tag;
				dz->data_len = dz_entry [i].data_len;
				memcpy(dz->data_buf,dz_entry [i].data_buf,dz->data_len);
	
				/* 释放互斥信号量 */
				sem_post (&dz_entry->mutex);
			}
			break;
		}
		dhook = dhook->next;
		i++;
	}
	return 0;
}

int dz_get_total_num (
	int app_id,
	unsigned int * num		/* 返回遥测总数目 */
	)
{
	if (num != NULL) {
		* num = DZ_NUMBER;
		return 0;
	}
	else {
		return -1;
	}
}

int dz_get_total_num_proto (
	int app_id,
	unsigned int * num		/* 返回定值总数目 */
	)
{
	dz_t * dz_entry = dz_lst;
	data_hook_t * dhook;
	int i, dz_num;

	if (num != NULL) {
		dz_num = 0;
		for (i = 0; i < DZ_NUMBER; i++) {
			dhook = dz_entry [i].p_hook;
			while (dhook != NULL) {
				if (dhook->app_id == app_id) {
					dz_num++;
					break;
				}
				dhook = dhook->next;
			}
		}
		* num = dz_num;
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
    time_type ts;

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

        _DPRINTF ("yk(%d) selected(%s) by app(%d)\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
        /* 保存事件 */
        //		get_time_sec_msec (&ts, NULL);
    }
    else {
        _DPRINTF ("yk(%d) selected(%s) by app(%d) check failed\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
    }

    /* 释放互斥信号量 */
    sem_post (&yk_entry->mutex);

    return ret;
}

int yk_select_proto (
        unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
        unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
        int app_id,				/* 遥控发起者的id */

        int flag				/* 是否发送给QT的rdb */
        )
{
	unsigned int i, ctl_val;
	int ret = -1, proto_no = -1;
	yk_t * yk_entry = yk_lst;
	data_hook_t * dhook;
	time_type ts;
	internal_msg_t * p_i_msg;
	unsigned char sbuf [MAX_MSG_LEN];
	rdb_yk_select_t * yk_select_temp;
	if (yk_no >= YK_NUMBER) {
		return -1;
	}

	ctl_val = val;
	if (ctl_val != DP_OPEN && ctl_val != DP_CLOSE) {
		return -1;
	}
	
	for (i = 0; i < YK_NUMBER; i++) {
		dhook = yk_entry [i].p_hook;
		while (dhook != NULL) {
			if (dhook->app_id == app_id) {
				if (dhook->proto_data_no == yk_no) {
					proto_no = i;
					break;
				}
			}
			dhook = dhook->next;
		}
		if (proto_no != -1) {
			break;
		}
	}
	if (proto_no == -1) {
		return -1;
	}

    yk_entry = &yk_entry [proto_no];
    /* 获取互斥信号量 */
    sem_timewait (&yk_entry->mutex, NULL);

    if (yk_entry->ctl_step == YK_VAL) {
        yk_entry->ctl_step = YK_SEL;
        yk_entry->sel_val = ctl_val;
        yk_entry->sel_timer = YK_SEL_TO * 1000000;
        yk_entry->app_id = app_id;
        ret = 0;

        //_DPRINTF ("yk(%d) selected(%s) by app(%d)\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
        /* 保存事件 */
        //	get_time_sec_msec (&ts);
        //	send_co_msg (yk_no, &ts, CO_SELECT, ctl_val /* 双点 */);
        rdb_udp->yk_select.yk_result = 0xff;
    }
    else {
        //_DPRINTF ("yk(%d) selected(%s) by app(%d) check failed\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
		 rdb_udp->yk_select.yk_result = 0x55;
    }
	if(flag){
		/*将参数发送给104的rdb*/
		p_i_msg = (internal_msg_t *)sbuf;
		p_i_msg->type = MSG_YK_EVENT;
		p_i_msg->sub_type = MSG_YK_SELECT;
		yk_select_temp = (rdb_yk_select_t *)p_i_msg->content;
		rdb_udp->yk_select.yk_no = yk_no;
		rdb_udp->yk_select.val = val;
		rdb_udp->yk_select.app_id = app_id;
		
		memcpy (yk_select_temp,&rdb_udp->yk_select,sizeof(rdb_yk_select_t));
		p_i_msg->content_len = sizeof(rdb_yk_select_t);
		printf("Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");	
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		
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
    time_type ts;

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

            _DPRINTF ("yk(%d) unselected by app(%d)\n", yk_no, app_id);
            /* 保存事件 */
            //			get_time_sec_msec (&ts, NULL);
        }
    }

    /* 释放互斥信号量 */
    sem_post (&yk_entry->mutex);

    return ret;
}

int yk_unselect_proto (
        unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
        int app_id,				/* 遥控发起者的id */

        int flag				/* 是否发送给QT的rdb */
        )
{
	unsigned int i;
	int ret = -1, proto_no = -1;
	yk_t * yk_entry = yk_lst;
	data_hook_t * dhook;
	time_type ts;
	
	internal_msg_t * p_i_msg;
	unsigned char sbuf [MAX_MSG_LEN];
	rdb_yk_unselect_t * yk_unselect_temp;
	
	if (yk_no >= YK_NUMBER) {
		return -1;
	}
	
	for (i = 0; i < YK_NUMBER; i++) {
		dhook = yk_entry [i].p_hook;
		while (dhook != NULL) {
			if (dhook->app_id == app_id) {
				if (dhook->proto_data_no == yk_no) {
					proto_no = i;
					break;
				}
			}
			dhook = dhook->next;
		}
		if (proto_no != -1) {
			break;
		}
	}

    if (proto_no == -1) {
        return -1;
    }

    yk_entry = &yk_entry [proto_no];

    /* 获取互斥信号量 */
    sem_timewait (&yk_entry->mutex, NULL);

    if (yk_entry->ctl_step == YK_SEL) {
        if ((yk_entry->app_id == app_id) ||
                (app_id == RDB_APP_ID)) {
            yk_entry->ctl_step = YK_VAL;
            yk_entry->sel_timer = 0;
            yk_entry->app_id = -1;
            ret = 0;

            //_DPRINTF ("yk(%d) unselected by app(%d)\n", yk_no, app_id);
            /* 保存事件 */
            //		get_time_sec_msec (&ts);
            //	send_co_msg (yk_no, &ts, CO_CANCEL, DP_OPEN /* 单点 */);
            rdb_udp->yk_unselect.yk_result = 0xff;
        }
		else{
		 	rdb_udp->yk_unselect.yk_result = 0x55;
		}
    }
	
	if(flag){
		/*将参数发送给QT的rdb*/
		p_i_msg = (internal_msg_t *)sbuf;
		p_i_msg->type = MSG_YK_EVENT;
		p_i_msg->sub_type = MSG_YK_UNSELECT;
		yk_unselect_temp = (rdb_yk_select_t *)p_i_msg->content;
		rdb_udp->yk_unselect.yk_no = yk_no;
		rdb_udp->yk_unselect.app_id = app_id;
		
		memcpy (yk_unselect_temp,&rdb_udp->yk_unselect,sizeof(rdb_yk_unselect_t));
		p_i_msg->content_len = sizeof(rdb_yk_unselect_t);
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		p_i_msg->content_len = sizeof(rdb_yk_select_t);
		printf("Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");
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
    time_type ts;

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

        /* 保存事件 */
        //		get_time_sec_msec (&ts, NULL);
    }
    else {
        _DPRINTF ("yk(%d) operate(%s) by app(%d) check failed\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
    }

    /* 释放互斥信号量 */
    sem_post (&yk_entry->mutex);

    return ret;
}

int yk_operate_proto (
        unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
        unsigned char val,		/* 双点入库，DP_OPEN or DP_CLOSE */
        int app_id,				/* 遥控发起者的id */
        int flag				/* 是否发送给QT的rdb */
        )
{
    unsigned int i, ctl_val;
    int ret = -1, proto_no = -1;
    yk_t * yk_entry = yk_lst;
    data_hook_t * dhook;
    time_type ts;
    internal_msg_t * p_i_msg;
    unsigned char sbuf [MAX_MSG_LEN];
    rdb_yk_operate_t * yk_operate_temp;
    if (yk_no >= YK_NUMBER) {
        return -1;
    }
	ctl_val = val;
	if (ctl_val != DP_OPEN && ctl_val != DP_CLOSE) {
		return -1;
    }
	for (i = 0; i < YK_NUMBER; i++) {
		dhook = yk_entry [i].p_hook;
		while (dhook != NULL) {
			if (dhook->app_id == app_id) {
				if (dhook->proto_data_no == yk_no) {
					proto_no = i;
					break;
				}
			}
			dhook = dhook->next;
		}
		if (proto_no != -1) {
			break;
		}
    }
    if (proto_no == -1) {
        return -1;
    }
    yk_entry = &yk_entry [proto_no];
    /* 获取互斥信号量 */
    sem_timewait (&yk_entry->mutex, NULL);
    if (yk_entry->ctl_step == YK_SEL &&
            yk_entry->app_id == app_id &&
            yk_entry->sel_val == ctl_val) {
        yk_entry->ctl_val = ctl_val;

        yk_entry->ctl_step = YK_OPER;
        yk_entry->sel_timer = 0;
        yk_entry->oper_timer = YK_OPER_TO * 1000000;
//       _DPRINTF ("yk(%d) operate(%s) by app(%d)\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
        /* 开出接点动作 */

        /* 保存事件 */
        //		get_time_sec_msec (&ts, NULL);
        rdb_udp->yk_operate.yk_result = 0xff;
        printf("rdb:yk_result%d\n",rdb_udp->yk_operate.yk_result);
        ret = 0;
    }
    else {
//       _DPRINTF ("yk(%d) operate(%s) by app(%d) check failed\n", yk_no, ctl_val == DP_OPEN ? "OPEN" : "CLOSE", app_id);
		rdb_udp->yk_operate.yk_result = 0x55;
        printf("rdb:yk_result%d\n",rdb_udp->yk_operate.yk_result);
    }
	if(flag){
		/*将参数发送给QT的rdb*/
		p_i_msg = (internal_msg_t *)sbuf;
		p_i_msg->type = MSG_YK_EVENT;
		p_i_msg->sub_type = MSG_YK_OPERATE;
		yk_operate_temp = (rdb_yk_operate_t *)p_i_msg->content;
		rdb_udp->yk_operate.yk_no = yk_no;
		rdb_udp->yk_operate.val = val;
		rdb_udp->yk_operate.app_id = app_id;
        memcpy (yk_operate_temp,&rdb_udp->yk_operate,sizeof(rdb_yk_operate_t));
		p_i_msg->content_len = sizeof(rdb_yk_operate_t);
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		printf("Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");	
	}
    /* 释放互斥信号量 */
    sem_post (&yk_entry->mutex);

    return ret;
}

int yk_done (
        unsigned int yk_no,		/* 从0到YK_NUMBER-1 */
        unsigned char ret,		/* 执行结果 */
        int app_id				/* 遥控发起者的id */
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

int yk_done_proto (
        unsigned int yk_no, 	/* 从0到YK_NUMBER-1 */
        unsigned char ret,		/* 执行结果 */
        int app_id,				/* 遥控发起者的id */
        int flag				/* 是否发送给QT的rdb */
        )
{
    unsigned int i;
    int proto_no = -1;
    yk_t * yk_entry = yk_lst;
    data_hook_t * dhook;
    time_type ts;
    internal_msg_t * p_i_msg;
    unsigned char sbuf [MAX_MSG_LEN];
    rdb_yk_done_t * yk_done_temp;

	if (yk_no >= YK_NUMBER) {
		return -1;
	}
	for (i = 0; i < YK_NUMBER; i++) {
		dhook = yk_entry [i].p_hook;
		while (dhook != NULL) {
			if (dhook->app_id == app_id) {
				if (dhook->proto_data_no == yk_no) {
					proto_no = i;
					break;
				}
			}
			dhook = dhook->next;
		}
		if (proto_no != -1) {
			break;
		}
	}
	if (proto_no == -1) {
		return -1;
	}

    yk_entry = &yk_entry [proto_no];

    /* 获取互斥信号量 */
    /*sem_timewait (&yk_entry->mutex, NULL);*/
    if (yk_entry->ctl_step == YK_OPER) {
        /* anyway done! */
        yk_entry->ctl_step = YK_DONE;
        yk_entry->oper_timer = 0;
//		rdb_udp->yk_done.yk_result = 0xff;
        //_DPRINTF ("yk(%d) operate(%s) by app(%d) done, ret %d\n", yk_no, yk_entry->ctl_val == DP_OPEN ? "OPEN" : "CLOSE", yk_entry->app_id, ret);
    }
    else {
//		rdb_udp->yk_done.yk_result = 0x55;
        //_DPRINTF ("yk(%d) operate(%s) by app(%d) done check failed\n", yk_no, yk_entry->ctl_val == DP_OPEN ? "OPEN" : "CLOSE", yk_entry->app_id);
    }
#if 0
	if(flag){
		/*将参数发送给QT的rdb*/
		p_i_msg = (internal_msg_t *)sbuf;
		p_i_msg->type = MSG_YK_EVENT;
		p_i_msg->sub_type = MSG_YK_DONE;
		yk_done_temp = (rdb_yk_done_t *)p_i_msg->content;
		rdb_udp->yk_done.yk_no = yk_no;
		rdb_udp->yk_done.ret = ret;
		rdb_udp->yk_done.app_id = app_id;
		
		memcpy (yk_done_temp,&rdb_udp->yk_done,sizeof(rdb_yk_done_t));
		p_i_msg->content_len = sizeof(rdb_yk_done_t);
		sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
		printf("Send rdb msg:");
		for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
			printf("0x%02x ",sbuf[i]);							
		}
		printf("\n");	
	}		
    /* 释放互斥信号量 */
#endif
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

int yx_rdb_index_to_proto_index (
        int app_id,
        unsigned int rdb_index,
        unsigned int * proto_index)
{
    yx_t * yx_entry = yx_lst;
    data_hook_t * dhook;

    if ((rdb_index >= YX_NUMBER) ||
            (proto_index == NULL)) {
        return -1;
    }

    dhook = yx_entry [rdb_index].p_hook;
    while (dhook != NULL) {
        if (dhook->app_id == app_id) {
            * proto_index = dhook->proto_data_no;
            return 0;
        }
        dhook = dhook->next;
    }

    return -1;
}

int yc_rdb_index_to_proto_index (
        int app_id,
        unsigned int rdb_index,
        unsigned int * proto_index)
{
    yc_t * yc_entry = yc_lst;
    data_hook_t * dhook;

    if ((rdb_index >= YC_NUMBER) ||
            (proto_index == NULL)) {
        return -1;
    }

    dhook = yc_entry [rdb_index].p_hook;
    while (dhook != NULL) {
        if (dhook->app_id == app_id) {
            * proto_index = dhook->proto_data_no;
            return 0;
        }
        dhook = dhook->next;
    }

    return -1;
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

static int init_external_com ()
{
    struct sockaddr_in server_addr;
    int server_sfd, client_sfd;


    server_sfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (server_sfd == -1) {
        _DPRINTF ("socket failed\n");
        return -1;
    }
    client_sfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (client_sfd == -1) {
        _DPRINTF ("socket failed\n");
        return -1;
    }

    memset ((unsigned char *)&(rdb_udp->client_addr), 0, sizeof (rdb_udp->client_addr));
    rdb_udp->client_addr.sin_family = AF_INET;
    rdb_udp->client_addr.sin_port = htons (RDB_SEND_PORT);
    rdb_udp->client_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");

    memset ((unsigned char *)&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (RDB_RECV_PORT);
    server_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");

    if (bind (server_sfd, (struct sockaddr *)&server_addr, sizeof (server_addr)) < 0) {
        _DPRINTF ("bind external udp socket failed\n");
        return -1;
    }

    rdb_udp->rdb_server_id = server_sfd;
    rdb_udp->rdb_client_id = client_sfd;

    return 0;
}
void send_path(unsigned char meas_path[],int len)
{
    internal_msg_t * p_i_msg;
    unsigned char sbuf [MAX_MSG_LEN];
    int i;

#if 1
    printf("Test send arry:");
    for(i=0;i<len;i++){
        printf("0x%02x ",meas_path[i]);
    }
    printf("\n");
#endif

    /*将测量的路径发送给IEC04的rdb*/
    p_i_msg = (internal_msg_t *)sbuf;
    p_i_msg->type = MSG_SENDPATH_EVENT;
    p_i_msg->sub_type = MSG_NONE_TYPE;
    memcpy (p_i_msg->content,meas_path,len);
    p_i_msg->content_len = len;
    sendto(rdb_udp->rdb_client_id,sbuf, p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN, 0, (struct sockaddr *)&(rdb_udp->client_addr), sizeof(rdb_udp->client_addr));
    printf("Send rdb msg:");
    for(i=0;i<p_i_msg->content_len+INTERNAL_MSG_HEAD_LEN;i++){
        printf("0x%02x ",sbuf[i]);
    }
    printf("\n");
}

static void rdb_recv_task (void * arg)
{
    int read_len, ret,temp_fd,offset;
    /*struct*/ fd_set read_fds;
    struct sockaddr_in cli;
	struct timeval tv;
    socklen_t cli_addr_len;
    unsigned char rbuf [MAX_MSG_LEN];
    internal_msg_t * p_i_msg;
    rdb_yx_param_t * yx_temp;
    rdb_yc_param_t * yc_temp;
    rdb_yk_select_t * yk_select_temp;
    rdb_yk_unselect_t * yk_unselect_temp;
    rdb_yk_operate_t * yk_operate_temp;
    rdb_yk_done_t * yk_done_temp;
    rdb_dz_param_t * dz_temp;
	int i;
    while (1) {
        /* external msg */
        temp_fd = rdb_udp->rdb_server_id;
        if(temp_fd != -1) {
            FD_ZERO (&read_fds);
            FD_SET (temp_fd, &read_fds);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            ret = select (temp_fd + 1, &read_fds, NULL, NULL, &tv);
            if (ret == -1) {
                printf ("rdb_recv select error\n");
                usleep (50000);
                continue;
            }
            else  if (ret == 0) {
                /* time out */
                continue;
            }

            if (FD_ISSET (temp_fd, &read_fds)) {/*检查是否有客户端发来消息*/
                read_len = recvfrom (temp_fd, (char *)rbuf, sizeof (rbuf),0, (struct sockaddr *)&cli, &cli_addr_len);
                if (read_len <= 0) {
                    _DPRINTF ("sfd(%d) recv external msg timeout or error, closed\n", temp_fd);                 
                    continue;
                }
                else {
                    offset = 0;
                    while (offset < read_len) {
#if 1
						printf("Recv rdb msg:");
						for(i=0;i<read_len;i++){
							printf("0x%02x ",rbuf[i+offset]);							
						}
                        printf("\n");
#endif
                        p_i_msg = (internal_msg_t *)(rbuf + offset);
                        switch (p_i_msg->type) {
                        case MSG_YX_EVENT:		/*检查发送列表*/
                            yx_temp = (rdb_yx_param_t *)p_i_msg->content;
                            yx_set_value(yx_temp->yx_no,yx_temp->val, yx_temp->ts,yx_temp->cos_soe_flag,0);
                            offset += (p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN);
                            break;
                        case MSG_YC_EVENT:
                            yc_temp = (rdb_yc_param_t *)p_i_msg->content;
                            yc_set_value(yc_temp->yc_no,yc_temp->val, yc_temp->qds,yc_temp->ts,yc_temp->b_event,0);
                            offset += (p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN);
                            break;
                        case MSG_YK_EVENT:
                            switch (p_i_msg->sub_type) {
                            case MSG_YK_SELECT:
                                yk_select_temp = (rdb_yk_select_t *)p_i_msg->content;
                                yk_select_proto(yk_select_temp->yk_no,yk_select_temp->val, yk_select_temp->app_id,1);
                                break;
                            case MSG_YK_UNSELECT:
                                yk_unselect_temp = (rdb_yk_unselect_t *)p_i_msg->content;
                                yk_unselect_proto(yk_unselect_temp->yk_no, yk_unselect_temp->app_id,1);
                                break;

                            case MSG_YK_OPERATE:
                                yk_operate_temp = (rdb_yk_operate_t *)p_i_msg->content;
                                yk_operate_proto(yk_operate_temp->yk_no, yk_operate_temp->val,yk_operate_temp->app_id,1);
                                break;
                            case MSG_YK_DONE:
                                yk_done_temp = (rdb_yk_done_t *)p_i_msg->content;
                                yk_done_proto(yk_done_temp->yk_no, yk_done_temp->ret,yk_done_temp->app_id,1);
                                break;
                            default:
                                break;
                            }
                            offset += (p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN);
                            break;
						case MSG_DZ_EVENT:
							dz_temp = (rdb_dz_param_t *)p_i_msg->content;
							dz_set_value(dz_temp,0);							
							offset += (p_i_msg->content_len + INTERNAL_MSG_HEAD_LEN);
                        default:
                            offset += sizeof (internal_msg_t);
                            break;
                        }
                    }
                }
            }
            else {
                usleep (50);
            }
        }
    }
}
