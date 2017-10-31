/********************************************************************
	created:	2016/06/12
	created:	12:6:2016   14:28
	filename: 	D:\rd\src_code\ftu\inc\setting_table.h
	file path:	D:\rd\src_code\ftu\inc
	file base:	setting_table
	file ext:	h
	author:		
	
	purpose:	device parameters and settings table
*********************************************************************/
#ifndef _SETTING_TABLE_H_
#define _SETTING_TABLE_H_

#define SETTING_PARAM_DESC_SIZE			32

#define S_P_TYPE_INT					0x00
#define	S_P_TYPE_UINT					0x01
#define S_P_TYPE_FLOAT					0x02

#define CONV_STR(STR) #STR

/* 装置参数（参数） */
enum device_parameter_e {
	device_address = 0,						/* 装置地址 */
	ip_addr_1,								/* ip地址1 */
	subnet_mask_1,							/* 子网掩码1 */
	ip_addr_2,								/* ip地址2 */
	subnet_mask_2,							/* 子网掩码2 */
	ip_addr_3,								/* ip地址3 */
	subnet_mask_3,							/* 子网掩码3 */
	ip_addr_4,								/* ip地址4 */
	subnet_mask_4,							/* 子网掩码4 */
	time_sync_mode,							/* 时间同步方式 */
	ring_net_mode,							/* 环网方式 */
	yc_deadband,							/* 遥测死区 */
	auto_reset_delay,						/* 自动复归延时 */
	num_of_device_paramter					/* must be at last */
};

/* 电压定值（定值） */
enum voltage_setting_e {
	pt_pri_vol = 0,							/* pt一次额定 */
	pt_sec_vol,								/* pt二次额定 */
	vol_upper_limit,						/* 电压上限 */
	vol_lower_limit,						/* 电压下限 */
	vol_unbalance_rate,						/* 电压不平衡率 */
	vol_u0_limit,							/* 母线u0越限定值 */
	num_of_voltage_setting					/* must be at last */
};

/* 电流定值（定值） */
enum current_setting_e {
	ct_pri_cur = 0,							/* ct一次额定 */
	ct_sec_cur,								/* ct二次额定 */
	i0_ct,									/* 线路有零序ct */
	i0_ct_pri_cur,							/* 零序ct一次额定 */
	i0_ct_sec_cur,							/* 零序ct二次额定 */
	rated_cap,								/* 线路额定容量 */
	two_meter_method,						/* 两表法测量 */
	i0_self_gen,							/* 自产i0 */
	switch_double_pos,						/* 开关双位置 */
	cur_upper_limit,						/* 电流上限 */
	cur_unbalance_rate_limit,				/* 电流不平衡率上限 */
	load_rate_limit,						/* 负载率上限 */
	num_of_current_setting					/* must be at last */
};

/* 保护定值（定值） */
enum protection_setting_e {
	protection_put_into = 0,				/* 保护投入 */
	over_cur_prot_1_setting,				/* 过流1段保护定值 */
	over_cur_prot_1_time,					/* 过流1段保护时间 */
	over_cur_prot_1_put_into,				/* 过流1段投入 */
	over_cur_prot_2_setting,				/* 过流2段保护定值 */
	over_cur_prot_2_time,					/* 过流2段保护时间 */
	over_cur_prot_2_put_into,				/* 过流2段投入 */
	over_cur_prot_3_setting,				/* 过流3段保护定值 */
	over_cur_prot_3_time,					/* 过流3段保护时间 */
	over_cur_prot_3_put_into,				/* 过流3段投入 */
	zero_cur_prot_1_setting,				/* 零序电流1段保护定值 */
	zero_cur_prot_1_time,					/* 零序电流1段保护时间 */
	zero_cur_prot_1_put_into,				/* 零序电流1段投入 */
	overload_prot_setting,					/* 过负荷定值 */
	overload_prot_time,						/* 过负荷时间 */
	overload_prot_put_into,					/* 过负荷投入 */
	num_of_protection_setting				/* must be at last */
};

/* 运行告警定值（定值） */
enum running_alarm_setting_e {
	optical_coupler_alarm_put_into = 0,		/* 光耦失电报警投入 */
	pt_breaking_put_into,					/* PT断线告警投入 */
	U0_alarm_put_into,						/* 3U0报警投入 */
	vol_over_upper_limit_put_into,			/* 电压越上限告警投入 */
	vol_over_lower_limit_put_into,			/* 电压越下限告警投入 */
	vol_unbalance_rate_over_limit_put_into,	/* 电压不平衡越限告警投入 */
	cur_over_limit_put_into,				/* 电流越限告警投入 */
	cur_unbalance_rate_over_limit_put_into,	/* 电流不平衡越限告警投入 */
	ac_power_source_loss_power_put_into,	/* 交流电源失电告警投入 */
	battery_alarm_put_into,					/* 电池异常告警投入 */
	battery_low_vol_alarm_put_into,			/* 电池欠压告警投入 */
	battery_switch_off_alarm_put_into,		/* 电池关断告警投入 */
	battery_active_alarm_put_into,			/* 电池活化告警投入 */
	num_of_running_alarm_setting			/* must be at last */
};

/* 遥信定值（定值） */
enum yx_setting_e {
	yx_1_anti_shake_time = 0,
	yx_2_anti_shake_time,
	yx_3_anti_shake_time,
	yx_4_anti_shake_time,
	yx_5_anti_shake_time,
	yx_6_anti_shake_time,
	yx_7_anti_shake_time,
	yx_8_anti_shake_time,
	yx_9_anti_shake_time,
	yx_10_anti_shake_time,
	yx_11_anti_shake_time,
	yx_12_anti_shake_time,
	yx_13_anti_shake_time,
	yx_14_anti_shake_time,
	yx_15_anti_shake_time,
	yx_16_anti_shake_time,
	yx_17_anti_shake_time,
	yx_18_anti_shake_time,
	yx_19_anti_shake_time,
	yx_20_anti_shake_time,
	yx_21_anti_shake_time,
	yx_22_anti_shake_time,
	yx_23_anti_shake_time,
	yx_24_anti_shake_time,
	num_of_yx_setting						/* must be at last */
};

/* 遥控定值（定值） */
enum yk_setting_e {
	yk_1_holding_time = 0,
	yk_2_holding_time,
	yk_3_holding_time,
	yk_4_holding_time,
	yk_5_holding_time,
	yk_6_holding_time,
	yk_7_holding_time,
	yk_8_holding_time,
	yk_9_holding_time,
	yk_10_holding_time,
	yk_1_select_time,
	yk_2_select_time,
	yk_3_select_time,
	yk_4_select_time,
	yk_5_select_time,
	yk_6_select_time,
	yk_7_select_time,
	yk_8_select_time,
	yk_9_select_time,
	yk_10_select_time,
	num_of_yk_setting						/* must be at last */
};

/* 交流调整定值（定值） */
enum ac_sample_adjust_setting_e {
	Ua_factor = 0,
	Ub_factor,
	Uc_factor,
	Uab_factor,
	Ubc_factor,
	Uca_factor,
	U0_factor,
	U1_factor,
	U2_factor,
	Ia_factor,
	Ib_factor,
	Ic_factor,
	I0_factor,
	Pa_factor,
	Pb_factor,
	Pc_factor,
	Qa_factor,
	Qb_factor,
	Qc_factor,
	Sa_factor,
	Sb_factor,
	Sc_factor,
	COSa_factor,
	COSb_factor,
	COSc_factor,
	P_factor,
	Q_factor,
	S_factor,
	COS_factor,
	num_of_ac_sample_adjust_setting			/* must be at last */
};

/* 交流基准定值（定值） */
enum ac_sample_base_setting_e {
	Ua_base = 0,
	Ub_base,
	Uc_base,
	Uab_base,
	Ubc_base,
	Uca_base,
	U0_base,
	U1_base,
	U2_base,
	Ia_base,
	Ib_base,
	Ic_base,
	I0_base,
	Pa_base,
	Pb_base,
	Pc_base,
	Qa_base,
	Qb_base,
	Qc_base,
	Sa_base,
	Sb_base,
	Sc_base,
	COSa_base,
	COSb_base,
	COSc_base,
	P_base,
	Q_base,
	S_base,
	COS_base,
	num_of_ac_sample_base_setting			/* must be at last */
};

/* 直流调零系数（定值） */
enum dc_sample_adjust_setting_e {
	dc_250v_1_zero = 0,
	dc_250v_2_zero,
	dc_10v_zero,
	dc_cur_zero,
	dc_temp_1_zero,
	dc_temp_2_zero,
	dc_250v_1_factor,
	dc_250v_2_factor,
	dc_10v_factor,
	dc_cur_factor,
	dc_temp_1_factor,
	dc_temp_2_factor,
	dc_250v_1_base,
	dc_250v_2_base,
	dc_10v_base,
	dc_cur_base,
	dc_temp_1_base,
	dc_temp_2_base,
	num_of_dc_sample_adjust_setting			/* must be at last */
};

enum setting_and_param_group {
	p_device_param = 0,
	s_voltage_setting,
	s_current_setting,
	s_protect_setting,
	s_running_alarm_setting,
	s_yx_setting,
	s_yk_setting,
	s_ac_sam_adj_setting,
	s_ac_sam_base_setting,
	s_dc_sam_adj_setting,
	num_of_setting_and_param_table			/* must be at last */
};

typedef struct setting_param_entry_s {
	union {
		int				i_val;
		unsigned int 	u_val;
		float			f_val;
	} value;
	int val_type;
	char desc [SETTING_PARAM_DESC_SIZE];
} setting_param_entry_t;

typedef struct setting_param_table_s {
	/*
	setting_param_entries为一个数组，数组长度为entry_num
	*/
	setting_param_entry_t * setting_param_entries;
	int entry_num;
	char desc [SETTING_PARAM_DESC_SIZE];
} setting_param_table_t;

int get_setting_param (
	int table_index, 
	int item_index, 
	setting_param_entry_t * p_mem
	);

/* set settings or parameters in memory */
int set_setting_param (
	int app_id,					/* who set settings or params */
	int table_index, 
	int item_index, 
	setting_param_entry_t * p_mem
	);

int save_setting_param (int app_id);

int setting_param_show ();

int init_device_setting_param ();


#endif /* _SETTING_TABLE_H_ */

