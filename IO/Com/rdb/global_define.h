/********************************************************************
	created:	2016/07/06
	created:	6:7:2016   14:53
	filename: 	D:\rd\src_code\F28M35_PRJ\dtu\common\inc\sav.h
	file path:	D:\rd\src_code\F28M35_PRJ\dtu\common\inc
	file base:	sav
	file ext:	h
	author:		
	purpose:	sample value defines
*********************************************************************/
#ifndef _GLOBAL_DEFINE_H_
#define _GLOBAL_DEFINE_H_

#include <time.h>

#include "data_types.h"

#define SYSTEM_TICK			100

#define SYS_FREQ_STD		50.0f
#define SYS_FREQ_ERR_MIN	0.3f
#define SYS_FREQ_ERR_MAX	5.5f
#define SYS_FREQ_LESS		(SYS_FREQ_STD - SYS_FREQ_ERR_MIN)
#define SYS_FREQ_MORE		(SYS_FREQ_STD + SYS_FREQ_ERR_MIN)
#define SYS_FREQ_MIN		(SYS_FREQ_STD - SYS_FREQ_ERR_MAX)
#define SYS_FREQ_MAX		(SYS_FREQ_STD + SYS_FREQ_ERR_MAX)

#define MAX_BAY_NUM			8	/* DTU */
#define MAX_VOL_NUM			3	/* DTU(2), 箱变3 */

#define MAX_BI_BRD_NUM		8
#define MAX_BO_BRD_NUM		8

#define MAX_NON_ELEC_NUM	6	/* 非电量保护数量 */
#define MAX_ARCP_NUM        8	/* 重合闸保护数量 (电流加电压) */

#define REM_LOC_NUM			1
#define SIG_RET_NUM			1
#define DEV_MANTN_NUM		1
#define PWR_ST_NUM			4
#define SPRING_NO_ENEGY_NUM	8
#define SWITCH_ST_NUM		8
#define SPEC_BI_NUM			(REM_LOC_NUM + SIG_RET_NUM + DEV_MANTN_NUM + PWR_ST_NUM + SPRING_NO_ENEGY_NUM + SWITCH_ST_NUM + MAX_NON_ELEC_NUM)	/* 特殊遥信映射 */
#define SPEC_BO_NUM			2	/* 特殊遥控映射，电源2 */

#define BAY_CUR_NUM			8	/* 电流间隔数，FTU 1，DTU 8 */
#define BUS_VOL_NUM			3	/* 电压间隔数，FTU 1，DTU 2，箱变 3 */

#define AC_CHAN_NUM					32		/* 7: 1间隔，3电压，4电流。30: 8间隔，6电压，24电流 */
#define DC_V_I_T_NUM				6		/* 电压/电流/温度复用通道 */
#define DC_I_NUM					2		/* 电流通道 */
#define DC_CPU_NUM					1		/* CPU温度通道 */
#define DC_CHAN_NUM					(DC_V_I_T_NUM + DC_I_NUM + DC_CPU_NUM)
#define BO_CHAN_NUM					52		/* 4 * 11 + 8/2(电源板上的开出/电源管理输出) */
#define BI_CHAN_NUM					88		/* 开入通道数目:4 * 21 + 4(电源)*/
//#define PWR_ST_CHAN_NUM				4
#define LED_NUM						8

#define RUN_NORMAL			0x0040
#define RUN_ALARM			0x0041
#define RUN_LOCKED			0x0042

#define DEF_TASK_STACK_SIZE		1024

#define PRIO_ADC_TASK		49		/* 响应AD采样 */
#define PRIO_INTERPOLATION	48		/* 插值 */
#define PRIO_PROT_FFT		14		/* 保护FFT计算 */
#define PRIO_PROT_CALC		13		/* 保护计算 */
#define PRIO_RECD_WAVE		12		/* 录波任务 */
#define PRIO_HARM_FFT		11		/* 谐波FFT计算 */
#define PRIO_DC_SAV			10		/* dc sample task */
#define PRIO_DC_SAV_SEND    9		/* dc sample task */

#define PRIO_INIT_TASK			5
#define PRIO_BI_INPUT			4
#define PRIO_BI_CHECK			4
#define PRIO_BO_TASK			4
#define PRIO_SYNC_TASK			4
#define PRIO_WAVE				3
#define PRIO_RDB				2
#define PRIO_YC_UPDATE			2
#define PRIO_101				2		/* IEC 101 */
#define PRIO_104				2		/* IEC 104 */
#define PRIO_MODBUS				2
#define PRIO_REC_LOG			2
#define PRIO_UPGRADE			1
#define PRIO_STATUS_MONI		1
#define PRIO_CLI_MENU			1		/* flash prog menu task */

/* version */
#define MANG_VER			"1.00"
#define PROT_VER			"1.00"

#define CFG_DIR_NAME		PROTECT_DIR"/cfg"
#define CFG_BAK_DIR_NAME	PROTECT_DIR"/cfg/bak"
#define CMTR_DIR_NAME		PROTECT_DIR"/comtrade"
#define REC_DIR_NAME		PROTECT_DIR"/rec"
#define REC_BAK_DIR_NAME	PROTECT_DIR"/rec/bak"
#define EXV_DIR_NAME		PROTECT_DIR"/exv"
#define EXV_BAK_DIR_NAME	PROTECT_DIR"/exv/bak"
#define FIXPT_DIR_NAME		PROTECT_DIR"/fixpt"
#define FIXPT_BAK_DIR_NAME	PROTECT_DIR"/fixpt/bak"
#define LOG_DIR_NAME		PROTECT_DIR"/log"
#define LOG_BAK_DIR_NAME	PROTECT_DIR"/log/bak"

enum mang_error_code_e {
	err_init_ver = 0,
	err_init_mem,
	err_init_dirs,
	err_init_param_setting,
	err_init_board_cfg,
	err_init_rdb,
	err_init_bibo_drv,
	err_init_sync,
	err_init_bi,
	err_init_bo,
	err_init_net_address,
	err_init_53101,
	err_init_screen,
	err_init_yc_update_rdb,
	err_init_status_monitor,
	err_init_iec104_server,
	err_init_iec101_service,
	err_init_modbus_service,
	err_init_cmtr_task,
	err_init_record_log,
	err_init_cli_menu,

	err_init_mang_cnt
};

enum prot_error_code_e {
	err_init_sample = 0,
	err_init_handle_data,
	err_init_protect,

	err_init_prot_cnt
};

typedef struct global_ver_s {
	char		mang_ver [16];
	char		prot_ver [16];
	char		firm_ver [16];
	Uint16		prog_chksum;
} global_ver_t;

typedef struct start_status_s {
	Uint32 start_error_flag_prot;
	Uint32 start_error_flag_mang;
	time_t device_start_time;
} start_status_t;

extern start_status_t start_status;
extern global_ver_t g_ver;

#endif /* _GLOBAL_DEFINE_H_ */

