/********************************************************************
	created:	2017/07/21
	created:	
	filename: 	D:\rd\src_code\ftu\inc\point_table.h
	file path:	D:\rd\src_code\ftu\inc
	file base:	point_table
	file ext:	h
	author:		
	
	purpose:	communication point table all stored in rdb
*********************************************************************/

#ifndef _POINT_TABLE_H_
#define _POINT_TABLE_H_

/*
 * 点表
 */
/* 遥信 */
enum yx_e {
    /* 起始地址0x0001 */
    device_status_yx,                       	//0x0001	装置状态
    
    TEV1_yellow_alarm_yx,                      	//0x0002	TEV1黄色告警
    TEV1_red_alarm_yx,						   	//0x0003	TEV1红色告警
    TEV2_yellow_alarm_yx,                      	//0x0004	TEV2黄色告警
    TEV2_red_alarm_yx,						   	//0x0005	TEV2红色告警

    HFCT1_yellow_alarm_yx,						//0x0006	HFCT1黄色告警
    HFCT1_red_alarm_yx,							//0x0007	HFCT1红色告警
    HFCT2_yellow_alarm_yx,						//0x0008	HFCT2黄色告警
    HFCT2_red_alarm_yx,							//0x0009	HFCT2红色告警
    HFCT3_yellow_alarm_yx,						//0x000a	HFCT3黄色告警
    HFCT3_red_alarm_yx,							//0x000b	HFCT3红色告警
    HFCT4_yellow_alarm_yx,						//0x000c	HFCT4黄色告警
    HFCT4_red_alarm_yx,							//0x000d	HFCT4红色告警
    HFCT5_yellow_alarm_yx,						//0x000e	HFCT5黄色告警
    HFCT5_red_alarm_yx,							//0x000f	HFCT5红色告警
    HFCT6_yellow_alarm_yx,						//0x0010	HFCT6黄色告警
    HFCT6_red_alarm_yx,							//0x0011	HFCT6红色告警

    UHF1_yellow_alarm_yx,                       //0x0012	UHF1黄色告警
    UHF1_red_alarm_yx,							//0x0013	UHF1红色告警
    UHF2_yellow_alarm_yx,						//0x0014	UHF2黄色告警
    UHF2_red_alarm_yx,							//0x0015	UHF2红色告警

    AA1_yellow_alarm_yx,                        //0x0016	AA1黄色告警
    AA1_red_alarm_yx,							//0x0017	AA1红色告警
    AA2_yellow_alarm_yx,						//0x0018	AA2黄色告警
    AA2_red_alarm_yx,							//0x0019	AA2红色告警

	AE1_yellow_alarm_yx,						//0x001a 	AE1黄色告警
	AE1_red_alarm_yx,							//0x001b    AE1红色告警
	AE2_yellow_alarm_yx,						//0x001c	AE2黄色告警
	AE2_red_alarm_yx,							//0x001d	AE2红色告警

    measure_end_yx,

/* 遥信数量 */
	num_of_yx								/* must be at last */
};

/* 遥测 */
enum yc_e {
/* 起始地址0x4001 */
    //读
	TEV1_amplitude_yc,			           	 	//0x4001	TEV1幅值	
	TEV1_num_yc,                               	//0x4002	TEV1脉冲个数
	TEV1_severity_yc,                          	//0x4003	TEV1严重度
    TEV1_center_biased_adv_yc,                 	//0x4004	TEV1中心偏置建议值
    TEV1_noise_biased_adv_yc,                  //0x4005	TEV1噪声偏置建议值
	TEV2_amplitude_yc,                         //0x4006	TEV2幅值
    TEV2_num_yc,                               //0x4007	TEV2脉冲个数
    TEV2_severity_yc,                          //0x4008	TEV2严重度
    TEV2_center_biased_adv_yc,                 //0x4009	TEV2中心偏置建议值
    TEV2_noise_biased_adv_yc,                  //0x400a	TEV2噪声偏置建议值
    HFCT1_amplitude_yc,                        //0x400b	HFCT1幅值
    HFCT1_num_yc,                              //0x400c	HFCT1脉冲个数
    HFCT1_severity_yc,                         //0x400d	HFCT1严重度
    HFCT1_center_biased_adv_yc,                //0x400e	HFCT1中心偏置建议值
    HFCT1_noise_biased_adv_yc,                 //0x400f	HFCT1噪声偏置建议值
    HFCT2_amplitude_yc,                        //0x4010	HFCT2幅值
    HFCT2_num_yc,                              //0x4011	HFCT2脉冲个数
    HFCT2_severity_yc,                         //0x4012	HFCT2严重度
    HFCT2_center_biased_adv_yc,                //0x4013	HFCT2中心偏置建议值
    HFCT2_noise_biased_adv_yc,                 //0x4014	HFCT2噪声偏置建议值
    HFCT3_amplitude_yc,                        //0x4015	HFCT3幅值
    HFCT3_num_yc,                              //0x4016	HFCT3脉冲个数
    HFCT3_severity_yc,                         //0x4017	HFCT3严重度
    HFCT3_center_biased_adv_yc,                //0x4018	HFCT3中心偏置建议值
    HFCT3_noise_biased_adv_yc,                 //0x4019	HFCT3噪声偏置建议值
    HFCT4_amplitude_yc,                        //0x401a	HFCT4幅值
    HFCT4_num_yc,                              //0x401b	HFCT4脉冲个数
    HFCT4_severity_yc,                         //0x401c	HFCT4严重度
    HFCT4_center_biased_adv_yc,                //0x401d	HFCT4中心偏置建议值
    HFCT4_noise_biased_adv_yc,                 //0x401e	HFCT4噪声偏置建议值
    HFCT5_amplitude_yc,                        //0x401f	HFCT5幅值
    HFCT5_num_yc,                              //0x4020	HFCT5脉冲个数
    HFCT5_severity_yc,                         //0x4021	HFCT5严重度
    HFCT5_center_biased_adv_yc,                //0x4022	HFCT5中心偏置建议值
    HFCT5_noise_biased_adv_yc,                 //0x4023	HFCT5噪声偏置建议值
    HFCT6_amplitude_yc,                        //0x4024	HFCT6幅值
    HFCT6_num_yc,                              //0x4025	HFCT6脉冲个数
    HFCT6_severity_yc,                         //0x4026	HFCT6严重度
    HFCT6_center_biased_adv_yc,                //0x4027	HFCT6中心偏置建议值
    HFCT6_noise_biased_adv_yc,                 //0x4028	HFCT6噪声偏置建议值

    UHF1_amplitude_yc,                         //0x4029	UHF1幅值
    UHF1_num_yc,                               //0x402a	UHF1脉冲个数
    UHF1_severity_yc,                          //0x402b	UHF1严重度
    UHF1_center_biased_adv_yc,                 //0x402c	UHF1中心偏置建议值
    UHF1_noise_biased_adv_yc,                  //0x402d	UHF1噪声偏置建议值
    UHF2_amplitude_yc,                         //0x402e	UHF2幅值
    UHF2_num_yc,                               //0x402f	UHF2脉冲个数
    UHF2_severity_yc,                          //0x4030	UHF2严重度
    UHF2_center_biased_adv_yc,                 //0x4031	UHF2中心偏置建议值
    UHF2_noise_biased_adv_yc,                  //0x4032	UHF2噪声偏置建议值

    AA1_amplitude_yc,                          //0x4033	AA1幅值
    AA1_num_yc,								   //0x4034	AA1脉冲个数
    AA1_severity_yc,                           //0x4035	AA1严重度
    AA1_noise_biased_adv_yc,                   //0x4036	AA1噪声偏置建议值
    AA2_amplitude_yc,                          //0x4037	AA2幅值
    AA2_num_yc,							 	   //0x4038	AA2脉冲个数
    AA2_severity_yc,                           //0x4039	AA2严重度
    AA2_noise_biased_adv_yc,                   //0x403a	AA2噪声偏置建议值

    AE1_amplitude_yc,                          //0x403b	AE1幅值
    AE1_num_yc,							 	   //0x403c	AE1脉冲个数
    AE1_severity_yc,                           //0x403d	AE1严重度
    AE1_noise_biased_adv_yc,                   //0x403e	AE1噪声偏置建议值
    AE2_amplitude_yc,                          //0x403f	AE2幅值
    AE2_num_yc,							 	   //0x4040	AE2脉冲个数
    AE2_severity_yc,                           //0x4041	AE2严重度
    AE2_noise_biased_adv_yc,                   //0x4042	AE2噪声偏置建议值

    CPU_temp_yc,                               //CPU温度
    CPU_vcc_yc,                                //CPU电压

    Battery_vcc_yc,                            //电池电压
    Battery_cur_yc,                            //电池电流

//	version=95,                                //0x4060	版本


/* 遥测数量 */
    num_of_yc								/* must be at last */
};

/* 遥控 */
enum yk_e {
	/* 起始地址0x6001 */
	TEV1_waverec_single_yk,			/* 单次录波 */
	TEV1_waverec_continuous_yk,		/* 连续录波指令 */
	TEV1_reset_meas_yk,				/* 测试值重置 */
	TEV2_waverec_single_yk,			/* 单次录波 */
	TEV2_waverec_continuous_yk, 		/* 连续录波指令 */
	TEV2_reset_meas_yk,				/* 测试值重置 */

	HFCT1_waverec_single_yk,			/* 单次录波 */
	HFCT1_waverec_continuous_yk, 		/* 连续录波指令 */
	HFCT1_reset_meas_yk,				/* 测试值重置 */
	HFCT2_waverec_continuous_yk, 		/* 连续录波指令 */
	HFCT2_waverec_single_yk,			/* 单次录波 */
	HFCT2_reset_meas_yk,				/* 测试值重置 */

	UHF1_waverec_single_yk,			/* 单次录波 */
	UHF1_waverec_continuous_yk,		/* 连续录波指令 */
	UHF1_reset_meas_yk,				/* 测试值重置 */
	UHF2_waverec_single_yk,			/* 单次录波 */
	UHF2_waverec_continuous_yk,		/* 连续录波指令 */
	UHF2_reset_meas_yk,				/* 测试值重置 */

	AA1_waverec_continuous_yk, 		/* 连续录波指令 */
	AA1_reset_meas_yk,					/* 测试值重置 */
	AA2_waverec_continuous_yk, 		/* 连续录波指令 */
	AA2_reset_meas_yk, 				/* 测试值重置 */
	
	AE1_waverec_continuous_yk, 		/* 连续录波指令 */
	AE1_reset_meas_yk,					/* 测试值重置 */
	AE2_waverec_continuous_yk, 		/* 连续录波指令 */
	AE2_reset_meas_yk, 				/* 测试值重置 */
	Start_Stop_Test,				/* 启动停止测试 */
/* 遥控数量 */
    num_of_yk								/* must be at last */
};

/* 设定值 */
enum dz_e {
/* 起始地址0x8201 */		
	TEV1_test_mode_dz,				/* 检测模式 */
	TEV1_show_mode_dz,				/* 图形显示 */
	TEV1_gain_dz,					/* 增益调节 */
	TEV1_low_threshold_dz,			/* 黄色报警预警值 */
	TEV1_high_threshold_dz,			/* 红色报警预警值 */
	TEV1_pulse_trigger_dz,			/* 脉冲触发 */
	TEV1_pulse_counttime_dz,		/* 脉冲计数时长 */
	TEV1_waverec_auto_dz,			/* 自动录波 */
	TEV1_waverec_time_dz,			/* 连续录波时长 */
	TEV1_center_biased_dz, 		/* 中心偏置 */
	TEV1_noise_biased_dz,			/* 噪声偏置 */
	
	TEV2_test_mode_dz,				/* 检测模式 */
	TEV2_show_mode_dz,				/* 图形显示 */
	TEV2_gain_dz,					/* 增益调节 */
	TEV2_low_threshold_dz,			/* 黄色报警预警值 */
	TEV2_high_threshold_dz, 		/* 红色报警预警值 */
	TEV2_pulse_trigger_dz,			/* 脉冲触发 */
	TEV2_pulse_counttime_dz,		/* 脉冲计数时长 */
	TEV2_waverec_auto_dz,			/* 自动录波 */
	TEV2_waverec_time_dz,			/* 连续录波时长 */
	TEV2_center_biased_dz, 			/* 中心偏置 */
	TEV2_noise_biased_dz,			/* 噪声偏置 */

	HFCT1_test_mode_dz,				/* 检测模式 */
	HFCT1_show_mode_dz,				/* 图形显示 */
	HFCT1_gain_dz, 					/* 增益调节 */
	HFCT1_passband_down_limit_dz,		/* 通带下限 */
	HFCT1_passband_up_limit_dz,		/* 通带上限 */
	HFCT1_pulse_trigger_dz,			/* 脉冲触发 */
	HFCT1_pulse_counttime_dz,			/* 脉冲计数时长 */
	HFCT1_waverec_auto_dz, 			/* 自动录波 */
	HFCT1_waverec_time_dz, 			/* 连续录波时长 */
	HFCT1_center_biased_dz,			/* 中心偏置 */
	HFCT1_noise_biased_dz, 			/* 噪声偏置 */

	HFCT2_test_mode_dz,				/* 检测模式 */
	HFCT2_show_mode_dz,				/* 图形显示 */
	HFCT2_gain_dz, 					/* 增益调节 */
	HFCT2_passband_down_limit_dz,		/* 通带下限 */
	HFCT2_passband_up_limit_dz,		/* 通带上限 */
	HFCT2_pulse_trigger_dz,			/* 脉冲触发 */
	HFCT2_pulse_counttime_dz,			/* 脉冲计数时长 */
	HFCT2_waverec_auto_dz, 			/* 自动录波 */
	HFCT2_waverec_time_dz, 			/* 连续录波时长 */
	HFCT2_center_biased_dz,			/* 中心偏置 */
	HFCT2_noise_biased_dz, 			/* 噪声偏置 */

	UHF1_test_mode_dz,				/* 检测模式 */
	UHF1_show_mode_dz,				/* 图形显示 */
	UHF1_gain_dz,					/* 增益调节 */
	UHF1_low_threshold_dz,			/* 黄色报警预警值 */
	UHF1_high_threshold_dz,			/* 红色报警预警值 */
	UHF1_pulse_trigger_dz,			/* 脉冲触发 */
	UHF1_pulse_counttime_dz,		/* 脉冲计数时长 */
	UHF1_waverec_auto_dz,			/* 自动录波 */
	UHF1_waverec_time_dz,			/* 连续录波时长 */
	UHF1_center_biased_dz, 			/* 中心偏置 */
	UHF1_noise_biased_dz,			/* 噪声偏置 */

	UHF2_test_mode_dz,				/* 检测模式 */
	UHF2_show_mode_dz,				/* 图形显示 */
	UHF2_gain_dz,					/* 增益调节 */
	UHF2_low_threshold_dz,			/* 黄色报警预警值 */
	UHF2_high_threshold_dz, 		/* 红色报警预警值 */
	UHF2_pulse_trigger_dz,			/* 脉冲触发 */
	UHF2_pulse_counttime_dz,		/* 脉冲计数时长 */	
	UHF2_waverec_auto_dz,			/* 自动录波 */
	UHF2_waverec_time_dz,			/* 连续录波时长 */
	UHF2_center_biased_dz, 			/* 中心偏置 */
	UHF2_noise_biased_dz,			/* 噪声偏置 */

    AA1_test_mode_dz,				/* 检测模式 */
    AA1_show_mode_dz,				/* 图形显示 */
    AA1_gain_dz,					/* 增益调节 */
    AA1_voice_dz,					/* 声音调节 */
    AA1_low_threshold_dz,			/* 黄色报警预警值 */
    AA1_high_threshold_dz,			/* 红色报警预警值 */
    AA1_waverec_auto_dz,			/* 自动录波 */
    AA1_waverec_time_dz,			/* 连续录波时长 */
    AA1_niose_biased_dz, 			/* 偏置 */

    AA2_test_mode_dz,				/* 检测模式 */
    AA2_show_mode_dz,				/* 图形显示 */
    AA2_gain_dz,					/* 增益调节 */
    AA2_voice_dz,					/* 声音调节 */
    AA2_low_threshold_dz,			/* 黄色报警预警值 */
    AA2_high_threshold_dz,			/* 红色报警预警值 */
    AA2_waverec_auto_dz,			/* 自动录波 */
    AA2_waverec_time_dz,			/* 连续录波时长 */
    AA2_niose_biased_dz, 			/* 偏置 */

    AE1_test_mode_dz,				/* 检测模式 */
    AE1_show_mode_dz,				/* 图形显示 */
    AE1_gain_dz,					/* 增益调节 */
    AE1_voice_dz,					/* 声音调节 */
    AE1_low_threshold_dz,			/* 黄色报警预警值 */
    AE1_high_threshold_dz,			/* 红色报警预警值 */
    AE1_waverec_auto_dz,			/* 自动录波 */
    AE1_waverec_time_dz,			/* 连续录波时长 */
    AE1_niose_biased_dz, 			/* 偏置 */


    AE2_test_mode_dz,				/* 检测模式 */
    AE2_show_mode_dz,				/* 图形显示 */
    AE2_gain_dz,					/* 增益调节 */
    AE2_voice_dz,					/* 声音调节 */
    AE2_low_threshold_dz,			/* 黄色报警预警值 */
    AE2_high_threshold_dz,			/* 红色报警预警值 */
    AE2_waverec_auto_dz,			/* 自动录波 */
    AE2_waverec_time_dz,			/* 连续录波时长 */
    AE2_niose_biased_dz, 			/* 偏置 */

	DualCH_Tri_mode,				/* 触发模式 */
	DualCH_show_mode,				/* 显示模式 */
	DualCH_Tri_CH,					/* 触发通道 */
	DualCH_Tri_time,				/* 触发时长 */
	DualCH_save_wave,				/* 保存当前波形 */

	HF_Channel1,					/* 高频通道1     */
	HF_Channel2,					/* 高频通道2	 */
	LF_Channel1,					/* 低频通道1     */
	LF_Channel2,					/* 低频通道2	 */
	Dual_Channel,					/* 双通道模式 */

	Get_cur_page_dz,				/* 获取当前各个页面配置指令 */

/* 设定值数量 */
	num_of_dz						/* must be at last */
};

/* YX */
#define	YX_VALID_NUM 	(num_of_yx)

/* YC */
#define YC_VALID_NUM	(num_of_yc)

/* YK */
#define YK_VALID_NUM	(num_of_yk)

/* SDZ */
#define DZ_VALID_NUM	(num_of_dz)

#endif /* _POINT_TABLE_H_ */

