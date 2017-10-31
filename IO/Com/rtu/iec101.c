 /********************************************************************
	created:	2016/07/09
	created:	
	filename: 	D:\cygwin64\home\ibm\prog\rtu\inc\iec101.h
	file path:	D:\cygwin64\home\ibm\prog\rtu\inc
	file base:	iec101
	file ext:	h
	author:	zxf	     
	purpose:	iec101 balance protocol
*********************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
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

#include "uart_driver.h"
#include "protocol.h"
#include "iec101.h"
#include "../rdb/point_table.h"
#include "../rdb/rdb.h"
#include "../Modbus/gpio_oper.h"


/*
#include "global_define.h"
#include "params_settings.h"
#include "base_functions.h"
#include "bin_out.h"
#include "monitor_status.h"
#include "bin_in.h"
#include "screen_driver.h"

*/
struct APPDEF * iec101_service;

int iec101_comm_stat = 0;

extern int cp56time2a_to_time (unsigned char cp56t [], time_type * tt);
extern int time_to_cp56time2a (time_type * tt, unsigned char cp56t []);

static int timeval_to_sys_time(struct timeval *t_val, struct SYS_TIME *sys_t);
static int init_iec101_reg_rdb(struct APPDEF *dp);
static int send_chk_time1s_task_msg(struct APPDEF *dp);
static void timer1s_task (void * arg);
static int init_internal_udp_com (struct APPDEF *dp);
static int iec101_check_event (struct APPDEF *dp);
static int iec101_save_yx_event (struct APPDEF *dp, yx_event_t * p_event);
static int iec101_save_yc_event (struct APPDEF *dp,  yc_event_t * p_event);
static void iec101_task(void * arg);
static int iec101_free_yx_event (struct APPDEF *dp, yx_event_t * p_event);
static int iec101_free_yc_event (struct APPDEF *dp, yc_event_t * p_event);
static void IEC101PeriodChk1s(struct APPDEF *dp);
static void IEC101PeriodChk200ms(struct APPDEF *dp);
static void IEC101ParameterInitClm(struct APPDEF *dp);
static void IEC101ParameterInitPre(struct APPDEF *dp);
static void IEC101ParameterInitPost(struct APPDEF *dp);
static void IEC101SaveReport(struct APPDEF *dp,u_int8 *mes,u_int32 len,u_int8 pri);
static void IEC101ReceiveDataProcess(struct APPDEF *dp,u_int8 *mes,u_int32 len);
static u_int8 IEC101CheckData(struct APPDEF *dp);
static void IEC101ReceiveDataClm(struct APPDEF *dp);
static void IEC101LinkDataClm(struct APPDEF *dp);
static void IEC101ApplicationDataClm(struct APPDEF *dp);
static void IEC101ReportInvalidAsdu(struct APPDEF *dp);
static void IEC101SendDataClm(struct APPDEF *dp);
static void IEC101SendData(struct APPDEF *dp,u_int8 *sbuf,u_int32 slen);
static void IEC101SendedDataChk(struct APPDEF *dp,u_int8 *sbuf,u_int32 slen);
static void IEC101ReportLinkState(struct APPDEF *dp,u_int8 linksta);
static void IEC101ReportFcbConfirm(struct APPDEF *dp);
static void IEC101ReportRstEnd(struct APPDEF *dp);
static void IEC101ReportACK(struct APPDEF *dp);
//static void IEC101ReportACK2(struct APPDEF *dp);
static void IEC101ReportNoData(struct APPDEF *dp);
static void IEC101CheckAcdStatus(struct APPDEF *dp);
static void IEC101ReportBhYc(struct APPDEF *dp,int event_num);
static u_int8 IEC101ReportCOS(struct APPDEF *dp,int event_num);
static u_int8 IEC101ReportSOE(struct APPDEF *dp,int event_num);
static u_int16 IEC101CheckTelegramLen(struct APPDEF *dp,u_int8 data_type,u_int8 gramfig);
//static float IEC101GetFloatFromBuf(u_int8* buf,int orderfig);
//static u_short IEC101GetUShortFromBuf(u_int8* buf,int orderfig);
static u_int16 IEC101AsduCommonInfChk(struct APPDEF *dp,u_int16 cot,u_int8 vsq);
static void IEC101ReportGlbI(struct APPDEF *dp);
static void IEC101ReportGlbIConfirm(struct APPDEF *dp,u_int8 cotfig);
static void IEC101ReportGlbIConfirm2(struct APPDEF *dp);
static void IEC101ReportGlbIData(struct APPDEF *dp);
static void IEC101ReportGlbIYxData(struct APPDEF *dp);
static void IEC101ReportGlbIYcData(struct APPDEF *dp);
static void IEC101ReportGlbIEndData(struct APPDEF *dp);
static void IEC101ReportICancel(struct APPDEF *dp,u_int8 qoi);
static void IEC101ReportSetClock(struct APPDEF *dp);
static u_int8 IEC101CheckTimeRationality(struct APPDEF *dp,struct SYS_TIME * tp);
static void IEC101ReportTimeSyn(struct APPDEF *dp,u_int16 cot,u_int8 valid_fig,struct SYS_TIME * rt);
static void IEC101ReportTest(struct APPDEF *dp);
static void IEC101ReportPeriodicYcScan(struct APPDEF *dp);
static struct MSGREPORT * IEC101ReportPtrCheckUnused(struct APPDEF *dp);
static struct MSGREPORT * IEC101ReportPtrCheckPriUnused(struct APPDEF *dp,u_int8 pri);
static void IEC101ReportPtrFree(struct APPDEF *dp,struct MSGREPORT * ReportPtr);
static u_int8 IEC101ReportPtrMallocInit(struct APPDEF *dp);
static void IEC101ClearBufferClm(struct APPDEF *dp,u_char fig);
static void IEC101ClearReportBuffer(struct APPDEF *dp,u_int8 fig);
static void IEC101PrintMsg(struct APPDEF *dp,u_int8 *msg,int32 len,int32 fig);
static void IEC101SetTime(struct APPDEF *dp,struct SYS_TIME *tp);
static void IEC101Printf(struct APPDEF *dp,const char *format,...);
static int IEC101GetAsduCommonFromMsg(struct APPDEF *dp,struct ASDUHEAD * pHead,u_char *buf);
static void IEC101UpdateCommusta(struct APPDEF *dp,u_int8 comm_status,u_int cot);
static int IEC101FillIntYcVal(struct APPDEF *dp,u_int8 *buf,int i,short sval);
static int IEC101FillFloatYcVal(struct APPDEF *dp,u_int8 *buf,int i,float fval);
static int IEC101GetTbufFromSysTime0(u_int8 *buf,int i,struct SYS_TIME *tp);
static void IEC101GetValidTimeFromWhole(struct SYS_TIME *tin,struct SYS_TIME *tout);
static void IEC101CommClearClm(struct APPDEF *dp);
static void IEC101AskLinkState(struct APPDEF *dp);
static void IEC101RstLinkState(struct APPDEF *dp);
static void IEC101TestLinkState(struct APPDEF *dp);
static void IEC101BalanceModeCheckSendDataClm(struct APPDEF *dp);
static u_int8 IEC101ReportData(struct APPDEF *dp);
static void IEC101BalanceModeSendDataClm(struct APPDEF *dp);
static u_int8 IEC101CanSendTest(struct APPDEF *dp);

static void IEC101YkYtClm(struct APPDEF *dp);
static int IEC101YkYtMsgDecodeChk0(struct APPDEF *dp,u_int8 *rbuf,struct APPYKDEF *pYkYt);
static int IEC101YkYtMsgDecodeChk(struct APPDEF *dp,u_int8 *rbuf,struct APPYKDEF *pYkYt);
static void IEC101YkYtReportReply(struct APPDEF *dp,struct APPYKDEF *pYkYt,u_int8 replyfig);
static void IEC101YkYtClearClm(struct APPDEF *dp,struct APPYKDEF *pYkYt);
static void IEC101YkYtCheckClm(struct APPDEF *dp);
int set_seconds_1900(time_type * tt_1900)
{
#if 0
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
#endif
	return 0;
}
static int init_iec101_data_local_mem ()
{
	iec101_service = (struct APPDEF *)malloc (sizeof (struct APPDEF));

	if (iec101_service == NULL) {
		return -1;
	}

	memset (iec101_service, 0, sizeof (struct APPDEF));
	
	return 0;
}

time_t sys_time_to_timet(struct SYS_TIME *t)
{
	struct 	 tm ansi_time;
	ansi_time.tm_sec=t->second;
	ansi_time.tm_min=t->minute;
	ansi_time.tm_hour=t->hour;
	if(t->day<1) ansi_time.tm_mday=1;
	else ansi_time.tm_mday=t->day;
	if(t->month<1)	ansi_time.tm_mon=0;
	else	ansi_time.tm_mon=t->month-1;
	if(t->year<1990) ansi_time.tm_year=1990-1900;
	else ansi_time.tm_year=t->year-1900;
	ansi_time.tm_isdst=1;
	return  mktime(&ansi_time);
}

int sys_open_com(struct APPDEF *dp, int databits, int stopbits, int parity, int baudrate, int com_no, int fd)
{
    int com_fd;
 #if 0
	char dev_name[32];
	int ret;
 	unsigned char val;

    	val=0;
	if(baudrate==600) {
		val |=0;
	} else if(baudrate==1200) {
		val |=1;
	} else if(baudrate==2400) {
		val |=2;
	} else if(baudrate==4800) {
		val |=3;
	} else if(baudrate==9600) {
		val |=4;
	} else if(baudrate==19200) {
		val |=5;
	} else if(baudrate==57600) {
		val |=6;
	} else if(baudrate==115200) {
		val |=7;
	} else {
		return(-1);
	}
	
	switch (parity) {
	    case 1: //��У��
	        break;
	    case 2: //��У��
	        val |=0x20;
	        break;
	    case 3: //żУ��
	        val |=0x28;
	        break;
	    default:
	        return(-1);
	        break;
	}

    sprintf(dev_name,"%s%d",des,com_no-1);

    com_fd=open(dev_name,O_RDWR);
    if(com_fd>=0) {
	    	ret=ioctl(com_fd,COMREGADDR,&val);
	    	if (ret < 0){
			_DPRINTF("ioctl failed,COMREGADDR =%04x\n", COMREGADDR);
		}
		else{
			_DPRINTF("ioctl OK \n");
		}		
	} 
	else {
	    return(-1);
	}
#else
	if (dp->rs_mode == 485) {
		/* export rs485 rw/rd line */
		if (gpio_open(GPIO_RS485_RW, "out") < 0) {
			printf("failed to export gpio %d\n", GPIO_RS485_RW);
			//return -1;
		}
		/* set rd */
		gpio_set(GPIO_RS485_RW, RS485_RD);
	}

	/* open uart */
//    com_fd = uart_open(UART_PORT, baudrate, 0, 8, 1, 'N');
    com_fd = fd;
    printf("hellow 101 = %d!\n\n\n ",com_fd );
    if (com_fd < 0) {
		printf("failed to open port %s\n", UART_PORT);
		return -1;
	}
#endif
	
	return com_fd;
}

static int timeval_to_sys_time(struct timeval *t_val, struct SYS_TIME *sys_t)
{
	time_t utc;
	struct tm *tm_time;

	utc = t_val->tv_sec;
	tm_time = localtime(&utc);
	sys_t->msecond = t_val->tv_usec / 1000;
	sys_t->second = tm_time->tm_sec;
	sys_t->minute = tm_time->tm_min;
	sys_t->hour = tm_time->tm_hour;
	sys_t->day = tm_time->tm_mday;
	sys_t->month = tm_time->tm_mon+1;
	sys_t->year = tm_time->tm_year + 1900;
	return 0;
}

int sys_get_time(struct SYS_TIME *sys_t)
{
	struct timeval t_val;
	time_t tt;

	tt = time (NULL);
	t_val.tv_sec = tt;
	t_val.tv_usec = 0;
	timeval_to_sys_time(&t_val,sys_t);
	return 0;
}

static int init_iec101_reg_rdb(struct APPDEF *dp)
{
	int i, app_id;

	app_id = dp->app_id;
#if 1
	/* reg yx data list */
	for (i = 0; i < num_of_yx; i++) {
		reg_rdb_data (app_id, RDB_TYPE_YX, i, i);
	}
	
	/* reg yc data list */
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
	reg_rdb_hook (app_id, send_iec101_internal_udp_data);
	
	return 0;
}

static int send_chk_time1s_task_msg(struct APPDEF *dp)
{
	internal_msg_t in_msg;

	in_msg.type = MSG_CHK_TIME1S_TASK;

	return send_iec101_internal_udp_data ((unsigned char *)&in_msg, sizeof (internal_msg_t));
}

static void timer1s_task (void * arg)
{
	struct APPDEF *dp = (struct APPDEF *)arg;
		
	while (1) {
		sleep(1);
		send_chk_time1s_task_msg (dp);
	}

	//fdCloseSession(Task_self());
}

static int init_internal_udp_com (struct APPDEF *dp)
{
	struct sockaddr_in serv_addr;
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

	memset ((unsigned char *)&serv_addr, 0, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons (IEC101_INTERNAL_UDP_PORT);
	serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);

	if (bind (server_sfd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) < 0) {
		_DPRINTF ("bind internal udp socket failed\n");
		return -1;
	}
	dp->udp_server_id = server_sfd;
	dp->udp_client_id = client_sfd;
	
	return 0;
}

int send_iec101_internal_udp_data (unsigned char * send_buf, int send_len)
{
	struct sockaddr_in addr_in;
	int bytes_send;
	int re_send_cnt;

	if (iec101_service->udp_client_id == -1 ||
		send_buf == NULL ||
		send_len <= 0) {
		return -1;
	}

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons (IEC101_INTERNAL_UDP_PORT);
	addr_in.sin_addr.s_addr = inet_addr ("127.0.0.1");

	re_send_cnt = 10;
	while (re_send_cnt-- > 0) {
		bytes_send = sendto (iec101_service->udp_client_id, send_buf, send_len, 0, 
			(struct sockaddr *)&addr_in, sizeof (addr_in));
		if (bytes_send == send_len) {
			break;
		}
		usleep (2000);
	}

	return bytes_send;
}

static int iec101_check_event (struct APPDEF *dp)
{
	if (dp->yx_event_num > 0) {
		IEC101ReportCOS(dp,dp->yx_event_num);
		IEC101ReportSOE(dp,dp->yx_event_num);
		iec101_free_yx_event (dp, NULL);
	}
	if (dp->yc_event_num > 0) {
		IEC101ReportBhYc(dp,dp->yc_event_num);
		iec101_free_yc_event (dp, NULL);
	}
	return 0;
}

static int iec101_save_yx_event (struct APPDEF *dp, yx_event_t * p_event)
{
	memcpy (&(dp->yx_event_lst [dp->yx_event_num]), p_event, sizeof (* p_event));
	dp->yx_event_num++;
	
	return dp->yx_event_num;
}

static int iec101_save_yc_event (struct APPDEF *dp,  yc_event_t * p_event)
{
	memcpy (&(dp->yc_event_lst [dp->yc_event_num]), p_event, sizeof (* p_event));
	dp->yc_event_num++;
	
	return dp->yc_event_num;
}

static int iec101_free_yx_event (struct APPDEF *dp, yx_event_t * p_event)
{
	dp->yx_event_num = 0;
	return 0;
}

static int iec101_free_yc_event (struct APPDEF *dp, yc_event_t * p_event)
{
	dp->yc_event_num = 0;
	return 0;
}


static void iec101_task(void * arg)
{
	int rv, max_fd, temp_fd;
	fd_set read_fds;
	struct timeval tv;
	struct sockaddr_in cli;
	int read_len, offset;
	socklen_t cli_addr_len;
	unsigned char rbuf [300];
	internal_msg_t * p_i_msg;
	//int     cnt;
	struct APPDEF *dp = (struct APPDEF *)arg;

	if (init_internal_udp_com (dp) != 0) {
		exit(0);
	}

	//cnt=0;
	temp_fd=-1;
	while (1) {
		/* clear */
		FD_ZERO (&read_fds);
		max_fd = 0;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		/* set fds */
		if(dp->udp_server_id >0 ){
			temp_fd = dp->udp_server_id;
			FD_SET (temp_fd, &read_fds);
			max_fd = (temp_fd > max_fd) ? temp_fd : max_fd;
		}
		
		/* select */
		rv = select (max_fd + 1, &read_fds, NULL, NULL, &tv);
		if (rv == -1) {
			_DPRINTF ("iec101_task select error\n");
			sleep (1);
			continue;
		}
		else if (rv == 0) {
			/* time out */
			continue;
		}

		/* ����״̬���� 
		task_running_cnt [IEC101_TASK_CNT]++;
		*/
		/* some fd set */
		/* internal msg */
		if(dp->udp_server_id > 0 ){
			if (FD_ISSET (dp->udp_server_id, &read_fds)) {
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
							break;
						case MSG_CHK_TIME1S_TASK:
							IEC101PeriodChk200ms(dp);
							//if(cnt++ >=5){
							//	cnt=0;
							IEC101PeriodChk1s (dp);
							//}
							offset += sizeof (internal_msg_t);
							break;
						case MSG_CHK_EVENT:
							switch (p_i_msg->sub_type) {
							case MSG_SUB_YX:
								if (iec101_save_yx_event (dp, (yx_event_t *)p_i_msg->content) >= 
									MAX_YX_EVENT_NUM_PER_FRAME_101) {
									IEC101ReportCOS(dp,dp->yx_event_num);
									IEC101ReportSOE(dp,dp->yx_event_num);
									iec101_free_yx_event (dp, NULL);
								}
								break;
							case MSG_SUB_YC:
								if (iec101_save_yc_event (dp, (yc_event_t *)p_i_msg->content) >= 
									MAX_YC_EVENT_NUM_PER_FRAME_101) {
									IEC101ReportBhYc(dp,dp->yc_event_num);
									iec101_free_yc_event (dp, NULL);
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
		}
	}

	//fdCloseSession(Task_self());
}

static void iec101_recv (void * arg)
{
	unsigned char rbuf [300];
	int read_len;
	struct APPDEF *dp = (struct APPDEF *)arg;
	
	while (1) {
		/* external msg */
		if(dp->comfd != (int)NULL) {
			//read_len=uart_read((UART_Handle)dp->comfd,rbuf,sizeof (rbuf),dp->rs_mode, dp->com_no);
			read_len=uart_recv((UART_Handle)dp->comfd, rbuf, sizeof(rbuf));
			if(read_len>0) {
				show_msg("recv msg", (char *)rbuf, read_len);
				if(dp->PrintMsgFig==PRTMSG_RAW) {
					IEC101PrintMsg(dp,rbuf,read_len,0);
				}
				IEC101ReceiveDataProcess(dp,rbuf,read_len);
			}
		}
		else {
			usleep (50000);
		}
	}

	//fdCloseSession(Task_self());
}

int init_iec101_service(int fd)
{
	struct APPDEF *dp;
	Uint32 boudrate;
	Uint32 parity;
	pthread_t tid;

	IEC101Printf(NULL,"IEC101 process begin --->\n");
	if (init_iec101_data_local_mem () < 0) {
		return -1;
	}

	dp= iec101_service;
	dp->app_id=IEC101_APPID;

	IEC101ParameterInitClm(dp);
	if(IEC101ReportPtrMallocInit(dp)==NO) {
		dp->ProtoEnableFig=NO;
	}

	init_iec101_reg_rdb(dp);
	
	/**********INTI COM ***********/
	dp->com_no=1;
	/*
	dp->rs_mode = device_setting_value->uart_mode [1] == 0 ? 232 : 485;
	boudrate = device_setting_value->uart_boundrate [1];
	parity = device_setting_value->uart_parity [1];
    */
    dp->rs_mode = 232;
    boudrate = 115200;
    parity = 0;
	memset(dp->app_name,0,32);
	sprintf(dp->app_name,"UART:1");		/* ����1 (�����2������) */
	
	//dp->comfd = sys_open_com(dp->app_name,8,1,parity,boudrate,dp->com_no);
    dp->comfd = sys_open_com(dp,8,1,parity,boudrate,dp->com_no,fd);
	if (dp->comfd == (int)NULL) {
		return -1;
	}
	/********************/
	tid = thread_create ("time task", 1000 * 2, 1, timer1s_task, (void *)dp);

    if ((int)tid == -1) {
		IEC101Printf (dp,"construct_task timer 200ms task error\n");
		return -1;
	}

	tid = thread_create ("101 task", 1000 * 4,1, iec101_task, (void *)dp);
    if ((int)tid == -1) {
		IEC101Printf (dp,"construct_task iec101 task error\n");
		return -1;
	}

	tid = thread_create ("101 recv", 1000 * 4, 1, iec101_recv, (void *)dp);
    if ((int)tid == -1) {
		IEC101Printf (dp,"construct_task iec101 recv error\n");
		return -1;
	}
	
	IEC101UpdateCommusta(dp,RDB_RET_DISCONN,COMMUCOT_INIT);
	IEC101Printf(dp,"init ok");
	return(0);
}

static void IEC101PeriodChk1s(struct APPDEF *dp)
{
	struct SYS_TIME t;

	dp->second_count++;
	if(dp->ProtoEnableFig==NO) {
		return;
	}
	if(dp->start_fig==NO) {
		if(dp->second_count>=dp->start_time) {
			sys_get_time(&t);
			if(dp->second_count>=dp->start_time) {
				IEC101Printf(dp,"\nservice open. %ds, (t=%04d-%02d-%02d %02d:%02d:%02d.%03d)",dp->start_time,t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);
			}
			else {
				IEC101Printf(dp,"\nservice open. rdb_ready, %ds, (t=%04d-%02d-%02d %02d:%02d:%02d.%03d)",dp->second_count,t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);
			}
			dp->start_fig=YES;
			dp->second_count=0;
		}
		return;
	}
	dp->comm_count++;
	if(dp->comm_count>=dp->commout_time) {
		dp->comm_count=dp->commout_time;
		IEC101UpdateCommusta(dp,RDB_RET_DISCONN,COMMUCOT_TIMEOUT);
	}
	if(dp->comm_status==RDB_RET_NORMAL) {
		if((dp->PeriodicScanYcFig==YES)&&(dp->PeriodicScanYcTime>0)&&(dp->PeriodicScanYcTime<65535)&&((dp->second_count)%(dp->PeriodicScanYcTime)==0)) {
			IEC101ReportPeriodicYcScan(dp);
		}
		IEC101YkYtCheckClm(dp);
		if (YES == IEC101CanSendTest(dp)) {
			dp->TestCount++;
			if(dp->TestCount >= 20){
				dp->TestCount=0;
				IEC101TestLinkState(dp);
			}
		}
	}
	if(dp->fsendfig2==YES) {
		dp->fsendcount2++;
		if((dp->fsendcount2) > 6)  // �˴��Ĳ������ܳ����������Ͳ���֡��TestCount�ķ�ֵ
		{
			sys_get_time(&t);
			IEC101Printf(dp,"���ͳ�ʱ%ds,���ط�-%ds.",dp->fsendcount2,t.second);
			dp->fsendcount2=0;
			dp->fresendfig2=YES;
		}
		if((dp->fresendframe2) > IEC101_MAX_RESEND_NUM)
		{
			IEC101Printf(dp,"�ط�����3��,����·��ʼ��.\n");
			if ((LINKRULE_ZHUHAI_XUJI != dp->LnkRuleType)&&(LINKRULE_DONGFANG != dp->LnkRuleType)){
				dp->LnkInitStartFig2=YES;
			}
			IEC101CommClearClm(dp);
		}
	}
	else {
		dp->fsendcount2=0;
	}	
	return;
}

static void IEC101PeriodChk200ms(struct APPDEF *dp)
{
	if(dp->ProtoEnableFig==NO) {
		return;
	}
	dp->Chk200ms_count++;
	if(dp->start_fig==YES){
		if(dp->comm_status==RDB_RET_NORMAL) {
				iec101_check_event(dp);
		}
		IEC101BalanceModeCheckSendDataClm(dp);
	}
	return;
}

static void IEC101ParameterInitClm(struct APPDEF *dp)
{
	IEC101ParameterInitPre(dp);
	IEC101ParameterInitPost(dp);
	return;
}

static void IEC101ParameterInitPre(struct APPDEF *dp)
{
	int i;
	
	dp->rptr=0;
	dp->rlen=0;
	dp->rfig=COMR_NO_DATA;
	dp->ReceSynFig=NO;
	dp->ReceBytes=0;
	dp->slen=0;
	dp->slen2=0;
	dp->slen3=0;
	
	dp->LnkRuleType = LINKRULE_ZHUHAI_XUJI;
	dp->AutoTest =YES;  
	dp->DebugOutFig=DBGOUT_STDPNT | DBGOUT_DBGPNT | DBGOUT_DBGSKT;//print msg
	//dp->DebugOutFig=DBGOUT_NULL;  //no print msg
	dp->DebugLanguage=DEBUG_CHINESE;
	dp->DebugTimeFig=DEBUGTIME_NULL;
	dp->PrintMsgFig=PRTMSG_FRM;
	dp->ProtoEnableFig=YES;
	dp->Version101=IEC101_V2002;
	/*
	if (device_setting_value->iec101_mode) {
		/* 1: ƽ��ʽ101 
	dp->CommuMode = MODE_BALANCE;
}
	else {
		/* 0: ��ƽ��ʽ101 
		dp->CommuMode = MODE_UNBALANCE;
	}
	*/
	
	dp->start_time=5;
	dp->commout_time=120;
	dp->linkaddr_bytes=2;	/* �����ն�101�淶Ϊ2 */
	dp->link_addr=1;
	//dp->link_addr=device_setting_value->dev_addr_101;
	dp->appaddr_bytes=2;
	dp->appaddr=dp->link_addr;
	dp->cot_bytes=2;
	dp->CotMatch_strategy=COTMATCH_LOWBYTE;
	dp->infaddr_bytes=2;
	dp->telegram_MAXbytes=255;
	dp->LinkACD_strategy=ACD_STD;
	dp->resendfig=NO;
	dp->LinkAnswerStgy=LINKPREINIT_NOWORK;
	dp->LinkIlctlStgy=LINKILCTL_NOANSWER;
	dp->ASDUCHK_strategy=ASDUCHK_VSQ_COT_ADDR;
	dp->COT_strategy=COT_VALID_INVALID;
	dp->E5_strategy=E5_INVALID;
	dp->E5_FcbRstValid=NO;
	dp->ACD_FcbRstValid=NO;
	dp->FcbStartFig=NO;
	dp->FcbPostState=FCBPOST_FUZZY;
	dp->FcbReplyStrategy=FCB_REPLY_RESEND_3;
	dp->FcbCount=0;
	dp->ASDU70ValidFig=NO;
	dp->ASDU70ValidStrategy=ASDU70_VALID_FCB;
	dp->InitLocationFig=0;
	dp->InitOverFig=INIT_FCBRST;
	dp->InitStepFig=INIT_NULL;
	dp->InitPeriodRuleFig=INITRULE_DEFINE;
	dp->Data1RespondRequest2Fig=YES;
	dp->TimeSynStrategyFig=TIMESYN_MASTER;
	dp->TimeStaticOffset=100;
	dp->LnkRstBufferFig=CLRBUF_BUTSOE;
	dp->YcModifyFig=YC_MODIFY_NULL;
	dp->YCUpMaxValue=32767;
	dp->IntYcByteFig=YES;
	dp->FloatYcByteFig=YES;

	dp->GlobalI_DataSeqFig[0]=GIDATA_SYX;
	dp->GlobalI_DataSeqFig[1]=GIDATA_DYX;
	dp->GlobalI_DataSeqFig[2]=GIDATA_YC;
	dp->ASDU_YC_GI=9;
	dp->ASDU_YX_GI=1;
	dp->ASDU_YP_GI=5;
	dp->GlbISyxStgy=YXSTATE_OPN_CLS_IV;
	dp->GlbIDyxStgy=YXSTATE_OPN_CLS_MID_IV;
	dp->YxNumPerFrmGlobalI=100;
	dp->YcNumPerFrmGlobalI=50;
	dp->YpNumPerFrmGlobalI=50;
	dp->GlobalI_NewStrategy=GINEW_REPONSE_FIFO;
	dp->ASDU_YM_PI=15;
	dp->ASDU_YC_GroupI=9;
	dp->ASDU_YX_GroupI=1;
	dp->ASDU_YP_GroupI=5;
	dp->GrpISyxStgy=YXSTATE_OPN_CLS_IV;
	dp->GrpIDyxStgy=YXSTATE_OPN_CLS_MID_IV;
	dp->YxNumPerFrmGroupI=100;
	dp->YcNumPerFrmGroupI=50;
	dp->YpNumPerFrmGroupI=50;
	dp->GroupNullReplyStgy=GROUPINODATA_APPACKEND;
	dp->ASDU_YC_ReadI=9;
	dp->ASDU_SYX_ReadI=1;
	dp->ASDU_DYX_ReadI=3;
	dp->ASDU_YM_ReadI=15;
	dp->ASDU_YP_ReadI=5;
	dp->SolISyxStgy=YXSTATE_OPN_CLS_IV;
	dp->SolIDyxStgy=YXSTATE_OPN_CLS_MID_IV;

	dp->YCBH_JudgeMode=YCBH_JUDGE_IDENTICAL;
	dp->YCBH_JudgeType=YCBH_JUDGE_PERCENT;
	dp->YCBH_judge_valve=0.2;
	dp->YCBH_SendStrategy=YCBH_SEND_FIRSTGI;
	dp->YPBH_SendStrategy=YPBH_SEND_FIRSTGI;
	dp->COS_SendFig=COSSEND_LINK;
	dp->SOE_SendFig=SOESEND_LINK;
	dp->CommErrSaveSbiCOSFig=YES;
	dp->CommErrSaveDbiCOSFig=YES;
	dp->CommErrSaveSSOEFig=YES;
	dp->CommErrSaveDSOEFig=YES;
	dp->COSNumPerFrame=40;
	dp->SOENumPerFrame=20;
	dp->YCBHNumPerFrame=20;
	dp->YPBHNumPerFrame=20;
	dp->ASDU_YCBH=9;
	dp->ASDU_YPBH=5;
	dp->ASDU_SSOE=30;
	dp->ASDU_DSOE=31;
	dp->ASDU_COS_TYPE =1;
	dp->ASDU_SOE_TYPE =dp->ASDU_SSOE;
	//dp->ASDU_COS_TYPE =2;
	//dp->ASDU_SOE_TYPE =dp->ASDU_DSOE;	
	dp->ChgSyxSrcFig=CHGSRC_DOWN_RDB;
	dp->ChgSyxStFig=YXSTATE_OPN_CLS_IV;
	dp->ChgDyxSrcFig=CHGSRC_DOWN_RDB;
	dp->ChgDyxStFig=YXSTATE_OPN_CLS_MID_IV;
	dp->TimeDiff_AlertFig=TIMEDIFF_INVALID;
	dp->TimeDiff_valve=3000;
	dp->infaddr_TimeDiff=0;
	dp->TimeDiff_YxIndex=0;
	dp->TimeDiff_YcIndex=0;

	dp->SYK_cmdexe_mode=CTLMODE_Std_S_E;
	dp->ASDU_SYK=45;
	dp->SYK_TimeOut=60;
	dp->SYkOverStgy=YKYTOVER_ALWAYS;
	dp->DYK_cmdexe_mode=CTLMODE_Std_S_E;
	dp->ASDU_DYK=46;
	dp->DYK_TimeOut=60;
	dp->DYkOverStgy=YKYTOVER_ALWAYS;
	dp->YT_cmdexe_mode=CTLMODE_Std_S_E;
	dp->ASDU_YT=47;
	dp->YT_TimeOut=60;
	dp->YtOverStgy=YKYTOVER_SUCESS;

	dp->SigRstStgy=SIGRST_YK_SEL_EXE;
	dp->SigRstIndex=0;
	dp->YkCloseMode_Strategy=YKCLOSEMODE_AUTO_ALWAYS;
	dp->YkCloseMode_YkIndex=0;
	dp->YkCloseMode_YxIndex=0;
	dp->YkCloseMode_count=0;
	dp->YkCloseMode_ValidTime=60;
	dp->FnChkCount=0;
	dp->ProcRstStgy=PROCRST_NOOPER;
	dp->RstRtuCount=0;
	dp->CtlCmdInfTransmitFig=NO;

	dp->BgdScanfig=NO;
	dp->BgdScanPrdTime=600;
	dp->BgdScanDataSeq[0]=GSDATA_SYX;
	dp->BgdScanDataSeq[1]=GSDATA_DYX;
	dp->BgdScanDataSeq[2]=GSDATA_YC;
	dp->BgdScanDataSeq[3]=GSDATA_YP;
	dp->ASDU_YC_BackgroundScan=9;
	dp->ASDU_YX_BackgroundScan=1;
	dp->ASDU_YP_BackgroundScan=5;
	dp->BgdSyxStrategy=YXSTATE_OPN_CLS_IV;
	dp->BgdDyxStrategy=YXSTATE_OPN_CLS_MID_IV;
	dp->YxNumPerFrmBgd=100;
	dp->YcNumPerFrmBgd=50;
	dp->YpNumPerFrmBgd=50;
	dp->PeriodicScanYcFig=YES;
	dp->PeriodicScanYcTime=10;
	dp->ASDU_YC_PeriodicScan=9;

	dp->SYxInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_syx=INF_OBJ_ADDR_YX_BASE;
	dp->DYxInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_dyx=INF_OBJ_ADDR_YX_BASE;
	dp->YcInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_yc=INF_OBJ_ADDR_YC_BASE;
	dp->YmInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_ym=INF_OBJ_ADDR_YM_BASE;
	dp->YpInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_yp=INF_OBJ_ADDR_YP_BASE;
	dp->SYkInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_syk=INF_OBJ_ADDR_YK_BASE;
	dp->DYkInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_dyk=INF_OBJ_ADDR_YK_BASE;
	dp->YtInfaddrType=INFADDR_CONTINUE;
	dp->InfAddr_yt=INF_OBJ_ADDR_YT_BASE;
	dp->InfAddr_PeriodicYcBegin=0;
	dp->InfAddr_PeriodicYcEnd=0;
	for(i=0;i<16;i++) {
		dp->InfAddr_GroupI_begin[i]=0;
		dp->InfAddr_GroupI_end[i]=0;
	}
	for(i=0;i<4;i++) {
		dp->InfAddr_TotalsI_begin[i]=0;
		dp->InfAddr_TotalsI_end[i]=0;
	}

	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		dp->Pri_Data1[i]=PRI_INVALID;
	}
	dp->Pri_Data1[0]=PRI_APPTEST;
	dp->Pri_Data1[1]=PRI_GLOBALI;
	dp->Pri_Data1[2]=PRI_GLOBALI_YX;
	dp->Pri_Data1[3]=PRI_GLOBALI_YC;
	dp->Pri_Data1[4]=PRI_GLOBALI_END;
	dp->Pri_Data1[5]=PRI_CLOCKSYN;
	dp->Pri_Data1[6]=PRI_LINKINITEND;
	dp->Pri_Data1[7]=PRI_YKYT;
	dp->Pri_Data1[8]=PRI_COS;
	dp->Pri_Data1[9]=PRI_SOE;
	dp->Pri_Data1[10]=PRI_YCBH;
	dp->Pri_Data1[11]=PRI_READ;
	dp->Pri_Data1[12]=PRI_PERIODICYC;	
	dp->Pri_Data1[13]=PRI_UNKNOWNAPPMIRROR;		
	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		dp->Pri_Data2[i]=PRI_INVALID;
	}

	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		dp->data1ptr[i]=NULL;
		dp->data2ptr[i]=NULL;
	}
	IEC101YkYtClearClm(dp,&(dp->YkYt));
	dp->fcb2=0;
	dp->LnkFstInitOkFig=NO;
	dp->fresendfig2=NO;
	dp->fsendfig2=NO;
	dp->fsendcount2=0;
	dp->fresendframe2=0;
	dp->LnkInitStartFig2=NO;	
	return;
}

static void IEC101ParameterInitPost(struct APPDEF *dp)
{
	if(dp->linkaddr_bytes==1) {
		dp->min_len=5;
		dp->HeadLen=6;
	}
	else if(dp->linkaddr_bytes==2) {
		dp->min_len=6;
		dp->HeadLen=7;
	}
	else dp->min_len=5;
	dp->comm_count=dp->commout_time;
	dp->DirStrategy=DIR_MASTER_0;
	dp->InitStepFig=INIT_NULL;
	dp->InitStepFig2=INIT_NULL;
	dp->dirfig=0x80;
	dp->scos_count=0;
	dp->dsoe_count=0;
	dp->scos_count=0;
	dp->dsoe_count=0;
	return;
}

static void IEC101SaveReport(struct APPDEF *dp,u_int8 *mes,u_int32 len,u_int8 pri)
{
	struct MSGREPORT *reportptr,*tempptr;
	int i,j;
	u_int8 sum;

	if((len==0)||(len>261)) {
		IEC101Printf(dp,"report save: error: report length is %d.",len);
		return;
	}
	
	if((pri ==PRI_PERIODICYC)||(pri ==PRI_YCBH)){
		reportptr=IEC101ReportPtrCheckPriUnused(dp,pri);
	}
	else{
		reportptr=IEC101ReportPtrCheckUnused(dp);
	}

	if(reportptr==NULL) {
		IEC101Printf(dp,"error: data buffer(%d) overflow, record reject.",RPT_MAX_NUM);
		return;
	}
	i=0;
	reportptr->buf[i++]=0x68;
	reportptr->buf[i++]=0x00;
	reportptr->buf[i++]=0x00;
	reportptr->buf[i++]=0x68;
	reportptr->buf[i++]=0x08;
	reportptr->buf[i++]=(u_int8)(dp->link_addr);
	if(dp->linkaddr_bytes==2) {
		reportptr->buf[i++]=(u_int8)((dp->link_addr)>>8);
	}
	memcpy(&(reportptr->buf[i]),mes,len);
	i+=len;
	sum=0;
	for(j=4;j<i;j++) sum+=reportptr->buf[j];
	reportptr->buf[i++]=sum;
	reportptr->buf[i++]=0x16;
	reportptr->buf[1]=i-6;
	reportptr->buf[2]=i-6;
	reportptr->len=i;
	reportptr->next=NULL;

	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		if(dp->Pri_Data1[i]==pri) break;
	}
	if(i<DATA_PRI_MAXNUM) {
		if(dp->data1ptr[i]==NULL) {
			dp->data1ptr[i]=reportptr;
		}
		else {
			tempptr=dp->data1ptr[i];
			while(tempptr->next!=NULL) {
				tempptr=tempptr->next;
			}
			tempptr->next=reportptr;
		}
		return;
	}
	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		if(dp->Pri_Data2[i]==pri) break;
	}
	if(i<DATA_PRI_MAXNUM) {
		if(dp->data2ptr[i]==NULL) {
			dp->data2ptr[i]=reportptr;
		}
		else {
			tempptr=dp->data2ptr[i];
			while(tempptr->next!=NULL) {
				tempptr=tempptr->next;
			}
			tempptr->next=reportptr;
		}
		return;
	}
	if(reportptr!=NULL) {
		IEC101ReportPtrFree(dp,reportptr);
	}
	if(pri==PRI_LINKINITEND) {
		IEC101Printf(dp,"error: INIT_END: pri unmatch.");
	}
	else if(pri==PRI_GLOBALI) {
		IEC101Printf(dp,"error: General-I: pri unmatch.");
	}
	else if(pri==PRI_GLOBALI_APPACK) {
		IEC101Printf(dp,"error: General-I: pri unmatch.");
	}
	else if(pri==PRI_GLOBALI_YX) {
		IEC101Printf(dp,"error: General-I_BI: pri unmatch.");
	}
	else if(pri==PRI_GLOBALI_YC) {
		IEC101Printf(dp,"error: General-I_Metering: pri unmatch.");
	}
	else if(pri==PRI_GLOBALI_YP) {
		IEC101Printf(dp,"error: General-I_Tap-Position: pri unmatch.");
	}
	else if(pri==PRI_GLOBALI_END) {
		IEC101Printf(dp,"error: General-I_End: pri unmatch.");
	}
	else if(pri==PRI_TOTALSQUERY) {
		IEC101Printf(dp,"error: Accumulator-I: pri unmatch.");
	}
	else if(pri==PRI_GROUPI) {
		IEC101Printf(dp,"error: Group-I: pri unmatch.");
	}
	else if(pri==PRI_CLOCKSYN) {
		IEC101Printf(dp,"error: Clock-SYN: pri unmatch.");
	}
	else if(pri==PRI_COLLECTDELAY) {
		IEC101Printf(dp,"error: Transmission-Delay-Collect: pri unmatch.");
	}
	else if(pri==PRI_YKYT) {
		IEC101Printf(dp,"error: Control/Regulation: pri unmatch.");
	}
	else if(pri==PRI_SETPOINT) {
		IEC101Printf(dp,"error: Set-Point: pri unmatch.");
	}
	else if(pri==PRI_COS) {
		IEC101Printf(dp,"error: COS: pri unmatch.");
	}
	else if(pri==PRI_YCBH) {
		IEC101Printf(dp,"error: Metering-Change: pri unmatch.");
	}
	else if(pri==PRI_YPBH) {
		IEC101Printf(dp,"error: Tap-Position-Change: pri unmatch.");
	}
	else if(pri==PRI_SOE) {
		IEC101Printf(dp,"error: SOE: pri unmatch.");
	}
	else if(pri==PRI_READ) {
		IEC101Printf(dp,"error: Sole-I: pri unmatch.");
	}
	else if(pri==PRI_APPTEST) {
		IEC101Printf(dp,"error: Application-Test: pri unmatch.");
	}
	else if(pri==PRI_RESET) {
		IEC101Printf(dp,"error: Process-Reset: pri unmatch.");
	}
	else if(pri==PRI_DOWNLOADPARA) {
		IEC101Printf(dp,"error: Parameter-Load: pri unmatch.");
	}
	else if(pri==PRI_BACKGROUNDSCAN) {
		IEC101Printf(dp,"error: Background-Scan: pri unmatch.");
	}
	else if(pri==PRI_PERIODICYC) {
		IEC101Printf(dp,"error: Cycle-Metering: pri unmatch.");
	}
	else if(pri==PRI_UNKNOWNAPPMIRROR) {
		IEC101Printf(dp,"error: Unknown-Application-Mirror: pri unmatch.");
	}
	else {
		IEC101Printf(dp,"error: pri unmatch.");
	}
	return;
}

static void IEC101ReceiveDataProcess(struct APPDEF *dp,u_int8 *mes,u_int32 len)
{
	u_int32   i;
	u_int8  *rbuf;
	u_int16 link_addr;
	
	if(dp->ProtoEnableFig==NO) {
		return;
	}
	if(dp->start_fig==NO) {
		IEC101Printf(dp,"warning: ģ����δ��,���Ժ�%d��..",dp->start_time-dp->second_count);
		return;
	}
	
	rbuf=dp->rbuf;
	for(i=0;i<len;i++) {
		printf("dp->ReceSynFig=%d\n", dp->ReceSynFig);
		if(dp->ReceSynFig==NO) {
			rbuf[3]=mes[i];
			if((rbuf[0]==0x68)&&(rbuf[3]==0x68)&&(rbuf[1]==rbuf[2])) {
				dp->ReceSynFig=YES;
				dp->ReceBytes=4+rbuf[1]+2;
				dp->rfig=COMR_LONG_DATA;
				dp->rptr=4;
			}
			else if (0x10 == rbuf[0]){			
				if(dp->linkaddr_bytes==1) link_addr=rbuf[2];
				else if(dp->linkaddr_bytes==2) link_addr=rbuf[2]+rbuf[3]*256;
				else link_addr=rbuf[2];
				if((link_addr==0xff)||(link_addr==0xffff)||(link_addr==dp->link_addr)) {
					dp->rfig=COMR_SHORT_DATA;
					dp->ReceSynFig = YES;
					dp->ReceBytes = dp->min_len;
					dp->rptr = 4;
				}
				else{
					memmove (rbuf, &rbuf[1], 3);
				}			
			}
			else {
				memmove (rbuf, &rbuf[1], 3);
			}
			continue;
		}
		else {
			rbuf[dp->rptr]=mes[i];
			dp->rptr++;					
			if(dp->rptr>=1000) {
				dp->ReceSynFig=NO;
				dp->ReceBytes=0;
				dp->rptr=0;
				dp->rlen=0;
				dp->rfig = COMR_NO_DATA;
				memset(dp->rbuf,0,1000);
				return;
			}
			if((dp->rptr)>=(dp->ReceBytes)) {
				dp->rptr = dp->ReceBytes;
				dp->rlen=dp->rptr;			
				if(dp->PrintMsgFig==PRTMSG_FRM) {
    					IEC101PrintMsg(dp,dp->rbuf,dp->rlen,0);
    				}
				if(IEC101CheckData(dp)==YES)
				{
					printf("dp->start_fig=%d\n",dp->start_fig);
					if(dp->start_fig==YES)
					{
						IEC101ReceiveDataClm(dp);
					}
					else {
						IEC101Printf(dp,"warning: ģ����δ��,���Ժ�%d��.",dp->start_time-dp->second_count);
					}
				}	 	
				dp->ReceSynFig=NO;
				dp->ReceBytes=0;
				dp->rptr=0;
				dp->rlen=0;
				dp->rfig =COMR_NO_DATA;
				memset(dp->rbuf,0,1000);
			}
		}
	}
	return;
}

static u_int8 IEC101CheckData(struct APPDEF *dp)
{
	u_int32 i;
	u_int8 sum;
	u_int16 link_addr = 0;

	sum=0;
	if(dp->rbuf[dp->rlen-1]!=0x16) {
		IEC101Printf(dp,"error: Receive error, last byte is not 0x16.");
		return(NO);
	}
	if(dp->rfig==COMR_LONG_DATA) {
		for(i=4;i<(dp->rlen-2);i++)  sum+=dp->rbuf[i];
		if(dp->linkaddr_bytes==1) link_addr=dp->rbuf[5];
		else if(dp->linkaddr_bytes==2) link_addr=dp->rbuf[5]+dp->rbuf[6]*256;
		else link_addr=dp->rbuf[5];
	}
	if(dp->rfig==COMR_SHORT_DATA) {
		for(i=1;i<(dp->rlen-2);i++) sum+=dp->rbuf[i];
		if(dp->linkaddr_bytes==1) link_addr=dp->rbuf[2];
		else if(dp->linkaddr_bytes==2) link_addr=dp->rbuf[2]+dp->rbuf[3]*256;
		else link_addr=dp->rbuf[2];
	}
	if((dp->linkaddr_bytes==1)&&(link_addr!=0xff)&&(link_addr!=dp->link_addr)) {
		IEC101Printf(dp,"error: Receive error, link address is not match,rece=%02x,para=%02x",link_addr,dp->link_addr);
		return(NO);
	}
	if((dp->linkaddr_bytes==2)&&(link_addr!=0xffff)&&(link_addr!=dp->link_addr)) {
		IEC101Printf(dp,"error: Receive error, link address is not match,rece=%02x,para=%02x",link_addr,dp->link_addr);
		return(NO);
	}
	if(sum!=dp->rbuf[dp->rlen-2]) {
		IEC101Printf(dp,"error: Receive error, CRC is wrong,receive %02x,should %02x.",dp->rbuf[dp->rlen-2],sum);
		return(NO);
	}
	return(YES);
}

static void IEC101ReceiveDataClm(struct APPDEF *dp)
{
	dp->comm_count=0;
	IEC101UpdateCommusta(dp,RDB_RET_NORMAL,COMMUCOT_RECV);
	IEC101LinkDataClm(dp);
	IEC101ApplicationDataClm(dp);
	IEC101SendDataClm(dp);
	return;
}

static void IEC101LinkDataClm(struct APPDEF *dp)
{
	u_int8 cw,fcv,fcb,prm,dir,fig;

	dp->datastep_fig=LINKPROCESS;
	if(dp->rfig==COMR_SHORT_DATA) {
		cw=(dp->rbuf[1])&0x0f;
		fcv=(dp->rbuf[1])&0x10;
		fcb=(dp->rbuf[1])&0x20;
		prm=(dp->rbuf[1])&0x40;
		dir=(dp->rbuf[1])&0x80;
	}
	else if(dp->rfig==COMR_LONG_DATA) {
		cw=(dp->rbuf[4])&0x0f;
		fcv=(dp->rbuf[4])&0x10;
		fcb=(dp->rbuf[4])&0x20;
		prm=(dp->rbuf[4])&0x40;
		dir=(dp->rbuf[4])&0x80;
	}
	else {
		return;
	}
	
	if((dp->InitStepFig) & INIT_LINKREPLY){
		if(dp->DirStrategy==DIR_MASTER_1){
			if(dir!=0x80) {
				IEC101Printf(dp,"warning: �յ��ı���DIR����(=0),���账��.");
				return;
			}
		}
		else {
			if(dir!=0) {
				IEC101Printf(dp,"warning: �յ��ı���DIR����(=1),���账��.");
				return;
			}
		}
	}

	fig=NO;
	dp->resendfig=NO;
	dp->TestCount=0;
	dp->fresendframe2 = 0; 
	if(prm==PRM) {
		if(fcv>0) {
			if(((dp->InitStepFig)&INIT_FCBRST)==0x00) {
				if(dp->LinkAnswerStgy==LINKPREINIT_NOANSWER) {
					return;
				}
				else if(dp->LinkAnswerStgy==LINKPREINIT_NOWORK) {
					IEC101ReportLinkState(dp,LINK_NOWORK);
				}
				else if(dp->LinkAnswerStgy==LINKPREINIT_NOWORK1) {
					return;
				}
				else if(dp->LinkAnswerStgy==LINKPREINIT_NOWORK2) {
					IEC101ReportLinkState(dp,LINK_NOWORK);
				}
				else {
					return;
				}
			}
			else {
				if((cw!=2)&&(cw!=3)) {
					IEC101Printf(dp,"error: ƽ����վ����������(%d)������.",cw);
					return;
				}
				if((dp->FcbStartFig==NO)&&(dp->FcbPostState==FCBPOST_FUZZY)) {
					if(fcb==FCB_RST0) {
						dp->fcb=FCB_RST1;
					}
					if(fcb==FCB_RST1) {
						dp->fcb=FCB_RST0;
					}
					dp->FcbStartFig=YES;
				}
				if((dp->fcb!=fcb)||(dp->FcbReplyStrategy==FCB_REPLY_NORMAL)) {
					dp->fcb=fcb;
					if((cw==3)&&((dp->rlen)>(dp->min_len))) {
						IEC101ReportACK(dp);
						fig=YES;
					}
					else if((cw==2)&&((dp->rlen)==(dp->min_len))){
						IEC101ReportACK(dp);
						IEC101Printf(dp,"ƽ����վ��֡������·,fcv=1ȷ��.");							
					}
					else {							
						if(dp->LinkIlctlStgy==LINKILCTL_NOWORK) {
							IEC101ReportLinkState(dp,LINK_NOWORK);
						}
						else if(dp->LinkIlctlStgy==LINKILCTL_NOCOMPLETE) {
							IEC101ReportLinkState(dp,LINK_NOCOMPLETE);
						}
						else{
							IEC101ReportLinkState(dp,LINK_NOWORK);
						}							
					}
				}
				else {
					IEC101Printf(dp,"warning: ƽ����վFCBû�з�ת,��վ���ط�.");
					dp->resendfig=YES;
				}
			}
		}
		else {
			if(cw==0) {
				IEC101Printf(dp,"ƽ����վ��λFCB״̬.");					
				if ((LINKRULE_ZHUHAI_XUJI == dp->LnkRuleType) || 
				    (LINKRULE_DONGFANG     == dp->LnkRuleType))
				{
						IEC101CommClearClm(dp);
						/* ���麣�����վ��ʼ���ҷ���·��ʱ�����Ҫ������Ҳ��ʼ���Է���·*/
				}
				IEC101ReportFcbConfirm(dp);
				if(dp->FcbPostState==FCBPOST_0) {
					dp->fcb=FCB_RST1;
				}
				else if(dp->FcbPostState==FCBPOST_1) {
					dp->fcb=FCB_RST0;
				}
				else {
					dp->fcb=FCB_RST0;
				}
				dp->FcbStartFig=NO;
				dp->InitStepFig=INIT_NULL;
				IEC101ClearBufferClm(dp,CLRBUF_FCBRST);
				if((dp->ASDU70ValidStrategy==ASDU70_VALID_FCB)
				||((dp->ASDU70ValidStrategy==ASDU70_VALID_L_FCB)&&(dp->InitLocationFig==0))
				||((dp->ASDU70ValidStrategy==ASDU70_VALID_R_FCB)&&(dp->InitLocationFig==2))
				||((dp->ASDU70ValidStrategy==ASDU70_VALID_LR_FCB)&&(dp->InitLocationFig!=1))) {
					IEC101ReportRstEnd(dp);
				}
				if(dp->InitLocationFig==0) {
					dp->InitLocationFig=1;
				}
				if(dp->ASDU70ValidFig==YES) {
					IEC101ReportRstEnd(dp);
				}
			}
			else if(cw==1) {
				IEC101ReportACK(dp);
				fig=YES;
			}
			else if((cw==2)&&((dp->rlen)==(dp->min_len))) {
				IEC101ReportACK(dp);
				IEC101Printf(dp,"ƽ����վ��֡������·,fcv=0ȷ��.");
			}				
			else if(cw==4) {
				if((dp->rlen)>(dp->min_len)) {
					fig=YES;
				}
			}
			else if(cw==9) {
				if(dir){ 
					dp->DirStrategy=DIR_MASTER_1; 
					dp->dirfig=0x00;
				}
				else{    
					dp->DirStrategy=DIR_MASTER_0; 
					dp->dirfig=0x80;
				}
				IEC101Printf(dp,"ƽ����վ��ѯ��·״̬.");
				IEC101ReportLinkState(dp,LINK_OK);				
			}
			else {
				if(((dp->InitStepFig)&INIT_FCBRST)==0x00) {						
					if(dp->LinkAnswerStgy==LINKPREINIT_NOANSWER) {
						return;
					}
					else if(dp->LinkAnswerStgy==LINKPREINIT_NOWORK) {
						IEC101ReportLinkState(dp,LINK_NOWORK);
					}
					else if(dp->LinkAnswerStgy==LINKPREINIT_NOCOMPLETE) {
						IEC101ReportLinkState(dp,LINK_NOCOMPLETE);
					}
					else if(dp->LinkAnswerStgy==LINKPREINIT_NOWORK1) {
						IEC101ReportLinkState(dp,LINK_NOWORK);
					}
					else if(dp->LinkAnswerStgy==LINKPREINIT_NOWORK2) {
						return;
					}
					else {
						return;
					}												
				}
				else {
					if(dp->LinkAnswerStgy==LINKPREINIT_NOCOMPLETE) {
						IEC101ReportLinkState(dp,LINK_NOCOMPLETE);
					}						
				}
			}
		}
	}
	else {
		if(dp->rfig==COMR_SHORT_DATA) {
			if(((dp->InitStepFig2)&INIT_LINKREPLY)==0) {
				if(cw==0x0b) {
					dp->InitStepFig2 |=INIT_LINKREPLY;
					IEC101Printf(dp,"�յ�ƽ����վӦ���ѯ��·.");
				}
			}
			if(((dp->InitStepFig2)&INIT_FCBRST)==0) {
				if(cw==0x00) {
					dp->InitStepFig2 |=INIT_LINKREPLY;
					dp->InitStepFig2 |=INIT_FCBRST;
					IEC101Printf(dp,"�յ�ƽ����վӦ��λ��·.");
				}
			}
			else{
				IEC101Printf(dp,"�յ�ƽ����վ��֡,cw=%d.",cw);
			}
			dp->fsendcount2=0;
			dp->fsendfig2=NO;
		}
		else{
			IEC101Printf(dp,"err,�յ�ƽ����վ�Ƕ�֡.");
		}			
		return;
	}
	if((fig==YES)&&(dp->resendfig==NO)) {
		dp->datastep_fig=APPLICATIONPROCESS;
		dp->FcbCount=0;
	}
	else {
		dp->datastep_fig=SENDPROCESS;
		if(dp->resendfig==YES) {
			if(dp->FcbReplyStrategy==FCB_REPLY_NO) {
				dp->datastep_fig=LINKPROCESS;
			}
			if(dp->FcbReplyStrategy==FCB_REPLY_RESEND_3) {
				dp->FcbCount++;
				if(dp->FcbCount>=4) {
					dp->datastep_fig=LINKPROCESS;
					dp->FcbCount=0;
					IEC101Printf(dp,"warning: ��վFCB��������ת4�Σ���վֹͣ��Ӧ.");
				}
			}
		}
		else {
			dp->FcbCount=0;
		}
	}
	
	return;
}

static void IEC101ApplicationDataClm(struct APPDEF *dp)
{
	u_int32 i;
	u_int16 cot;
	u_int8 asdutype,cw/*,vsq*/,qoi,fig;
	struct ASDUHEAD head;
	//u_int16 cot_temp;

	if(dp->datastep_fig!=APPLICATIONPROCESS) {
		return;
	}
	if((dp->rlen)<=(dp->min_len)) {
		return;
	}
	#if 0
	if(dp->linkaddr_bytes==2) {
		cot_temp = 7;
	}
	else{
		cot_temp = 6;
	}
	#endif
	i=IEC101GetAsduCommonFromMsg(dp,&head,dp->rbuf);
	asdutype=head.asdutype;
	//vsq=head.vsq;
	cot=head.cot;
	dp->rece_appaddr=head.appaddr;
	if(dp->infaddr_bytes==2) {
		dp->rece_infaddr=dp->rbuf[i]+dp->rbuf[i+1]*256;
		i+=2;
	}
	else if(dp->infaddr_bytes==3) {
		dp->rece_infaddr=dp->rbuf[i]+dp->rbuf[i+1]*256+dp->rbuf[i+2]*65536;
		i+=3;
	}
	else {
		dp->rece_infaddr=dp->rbuf[i]+dp->rbuf[i+1]*256;
		i+=2;
	}
	if((dp->appaddr_bytes==1)&&((dp->rece_appaddr)==0xff)) {
		dp->rece_appaddr=dp->appaddr;
	}
	if((dp->appaddr_bytes==2)&&((dp->rece_appaddr)==0xffff)) {
		dp->rece_appaddr=dp->appaddr;
	}
	qoi=dp->rbuf[i];
	cw=(dp->rbuf[4])&0x0f;

	fig=NO;
	if((cw==1)||(cw==2)||(cw==3)||(cw==4)) {
		if((asdutype==45)||(asdutype==46)) {
			IEC101YkYtClm(dp);
			fig=YES;
		}
		if(asdutype==70) {
			fig=YES;
		}
		if(asdutype ==100) {
			if((cot!=8)&&(qoi==20)) {
				IEC101Printf(dp,"General-I: master active");
				IEC101ReportGlbI(dp);
				fig=YES;
			}
			if((cot==8)&&(qoi==20)) {
				IEC101Printf(dp,"General/Group-I: master cancel.");
				IEC101ReportICancel(dp,qoi);
				fig=YES;
			}
			if(fig==NO) {
				IEC101ReportGlbIConfirm2(dp);
				IEC101Printf(dp,"General-I: error: command invalid, cot=%d,qoi=%d",cot,qoi);
				fig=YES;
			}
		}
		if(asdutype==103) {
			IEC101ReportSetClock(dp);
			fig=YES;
		}
		if(asdutype==104) {
			IEC101ReportTest(dp);
			fig=YES;
		}
	}
	if(fig==NO) {
		IEC101ReportInvalidAsdu(dp);
	}
	if(cw==4) {
		dp->datastep_fig=LINKPROCESS;
	}
	else {
		dp->datastep_fig=SENDPROCESS;
	}
	return;
}

static void IEC101ReportInvalidAsdu(struct APPDEF *dp)
{
	u_int16 i,j;
	u_int8 sbuf[300];
	struct ASDUHEAD head;

	IEC101GetAsduCommonFromMsg(dp,&head,dp->rbuf);
	i=0;
	sbuf[i++]=head.asdutype;
	sbuf[i++]=head.vsq;
	sbuf[i++]=44|P_N;
	if(dp->cot_bytes==2) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	for(j=i+dp->HeadLen;j<(dp->rlen-2);j++) {
		sbuf[i++]=dp->rbuf[j];
	}
	IEC101SaveReport(dp,sbuf,i,PRI_UNKNOWNAPPMIRROR);
	IEC101Printf(dp,"application answer: error: unknown ASDU-TI(%d).",head.asdutype);
	return;
}

static void IEC101SendDataClm(struct APPDEF *dp)
{
	u_int32 i;
	u_int8 sum;

	if(dp->datastep_fig!=SENDPROCESS) {
		return;
	}
	if(dp->slen==0) {
		IEC101ReportNoData(dp);
	}
	if(dp->resendfig==NO) {
		IEC101CheckAcdStatus(dp);
		if((dp->sbuf[0]==0x10)&&((dp->slen)==(dp->min_len))) {
				dp->sbuf[1]+=dp->acd + (dp->dirfig);
				dp->sbuf[dp->slen-2]+=dp->acd + (dp->dirfig);				
		}
		else {
				dp->sbuf[4]+=dp->acd + (dp->dirfig);
				dp->sbuf[dp->slen-2]+=dp->acd + (dp->dirfig);	
		}
	}
	if((dp->sbuf[0]==0x10)&&(((dp->sbuf[1])&0x0f)==0x00)){
		dp->InitStepFig |=INIT_LINKREPLY;
		dp->InitStepFig |=INIT_FCBRST;
		//dp->InitStepFig2=dp->InitStepFig;
	 }	
	if((dp->sbuf[0]==0x10)&&(((dp->sbuf[1])&0x0f)==0x0b)){
		dp->sbuf[1] &=0x8f;
		sum=0;
		for(i=1;i<(dp->slen-2);i++){
			sum +=dp->sbuf[i];
		}
		dp->sbuf[dp->slen-2]=sum;
		dp->InitStepFig |=INIT_LINKREPLY;
	}
	IEC101SendData(dp,dp->sbuf,dp->slen);
	dp->datastep_fig=LINKPROCESS;
	return;
}

static void IEC101SendData(struct APPDEF *dp,u_int8 *sbuf,u_int32 slen)
{
	int ret;
	
	if(slen>0) {
		//ret = uart_write((UART_Handle)dp->comfd,sbuf,slen,dp->rs_mode, dp->com_no);
		show_msg("send msg", (char *)sbuf, slen);
		ret = UART_write((UART_Handle)dp->comfd, sbuf, slen);
		if(ret != slen){
    			_DPRINTF("iec101 send data error.\n");
    		}
		IEC101PrintMsg(dp,sbuf,slen,1);
		IEC101SendedDataChk(dp,sbuf,slen);
		dp->TestCount=0;
	}
	return;
}

static void IEC101SendedDataChk(struct APPDEF *dp,u_int8 *sbuf,u_int32 slen)
{
	int i;
	struct ASDUHEAD head;
	u_int8 qoi;

	if(slen > (dp->min_len)){
		i=IEC101GetAsduCommonFromMsg(dp,&head,sbuf);
		qoi=sbuf[i+dp->infaddr_bytes];
		if((head.asdutype==0x64)&&(head.cot==0x0a)&&(qoi==0x14)) {
			dp->InitStepFig |=INIT_GI;
		}
		if((head.asdutype==0x67)&&(head.cot==0x07)) {
			dp->InitStepFig |=INIT_TIMESYN;
		}
	}
	return;
}

static void IEC101ReportNoData(struct APPDEF *dp)
{
	u_int8 sbuf[10],sum;
	u_int16 i,j;

	i=0;
	sum=0;
	sbuf[i++]=0x10;
	sbuf[i++]=0x09;
	sbuf[i++]=(u_int8)(dp->link_addr);
	if(dp->linkaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->link_addr)>>8);
	}
	for(j=1;j<i;j++) sum+=sbuf[j];
	sbuf[i++]=sum;
	sbuf[i++]=0x16;
	memcpy(dp->sbuf,sbuf,i);
	dp->slen=i;
	return;
}

static void IEC101ReportLinkState(struct APPDEF *dp,u_int8 linksta)
{
	u_int8 sbuf[10],sum;
	u_int16 i,j;

	i=0;
	sum=0;
	sbuf[i++]=0x10;
	sbuf[i++]=linksta;
	sbuf[i++]=(u_int8)(dp->link_addr);
	if(dp->linkaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->link_addr)>>8);
	}
	for(j=1;j<i;j++) sum+=sbuf[j];
	sbuf[i++]=sum;
	sbuf[i++]=0x16;
	memcpy(dp->sbuf,sbuf,i);
	dp->slen=i;
	if(linksta==LINK_OK) {
		IEC101Printf(dp,"answer link-status : OK.");
	}
	else if(linksta==LINK_NOWORK) {
		IEC101Printf(dp,"answer link-status : NO WORK.");
	}
	else if(linksta==LINK_NOCOMPLETE) {
		IEC101Printf(dp,"answer link-status : NO COMPLETE .");
	}
	else {
		IEC101Printf(dp,"answer link-status : ERROR.");
	}
	return;
}

static void IEC101ReportFcbConfirm(struct APPDEF *dp)
{
	u_int8 sbuf[10],sum;
	u_int16 i,j;

	i=0;
	sum=0;
	sbuf[i++]=0x10;
	sbuf[i++]=0x00;
	sbuf[i++]=(u_int8)(dp->link_addr);
	if(dp->linkaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->link_addr)>>8);
	}
	for(j=1;j<i;j++) sum+=sbuf[j];
	sbuf[i++]=sum;
	sbuf[i++]=0x16;
	memcpy(dp->sbuf,sbuf,i);
	dp->slen=i;
	return;
}

static void IEC101ReportRstEnd(struct APPDEF *dp)
{
	u_int8 sbuf[100];
	int i;

	i=0;
	sbuf[i++]=0x46;
	sbuf[i++]=0x01;
	sbuf[i++]=0x04;
	if(dp->cot_bytes==2) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	sbuf[i++]=0x00;
	sbuf[i++]=0x00;
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=0x00;
	}
	if(dp->ASDU70ValidFig==YES) {
		sbuf[i++]=0x00;/*��ʼ��ԭ�򣺱����ϵ縴λ*/
	}
	else {
		if((dp->ASDU70ValidStrategy==ASDU70_VALID_L_FCB)&&(dp->InitLocationFig==0)) {
			sbuf[i++]=0x00;/*��ʼ��ԭ�򣺱����ϵ縴λ*/
		}
		else if((dp->ASDU70ValidStrategy==ASDU70_VALID_R_FCB)&&(dp->InitLocationFig==2)) {
			sbuf[i++]=0x02;/*��ʼ��ԭ��Զ����λ*/
		}
		else {
			sbuf[i++]=0x00;/*��ʼ��ԭ�򣺱����ϵ縴λ*/
		}
	}
	IEC101SaveReport(dp,sbuf,i,PRI_LINKINITEND);
	return;
}

static void IEC101ReportACK(struct APPDEF *dp)
{
	u_int8 sbuf[10],sum;
	u_int16 i,j;

	i=0;
	sum=0;
	sbuf[i++]=0x10;
	sbuf[i++]=0x00;
	sbuf[i++]=(u_int8)(dp->link_addr);
	if(dp->linkaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->link_addr)>>8);
	}
	for(j=1;j<i;j++) sum+=sbuf[j];
	sbuf[i++]=sum;
	sbuf[i++]=0x16;
	memcpy(dp->sbuf,sbuf,i);
	dp->slen=i;
	return;
}
/*
static void IEC101ReportACK2(struct APPDEF *dp)
{
	u_int8 sbuf[10],sum;
	u_int16 i,j;

	i=0;
	sum=0;
	sbuf[i++]=0x10;
	sbuf[i++]=0x00;
	sbuf[i++]=(u_int8)(dp->link_addr);
	if(dp->linkaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->link_addr)>>8);
	}
	for(j=1;j<i;j++) sum+=sbuf[j];
	sbuf[i++]=sum;
	sbuf[i++]=0x16;
	memcpy(dp->sbuf,sbuf,i);
	dp->slen=i;
	return;
}
*/
static void IEC101CheckAcdStatus(struct APPDEF *dp)
{
	
	if(dp->CommuMode==MODE_BALANCE) {
		dp->acd=0;
		return;
	}
	return;
}

static void IEC101ReportBhYc(struct APPDEF *dp,int event_num)
{
	u_int32 i,j,ptr,bnum,num,infaddr;
	u_int8 squality,fquality;
	u_int8 sbuf[300];
	u_int16 temp;
	short sval=0;
	float fval=0;
	unsigned char cp56t [7];
	
	if(event_num>0) {
		ptr=0;
		if(dp->ASDU_YCBH==9) {
			bnum=IEC101CheckTelegramLen(dp,DGLEN_YC_SINGLE_NOTIME_ASDU9,GRAM_NULL);
		}
		else if(dp->ASDU_YCBH==11){
			bnum=IEC101CheckTelegramLen(dp,DGLEN_YC_SINGLE_NOTIME_ASDU11,GRAM_NULL);
		}
		else if(dp->ASDU_YCBH==13){
			bnum=IEC101CheckTelegramLen(dp,DGLEN_YC_SINGLE_NOTIME_ASDU13,GRAM_NULL);
		}
		else if(dp->ASDU_YCBH==21){
			bnum=IEC101CheckTelegramLen(dp,DGLEN_YC_SINGLE_NOTIME_ASDU21,GRAM_NULL);
		}
		else {
			bnum=IEC101CheckTelegramLen(dp,DGLEN_YC_SINGLE_TIME,GRAM_NULL);
		}
		
		if(bnum==0) {
			return;
		}
		
		while(ptr<event_num) {
			i=0;
			sbuf[i++]=dp->ASDU_YCBH;
			temp=i;
			sbuf[i++]=0x00;
			sbuf[i++]=0x03;
			if(dp->cot_bytes==2) {
				sbuf[i++]=0x00;
			}
			sbuf[i++]=(u_int8)(dp->appaddr);
			if(dp->appaddr_bytes==2) {
				sbuf[i++]=(u_int8)((dp->appaddr)>>8);
			}
			num=0;
			for(j=ptr;j<event_num;j++) {
				ptr++;				
				infaddr=dp->yc_event_lst [j].data_no + dp->InfAddr_yc;
				sbuf[i++]=(u_int8)(infaddr);
				sbuf[i++]=(u_int8)(infaddr>>8);
				if(dp->infaddr_bytes==3) {
					sbuf[i++]=(u_int8)(infaddr>>16);
				}
				if (dp->yc_event_lst [j].data_type == YC_INT) {
					sval = dp->yc_event_lst [j].event_val.i_val;
					fval = sval;
				}
				else {
					fval = dp->yc_event_lst [j].event_val.f_val;
					sval = fval;
				}
				
				fquality=dp->yc_event_lst [j].qds;
				squality=dp->yc_event_lst [j].qds;
				if(dp->ASDU_YCBH==9) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
				}
				else if(dp->ASDU_YCBH==10) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
					time_to_cp56time2a (&(dp->yc_event_lst [j].time_val), cp56t);
					sbuf[i++] = cp56t [0];
					sbuf[i++] = cp56t [1];
					sbuf[i++] = cp56t [2];					
				}
				else if(dp->ASDU_YCBH==11) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
				}
				else if(dp->ASDU_YCBH==12) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
					time_to_cp56time2a (&(dp->yc_event_lst [j].time_val), cp56t);
					sbuf[i++] = cp56t [0];
					sbuf[i++] = cp56t [1];
					sbuf[i++] = cp56t [2];		
				}
				else if(dp->ASDU_YCBH==13) {
					i=IEC101FillFloatYcVal(dp,sbuf,i,fval);
					sbuf[i++]=fquality;
				}
				else if(dp->ASDU_YCBH==14) {
					i=IEC101FillFloatYcVal(dp,sbuf,i,fval);
					sbuf[i++]=fquality;
					time_to_cp56time2a (&(dp->yc_event_lst [j].time_val), cp56t);
					sbuf[i++] = cp56t [0];
					sbuf[i++] = cp56t [1];
					sbuf[i++] = cp56t [2];		
				}
				else if(dp->ASDU_YCBH==21) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
				}
				else if(dp->ASDU_YCBH==34) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
					time_to_cp56time2a (&(dp->yc_event_lst [j].time_val), cp56t);
					sbuf[i++] = cp56t [0];
					sbuf[i++] = cp56t [1];
					sbuf[i++] = cp56t [2];			
					sbuf[i++] = cp56t [3];
					sbuf[i++] = cp56t [4];
					sbuf[i++] = cp56t [5];		
					sbuf[i++] = cp56t [6];								
				}
				else if(dp->ASDU_YCBH==35) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
					time_to_cp56time2a (&(dp->yc_event_lst [j].time_val), cp56t);
					sbuf[i++] = cp56t [0];
					sbuf[i++] = cp56t [1];
					sbuf[i++] = cp56t [2];			
					sbuf[i++] = cp56t [3];
					sbuf[i++] = cp56t [4];
					sbuf[i++] = cp56t [5];		
					sbuf[i++] = cp56t [6];	
				}
				else if(dp->ASDU_YCBH==36) {
					i=IEC101FillFloatYcVal(dp,sbuf,i,fval);
					sbuf[i++]=fquality;
					time_to_cp56time2a (&(dp->yc_event_lst [j].time_val), cp56t);
					sbuf[i++] = cp56t [0];
					sbuf[i++] = cp56t [1];
					sbuf[i++] = cp56t [2];			
					sbuf[i++] = cp56t [3];
					sbuf[i++] = cp56t [4];
					sbuf[i++] = cp56t [5];		
					sbuf[i++] = cp56t [6];	
				}
				else {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
				}
				num++;
				if(num>=bnum) break;
			}
			sbuf[temp]=num;
			if(num==0) {
				if(j>=event_num) break;
				continue;
			}
			IEC101SaveReport(dp,sbuf,i,PRI_YCBH);
		}
		return;
	}
}

static u_int8 IEC101ReportCOS(struct APPDEF *dp,int event_num)
{
	u_int32 i,bnum,num;
	u_int8 sbuf[300],result,asdutype=0,yx_val=0;
	u_int32   j=0,infaddr=0;

	result=NO;
	if(event_num > 0) {
		bnum=IEC101CheckTelegramLen(dp,DGLEN_YX_SINGLE,GRAM_NULL);
		if(bnum==0) {
			return(result);
		}
		i=0;
		sbuf[i++]=0;
		sbuf[i++]=0;
		sbuf[i++]=0x03;
		if(dp->cot_bytes==2) {
			sbuf[i++]=0x00;
		}
		sbuf[i++]=(u_int8)(dp->appaddr);
		if(dp->appaddr_bytes==2) {
			sbuf[i++]=(u_int8)((dp->appaddr)>>8);
		}
		num=0;
		j=0;
		while(1) {
				asdutype=dp->ASDU_COS_TYPE;
				if(asdutype == 1){
						infaddr = dp->yx_event_lst [j].data_no + dp->InfAddr_syx;
						switch (dp->yx_event_lst [j].event_val) {
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
				else if(asdutype == 3){
						infaddr = dp->yx_event_lst [j].data_no + dp->InfAddr_dyx; 
						switch (dp->yx_event_lst [j].event_val) {
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
				sbuf[i++]=(u_int8)(infaddr);
				sbuf[i++]=(u_int8)(infaddr>>8);
				if(dp->infaddr_bytes==3) {
					sbuf[i++]=(u_int8)(infaddr>>16);
				}
				sbuf[i++]=yx_val;
				j++;
				num++;
				if(num>=bnum) {
					break;
				}

		}
		if(num==0) {
			return(result);
		}
		sbuf[0]=asdutype;
		sbuf[1]=num;
		IEC101SaveReport(dp,sbuf,i,PRI_COS);
		result=YES;
		if(asdutype==1) {
			dp->scos_count+=num;
			IEC101Printf(dp,"COS_RPT: SBI, num.%d, (s_total.%d).",num,dp->scos_count);
		}
		else if(asdutype==3) {
			dp->dcos_count+=num;
			IEC101Printf(dp,"COS_RPT: DBI, num.%d, (d_total.%d).",num,dp->dcos_count);
		}
		else {
			IEC101Printf(dp,"COS: error: COS-frame asdu type error(%d), include %d COS.",asdutype,num);
		}
	}
	return(result);
}

static u_int8 IEC101ReportSOE(struct APPDEF *dp,int event_num)
{
	u_int32 i,j,bnum,num;
	u_int8 sbuf[300],result,asdutype=0,yx_val=0;
	u_int32 infaddr=0;
	unsigned char cp56t [7];

	result=NO;
	if(event_num > 0) {
		bnum=IEC101CheckTelegramLen(dp,DGLEN_SOE,GRAM_NULL);
		if(bnum==0) {
			return(result);
		}
		i=0;
		sbuf[i++]=0;
		sbuf[i++]=0;
		sbuf[i++]=0x03;
		if(dp->cot_bytes==2) {
			sbuf[i++]=0x00;
		}
		sbuf[i++]=(u_int8)(dp->appaddr);
		if(dp->appaddr_bytes==2) {
			sbuf[i++]=(u_int8)((dp->appaddr)>>8);
		}
		num=0;
		j=0;
		while(1) {
				asdutype=dp->ASDU_SOE_TYPE;
				if((asdutype == 30)||(asdutype == 2)){
						infaddr = dp->yx_event_lst [j].data_no + dp->InfAddr_syx;
						switch (dp->yx_event_lst [j].event_val) {
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
				else if((asdutype == 31)||(asdutype == 4)){
						infaddr = dp->yx_event_lst [j].data_no + dp->InfAddr_dyx; 
						switch (dp->yx_event_lst [j].event_val) {
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
				sbuf[i++]=(u_int8)(infaddr);
				sbuf[i++]=(u_int8)(infaddr>>8);
				if(dp->infaddr_bytes==3) {
					sbuf[i++]=(u_int8)(infaddr>>16);
				}
				sbuf[i++]=yx_val;
				time_to_cp56time2a (&(dp->yx_event_lst [j].time_val), cp56t);				
				sbuf[i++]=cp56t[0];
				sbuf[i++]=cp56t[1];
				sbuf[i++]=cp56t[2];
				if((asdutype==30)||(asdutype==31)) {
					sbuf[i++]=cp56t[3];
					sbuf[i++]=cp56t[4];
					sbuf[i++]=cp56t[5];
					sbuf[i++]=cp56t[6];
				}
				j++;
				num++;
				if(num>=bnum) {
					break;
				}

		}
		if(num==0) {
			return(result);
		}
		sbuf[0]=asdutype;
		sbuf[1]=num;
		IEC101SaveReport(dp,sbuf,i,PRI_SOE);
		result=YES;
		if((asdutype==2)||(asdutype==30)) {
			IEC101Printf(dp,"SOE_RPT: SBI, num.%d, (s_total.%d).",num,dp->ssoe_count);
		}
		else if((asdutype==4)||(asdutype==31)) {
			IEC101Printf(dp,"SOE_RPT: DBI, num.%d, (d_total.%d).",num,dp->dsoe_count);
		}
		else {
			IEC101Printf(dp,"SOE_RPT: error: SOE-frame asdu type error(%d), include %d SOE.",asdutype,num);
		}
	}
	return(result);
}

static u_int16 IEC101CheckTelegramLen(struct APPDEF *dp,u_int8 data_type,u_int8 gramfig)
{
	u_int16 num,len;

	len=dp->telegram_MAXbytes-6;
	if(dp->linkaddr_bytes==1) {
		len=len-1;
	}
	else if(dp->linkaddr_bytes==2) {
		len=len-2;
	}
	else {
		len=len-1;
	}
	len--;
	len--;
	if(dp->cot_bytes==1) {
		len=len-1;
	}
	else if(dp->cot_bytes==2) {
		len=len-2;
	}
	else {
		len=len-1;
	}
	if(dp->appaddr_bytes==1) {
		len=len-1;
	}
	else if(dp->appaddr_bytes==2) {
		len=len-2;
	}
	else {
		len=len-1;
	}
	len--;
	len--;

	if(data_type==DGLEN_YX_SINGLE) {
		num=((len/4)/5)*5;
		if(num>dp->COSNumPerFrame) {
			num=dp->COSNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YX_CONTINUE) {
		if(dp->infaddr_bytes==2) {
			len=len-2;
		}
		else if(dp->infaddr_bytes==3) {
			len=len-3;
		}
		else {
			len=len-2;
		}
		if(gramfig==GRAM_YX_GLOBALI) {
			if(len>=(dp->YxNumPerFrmGlobalI)) {
				num=dp->YxNumPerFrmGlobalI;
			}
			else {
				if(len<=10)	num=len;
				else {
					num=(len/10)*10;
					if(num>100) {
						num=100;
					}
				}
			}
		}
		else if(gramfig==GRAM_YX_GROUPI) {
			if(len>=(dp->YxNumPerFrmGroupI)) {
				num=dp->YxNumPerFrmGroupI;
			}
			else {
				if(len<=10)	num=len;
				else {
					num=(len/10)*10;
					if(num>100) {
						num=100;
					}
				}
			}
		}
		else if(gramfig==GRAM_YX_BACKGROUND) {
			if(len>=(dp->YxNumPerFrmBgd)) {
				num=dp->YxNumPerFrmBgd;
			}
			else {
				if(len<=10)	num=len;
				else {
					num=(len/10)*10;
					if(num>100) {
						num=100;
					}
				}
			}
		}
		else {
			if(len<=10)	num=len;
			else {
				num=(len/10)*10;
				if(num>100) {
					num=100;
				}
			}
		}
	}
	else if(data_type==DGLEN_YX_ASDU20) {
		if(dp->infaddr_bytes==2) {
			num=len/7;
		}
		else if(dp->infaddr_bytes==3) {
			num=len/8;
		}
		else {
			num=len/7;
		}
		num=num*16;
		if(num>256) num=256;
	}
	else if(data_type==DGLEN_SOE) {
		num=((len/11)/5)*5;
		if(num>dp->SOENumPerFrame) {
			num=dp->SOENumPerFrame;
		}
	}
	else if(data_type==DGLEN_YC_CONTINUE) {
		if(dp->infaddr_bytes==2) {
			len=len-2;
		}
		else if(dp->infaddr_bytes==3) {
			len=len-3;
		}
		else {
			len=len-2;
		}
		if(len<6) {
			num=1;
		}
		else {
			num=len/6;
			if(gramfig==GRAM_YC_GLOBALI) {
				if(num>=(dp->YcNumPerFrmGlobalI)) {
					num=dp->YcNumPerFrmGlobalI;
				}
			}
			else if(gramfig==GRAM_YC_GROUPI) {
				if(num>=(dp->YcNumPerFrmGroupI)) {
					num=dp->YcNumPerFrmGroupI;
				}
			}
			else if(gramfig==GRAM_YC_BACKGROUND) {
				if(num>=(dp->YcNumPerFrmBgd)) {
					num=dp->YcNumPerFrmBgd;
				}
			}
			else {
				if(num>=10) {
					num=(num/10)*10;
				}
			}
		}
	}
	else if(data_type==DGLEN_YC_SINGLE_NOTIME) {
		num=len/10;
		if(num>dp->YCBHNumPerFrame) {
			num=dp->YCBHNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YC_SINGLE_NOTIME_ASDU9) {
		if(dp->infaddr_bytes==3) {
			num=len/6;
		}
		else {
			num=len/5;
		}
		if(num>dp->YCBHNumPerFrame) {
			num=dp->YCBHNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YC_SINGLE_NOTIME_ASDU11) {
		if(dp->infaddr_bytes==3) {
			num=len/6;
		}
		else {
			num=len/5;
		}
		if(num>dp->YCBHNumPerFrame) {
			num=dp->YCBHNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YC_SINGLE_NOTIME_ASDU13) {
		if(dp->infaddr_bytes==3) {
			num=len/8;
		}
		else {
			num=len/7;
		}
		if(num>dp->YCBHNumPerFrame) {
			num=dp->YCBHNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YC_SINGLE_NOTIME_ASDU21) {
		if(dp->infaddr_bytes==3) {
			num=len/5;
		}
		else {
			num=len/4;
		}
		if(num>dp->YCBHNumPerFrame) {
			num=dp->YCBHNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YC_SINGLE_TIME) {
		num=len/15;
		if(num>dp->YCBHNumPerFrame) {
			num=dp->YCBHNumPerFrame;
		}
	}
	else if(data_type==DGLEN_YM_NOTIME) {
		if(dp->infaddr_bytes==2) {
			len=len-2;
		}
		else if(dp->infaddr_bytes==3) {
			len=len-3;
		}
		else {
			len=len-2;
		}
		num=len/10;
	}
	else if(data_type==DGLEN_YM_TIME) {
		if(dp->infaddr_bytes==2) {
			len=len-2;
		}
		else if(dp->infaddr_bytes==3) {
			len=len-3;
		}
		else {
			len=len-2;
		}
		num=len/15;
	}
	else if(data_type==DGLEN_YP_CONTINUE) {
		if(dp->infaddr_bytes==2) {
			len=len-2;
		}
		else if(dp->infaddr_bytes==3) {
			len=len-3;
		}
		else {
			len=len-2;
		}
		num=len/5;
		if(gramfig==GRAM_YP_GLOBALI) {
			if(num>=(dp->YpNumPerFrmGlobalI)) {
				num=dp->YpNumPerFrmGlobalI;
			}
		}
		if(gramfig==GRAM_YP_GROUPI) {
			if(num>=(dp->YpNumPerFrmGroupI)) {
				num=dp->YpNumPerFrmGroupI;
			}
		}
		if(gramfig==GRAM_YP_BACKGROUND) {
			if(num>=(dp->YpNumPerFrmBgd)) {
				num=dp->YpNumPerFrmBgd;
			}
		}
	}
	else if(data_type==DGLEN_YP_SINGLE) {
		num=len/15;
		if(num>dp->YPBHNumPerFrame) {
			num=dp->YPBHNumPerFrame;
		}
	}
	else {
		num=0;
	}
	return(num);
}
/*
static float IEC101GetFloatFromBuf(u_int8* buf,int orderfig)
{
	u_int8 temp[4];
	
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		if(orderfig==BYTEORDER_LITTLE) {
	 		temp[0]=buf[0];
	 		temp[1]=buf[1];
	 		temp[2]=buf[2];
	 		temp[3]=buf[3];
		}
		else {
	 		temp[0]=buf[3];
	 		temp[1]=buf[2];
	 		temp[2]=buf[1];
	 		temp[3]=buf[0];
		}
	#elif __BYTE_ORDER == __BIG_ENDIAN
		if(orderfig==BYTEORDER_LITTLE) {
	 		temp[0]=buf[3];
	 		temp[1]=buf[2];
	 		temp[2]=buf[1];
	 		temp[3]=buf[0];
		}
		else {
			temp[0]=buf[0];
	 		temp[1]=buf[1];
	 		temp[2]=buf[2];
	 		temp[3]=buf[3];
	 		
		}
	#else
  #error byte order not define
  #endif
  
	return(*(float *)temp);
}

static u_short IEC101GetUShortFromBuf(u_int8* buf,int orderfig)
{
	if(orderfig==BYTEORDER_LITTLE) {
		return(buf[0]+buf[1]*256);
	}
	else {
		return(buf[1]+buf[0]*256);
	}
}
*/
static u_int16 IEC101AsduCommonInfChk(struct APPDEF *dp,u_int16 cot,u_int8 vsq)
{
	u_int16 cot_result;

	if(dp->ASDUCHK_strategy==ASDUCHK_NULL) {
		cot_result=cot+1;
	}
	else if(dp->ASDUCHK_strategy==ASDUCHK_VSQ) {
		if((vsq==0x01)) {
			cot_result=cot+1;
		}
		else {
			cot_result=(cot+1)|P_N;
		}
	}
	else if(dp->ASDUCHK_strategy==ASDUCHK_COT) {
		if((cot==0x06)||(cot==0x08)) {
			cot_result=cot+1;
		}
		else {
			cot_result=45|P_N;
		}
	}
	else if(dp->ASDUCHK_strategy==ASDUCHK_ADDR) {
		if((dp->rece_appaddr)==(dp->appaddr)) {
			cot_result=cot+1;
		}
		else {
			cot_result=46|P_N;
		}
	}
	else if(dp->ASDUCHK_strategy==ASDUCHK_VSQ_ADDR) {
		if((dp->rece_appaddr)==(dp->appaddr)) {
			if((vsq==0x01)) {
				cot_result=cot+1;
			}
			else {
				cot_result=(cot+1)|P_N;
			}
		}
		else {
			cot_result=46|P_N;
		}
	}
	else if(dp->ASDUCHK_strategy==ASDUCHK_COT_ADDR) {
		if((dp->rece_appaddr)==(dp->appaddr)) {
			if((cot==0x06)||(cot==0x08)) {
				cot_result=cot+1;
			}
			else {
				cot_result=45|P_N;
			}
		}
		else {
			cot_result=46|P_N;
		}
	}
	else if(dp->ASDUCHK_strategy==ASDUCHK_VSQ_COT_ADDR) {
		if((dp->rece_appaddr)==(dp->appaddr)) {
			if((cot==0x06)||(cot==0x08)) {
				if((vsq==0x01)) {
					cot_result=cot+1;
				}
				else {
					cot_result=(cot+1)|P_N;
				}
			}
			else {
				cot_result=45|P_N;
			}
		}
		else {
			cot_result=46|P_N;
		}
	}
	else {
		cot_result=cot+1;
	}
	if(cot_result!=(cot+1)) {
		if((dp->rece_appaddr)!=(dp->appaddr)) {
			IEC101Printf(dp,"ASDU CHK: error, recv asdu common address(%d) is not match with para(%d).",dp->rece_appaddr,dp->appaddr);
		}
		if(vsq!=0x01) {
			IEC101Printf(dp,"ASDU CHK: error, recv vsq(%d) is not rational.",vsq);
		}
		if((cot!=0x06)&&(cot!=0x08)) {
			IEC101Printf(dp,"ASDU CHK: error, recv cot(%d) is not rational.",cot);
		}
	}
	return(cot_result);
}

static void IEC101ReportGlbI(struct APPDEF *dp)
{
	IEC101ReportGlbIConfirm(dp,COTFIG_NORMAL);
	IEC101ReportGlbIData(dp);
	IEC101ReportGlbIEndData(dp);
	return;
}

static void IEC101ReportGlbIConfirm(struct APPDEF *dp,u_int8 cotfig)
{
	u_int8 sbuf[100],vsq;
	u_int16 i,cot,cot_result;

	i=7;
	if(dp->linkaddr_bytes==2) {
		i++;
	}
	vsq=dp->rbuf[i];
	i++;
	if(dp->CotMatch_strategy==COTMATCH_LOW4BIT) {
		cot=(dp->rbuf[i])&0x0f;
	}
	else {
		cot=dp->rbuf[i];
	}
	i=0;
	sbuf[i++]=0x64;
	sbuf[i++]=0x01;
	cot_result=IEC101AsduCommonInfChk(dp,cot,vsq);
	if(cotfig==COTFIG_NAK) {
		cot_result=(cot+1)|P_N;
	}
	if(cot_result==(cot+1)) {
		if(dp->rece_infaddr!=0x00) {
			cot_result=47|P_N;
		}
	}
	if(cot_result!=(cot+1)) {
		if(dp->COT_strategy==COT_VALID_INVALID) {
			cot_result=(cot+1)|P_N;
		}
	}
	sbuf[i++]=(u_int8)cot_result;
	if(dp->cot_bytes==2) {
		sbuf[i++]=(u_int8)(cot_result>>8);
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	sbuf[i++]=0x00;
	sbuf[i++]=0x00;
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=0x14;
	IEC101SaveReport(dp,sbuf,i,PRI_GLOBALI);
	
	if(cot_result==(cot+1)) {
		dp->GlobalIReplyFig=YES;
	}
	else {
		dp->GlobalIReplyFig=NO;
	}
	if(dp->GlobalIReplyFig==YES) IEC101Printf(dp,"General-I: confirm ack.");
	else IEC101Printf(dp,"General-I: confirm nak.");
	return;
}

static void IEC101ReportGlbIConfirm2(struct APPDEF *dp)
{
	u_int8 sbuf[100],vsq,qoi;
	u_int16 i,cot,cot_result,rece_appaddr;
	u_int32 rece_infaddr;

	i=7;
	if(dp->linkaddr_bytes==2) {
		i++;
	}
	vsq=dp->rbuf[i];
	i++;
	if(dp->CotMatch_strategy==COTMATCH_LOW4BIT) {
		cot=(dp->rbuf[i])&0x0f;
	}
	else {
		cot=dp->rbuf[i];
	}
	if(dp->cot_bytes==1) {
		i++;
	}
	else if(dp->cot_bytes==2) {
		i+=2;
	}
	else {
		i+=2;
	}
	if(dp->appaddr_bytes==1) {
		rece_appaddr=dp->rbuf[i];
		i++;
	}
	else if(dp->appaddr_bytes==2) {
		rece_appaddr=dp->rbuf[i]+dp->rbuf[i+1]*256;
		i+=2;
	}
	else {
		rece_appaddr=dp->rbuf[i];
		i++;
	}
	if(dp->infaddr_bytes==2) {
		rece_infaddr=dp->rbuf[i]+dp->rbuf[i+1]*256;
		i+=2;
	}
	else if(dp->infaddr_bytes==3) {
		rece_infaddr=dp->rbuf[i]+dp->rbuf[i+1]*256+dp->rbuf[i+2]*65536;
		i+=3;
	}
	else {
		rece_infaddr=dp->rbuf[i]+dp->rbuf[i+1]*256;
		i+=2;
	}
	qoi=dp->rbuf[i];
	i=0;
	sbuf[i++]=0x64;
	sbuf[i++]=0x01;
	cot_result=IEC101AsduCommonInfChk(dp,cot,vsq);
	if(cot_result==(cot+1)) {
		if(dp->rece_infaddr!=0x00) {
			cot_result=47|P_N;
		}
	}
	if(cot_result==(cot+1)) {
		cot_result=(cot+1)|P_N;
	}
	if(cot_result!=(cot+1)) {
		if(dp->COT_strategy==COT_VALID_INVALID) {
			cot_result=(cot+1)|P_N;
		}
	}
	sbuf[i++]=(u_int8)cot_result;
	if(dp->cot_bytes==2) {
		sbuf[i++]=(u_int8)(cot_result>>8);
	}
	sbuf[i++]=(u_int8)(rece_appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((rece_appaddr)>>8);
	}
	sbuf[i++]=(u_int8)rece_infaddr;
	sbuf[i++]=(u_int8)(rece_infaddr>>8);
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=(u_int8)(rece_infaddr>>16);
	}
	sbuf[i++]=qoi;
	IEC101SaveReport(dp,sbuf,i,PRI_GLOBALI);
	
	if(cot_result==(cot+1)) {
		dp->GlobalIReplyFig=YES;
	}
	else {
		dp->GlobalIReplyFig=NO;
	}
	if(dp->GlobalIReplyFig==YES) IEC101Printf(dp,"General-I: confirm ack.");
	else IEC101Printf(dp,"General-I: confirm nak.");
	return;
}

static void IEC101ReportGlbIData(struct APPDEF *dp)
{

	if(dp->GlobalIReplyFig==NO) {
		return;
	}
	IEC101ReportGlbIYxData(dp);
	IEC101ReportGlbIYcData(dp);	
	return;
}

static void IEC101ReportGlbIYxData(struct APPDEF *dp)
{
	u_int32 i,j,rnum,bnum,ptr;
	u_int8 sbuf[300];
	u_int8 yx_val;
	unsigned int yx_total_num;
	unsigned int yx_value [100];
	
	yx_total_num =0;
	yx_get_total_num_proto (dp->app_id, &yx_total_num);
	if(yx_total_num>0) {
		IEC101Printf(dp,"General-I: answer Single_BI.");
		if(dp->ASDU_YX_GI==1) {
			if(dp->SYxInfaddrType==INFADDR_CONTINUE) {
				ptr=0;
				rnum=IEC101CheckTelegramLen(dp,DGLEN_YX_CONTINUE,GRAM_YX_GLOBALI);
				if(rnum==0) {
					return;
				}
				while(ptr<yx_total_num) {
					if((ptr+rnum)>yx_total_num) {
						rnum=yx_total_num-ptr;
					}
					bnum=rnum;
					yx_get_value_proto (dp->app_id, ptr, bnum, yx_value);
					i=0;
					sbuf[i++]=0x01;
					sbuf[i++]=bnum+0x80;
					sbuf[i++]=0x14;
					if(dp->cot_bytes==2) {
						sbuf[i++]=0x00;
					}
					sbuf[i++]=(u_int8)(dp->appaddr);
					if(dp->appaddr_bytes==2) {
						sbuf[i++]=(u_int8)((dp->appaddr)>>8);
					}
					sbuf[i++]=(u_int8)(dp->InfAddr_syx+ptr);
					sbuf[i++]=(u_int8)((dp->InfAddr_syx+ptr)>>8);
					if(dp->infaddr_bytes==3) {
						sbuf[i++]=(u_int8)((dp->InfAddr_syx+ptr)>>16);
					}
					for(j=0;j<bnum;j++) {
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
						sbuf[i++]=yx_val;
						ptr++;
					}
					IEC101SaveReport(dp,sbuf,i,PRI_GLOBALI);
				}
				return;
			}
		}
	}
	return;
}

static void IEC101ReportGlbIYcData(struct APPDEF *dp)
{
	u_int32 i,j,rnum,bnum,ptr;
	short sval;
	float fval;
	u_int8 sbuf[300];
	u_int8 squality,fquality;
	yc_data_type yc_value [100];
	unsigned char yc_qds [100], data_type [100];	
	unsigned int yc_total_num;
	
	yc_total_num=0;
	yc_get_total_num_proto (dp->app_id, &yc_total_num);
	if(yc_total_num>0) {
		IEC101Printf(dp,"General-I: answer metering.");
		if(dp->YcInfaddrType==INFADDR_CONTINUE) {
			ptr=0;
			rnum=IEC101CheckTelegramLen(dp,DGLEN_YC_CONTINUE,GRAM_YC_GLOBALI);
			if(rnum==0) {
				return;
			}
			while(ptr<yc_total_num) {
				if((ptr+rnum)>yc_total_num) {
					rnum=yc_total_num-ptr;
				}
				bnum=rnum;
				yc_get_value_proto (dp->app_id, ptr, bnum, yc_value, yc_qds, data_type);
				i=0;
				sbuf[i++]=dp->ASDU_YC_GI;
				sbuf[i++]=0x80+bnum;
				sbuf[i++]=0x14;
				if(dp->cot_bytes==2) {
					sbuf[i++]=0x00;
				}
				sbuf[i++]=(u_int8)(dp->appaddr);
				if(dp->appaddr_bytes==2) {
					sbuf[i++]=(u_int8)((dp->appaddr)>>8);
				}
				sbuf[i++]=(u_int8)(dp->InfAddr_yc+ptr);
				sbuf[i++]=(u_int8)((dp->InfAddr_yc+ptr)>>8);
				if(dp->infaddr_bytes==3) {
					sbuf[i++]=(u_int8)((dp->InfAddr_yc+ptr)>>16);
				}
				for(j=0;j<bnum;j++) {
					fval=0;
					sval=0;
					fquality=0;
					squality=0;
					if (data_type [j] == YC_INT) {
						sval = yc_value [j].i_val;
						fval =sval;
					}
					else {
						fval = yc_value [j].f_val;
						sval =fval;
					}
					fquality=yc_qds [j];
					squality=yc_qds [j];
					if(dp->ASDU_YC_GI==9) {
						i=IEC101FillIntYcVal(dp,sbuf,i,sval);
						sbuf[i++]=squality;
					}
					else if(dp->ASDU_YC_GI==11) {
						i=IEC101FillIntYcVal(dp,sbuf,i,sval);
						sbuf[i++]=squality;
					}
					else if(dp->ASDU_YC_GI==13) {
						i=IEC101FillFloatYcVal(dp,sbuf,i,fval);
						sbuf[i++]=fquality;
					}
					else if(dp->ASDU_YC_GI==21) {
						i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					}
					else {
						i=IEC101FillIntYcVal(dp,sbuf,i,sval);
						sbuf[i++]=squality;
					}
				}
				ptr +=bnum;
				IEC101SaveReport(dp,sbuf,i,PRI_GLOBALI);
			}
			return;
		}
	}
	return;
}

static void IEC101ReportGlbIEndData(struct APPDEF *dp)
{
	u_int8 sbuf[100];
	u_int16 i;

	if(dp->GlobalIReplyFig==NO) {
		return;
	}
	i=0;
	sbuf[i++]=0x64;
	sbuf[i++]=0x01;
	sbuf[i++]=0x0a;
	if(dp->cot_bytes==2) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	sbuf[i++]=0x00;
	sbuf[i++]=0x00;
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=0x14;
	IEC101SaveReport(dp,sbuf,i,PRI_GLOBALI);
	IEC101Printf(dp,"General-I: over.");
	return;
}

static void IEC101ReportICancel(struct APPDEF *dp,u_int8 qoi)
{
	u_int8 sbuf[100],vsq;
	u_int16 i,cot,cot_result;

	i=7;
	if(dp->linkaddr_bytes==2) {
		i++;
	}
	vsq=dp->rbuf[i];
	i++;
	if(dp->CotMatch_strategy==COTMATCH_LOW4BIT) {
		cot=(dp->rbuf[i])&0x0f;
	}
	else {
		cot=dp->rbuf[i];
	}
	i=0;
	sbuf[i++]=0x64;
	sbuf[i++]=0x01;
	cot_result=IEC101AsduCommonInfChk(dp,cot,vsq);
	if(cot_result!=(cot+1)) {
		if(dp->COT_strategy==COT_VALID_INVALID) {
			cot_result=(cot+1)|P_N;
		}
	}
	sbuf[i++]=(u_int8)cot_result;
	if(dp->cot_bytes==2) {
		sbuf[i++]=(u_int8)(cot_result>>8);
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	sbuf[i++]=0x00;
	sbuf[i++]=0x00;
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=qoi;
	if(cot_result==(cot+1)) IEC101Printf(dp,"General-I: confirm ack.");
	else IEC101Printf(dp,"General-I: confirm nak.");
	IEC101SaveReport(dp,sbuf,i,PRI_GLOBALI);
	return;
}

static void IEC101ReportSetClock(struct APPDEF *dp)
{
	u_int32 j;
	struct SYS_TIME t0,t1;
	u_int8 timevalid_fig,vsq;
	u_int16 ms,cot,cot_result;

	timevalid_fig=0x00;
	j=7;
	if(dp->linkaddr_bytes==2) {
		j++;
	}
	vsq=dp->rbuf[j];
	j++;
	if(dp->CotMatch_strategy==COTMATCH_LOW4BIT) {
		cot=(dp->rbuf[j])&0x0f;
	}
	else {
		cot=dp->rbuf[j];
	}
	
	j=12;
	if(dp->linkaddr_bytes==2) {
		j++;
	}
	if(dp->cot_bytes==2) {
		j++;
	}
	if(dp->appaddr_bytes==2) {
		j++;
	}
	if(dp->infaddr_bytes==3) {
		j++;
	}
	ms=dp->rbuf[j]+dp->rbuf[j+1]*256;
	t0.msecond=ms%1000;
	t0.second=ms/1000;
	t0.minute=dp->rbuf[j+2];
	t0.hour=dp->rbuf[j+3];
	t0.day=dp->rbuf[j+4];
	t0.month=dp->rbuf[j+5];
	t0.year=dp->rbuf[j+6];
	
	if(cot == 0x05){
		cot_result=cot;
		IEC101ReportTimeSyn(dp,cot_result,timevalid_fig,&t0);
		return;
	}
	
	t1.msecond=ms%1000;
	t1.second=ms/1000;
	t1.minute=(dp->rbuf[j+2])&0x3f;
	t1.hour=(dp->rbuf[j+3])&0x1f;
	t1.day=(dp->rbuf[j+4])&0x1f;
	t1.month=(dp->rbuf[j+5])&0x0f;
	t1.year=((dp->rbuf[j+6])&0x7f)+2000;	
	IEC101Printf(dp,"TIME-SYN: recv time(%04d-%02d-%02d %02d:%02d:%02d.%03d).",t1.year,t1.month,t1.day,t1.hour,t1.minute,t1.second,t1.msecond);

	if((IEC101CheckTimeRationality(dp,&t1))!=YES) {
		timevalid_fig |=0x01;
	}
	if((vsq!=0x01)) {
		timevalid_fig |=0x02;
	}
	if((dp->rece_appaddr)!=(dp->appaddr)) {
		timevalid_fig |=0x04;
	}
	if(cot!=0x06) {
		timevalid_fig |=0x08;
	}
	cot_result=IEC101AsduCommonInfChk(dp,cot,vsq);
	if((cot_result==0x07)&&(dp->rece_infaddr!=0x00)) {
		cot_result=47|P_N;
		timevalid_fig |=0x10;
	}
	if((cot_result==0x07)&&((timevalid_fig&0x01)>0)) {
		cot_result=0x07|P_N;
	}
	if(timevalid_fig==0x00) {
		IEC101Printf(dp,"TIME-SYN: time syn sucess1(%04d-%02d-%02d %02d:%02d:%02d.%03d).\n",t1.year,t1.month,t1.day,t1.hour,t1.minute,t1.second,t1.msecond);
		IEC101SetTime(dp,&t1);
	}
	if(cot_result!=(cot+1)) {
		if(dp->COT_strategy==COT_VALID_INVALID) {
			cot_result=(cot+1)|P_N;
		}
	}

	IEC101ReportTimeSyn(dp,cot_result,timevalid_fig,&t0);
	return;
}

static u_int8 IEC101CheckTimeRationality(struct APPDEF *dp,struct SYS_TIME * tp)
{
	u_int16 year,days;

	if((tp->second)>59) {
		IEC101Printf(dp,"TIME-SYN: error: recv time is not rational,second=%d.",tp->second);
		return(NO);
	}
	if((tp->minute)>59) {
		IEC101Printf(dp,"TIME-SYN: error: recv time is not rational,minute=%d.",tp->minute);
		return(NO);
	}
	if((tp->hour)>=24) {
		IEC101Printf(dp,"TIME-SYN: error: recv time is not rational,hour=%d.",tp->hour);
		return(NO);
	}
	if(((tp->month)<=0)||((tp->month)>12)) {
		IEC101Printf(dp,"TIME-SYN: error: recv date is not rational,month=%d.",tp->month);
		return(NO);
	}
	if(((tp->day)<=0)||((tp->day)>31)) {
		IEC101Printf(dp,"TIME-SYN: error: recv date is not rational,day=%d.",tp->day);
		return(NO);
	}
	if(((tp->month==1)||(tp->month==3)||(tp->month==5)||(tp->month==7)||(tp->month==8)||(tp->month==10)||(tp->month==12))&&((tp->day)>31)) {
		IEC101Printf(dp,"TIME-SYN: error: recv date is not rational,month=%d,day=%d.",tp->month,tp->day);
		return(NO);
	}
	if(((tp->month==2)||(tp->month==4)||(tp->month==6)||(tp->month==9)||(tp->month==11))&&((tp->day)>30)) {
		IEC101Printf(dp,"TIME-SYN: error: recv date is not rational,month=%d,day=%d.",tp->month,tp->day);
		return(NO);
	}
	year=tp->year;
	if(year%4==0){
		if(year%100==0){
			if(year%400==0) days=366;
			else days=365;
		}
		else days=366;
	}
	else days=365;
	if((days==366)&&(tp->month==2)&&((tp->day)>29)) {
		IEC101Printf(dp,"TIME-SYN: error: recv date is not rational,month=%d,day=%d.",tp->month,tp->day);
		return(NO);
	}
	if((days==365)&&(tp->month==2)&&((tp->day)>28)){
		IEC101Printf(dp,"TIME-SYN: error: recv date is not rational,month=%d,day=%d.",tp->month,tp->day);
		return(NO);
	}
	return(YES);
}

static void IEC101ReportTimeSyn(struct APPDEF *dp,u_int16 cot,u_int8 valid_fig,struct SYS_TIME * rt)
{
	u_int32 i;
	u_int8 sbuf[300];
	struct SYS_TIME t;

	i=0;
	sbuf[i++]=0x67;
	sbuf[i++]=0x01;
	sbuf[i++]=(u_int8)cot;
	if(dp->cot_bytes==2) {
		sbuf[i++]=(u_int8)(cot>>8);
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	sbuf[i++]=0x00;
	sbuf[i++]=0x00;
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=0x00;
	}
	if(cot != 0x05){
		i=IEC101GetTbufFromSysTime0(sbuf,i,rt);
		IEC101GetValidTimeFromWhole(rt,&t);
		if((valid_fig==0x00)&&(cot==0x07)) IEC101Printf(dp,"TIME-SYN: app confirm, ack, t=%04d.%02d.%02d %02d:%02d:%02d.%03d.",t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);
		else IEC101Printf(dp,"TIME-SYN: app confirm, nak, t=%04d.%02d.%02d %02d:%02d:%02d.%03d.",t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);		
	}
	else{
		sys_get_time(&t);
		i=IEC101GetTbufFromSysTime0(sbuf,i,&t);
		IEC101Printf(dp,"TIME-SYN: read time confirm, t=%04d.%02d.%02d %02d:%02d:%02d.%03d.",t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);				
	}
	IEC101SaveReport(dp,sbuf,i,PRI_CLOCKSYN);
	return;
}

static void IEC101ReportTest(struct APPDEF *dp)
{
	u_int8 sbuf[100],vsq;
	u_int16 i,j,cot,cot_result;

	i=7;
	if(dp->linkaddr_bytes==2) {
		i++;
	}
	vsq=dp->rbuf[i];
	i++;
	if(dp->CotMatch_strategy==COTMATCH_LOW4BIT) {
		cot=(dp->rbuf[i])&0x0f;
	}
	else {
		cot=dp->rbuf[i];
	}
	i=0;
	sbuf[i++]=0x68;
	sbuf[i++]=0x01;
	cot_result=IEC101AsduCommonInfChk(dp,cot,vsq);
	if(cot_result==(cot+1)) {
		if(dp->rece_infaddr!=0x00) {
			cot_result=47|P_N;
		}
	}
	if(cot_result!=(cot+1)) {
		if(dp->COT_strategy==COT_VALID_INVALID) {
			cot_result=(cot+1)|P_N;
		}
	}
	sbuf[i++]=(u_int8)cot_result;
	if(dp->cot_bytes==2) {
		sbuf[i++]=(u_int8)(cot_result>>8);
	}
	sbuf[i++]=(u_int8)(dp->appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((dp->appaddr)>>8);
	}
	j=i+dp->HeadLen;
	sbuf[i++]=dp->rbuf[j++];
	sbuf[i++]=dp->rbuf[j++];
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=dp->rbuf[j++];
	}
	sbuf[i++]=dp->rbuf[j++];
	sbuf[i++]=dp->rbuf[j++];
	IEC101SaveReport(dp,sbuf,i,PRI_APPTEST);
	if(cot_result==(cot+1)) IEC101Printf(dp,"APPTEST: app confirm ack.");
	else IEC101Printf(dp,"APPTEST: appconfirm nak.");
	return;
}

static void IEC101ReportPeriodicYcScan(struct APPDEF *dp)
{
	u_int32 i,j,rnum,rnum0,ptr;
	short sval;
	float fval;
	u_int8 sbuf[300];
	u_int8 squality,fquality;
	yc_data_type yc_value [100];
	unsigned char yc_qds [100], data_type [100];	
	unsigned int yc_total_num;
	
	yc_total_num=0;
	yc_get_total_num_proto (dp->app_id, &yc_total_num);
	if(yc_total_num>0) {
		rnum0=IEC101CheckTelegramLen(dp,DGLEN_YC_CONTINUE,GRAM_NULL);
		if(rnum0==0) {
			return;
		}
		ptr=0;
		while(ptr < yc_total_num) {
			if((ptr+rnum0)>(yc_total_num)) {
				rnum= yc_total_num -ptr;
			}
			else {
				rnum=rnum0;
			}
			if(rnum==0) {
				break;
			}
			yc_get_value_proto (dp->app_id, ptr, rnum, yc_value, yc_qds, data_type);
			i=0;
			sbuf[i++]=dp->ASDU_YC_PeriodicScan;
			sbuf[i++]=rnum+0x80;
			sbuf[i++]=0x01;
			if(dp->cot_bytes==2) {
				sbuf[i++]=0x00;
			}
			sbuf[i++]=(u_int8)(dp->appaddr);
			if(dp->appaddr_bytes==2) {
				sbuf[i++]=(u_int8)((dp->appaddr)>>8);
			}
			sbuf[i++]=(u_int8)(dp->InfAddr_yc+ptr);
			sbuf[i++]=(u_int8)((dp->InfAddr_yc+ptr)>>8);
			if(dp->infaddr_bytes==3) {
				sbuf[i++]=(u_int8)((dp->InfAddr_yc+ptr)>>16);
			}
			for(j=0;j<rnum;j++) {
				fval=0;
				sval=0;
				fquality=0;
				squality=0;
				if (data_type [j] == YC_INT) {
					sval = yc_value [j].i_val;
					fval =sval;
				}
				else {
					fval = yc_value [j].f_val;
					sval =fval;
				}
				fquality=yc_qds [j];
				squality=yc_qds [j];
				if(dp->ASDU_YC_PeriodicScan==9) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
				}
				else if(dp->ASDU_YC_PeriodicScan==11) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
				}
				else if(dp->ASDU_YC_PeriodicScan==13) {
					i=IEC101FillFloatYcVal(dp,sbuf,i,fval);
					sbuf[i++]=fquality;
				}
				else if(dp->ASDU_YC_PeriodicScan==21) {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
				}
				else {
					i=IEC101FillIntYcVal(dp,sbuf,i,sval);
					sbuf[i++]=squality;
				}
			}
			ptr +=rnum;
			IEC101SaveReport(dp,sbuf,i,PRI_PERIODICYC);
		}
		return;
	}
	return;
}

static struct MSGREPORT * IEC101ReportPtrCheckUnused(struct APPDEF *dp)
{
	int32 i;

	for(i=0;i<RPT_MAX_NUM-RPT_MID_NUM;i++) {
		if(dp->ReportPtr[i]!=NULL) {
			if((dp->ReportPtr[i]->len)==0) {
				return(dp->ReportPtr[i]);
			}
		}
	}
	return(NULL);
}

static struct MSGREPORT * IEC101ReportPtrCheckPriUnused(struct APPDEF *dp,u_int8 pri)
{
	int32 i;

	if((pri != PRI_YCBH)&&(pri !=PRI_PERIODICYC))  return(NULL);
	for(i=RPT_MAX_NUM-RPT_MID_NUM;i<RPT_MAX_NUM;i++) {
		if(dp->ReportPtr[i]!=NULL) {
			if((dp->ReportPtr[i]->len)==0) {
				return(dp->ReportPtr[i]);
			}
		}
	}
	return(NULL);
}

static void IEC101ReportPtrFree(struct APPDEF *dp,struct MSGREPORT * ReportPtr)
{
	if(ReportPtr!=NULL) {
		ReportPtr->len=0;
		ReportPtr->next=NULL;
		memset(ReportPtr->buf,0,300);
	}
	else {
		IEC101Printf(dp,"error: to free ReportPtr, but NULL.");
	}
	return;
}

static u_int8 IEC101ReportPtrMallocInit(struct APPDEF *dp)
{
	struct MSGREPORT *ReportPtr;
	int32 i;

	dp->yx_event_lst = (yx_event_t *)malloc (sizeof (yx_event_t) * (MAX_YX_EVENT_NUM_PER_FRAME_101 + 1));
	if(dp->yx_event_lst==NULL) {
		IEC101Printf(dp,"REPORT_MALLOC: error(yx_event_t): malloc fail.");
		return(NO);
	}		
	dp->yx_event_num = 0;

	dp->yc_event_lst = (yc_event_t *)malloc (sizeof (yc_event_t) * (MAX_YC_EVENT_NUM_PER_FRAME_101 + 1));
	if(dp->yc_event_lst==NULL) {
		IEC101Printf(dp,"REPORT_MALLOC: error(yc_event_t): malloc fail.");
		return(NO);
	}		
	dp->yc_event_num = 0;

	for(i=0;i<RPT_MAX_NUM;i++) {
		dp->ReportPtr[i]=NULL;
	}
	for(i=0;i<RPT_MAX_NUM;i++) {
		ReportPtr = (struct MSGREPORT *)malloc (sizeof (struct MSGREPORT));
		if(ReportPtr==NULL) {
			IEC101Printf(dp,"REPORT_MALLOC: error(%i): malloc fail.",i);
			return(NO);
		}
		dp->ReportPtr[i]=ReportPtr;
		dp->ReportPtr[i]->len=0;
		dp->ReportPtr[i]->next=NULL;
		memset(dp->ReportPtr[i]->buf,0,300);
	}
	return(YES);
}

static void IEC101ClearBufferClm(struct APPDEF *dp,u_char fig)
{
	if(fig==CLRBUF_PROCRST) {
		IEC101ClearReportBuffer(dp,CLRBUF_ALL);
	}
	if(fig==CLRBUF_FCBRST) {
		IEC101ClearReportBuffer(dp,dp->LnkRstBufferFig);
	}
	return;
}

static void IEC101ClearReportBuffer(struct APPDEF *dp,u_int8 fig)
{
	int i;
	struct MSGREPORT *tempptr;
	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		if(fig==CLRBUF_NOCLEAR) continue;
		if(((fig==CLRBUF_BUTCOS)||(fig==CLRBUF_BUTCOSSOE))&&(dp->Pri_Data1[i]==PRI_COS)) continue;
		if(((fig==CLRBUF_BUTSOE)||(fig==CLRBUF_BUTCOSSOE))&&(dp->Pri_Data1[i]==PRI_SOE)) continue;
		while(dp->data1ptr[i]!=NULL) {
			tempptr=dp->data1ptr[i];
			dp->data1ptr[i]=dp->data1ptr[i]->next;
			IEC101ReportPtrFree(dp,tempptr);
		}
	}
	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		if(fig==CLRBUF_NOCLEAR) continue;
		if(((fig==CLRBUF_BUTCOS)||(fig==CLRBUF_BUTCOSSOE))&&(dp->Pri_Data2[i]==PRI_COS)) continue;
		if(((fig==CLRBUF_BUTSOE)||(fig==CLRBUF_BUTCOSSOE))&&(dp->Pri_Data2[i]==PRI_SOE)) continue;
		while(dp->data2ptr[i]!=NULL) {
			tempptr=dp->data2ptr[i];
			dp->data2ptr[i]=dp->data2ptr[i]->next;
			IEC101ReportPtrFree(dp,tempptr);
		}
	}
	return;
}

static void IEC101PrintMsg(struct APPDEF *dp,u_int8 *msg,int32 len,int32 fig)
{
	int32 i;

	if(fig==0) {
		if(dp->DebugOutFig&DBGOUT_STDPNT) {
			_DPRINTF("\n[PW101_C%d] MSG-RXD: ",dp->com_no);
			for(i=0;i<len;i++) {
				_DPRINTF(" %02x",msg[i]);
			}
			_DPRINTF("\n");
		}
	}
	if(fig==1) {
		if(dp->DebugOutFig&DBGOUT_STDPNT) {
			_DPRINTF("\n[PW101_C%d] MSG-TXD: ",dp->com_no);
			for(i=0;i<len;i++) {
				_DPRINTF(" %02x",msg[i]);
			}
			_DPRINTF("\n");
		}
	}
	return;
}

static void IEC101SetTime(struct APPDEF *dp,struct SYS_TIME *tp)
{
	struct timeval time;
	time_type tt_1900;
	struct tm * ptm;
	time_t tt;

	time.tv_sec = sys_time_to_timet(tp);
	time.tv_usec = tp->msecond * 1000;
	//settimeofday(&time,NULL);
	tt_1900.seconds = time.tv_sec;
	tt_1900.msec = tp->msecond;
	tt = tt_1900.seconds;
	ptm = localtime (&tt);
	/* set system time */
	set_seconds_1900 (&tt_1900);
//	rtc_write (ptm);
	
	return;
}

static void IEC101Printf(struct APPDEF *dp,const char *format,...)
{
	struct SYS_TIME t;
	char str0[100],str[DUBUGSTRMAXLEN];
	va_list _ap;
	int i,len;

	memset(str,0,DUBUGSTRMAXLEN);
	va_start(_ap, format);
	len=vsprintf(str,format, _ap);
	va_end(_ap);
	if(len==0) {
		return;
	}
	for(i=0;i<len;i++) {
		if(str[i]=='\n') {
			_DPRINTF("\n");
		}
		else {
			break;
		}
	}
	if(dp==NULL) {
		_DPRINTF("[STD101]: %s\n",&str[i]);
		_DPRINTF("\n");
		return;
	}
	else {
		if(dp->DebugTimeFig==DEBUGTIME_NULL) {
			if(dp->DebugOutFig&DBGOUT_STDPNT) {
				_DPRINTF("[STD101_C%d]: %s\n",dp->com_no,&str[i]);
			}
		}
		else {
			sys_get_time(&t);
			memset(str0,0,100);
			sprintf(str0,"(%02d-%02d-%02d %02d:%02d:%02d):",t.year%100,t.month,t.day,t.hour,t.minute,t.second);
			if(dp->DebugOutFig&DBGOUT_STDPNT) {
				_DPRINTF("[STD101_C%d]: %s: %s\n",dp->com_no,str0,&str[i]);
			}
		}
	}
	return;
}

static int IEC101GetAsduCommonFromMsg(struct APPDEF *dp,struct ASDUHEAD * pHead,u_char *buf)
{
	int i;

	i=6;
	if(dp->linkaddr_bytes==2) {
		i++;
	}
	pHead->asdutype=buf[i];
	i++;
	pHead->vsq=buf[i];
	i++;
	if(dp->cot_bytes==2) {
		pHead->cot=buf[i]+(buf[i+1])*256;
		i+=2;
	}
	else {
		if(dp->CotMatch_strategy==COTMATCH_LOW4BIT) {
			pHead->cot=buf[i]&0x0f;
		}
		else {
			pHead->cot=buf[i];
		}
		i++;
	}
	if(dp->appaddr_bytes==1) {
		pHead->appaddr=buf[i];
		i++;
	}
	else if(dp->appaddr_bytes==2) {
		pHead->appaddr=buf[i]+(buf[i+1])*256;
		i+=2;
	}
	else {
		pHead->appaddr=buf[i];
		i++;
	}
	return(i);
}

static void IEC101UpdateCommusta(struct APPDEF *dp,u_int8 comm_status,u_int cot)
{
	if(cot==COMMUCOT_INIT) {
		dp->comm_status=comm_status;
		if(comm_status==RDB_RET_DISCONN) {
			IEC101Printf(dp,"commustation init state: break");
		}
		else {
			IEC101Printf(dp,"commustation init state: normal");
		}
	}
	if(cot==COMMUCOT_RECV) {
		if((comm_status==RDB_RET_NORMAL)&&(dp->comm_status!=comm_status)) {
			dp->comm_status=comm_status;
			IEC101Printf(dp,"communication normal");
		}
	}
	if(cot==COMMUCOT_TIMEOUT) {
		if((comm_status==RDB_RET_DISCONN)&&(dp->comm_status!=comm_status)) {
			dp->comm_status=comm_status;
			IEC101Printf(dp,"communication break,delay %ds",dp->commout_time);
		}
	}

	if ((comm_status == RDB_RET_NORMAL) && !iec101_comm_stat) {
		iec101_comm_stat = 1;
		//screen_iec101_light (iec101_comm_stat);
	}
	else if ((comm_status == RDB_RET_DISCONN) && iec101_comm_stat) {
		iec101_comm_stat = 0;
		//screen_iec101_light (iec101_comm_stat);
	}

	return;
}

static int IEC101FillIntYcVal(struct APPDEF *dp,u_int8 *buf,int i,short sval)
{
	if(dp->IntYcByteFig==YES) {
		buf[i++]=LSB(sval);
		buf[i++]=MSB(sval);
	}
	if(dp->IntYcByteFig==NO) {
		buf[i++]=MSB(sval);
		buf[i++]=LSB(sval);
	}
	return(i);
}
static int IEC101FillFloatYcVal(struct APPDEF *dp,u_int8 *buf,int i,float fval)
{
	u_int8 *fvalptr;

	fvalptr=(u_int8 *)(&fval);
	
	if(dp->FloatYcByteFig==YES) {
		#if 1
			buf[i++]=*(fvalptr+0);
			buf[i++]=*(fvalptr+1);
			buf[i++]=*(fvalptr+2);
			buf[i++]=*(fvalptr+3);		
		#elif 0
			buf[i++]=*(fvalptr+3);
			buf[i++]=*(fvalptr+2);
			buf[i++]=*(fvalptr+1);
			buf[i++]=*(fvalptr+0);
		#else
  	#error byte order not define
  	#endif 	
	}
	if(dp->FloatYcByteFig==NO) {
		#if 1
			buf[i++]=*(fvalptr+3);
			buf[i++]=*(fvalptr+2);
			buf[i++]=*(fvalptr+1);
			buf[i++]=*(fvalptr+0);		
		#elif 0
			buf[i++]=*(fvalptr+0);
			buf[i++]=*(fvalptr+1);
			buf[i++]=*(fvalptr+2);
			buf[i++]=*(fvalptr+3);
		#else
  	#error byte order not define
  	#endif 	
	}
	
	return(i);
}
static int IEC101GetTbufFromSysTime0(u_int8 *buf,int i,struct SYS_TIME *tp)
{
	u_int16 ms;
	ms=tp->second*1000+tp->msecond;
	buf[i++]=LSB(ms);
	buf[i++]=MSB(ms);
	buf[i++]=tp->minute;
	buf[i++]=tp->hour;
	buf[i++]=tp->day;
	buf[i++]=tp->month;
	buf[i++]=tp->year;
	return(i);
}

static void IEC101GetValidTimeFromWhole(struct SYS_TIME *tin,struct SYS_TIME *tout)
{
	tout->year=((tin->year)&0x3f)+2000;
	tout->month=(tin->month)&0x0f;
	tout->day=(tin->day)&0x1f;
	tout->hour=(tin->hour)&0x1f;
	tout->minute=(tin->minute)&0x3f;
	tout->second=tin->second;
	tout->msecond=tin->msecond;
	return;
}

static void IEC101CommClearClm(struct APPDEF *dp)
{
	dp->InitStepFig=INIT_NULL;
	dp->InitStepFig2=INIT_NULL;
	dp->rptr=0;
	dp->rlen=0;
	dp->rfig=COMR_NO_DATA;
	dp->datastep_fig=LINKPROCESS;
	IEC101ClearBufferClm(dp,CLRBUF_FCBRST);
	dp->FcbCount=0;
	dp->FcbStartFig=NO;
	dp->fresendfig2=NO;
	dp->fsendfig2=NO;
	dp->fsendcount2=0;
	dp->fresendframe2=0;
	dp->LnkFstInitOkFig=NO;
	dp->TestCount=0;
	return;
}

static void IEC101AskLinkState(struct APPDEF *dp)
{
	u_int32 i;
	sleep(3);
	i=0;
	dp->sbuf2[i++]=0x10;
	dp->sbuf2[i++]=0x09 | (dp->dirfig) | PRM;
	if(dp->linkaddr_bytes==1) {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2];
	}
	else if(dp->linkaddr_bytes==2) {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=(u_int8)((dp->link_addr)>>8);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2]+dp->sbuf2[3];
	}
	else {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2];
	}
	dp->sbuf2[i++]=0x16;
	dp->slen2=i;
	IEC101SendData(dp,dp->sbuf2,dp->slen2);
	dp->fsendfig2=YES;
	return;
}

static void IEC101RstLinkState(struct APPDEF *dp)
{
	u_int32 i;

	i=0;
	dp->sbuf2[i++]=0x10;
	dp->sbuf2[i++]=0x00 | (dp->dirfig) | PRM;
	if(dp->linkaddr_bytes==1) {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2];
	}
	else if(dp->linkaddr_bytes==2) {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=(u_int8)((dp->link_addr)>>8);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2]+dp->sbuf2[3];
	}
	else {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2];
	}
	dp->sbuf2[i++]=0x16;
	dp->slen2=i;
	IEC101SendData(dp,dp->sbuf2,dp->slen2);
	dp->fsendfig2=YES;	
	return;
}

static void IEC101TestLinkState(struct APPDEF *dp)
{
	u_int32 i;

	i=0;
	dp->sbuf2[i++]=0x10;
	dp->sbuf2[i++]=0x02 | (dp->dirfig) | PRM;
	if(dp->linkaddr_bytes==1) {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2];
	}
	else if(dp->linkaddr_bytes==2) {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=(u_int8)((dp->link_addr)>>8);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2]+dp->sbuf2[3];
	}
	else {
		dp->sbuf2[i++]=(u_int8)(dp->link_addr);
		dp->sbuf2[i++]=dp->sbuf2[1]+dp->sbuf2[2];
	}
	dp->sbuf2[i++]=0x16;
	dp->slen2=i;
	IEC101SendData(dp,dp->sbuf2,dp->slen2);
	dp->fsendfig2=YES;	
	return;
}

static void IEC101BalanceModeCheckSendDataClm(struct APPDEF *dp)
{
	if(dp->CommuMode==MODE_BALANCE) {
		if((dp->LnkRuleType==LINKRULE_DONGFANG)||(dp->LnkRuleType == LINKRULE_ZHUHAI_XUJI))
		{
			if(((((dp->InitStepFig)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig)&INIT_FCBRST)>0))&&
			  ((((dp->InitStepFig2)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig2)&INIT_FCBRST)>0))) 
			 {
				dp->LnkFstInitOkFig=YES;
			 }
			if(dp->fsendfig2==NO) {
				if(((((dp->InitStepFig)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig)&INIT_FCBRST)>0))
				&&((((dp->InitStepFig2)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig2)&INIT_FCBRST)>0))) {
					if(IEC101ReportData(dp)==YES) {
						IEC101BalanceModeSendDataClm(dp);
					}
				}
				if(dp->LnkFstInitOkFig==NO) {
					if((((dp->InitStepFig)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig)&INIT_FCBRST)>0)) 
					{
						if(((dp->InitStepFig2)&INIT_LINKREPLY)==0){
							IEC101AskLinkState(dp);
//							IEC101Printf(dp,"��һ����·��ʼ������ѯ��վ��·״̬.\n");
						}
						if((((dp->InitStepFig2)&INIT_LINKREPLY)>0)&&
						  (((dp->InitStepFig2)&INIT_FCBRST)==0)){
							IEC101RstLinkState(dp);
//							IEC101Printf(dp,"��һ����·��ʼ������λ��վ��·״̬.\n");
						}
					}
				}
				if(dp->LnkInitStartFig2==YES) {
					if(((dp->InitStepFig2)&INIT_LINKREPLY)==0) {
						IEC101AskLinkState(dp);
//						IEC101Printf(dp,"��վ����·��ʼ������ѯ��վ��·״̬.\n");
					}
					if((((dp->InitStepFig2)&INIT_LINKREPLY)>0)
					&&(((dp->InitStepFig2)&INIT_FCBRST)==0)) {
						IEC101RstLinkState(dp);
//						IEC101Printf(dp,"��վ����·��ʼ������λ��վ��·״̬.\n");
					}
				}
			}
			if((dp->fsendfig2==YES)&&(dp->fresendfig2==YES)) {
				dp->fresendframe2++;
				if((dp->fresendframe2) <= IEC101_MAX_RESEND_NUM)
				{
					IEC101SendData(dp,dp->sbuf2,dp->slen2);
//					IEC101Printf(dp,"warning ����ʱ�ط�,��%d��\n",dp->fresendframe2);
				}
				dp->fresendfig2=NO;
			}
		}
		else if(dp->LnkRuleType==LINKRULE_IEC) {
			if(dp->fsendfig2==NO) {
				if(((dp->InitStepFig2)&INIT_LINKREPLY)==0) {
					IEC101AskLinkState(dp);
//					IEC101Printf(dp,"��վ����·��ʼ������ѯ��վ��·״̬.\n");
					return;
				}
				if((((dp->InitStepFig2)&INIT_LINKREPLY)>0)
				&&(((dp->InitStepFig2)&INIT_FCBRST)==0)) {
					IEC101RstLinkState(dp);
//					IEC101Printf(dp,"��վ����·��ʼ������λ��վ��·״̬.\n");
					return;
				}
				if(((((dp->InitStepFig)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig)&INIT_FCBRST)>0))
				&&((((dp->InitStepFig2)&INIT_LINKREPLY)>0)&&(((dp->InitStepFig2)&INIT_FCBRST)>0))) {
					if(IEC101ReportData(dp)==YES) {
						IEC101BalanceModeSendDataClm(dp);
					}
				}
			}
			if((dp->fsendfig2==YES)&&(dp->fresendfig2==YES)){
				dp->fresendframe2++;
				if((dp->fresendframe2) <= IEC101_MAX_RESEND_NUM) {
					IEC101SendData(dp,dp->sbuf2,dp->slen2);
//					IEC101Printf(dp,"warning :��ʱ�ط�,��%d��\n",dp->fresendframe2);
				}
				dp->fresendfig2=NO;
			}
		}
		else{
		}
	}
	else{
		return;
	}	
}

static u_int8 IEC101ReportData(struct APPDEF *dp)
{
	u_int32 i,j;
	u_int8 fig;
	struct MSGREPORT *tempptr;

	fig=NO;
	for(i=0;i<DATA_PRI_MAXNUM;i++) {
		if(((dp->InitStepFig)&INIT_FCBRST)==0) continue;
		if(((dp->InitStepFig2)&INIT_FCBRST)==0) continue;
		while((dp->data1ptr[i])!=NULL) {
			if(((dp->data1ptr[i]->len)>0)&&((dp->data1ptr[i]->len)<=300)) {
				memcpy(dp->sbuf2,dp->data1ptr[i]->buf,dp->data1ptr[i]->len);
				dp->slen2=dp->data1ptr[i]->len;
				tempptr=dp->data1ptr[i];
				dp->data1ptr[i]=dp->data1ptr[i]->next;
				if(tempptr!=NULL) {
					IEC101ReportPtrFree(dp,tempptr);
				}
				fig=YES;
				break;
			}
			else {
				tempptr=dp->data1ptr[i];
				dp->data1ptr[i]=dp->data1ptr[i]->next;
				if(tempptr!=NULL) {
					IEC101ReportPtrFree(dp,tempptr);
				}
			}
		}
		if(fig==YES) {
			break;
		}
	}
	if(i>=DATA_PRI_MAXNUM) {
		fig=NO;
		for(j=0;j<DATA_PRI_MAXNUM;j++) {
			if(((dp->InitStepFig)&INIT_FCBRST)==0) continue;
			if(((dp->InitStepFig2)&INIT_FCBRST)==0) continue;
			while(dp->data2ptr[j]!=NULL) {
				if(((dp->data2ptr[j]->len)>0)&&((dp->data2ptr[j]->len)<=300)) {
					memcpy(dp->sbuf2,dp->data2ptr[j]->buf,dp->data2ptr[j]->len);
					dp->slen2=dp->data2ptr[j]->len;
					tempptr=dp->data2ptr[j];
					dp->data2ptr[j]=dp->data2ptr[j]->next;
					if(tempptr!=NULL) {
						IEC101ReportPtrFree(dp,tempptr);
					}
					fig=YES;
					break;
				}
				else {
					tempptr=dp->data2ptr[j];
					dp->data2ptr[j]=dp->data2ptr[j]->next;
					if(tempptr!=NULL) {
						IEC101ReportPtrFree(dp,tempptr);
					}
				}
			}
			if(fig==YES) {
				break;
			}
		}
	}
	return(fig);
}

static void IEC101BalanceModeSendDataClm(struct APPDEF *dp)
{
	u_int32 i;
	u_int8 /*asdutype,*/sum;
	u_int16 cot;

	if((dp->slen2)<=(dp->min_len)) {
		return;
	}
	i=6;
	if(dp->linkaddr_bytes==2) {
		i++;
	}
	//asdutype=dp->sbuf2[i];
	if(dp->cot_bytes==1) {
		cot=dp->sbuf2[i+2];
	}
	else if(dp->cot_bytes==2) {
		cot=dp->sbuf2[i+2]+(dp->sbuf2[i+3])*256;
	}
	else {
		cot=dp->sbuf2[i+2];
	}
	if((cot==0x01)&&(cot==0x02)&&(cot==0x03)){/*��վ�������*/
		dp->sbuf2[4]=0x03;
		dp->sbuf2[4] |=((dp->dirfig) | PRM | FCV | (dp->fcb2));
	}
	else{/*��վ��Ӧ������*/
		dp->sbuf2[4]=0x03;
		dp->sbuf2[4] |=((dp->dirfig) | PRM | FCV | (dp->fcb2));
	}
	dp->fcb2 ^=FCB;
	sum=0;
	for(i=4;i<(dp->slen2-2);i++) {
		sum+=dp->sbuf2[i];
	}
	dp->sbuf2[dp->slen2-2]=sum;
	IEC101SendData(dp,dp->sbuf2,dp->slen2);
	dp->fsendfig2=YES;
	return;
}

static u_int8 IEC101CanSendTest(struct APPDEF *dp)
{
	if (NO == dp->AutoTest) return NO;
	if (LINKRULE_ZHUHAI_XUJI == dp->LnkRuleType) return NO;		
	if (0 == (dp->InitStepFig2 & INIT_LINKREPLY)) return NO;
	if (0 == (dp->InitStepFig2 & INIT_FCBRST))    return NO;
	if (0 == (dp->InitStepFig & INIT_LINKREPLY)) return NO;
	if (0 == (dp->InitStepFig & INIT_FCBRST))    return NO;
	return YES;
}

static void IEC101YkYtClm(struct APPDEF *dp)
{
	u_int8 asdutype;
	struct SYS_TIME t;
	unsigned int  yk_total_num;
	int rv;
	unsigned int b_remote = 0;
	u_int8 cmd;

	cmd=255;
	asdutype=dp->rbuf[dp->HeadLen];
	sys_get_time(&t);
	if(asdutype==45) {
		IEC101Printf(dp,"\nCTL: recv single cmd.(t=%04d.%02d.%02d %02d:%02d:%02d.%03d).",t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);
	}
	if(asdutype==46) {
		IEC101Printf(dp,"\nCTL: recv double cmd.(t=%04d.%02d.%02d %02d:%02d:%02d.%03d).",t.year,t.month,t.day,t.hour,t.minute,t.second,t.msecond);
	}
	yk_get_total_num(&yk_total_num);	
	if(((asdutype==45)&&(yk_total_num==0))
	||((asdutype==46)&&(yk_total_num==0))) {
		IEC101Printf(dp,"CTL: error, list is null.");
		IEC101YkYtMsgDecodeChk0(dp,dp->rbuf,&(dp->YkYt));
		IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
		IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
		IEC101YkYtClearClm(dp,&(dp->YkYt));
		return;
	}
	if(((asdutype==45)&&((dp->SYK_cmdexe_mode==CTLMODE_Std_S_E)))
	||((asdutype==46)&&((dp->DYK_cmdexe_mode==CTLMODE_Std_S_E)))
	) {
		if(IEC101YkYtMsgDecodeChk(dp,dp->rbuf,&(dp->YkYt))<0) {
			return;
		}
	}

	if(asdutype==45){
		if(((dp->YkYt.dco)&0x7f)==1) cmd=DP_CLOSE;
		else                                        cmd=DP_OPEN;
	}
	if(asdutype==46){
		if(((dp->YkYt.dco)&0x7f)==2) cmd=DP_CLOSE;
		else                                        cmd=DP_OPEN;
	}

	if ((((dp->YkYt.dco)&0x80)==0x80)&&(dp->YkYt.cot==6)) {
		if(dp->YkYt.step !=YKYT_STEP_NULL){
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
			return;
		}				
		dp->YkYt.step=YKYT_STEP_SELECT_CMD;
		//if (yx_get_value (p_client->sid, get_yx_remote_local_index (), 1, &b_remote)(b_remote = 1) < 0) {
		if ((b_remote = 1) < 0) {
			rv = -1;
		}
		else {
			if (b_remote == DP_CLOSE) {
				/* Զ�� */
				rv = yk_select (dp->YkYt.point, cmd, dp->comfd);
			}
			else {
				/* �͵� */
				rv = -1;
			}
		}
		if(rv < 0){
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
		}
		else{
			dp->YkYt.dco_bak=dp->YkYt.dco;
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_SELECT);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
		}
	}
	if((((dp->YkYt.dco)&0x80)==0x00)&&(dp->YkYt.cot==6)) {
		if(dp->YkYt.step != YKYT_STEP_SELECT_CMD){
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
			return;
		}
		if(((dp->YkYt.dco_bak)&0x7f) != ((dp->YkYt.dco)&0x7f)){
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
			return;			
		}
		dp->YkYt.step=YKYT_STEP_EXECUTE_CMD;
		//if (yx_get_value (p_client->sid, get_yx_remote_local_index (), 1, &b_remote)(b_remote = 1) < 0) {
		if ((b_remote = 1) < 0) {
			rv = -1;
		}
		else {
			if (b_remote == DP_CLOSE) {
				/* Զ�� */
				rv = yk_operate (dp->YkYt.point, dp->YkYt.dco, dp->comfd);
			}
			else {
				/* �͵� */
				rv = -1;
			}
		}
		if(rv < 0){
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
		}
		else{
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_EXECUTE);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
		}
	}
	if(dp->YkYt.cot==8) {
		dp->YkYt.step=YKYT_STEP_CANCEL_CMD;
		rv = yk_unselect (dp->YkYt.point, dp->comfd);
		if(rv < 0){
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
		}
		else{
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_CANCEL);
			IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
			IEC101YkYtClearClm(dp,&(dp->YkYt));
		}
	}
	return;
}

static int IEC101YkYtMsgDecodeChk0(struct APPDEF *dp,u_int8 *rbuf,struct APPYKDEF *pYkYt)
{
	int i;
	u_int infaddr;
	u_int8 dco;
	struct ASDUHEAD head;

	i=IEC101GetAsduCommonFromMsg(dp,&head,rbuf);
	if(dp->infaddr_bytes==2) {
		infaddr=rbuf[i]+rbuf[i+1]*256;
		i+=2;
	}
	else if(dp->infaddr_bytes==3) {
		infaddr=rbuf[i]+rbuf[i+1]*256+rbuf[i+2]*65536;
		i+=3;
	}
	else {
		infaddr=rbuf[i]+rbuf[i+1]*256;
		i+=2;
	}
	dco=rbuf[i];
	if(head.asdutype==45) {
		pYkYt->cmd=CMD_SYK;
	}
	if(head.asdutype==46) {
		pYkYt->cmd=CMD_DYK;
	}
	if(head.asdutype==47) {
		pYkYt->cmd=CMD_YT;
	}
	//pYkYt->byYKType=RDB_YKTYPE_YK;
	pYkYt->asdutype=head.asdutype;
	pYkYt->vsq=head.vsq;
	pYkYt->cot=head.cot;
	pYkYt->appaddr=head.appaddr;
	pYkYt->infaddr=infaddr;
	pYkYt->dco=dco;
	pYkYt->rece_vsq=pYkYt->vsq;
	pYkYt->rece_cot=pYkYt->cot;
	pYkYt->rece_appaddr=pYkYt->appaddr;
	pYkYt->rece_Infaddr=pYkYt->infaddr;
	pYkYt->rece_dco=pYkYt->dco;
	return(0);
}

static int IEC101YkYtMsgDecodeChk(struct APPDEF *dp,u_int8 *rbuf,struct APPYKDEF *pYkYt)
{
	int i,point = 0;
	u_int infaddr;
	u_int8 dco;
	struct ASDUHEAD head;

	i=IEC101GetAsduCommonFromMsg(dp,&head,rbuf);
	if(dp->infaddr_bytes==2) {
		infaddr=rbuf[i]+rbuf[i+1]*256;
		i+=2;
	}
	else if(dp->infaddr_bytes==3) {
		infaddr=rbuf[i]+rbuf[i+1]*256+rbuf[i+2]*65536;
		i+=3;
	}
	else {
		infaddr=rbuf[i]+rbuf[i+1]*256;
		i+=2;
	}
	dco=rbuf[i];
	if(head.asdutype==45) {
		pYkYt->cmd=CMD_SYK;
	}
	if(head.asdutype==46) {
		pYkYt->cmd=CMD_DYK;
	}
	//pYkYt->byYKType=RDB_YKTYPE_YK;
	pYkYt->asdutype=head.asdutype;
	pYkYt->vsq=head.vsq;
	pYkYt->cot=head.cot;
	pYkYt->appaddr=head.appaddr;
	pYkYt->infaddr=infaddr;
	pYkYt->dco=dco;
	pYkYt->rece_vsq=pYkYt->vsq;
	pYkYt->rece_cot=pYkYt->cot;
	pYkYt->rece_appaddr=pYkYt->appaddr;
	pYkYt->rece_Infaddr=pYkYt->infaddr;
	pYkYt->rece_dco=pYkYt->dco;

	if(((dp->ASDUCHK_strategy==ASDUCHK_VSQ_COT_ADDR)||(dp->ASDUCHK_strategy==ASDUCHK_VSQ)||(dp->ASDUCHK_strategy==ASDUCHK_VSQ_ADDR))
	&&((head.vsq)!=0x01)) {
		IEC101Printf(dp,"CTL: error, receive vsq(%d) is not rational.",head.vsq);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_NAK);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_END);
		IEC101YkYtClearClm(dp,pYkYt);
		return(-1);
	}
	if(((dp->ASDUCHK_strategy==ASDUCHK_VSQ_COT_ADDR)||(dp->ASDUCHK_strategy==ASDUCHK_COT)||(dp->ASDUCHK_strategy==ASDUCHK_COT_ADDR))
	&&((head.cot!=0x06)&&(head.cot!=0x08))) {
		IEC101Printf(dp,"CTL: error, cot(%d) is not correct.",head.cot);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_NAK_COTERR);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_END);
		IEC101YkYtClearClm(dp,pYkYt);
		return(-1);
	}
	if(((dp->ASDUCHK_strategy==ASDUCHK_VSQ_COT_ADDR)||(dp->ASDUCHK_strategy==ASDUCHK_ADDR)||(dp->ASDUCHK_strategy==ASDUCHK_VSQ_ADDR)||(dp->ASDUCHK_strategy==ASDUCHK_COT_ADDR))
	&&(((dp->appaddr_bytes==1)&&((dp->YkYt.appaddr)!=0xff)&&((dp->YkYt.appaddr)!=(dp->appaddr)))
	||((dp->appaddr_bytes==2)&&((dp->YkYt.appaddr)!=0xffff)&&((dp->YkYt.appaddr)!=(dp->appaddr))))) {
		IEC101Printf(dp,"CTL: error, recv app addr(%d) is not match with para(%d).",dp->rece_appaddr,dp->appaddr);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_NAK_APPADDERR);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_END);
		IEC101YkYtClearClm(dp,pYkYt);
		return(-1);
	}
	if(
		((pYkYt->cmd==CMD_SYK)&&(dp->SYK_cmdexe_mode==CTLMODE_Std_S_E)&&((dco&0x83)!=0x80)&&((dco&0x83)!=0x81)&&((dco&0x83)!=0x00)&&((dco&0x83)!=0x01))
		||((pYkYt->cmd==CMD_DYK)&&(dp->DYK_cmdexe_mode==CTLMODE_Std_S_E)&&((dco&0x83)!=0x81)&&((dco&0x83)!=0x82)&&((dco&0x83)!=0x01)&&((dco&0x83)!=0x02))
	  ){
		IEC101Printf(dp,"CTL: error, dco(%d) is not correct.",dco);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_NAK);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_END);
		IEC101YkYtClearClm(dp,pYkYt);
		return(-1);
	}
	if(pYkYt->cmd==CMD_SYK) {
		point=pYkYt->infaddr - dp->InfAddr_syk;
	}
	if(pYkYt->cmd==CMD_DYK) {
		point=pYkYt->infaddr - dp->InfAddr_dyk;
	}
	if(point<0) {
		IEC101Printf(dp,"CTL: error, infaddr map point fail(rece infaddr=%d,map point=%d).",infaddr,point);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_NAK_INFADDERR);
		IEC101YkYtReportReply(dp,pYkYt,YKYT_REPLY_END);
		IEC101YkYtClearClm(dp,pYkYt);
		return(-1);
	}
	pYkYt->point=point;
	return(0);
}

static void IEC101YkYtReportReply(struct APPDEF *dp,struct APPYKDEF *pYkYt,u_int8 replyfig)
{
	u_int8 sbuf[100];
	u_int16 i;

	i=0;
	sbuf[i++]=pYkYt->asdutype;
	sbuf[i++]=pYkYt->vsq;
	if(replyfig==YKYT_REPLY_ACK) {
		sbuf[i++]=pYkYt->cot+1;
		IEC101Printf(dp,"CTL: respond, ack.");
	}
	else if(replyfig==YKYT_REPLY_NAK) {
		sbuf[i++]=(pYkYt->cot+1)|P_N;
		IEC101Printf(dp,"CTL: respond, nak.");
		pYkYt->ResultFig=NO;
	}
	else if(replyfig==YKYT_REPLY_NAK_APPADDERR) {
		if(dp->COT_strategy==COT_IEC) {
			sbuf[i++]=46|P_N;
		}
		else {
			sbuf[i++]=(pYkYt->cot+1)|P_N;
		}
		IEC101Printf(dp,"CTL: respond, nak.");
		pYkYt->ResultFig=NO;
	}
	else if(replyfig==YKYT_REPLY_NAK_COTERR) {
		if(dp->COT_strategy==COT_IEC) {
			sbuf[i++]=45|P_N;
		}
		else {
			sbuf[i++]=(pYkYt->cot+1)|P_N;
		}
		IEC101Printf(dp,"CTL: respond, nak.");
		pYkYt->ResultFig=NO;
	}
	else if(replyfig==YKYT_REPLY_NAK_INFADDERR) {
		if(dp->COT_strategy==COT_IEC) {
			sbuf[i++]=47|P_N;
		}
		else {
			sbuf[i++]=(pYkYt->cot+1)|P_N;
		}
		IEC101Printf(dp,"CTL: respond, nak.");
		pYkYt->ResultFig=NO;
	}
	else if(replyfig==YKYT_REPLY_CANCEL) {
		sbuf[i++]=0x09;
		IEC101Printf(dp,"CTL: respond, cancel ack.");
		pYkYt->ResultFig=NO;
	}
	else if(replyfig==YKYT_REPLY_SELECT) {
		sbuf[i++]=0x07;
		IEC101Printf(dp,"CTL: respond, ack.");
	}
	else if(replyfig==YKYT_REPLY_EXECUTE) {
		sbuf[i++]=0x07;
		IEC101Printf(dp,"CTL: respond, ack.");
	}
	else if(replyfig==YKYT_REPLY_END) {
		if(pYkYt->asdutype==dp->ASDU_SYK) {
			if(dp->SYkOverStgy==YKYTOVER_NO) {
				return;
			}
			if(dp->SYkOverStgy!=YKYTOVER_ALWAYS) {
				if(pYkYt->ResultFig==YES) {
					if(dp->SYkOverStgy!=YKYTOVER_SUCESS) {
						return;
					}
				}
				else {
					if(dp->SYkOverStgy!=YKYTOVER_FAIL) {
						return;
					}
				}
			}
		}
		if(pYkYt->asdutype==dp->ASDU_DYK) {
			if(dp->DYkOverStgy==YKYTOVER_NO) {
				return;
			}
			if(dp->DYkOverStgy!=YKYTOVER_ALWAYS) {
				if(pYkYt->ResultFig==YES) {
					if(dp->DYkOverStgy!=YKYTOVER_SUCESS) {
						return;
					}
				}
				else {
					if(dp->DYkOverStgy!=YKYTOVER_FAIL) {
						return;
					}
				}
			}
		}
		sbuf[i++]=0x0a;
		IEC101Printf(dp,"CTL: respond, over.");
	}
	else {
		sbuf[i++]=(pYkYt->cot+1)|P_N;
		IEC101Printf(dp,"CTL: respond, nak.");
	}
	if(dp->cot_bytes==2) {
		sbuf[i++]=0x00;
	}
	sbuf[i++]=(u_int8)(pYkYt->rece_appaddr);
	if(dp->appaddr_bytes==2) {
		sbuf[i++]=(u_int8)((pYkYt->rece_appaddr)>>8);
	}
	sbuf[i++]=(u_int8)(pYkYt->rece_Infaddr);
	sbuf[i++]=(u_int8)((pYkYt->rece_Infaddr)>>8);
	if(dp->infaddr_bytes==3) {
		sbuf[i++]=(u_int8)((pYkYt->rece_Infaddr)>>16);
	}
	sbuf[i++]=pYkYt->rece_dco;
	IEC101SaveReport(dp,sbuf,i,PRI_YKYT);
	return;
}

static void IEC101YkYtClearClm(struct APPDEF *dp,struct APPYKDEF *pYkYt)
{
	if((dp->InitOkFig==YES)&&(pYkYt->step!=YKYT_STEP_NULL)) {
		IEC101Printf(dp,"CTL: clear flags.");
	}
	pYkYt->cmd=CMD_NULL;
	pYkYt->step=YKYT_STEP_NULL;
	pYkYt->asdutype=0;
	pYkYt->vsq=0;
	pYkYt->cot=0;
	pYkYt->appaddr=0;
	pYkYt->infaddr=0;
	pYkYt->point=0;
	pYkYt->index=0;
	pYkYt->index_bak=0;
	pYkYt->index2=0;
	pYkYt->dco=0xff;
	pYkYt->dco_bak=0xff;
	pYkYt->dco_old=0xff;
	pYkYt->Yk_TimeCount=0;
	pYkYt->Yt_TimeCount=0;
	pYkYt->ResultFig=YES;
	pYkYt->rece_vsq=0;
	pYkYt->rece_cot=0;
	pYkYt->rece_appaddr=0;
	pYkYt->rece_Infaddr=0;
	pYkYt->rece_dco=0;

	pYkYt->dest_appid=0;
	pYkYt->return_id=0;
	pYkYt->byCommandStage=0;
	pYkYt->byActionType=0;
	pYkYt->byYKType=0;
	pYkYt->bySynType=0;
	pYkYt->ocLock=0;

	return;
}

static void IEC101YkYtCheckClm(struct APPDEF *dp)
{

	if(dp->YkYt.step!=YKYT_STEP_NULL) {
		if(dp->YkYt.asdutype==dp->ASDU_SYK) {
			dp->YkYt.Yk_TimeCount++;
			if(dp->YkYt.Yk_TimeCount>dp->SYK_TimeOut) {
				IEC101Printf(dp,"CTL: check, single time out %ds.",dp->SYK_TimeOut);
				if((dp->YkYt.step==YKYT_STEP_SELECT_CMD)||(dp->YkYt.step==YKYT_STEP_EXECUTE_CMD)||(dp->YkYt.step==YKYT_STEP_CANCEL_CMD)) {
					//IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
				}
				dp->YkYt.ResultFig=NO;
				IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
				IEC101YkYtClearClm(dp,&(dp->YkYt));
			}
		}
		else if(dp->YkYt.asdutype==dp->ASDU_DYK) {
			dp->YkYt.Yk_TimeCount++;
			if(dp->YkYt.Yk_TimeCount>dp->DYK_TimeOut) {
				if(dp->YkYt.Yk_TimeCount>dp->DYK_TimeOut) {
					IEC101Printf(dp,"CTL: check, double time out %ds.",dp->DYK_TimeOut);
				}
				if((dp->YkYt.step==YKYT_STEP_SELECT_CMD)||(dp->YkYt.step==YKYT_STEP_EXECUTE_CMD)||(dp->YkYt.step==YKYT_STEP_CANCEL_CMD)) {
					//IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_NAK);
				}
				dp->YkYt.ResultFig=NO;
				IEC101YkYtReportReply(dp,&(dp->YkYt),YKYT_REPLY_END);
				IEC101YkYtClearClm(dp,&(dp->YkYt));
			}
		}
		else {
			IEC101Printf(dp,"CTL: check, error, cmd-course is unknown.");
			IEC101YkYtClearClm(dp,&(dp->YkYt));
		}
	}
	return;
}












