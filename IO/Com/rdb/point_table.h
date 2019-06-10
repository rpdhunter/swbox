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
    AA1_50Hz_yc,                            //AA1频率分量１
    AA1_100Hz_yc,                           //AA1频率分量２
    AA1_effective_yc,                       //AA1测量有效值
    AA1_peak_yc,                            //AA1频率峰值
    AA2_amplitude_yc,                          //0x4037	AA2幅值
    AA2_num_yc,							 	   //0x4038	AA2脉冲个数
    AA2_severity_yc,                           //0x4039	AA2严重度
    AA2_noise_biased_adv_yc,                   //0x403a	AA2噪声偏置建议值
    AA2_50Hz_yc,                            //AA2频率分量１
    AA2_100Hz_yc,                           //AA2频率分量２
    AA2_effective_yc,                       //AA2测量有效值
    AA2_peak_yc,                            //AA2频率峰值

    AE1_amplitude_yc,                          //0x403b	AE1幅值
    AE1_num_yc,							 	   //0x403c	AE1脉冲个数
    AE1_severity_yc,                           //0x403d	AE1严重度
    AE1_noise_biased_adv_yc,                   //0x403e	AE1噪声偏置建议值
    AE1_50Hz_yc,                            //AE1频率分量１
    AE1_100Hz_yc,                           //AE1频率分量２
    AE1_effective_yc,                       //AE1测量有效值
    AE1_peak_yc,                            //AE1频率峰值
    AE2_amplitude_yc,                          //0x403f	AE2幅值
    AE2_num_yc,							 	   //0x4040	AE2脉冲个数
    AE2_severity_yc,                           //0x4041	AE2严重度
    AE2_noise_biased_adv_yc,                   //0x4042	AE2噪声偏置建议值
    AE2_50Hz_yc,                            //AE2频率分量１
    AE2_100Hz_yc,                           //AE2频率分量２
    AE2_effective_yc,                       //AE2测量有效值
    AE2_peak_yc,                            //AE2频率峰值

    CPU_temp_yc,                               //CPU温度
    CPU_vcc_yc,                                //CPU电压

    Battery_vcc_yc,                            //电池电压
    Battery_cur_yc,                            //电池电流

    TEV1_RPRD1_yc,                             //TEV1通道PRPD点1
    TEV1_RPRD2_yc,                             //TEV1通道PRPD点2
    TEV1_RPRD3_yc,                             //TEV1通道PRPD点3
    TEV1_RPRD4_yc,                             //TEV1通道PRPD点4
    TEV1_RPRD5_yc,                             //TEV1通道PRPD点5
    TEV1_RPRD6_yc,                             //TEV1通道PRPD点6
    TEV1_RPRD7_yc,                             //TEV1通道PRPD点7
    TEV1_RPRD8_yc,                             //TEV1通道PRPD点8
    TEV1_RPRD9_yc,                             //TEV1通道PRPD点9
    TEV1_RPRD10_yc,                            //TEV1通道PRPD点10
    TEV1_RPRD11_yc,                            //TEV1通道PRPD点11
    TEV1_RPRD12_yc,                            //TEV1通道PRPD点12
    TEV1_RPRD13_yc,                            //TEV1通道PRPD点13
    TEV1_RPRD14_yc,                            //TEV1通道PRPD点14
    TEV1_RPRD15_yc,                            //TEV1通道PRPD点15
    TEV1_RPRD16_yc,                            //TEV1通道PRPD点16
    TEV1_RPRD17_yc,                            //TEV1通道PRPD点17
    TEV1_RPRD18_yc,                            //TEV1通道PRPD点18
    TEV1_RPRD19_yc,                            //TEV1通道PRPD点19
    TEV1_RPRD20_yc,                            //TEV1通道PRPD点20
    TEV1_RPRD21_yc,                            //TEV1通道PRPD点21
    TEV1_RPRD22_yc,                            //TEV1通道PRPD点22
    TEV1_RPRD23_yc,                            //TEV1通道PRPD点23
    TEV1_RPRD24_yc,                            //TEV1通道PRPD点24
    TEV1_RPRD25_yc,                            //TEV1通道PRPD点25
    TEV1_RPRD26_yc,                            //TEV1通道PRPD点26
    TEV1_RPRD27_yc,                            //TEV1通道PRPD点27
    TEV1_RPRD28_yc,                            //TEV1通道PRPD点28
    TEV1_RPRD29_yc,                            //TEV1通道PRPD点29
    TEV1_RPRD30_yc,                            //TEV1通道PRPD点30
    TEV1_RPRD31_yc,                            //TEV1通道PRPD点31
    TEV1_RPRD32_yc,                            //TEV1通道PRPD点32
    TEV1_RPRD33_yc,                            //TEV1通道PRPD点33
    TEV1_RPRD34_yc,                            //TEV1通道PRPD点34
    TEV1_RPRD35_yc,                            //TEV1通道PRPD点35
    TEV1_RPRD36_yc,                            //TEV1通道PRPD点36
    TEV1_RPRD37_yc,                            //TEV1通道PRPD点37
    TEV1_RPRD38_yc,                            //TEV1通道PRPD点38
    TEV1_RPRD39_yc,                            //TEV1通道PRPD点39
    TEV1_RPRD40_yc,                            //TEV1通道PRPD点40

    TEV2_RPRD1_yc,                             //TEV2通道PRPD点1
    TEV2_RPRD2_yc,                             //TEV2通道PRPD点2
    TEV2_RPRD3_yc,                             //TEV2通道PRPD点3
    TEV2_RPRD4_yc,                             //TEV2通道PRPD点4
    TEV2_RPRD5_yc,                             //TEV2通道PRPD点5
    TEV2_RPRD6_yc,                             //TEV2通道PRPD点6
    TEV2_RPRD7_yc,                             //TEV2通道PRPD点7
    TEV2_RPRD8_yc,                             //TEV2通道PRPD点8
    TEV2_RPRD9_yc,                             //TEV2通道PRPD点9
    TEV2_RPRD10_yc,                            //TEV2通道PRPD点10
    TEV2_RPRD11_yc,                            //TEV2通道PRPD点11
    TEV2_RPRD12_yc,                            //TEV2通道PRPD点12
    TEV2_RPRD13_yc,                            //TEV2通道PRPD点13
    TEV2_RPRD14_yc,                            //TEV2通道PRPD点14
    TEV2_RPRD15_yc,                            //TEV2通道PRPD点15
    TEV2_RPRD16_yc,                            //TEV2通道PRPD点16
    TEV2_RPRD17_yc,                            //TEV2通道PRPD点17
    TEV2_RPRD18_yc,                            //TEV2通道PRPD点18
    TEV2_RPRD19_yc,                            //TEV2通道PRPD点19
    TEV2_RPRD20_yc,                            //TEV2通道PRPD点20
    TEV2_RPRD21_yc,                            //TEV2通道PRPD点21
    TEV2_RPRD22_yc,                            //TEV2通道PRPD点22
    TEV2_RPRD23_yc,                            //TEV2通道PRPD点23
    TEV2_RPRD24_yc,                            //TEV2通道PRPD点24
    TEV2_RPRD25_yc,                            //TEV2通道PRPD点25
    TEV2_RPRD26_yc,                            //TEV2通道PRPD点26
    TEV2_RPRD27_yc,                            //TEV2通道PRPD点27
    TEV2_RPRD28_yc,                            //TEV2通道PRPD点28
    TEV2_RPRD29_yc,                            //TEV2通道PRPD点29
    TEV2_RPRD30_yc,                            //TEV2通道PRPD点30
    TEV2_RPRD31_yc,                            //TEV2通道PRPD点31
    TEV2_RPRD32_yc,                            //TEV2通道PRPD点32
    TEV2_RPRD33_yc,                            //TEV2通道PRPD点33
    TEV2_RPRD34_yc,                            //TEV2通道PRPD点34
    TEV2_RPRD35_yc,                            //TEV2通道PRPD点35
    TEV2_RPRD36_yc,                            //TEV2通道PRPD点36
    TEV2_RPRD37_yc,                            //TEV2通道PRPD点37
    TEV2_RPRD38_yc,                            //TEV2通道PRPD点38
    TEV2_RPRD39_yc,                            //TEV2通道PRPD点39
    TEV2_RPRD40_yc,                            //TEV2通道PRPD点40

    HFCT1_RPRD1_yc,                             //HFCT1通道PRPD点1
    HFCT1_RPRD2_yc,                             //HFCT1通道PRPD点2
    HFCT1_RPRD3_yc,                             //HFCT1通道PRPD点3
    HFCT1_RPRD4_yc,                             //HFCT1通道PRPD点4
    HFCT1_RPRD5_yc,                             //HFCT1通道PRPD点5
    HFCT1_RPRD6_yc,                             //HFCT1通道PRPD点6
    HFCT1_RPRD7_yc,                             //HFCT1通道PRPD点7
    HFCT1_RPRD8_yc,                             //HFCT1通道PRPD点8
    HFCT1_RPRD9_yc,                             //HFCT1通道PRPD点9
    HFCT1_RPRD10_yc,                            //HFCT1通道PRPD点10
    HFCT1_RPRD11_yc,                            //HFCT1通道PRPD点11
    HFCT1_RPRD12_yc,                            //HFCT1通道PRPD点12
    HFCT1_RPRD13_yc,                            //HFCT1通道PRPD点13
    HFCT1_RPRD14_yc,                            //HFCT1通道PRPD点14
    HFCT1_RPRD15_yc,                            //HFCT1通道PRPD点15
    HFCT1_RPRD16_yc,                            //HFCT1通道PRPD点16
    HFCT1_RPRD17_yc,                            //HFCT1通道PRPD点17
    HFCT1_RPRD18_yc,                            //HFCT1通道PRPD点18
    HFCT1_RPRD19_yc,                            //HFCT1通道PRPD点19
    HFCT1_RPRD20_yc,                            //HFCT1通道PRPD点20
    HFCT1_RPRD21_yc,                            //HFCT1通道PRPD点21
    HFCT1_RPRD22_yc,                            //HFCT1通道PRPD点22
    HFCT1_RPRD23_yc,                            //HFCT1通道PRPD点23
    HFCT1_RPRD24_yc,                            //HFCT1通道PRPD点24
    HFCT1_RPRD25_yc,                            //HFCT1通道PRPD点25
    HFCT1_RPRD26_yc,                            //HFCT1通道PRPD点26
    HFCT1_RPRD27_yc,                            //HFCT1通道PRPD点27
    HFCT1_RPRD28_yc,                            //HFCT1通道PRPD点28
    HFCT1_RPRD29_yc,                            //HFCT1通道PRPD点29
    HFCT1_RPRD30_yc,                            //HFCT1通道PRPD点30
    HFCT1_RPRD31_yc,                            //HFCT1通道PRPD点31
    HFCT1_RPRD32_yc,                            //HFCT1通道PRPD点32
    HFCT1_RPRD33_yc,                            //HFCT1通道PRPD点33
    HFCT1_RPRD34_yc,                            //HFCT1通道PRPD点34
    HFCT1_RPRD35_yc,                            //HFCT1通道PRPD点35
    HFCT1_RPRD36_yc,                            //HFCT1通道PRPD点36
    HFCT1_RPRD37_yc,                            //HFCT1通道PRPD点37
    HFCT1_RPRD38_yc,                            //HFCT1通道PRPD点38
    HFCT1_RPRD39_yc,                            //HFCT1通道PRPD点39
    HFCT1_RPRD40_yc,                            //HFCT1通道PRPD点40

    HFCT2_RPRD1_yc,                             //HFCT2通道PRPD点1
    HFCT2_RPRD2_yc,                             //HFCT2通道PRPD点2
    HFCT2_RPRD3_yc,                             //HFCT2通道PRPD点3
    HFCT2_RPRD4_yc,                             //HFCT2通道PRPD点4
    HFCT2_RPRD5_yc,                             //HFCT2通道PRPD点5
    HFCT2_RPRD6_yc,                             //HFCT2通道PRPD点6
    HFCT2_RPRD7_yc,                             //HFCT2通道PRPD点7
    HFCT2_RPRD8_yc,                             //HFCT2通道PRPD点8
    HFCT2_RPRD9_yc,                             //HFCT2通道PRPD点9
    HFCT2_RPRD10_yc,                            //HFCT2通道PRPD点10
    HFCT2_RPRD11_yc,                            //HFCT2通道PRPD点11
    HFCT2_RPRD12_yc,                            //HFCT2通道PRPD点12
    HFCT2_RPRD13_yc,                            //HFCT2通道PRPD点13
    HFCT2_RPRD14_yc,                            //HFCT2通道PRPD点14
    HFCT2_RPRD15_yc,                            //HFCT2通道PRPD点15
    HFCT2_RPRD16_yc,                            //HFCT2通道PRPD点16
    HFCT2_RPRD17_yc,                            //HFCT2通道PRPD点17
    HFCT2_RPRD18_yc,                            //HFCT2通道PRPD点18
    HFCT2_RPRD19_yc,                            //HFCT2通道PRPD点19
    HFCT2_RPRD20_yc,                            //HFCT2通道PRPD点20
    HFCT2_RPRD21_yc,                            //HFCT2通道PRPD点21
    HFCT2_RPRD22_yc,                            //HFCT2通道PRPD点22
    HFCT2_RPRD23_yc,                            //HFCT2通道PRPD点23
    HFCT2_RPRD24_yc,                            //HFCT2通道PRPD点24
    HFCT2_RPRD25_yc,                            //HFCT2通道PRPD点25
    HFCT2_RPRD26_yc,                            //HFCT2通道PRPD点26
    HFCT2_RPRD27_yc,                            //HFCT2通道PRPD点27
    HFCT2_RPRD28_yc,                            //HFCT2通道PRPD点28
    HFCT2_RPRD29_yc,                            //HFCT2通道PRPD点29
    HFCT2_RPRD30_yc,                            //HFCT2通道PRPD点30
    HFCT2_RPRD31_yc,                            //HFCT2通道PRPD点31
    HFCT2_RPRD32_yc,                            //HFCT2通道PRPD点32
    HFCT2_RPRD33_yc,                            //HFCT2通道PRPD点33
    HFCT2_RPRD34_yc,                            //HFCT2通道PRPD点34
    HFCT2_RPRD35_yc,                            //HFCT2通道PRPD点35
    HFCT2_RPRD36_yc,                            //HFCT2通道PRPD点36
    HFCT2_RPRD37_yc,                            //HFCT2通道PRPD点37
    HFCT2_RPRD38_yc,                            //HFCT2通道PRPD点38
    HFCT2_RPRD39_yc,                            //HFCT2通道PRPD点39
    HFCT2_RPRD40_yc,                            //HFCT2通道PRPD点40

    HFCT3_RPRD1_yc,                             //HFCT3通道PRPD点1
    HFCT3_RPRD2_yc,                             //HFCT3通道PRPD点2
    HFCT3_RPRD3_yc,                             //HFCT3通道PRPD点3
    HFCT3_RPRD4_yc,                             //HFCT3通道PRPD点4
    HFCT3_RPRD5_yc,                             //HFCT3通道PRPD点5
    HFCT3_RPRD6_yc,                             //HFCT3通道PRPD点6
    HFCT3_RPRD7_yc,                             //HFCT3通道PRPD点7
    HFCT3_RPRD8_yc,                             //HFCT3通道PRPD点8
    HFCT3_RPRD9_yc,                             //HFCT3通道PRPD点9
    HFCT3_RPRD10_yc,                            //HFCT3通道PRPD点10
    HFCT3_RPRD11_yc,                            //HFCT3通道PRPD点11
    HFCT3_RPRD12_yc,                            //HFCT3通道PRPD点12
    HFCT3_RPRD13_yc,                            //HFCT3通道PRPD点13
    HFCT3_RPRD14_yc,                            //HFCT3通道PRPD点14
    HFCT3_RPRD15_yc,                            //HFCT3通道PRPD点15
    HFCT3_RPRD16_yc,                            //HFCT3通道PRPD点16
    HFCT3_RPRD17_yc,                            //HFCT3通道PRPD点17
    HFCT3_RPRD18_yc,                            //HFCT3通道PRPD点18
    HFCT3_RPRD19_yc,                            //HFCT3通道PRPD点19
    HFCT3_RPRD20_yc,                            //HFCT3通道PRPD点20
    HFCT3_RPRD21_yc,                            //HFCT3通道PRPD点21
    HFCT3_RPRD22_yc,                            //HFCT3通道PRPD点22
    HFCT3_RPRD23_yc,                            //HFCT3通道PRPD点23
    HFCT3_RPRD24_yc,                            //HFCT3通道PRPD点24
    HFCT3_RPRD25_yc,                            //HFCT3通道PRPD点25
    HFCT3_RPRD26_yc,                            //HFCT3通道PRPD点26
    HFCT3_RPRD27_yc,                            //HFCT3通道PRPD点27
    HFCT3_RPRD28_yc,                            //HFCT3通道PRPD点28
    HFCT3_RPRD29_yc,                            //HFCT3通道PRPD点29
    HFCT3_RPRD30_yc,                            //HFCT3通道PRPD点30
    HFCT3_RPRD31_yc,                            //HFCT3通道PRPD点31
    HFCT3_RPRD32_yc,                            //HFCT3通道PRPD点32
    HFCT3_RPRD33_yc,                            //HFCT3通道PRPD点33
    HFCT3_RPRD34_yc,                            //HFCT3通道PRPD点34
    HFCT3_RPRD35_yc,                            //HFCT3通道PRPD点35
    HFCT3_RPRD36_yc,                            //HFCT3通道PRPD点36
    HFCT3_RPRD37_yc,                            //HFCT3通道PRPD点37
    HFCT3_RPRD38_yc,                            //HFCT3通道PRPD点38
    HFCT3_RPRD39_yc,                            //HFCT3通道PRPD点39
    HFCT3_RPRD40_yc,                            //HFCT3通道PRPD点40

    HFCT4_RPRD1_yc,                             //HFCT4通道PRPD点1
    HFCT4_RPRD2_yc,                             //HFCT4通道PRPD点2
    HFCT4_RPRD3_yc,                             //HFCT4通道PRPD点3
    HFCT4_RPRD4_yc,                             //HFCT4通道PRPD点4
    HFCT4_RPRD5_yc,                             //HFCT4通道PRPD点5
    HFCT4_RPRD6_yc,                             //HFCT4通道PRPD点6
    HFCT4_RPRD7_yc,                             //HFCT4通道PRPD点7
    HFCT4_RPRD8_yc,                             //HFCT4通道PRPD点8
    HFCT4_RPRD9_yc,                             //HFCT4通道PRPD点9
    HFCT4_RPRD10_yc,                            //HFCT4通道PRPD点10
    HFCT4_RPRD11_yc,                            //HFCT4通道PRPD点11
    HFCT4_RPRD12_yc,                            //HFCT4通道PRPD点12
    HFCT4_RPRD13_yc,                            //HFCT4通道PRPD点13
    HFCT4_RPRD14_yc,                            //HFCT4通道PRPD点14
    HFCT4_RPRD15_yc,                            //HFCT4通道PRPD点15
    HFCT4_RPRD16_yc,                            //HFCT4通道PRPD点16
    HFCT4_RPRD17_yc,                            //HFCT4通道PRPD点17
    HFCT4_RPRD18_yc,                            //HFCT4通道PRPD点18
    HFCT4_RPRD19_yc,                            //HFCT4通道PRPD点19
    HFCT4_RPRD20_yc,                            //HFCT4通道PRPD点20
    HFCT4_RPRD21_yc,                            //HFCT4通道PRPD点21
    HFCT4_RPRD22_yc,                            //HFCT4通道PRPD点22
    HFCT4_RPRD23_yc,                            //HFCT4通道PRPD点23
    HFCT4_RPRD24_yc,                            //HFCT4通道PRPD点24
    HFCT4_RPRD25_yc,                            //HFCT4通道PRPD点25
    HFCT4_RPRD26_yc,                            //HFCT4通道PRPD点26
    HFCT4_RPRD27_yc,                            //HFCT4通道PRPD点27
    HFCT4_RPRD28_yc,                            //HFCT4通道PRPD点28
    HFCT4_RPRD29_yc,                            //HFCT4通道PRPD点29
    HFCT4_RPRD30_yc,                            //HFCT4通道PRPD点30
    HFCT4_RPRD31_yc,                            //HFCT4通道PRPD点31
    HFCT4_RPRD32_yc,                            //HFCT4通道PRPD点32
    HFCT4_RPRD33_yc,                            //HFCT4通道PRPD点33
    HFCT4_RPRD34_yc,                            //HFCT4通道PRPD点34
    HFCT4_RPRD35_yc,                            //HFCT4通道PRPD点35
    HFCT4_RPRD36_yc,                            //HFCT4通道PRPD点36
    HFCT4_RPRD37_yc,                            //HFCT4通道PRPD点37
    HFCT4_RPRD38_yc,                            //HFCT4通道PRPD点38
    HFCT4_RPRD39_yc,                            //HFCT4通道PRPD点39
    HFCT4_RPRD40_yc,                            //HFCT4通道PRPD点40

    HFCT5_RPRD1_yc,                             //HFCT5通道PRPD点1
    HFCT5_RPRD2_yc,                             //HFCT5通道PRPD点2
    HFCT5_RPRD3_yc,                             //HFCT5通道PRPD点3
    HFCT5_RPRD4_yc,                             //HFCT5通道PRPD点4
    HFCT5_RPRD5_yc,                             //HFCT5通道PRPD点5
    HFCT5_RPRD6_yc,                             //HFCT5通道PRPD点6
    HFCT5_RPRD7_yc,                             //HFCT5通道PRPD点7
    HFCT5_RPRD8_yc,                             //HFCT5通道PRPD点8
    HFCT5_RPRD9_yc,                             //HFCT5通道PRPD点9
    HFCT5_RPRD10_yc,                            //HFCT5通道PRPD点10
    HFCT5_RPRD11_yc,                            //HFCT5通道PRPD点11
    HFCT5_RPRD12_yc,                            //HFCT5通道PRPD点12
    HFCT5_RPRD13_yc,                            //HFCT5通道PRPD点13
    HFCT5_RPRD14_yc,                            //HFCT5通道PRPD点14
    HFCT5_RPRD15_yc,                            //HFCT5通道PRPD点15
    HFCT5_RPRD16_yc,                            //HFCT5通道PRPD点16
    HFCT5_RPRD17_yc,                            //HFCT5通道PRPD点17
    HFCT5_RPRD18_yc,                            //HFCT5通道PRPD点18
    HFCT5_RPRD19_yc,                            //HFCT5通道PRPD点19
    HFCT5_RPRD20_yc,                            //HFCT5通道PRPD点20
    HFCT5_RPRD21_yc,                            //HFCT5通道PRPD点21
    HFCT5_RPRD22_yc,                            //HFCT5通道PRPD点22
    HFCT5_RPRD23_yc,                            //HFCT5通道PRPD点23
    HFCT5_RPRD24_yc,                            //HFCT5通道PRPD点24
    HFCT5_RPRD25_yc,                            //HFCT5通道PRPD点25
    HFCT5_RPRD26_yc,                            //HFCT5通道PRPD点26
    HFCT5_RPRD27_yc,                            //HFCT5通道PRPD点27
    HFCT5_RPRD28_yc,                            //HFCT5通道PRPD点28
    HFCT5_RPRD29_yc,                            //HFCT5通道PRPD点29
    HFCT5_RPRD30_yc,                            //HFCT5通道PRPD点30
    HFCT5_RPRD31_yc,                            //HFCT5通道PRPD点31
    HFCT5_RPRD32_yc,                            //HFCT5通道PRPD点32
    HFCT5_RPRD33_yc,                            //HFCT5通道PRPD点33
    HFCT5_RPRD34_yc,                            //HFCT5通道PRPD点34
    HFCT5_RPRD35_yc,                            //HFCT5通道PRPD点35
    HFCT5_RPRD36_yc,                            //HFCT5通道PRPD点36
    HFCT5_RPRD37_yc,                            //HFCT5通道PRPD点37
    HFCT5_RPRD38_yc,                            //HFCT5通道PRPD点38
    HFCT5_RPRD39_yc,                            //HFCT5通道PRPD点39
    HFCT5_RPRD40_yc,                            //HFCT5通道PRPD点40

    HFCT6_RPRD1_yc,                             //HFCT6通道PRPD点1
    HFCT6_RPRD2_yc,                             //HFCT6通道PRPD点2
    HFCT6_RPRD3_yc,                             //HFCT6通道PRPD点3
    HFCT6_RPRD4_yc,                             //HFCT6通道PRPD点4
    HFCT6_RPRD5_yc,                             //HFCT6通道PRPD点5
    HFCT6_RPRD6_yc,                             //HFCT6通道PRPD点6
    HFCT6_RPRD7_yc,                             //HFCT6通道PRPD点7
    HFCT6_RPRD8_yc,                             //HFCT6通道PRPD点8
    HFCT6_RPRD9_yc,                             //HFCT6通道PRPD点9
    HFCT6_RPRD10_yc,                            //HFCT6通道PRPD点10
    HFCT6_RPRD11_yc,                            //HFCT6通道PRPD点11
    HFCT6_RPRD12_yc,                            //HFCT6通道PRPD点12
    HFCT6_RPRD13_yc,                            //HFCT6通道PRPD点13
    HFCT6_RPRD14_yc,                            //HFCT6通道PRPD点14
    HFCT6_RPRD15_yc,                            //HFCT6通道PRPD点15
    HFCT6_RPRD16_yc,                            //HFCT6通道PRPD点16
    HFCT6_RPRD17_yc,                            //HFCT6通道PRPD点17
    HFCT6_RPRD18_yc,                            //HFCT6通道PRPD点18
    HFCT6_RPRD19_yc,                            //HFCT6通道PRPD点19
    HFCT6_RPRD20_yc,                            //HFCT6通道PRPD点20
    HFCT6_RPRD21_yc,                            //HFCT6通道PRPD点21
    HFCT6_RPRD22_yc,                            //HFCT6通道PRPD点22
    HFCT6_RPRD23_yc,                            //HFCT6通道PRPD点23
    HFCT6_RPRD24_yc,                            //HFCT6通道PRPD点24
    HFCT6_RPRD25_yc,                            //HFCT6通道PRPD点25
    HFCT6_RPRD26_yc,                            //HFCT6通道PRPD点26
    HFCT6_RPRD27_yc,                            //HFCT6通道PRPD点27
    HFCT6_RPRD28_yc,                            //HFCT6通道PRPD点28
    HFCT6_RPRD29_yc,                            //HFCT6通道PRPD点29
    HFCT6_RPRD30_yc,                            //HFCT6通道PRPD点30
    HFCT6_RPRD31_yc,                            //HFCT6通道PRPD点31
    HFCT6_RPRD32_yc,                            //HFCT6通道PRPD点32
    HFCT6_RPRD33_yc,                            //HFCT6通道PRPD点33
    HFCT6_RPRD34_yc,                            //HFCT6通道PRPD点34
    HFCT6_RPRD35_yc,                            //HFCT6通道PRPD点35
    HFCT6_RPRD36_yc,                            //HFCT6通道PRPD点36
    HFCT6_RPRD37_yc,                            //HFCT6通道PRPD点37
    HFCT6_RPRD38_yc,                            //HFCT6通道PRPD点38
    HFCT6_RPRD39_yc,                            //HFCT6通道PRPD点39
    HFCT6_RPRD40_yc,                            //HFCT6通道PRPD点40

    UHF1_RPRD1_yc,                             //UHF1通道PRPD点1
    UHF1_RPRD2_yc,                             //UHF1通道PRPD点2
    UHF1_RPRD3_yc,                             //UHF1通道PRPD点3
    UHF1_RPRD4_yc,                             //UHF1通道PRPD点4
    UHF1_RPRD5_yc,                             //UHF1通道PRPD点5
    UHF1_RPRD6_yc,                             //UHF1通道PRPD点6
    UHF1_RPRD7_yc,                             //UHF1通道PRPD点7
    UHF1_RPRD8_yc,                             //UHF1通道PRPD点8
    UHF1_RPRD9_yc,                             //UHF1通道PRPD点9
    UHF1_RPRD10_yc,                            //UHF1通道PRPD点10
    UHF1_RPRD11_yc,                            //UHF1通道PRPD点11
    UHF1_RPRD12_yc,                            //UHF1通道PRPD点12
    UHF1_RPRD13_yc,                            //UHF1通道PRPD点13
    UHF1_RPRD14_yc,                            //UHF1通道PRPD点14
    UHF1_RPRD15_yc,                            //UHF1通道PRPD点15
    UHF1_RPRD16_yc,                            //UHF1通道PRPD点16
    UHF1_RPRD17_yc,                            //UHF1通道PRPD点17
    UHF1_RPRD18_yc,                            //UHF1通道PRPD点18
    UHF1_RPRD19_yc,                            //UHF1通道PRPD点19
    UHF1_RPRD20_yc,                            //UHF1通道PRPD点20
    UHF1_RPRD21_yc,                            //UHF1通道PRPD点21
    UHF1_RPRD22_yc,                            //UHF1通道PRPD点22
    UHF1_RPRD23_yc,                            //UHF1通道PRPD点23
    UHF1_RPRD24_yc,                            //UHF1通道PRPD点24
    UHF1_RPRD25_yc,                            //UHF1通道PRPD点25
    UHF1_RPRD26_yc,                            //UHF1通道PRPD点26
    UHF1_RPRD27_yc,                            //UHF1通道PRPD点27
    UHF1_RPRD28_yc,                            //UHF1通道PRPD点28
    UHF1_RPRD29_yc,                            //UHF1通道PRPD点29
    UHF1_RPRD30_yc,                            //UHF1通道PRPD点30
    UHF1_RPRD31_yc,                            //UHF1通道PRPD点31
    UHF1_RPRD32_yc,                            //UHF1通道PRPD点32
    UHF1_RPRD33_yc,                            //UHF1通道PRPD点33
    UHF1_RPRD34_yc,                            //UHF1通道PRPD点34
    UHF1_RPRD35_yc,                            //UHF1通道PRPD点35
    UHF1_RPRD36_yc,                            //UHF1通道PRPD点36
    UHF1_RPRD37_yc,                            //UHF1通道PRPD点37
    UHF1_RPRD38_yc,                            //UHF1通道PRPD点38
    UHF1_RPRD39_yc,                            //UHF1通道PRPD点39
    UHF1_RPRD40_yc,                            //UHF1通道PRPD点40

    UHF2_RPRD1_yc,                             //UHF2通道PRPD点1
    UHF2_RPRD2_yc,                             //UHF2通道PRPD点2
    UHF2_RPRD3_yc,                             //UHF2通道PRPD点3
    UHF2_RPRD4_yc,                             //UHF2通道PRPD点4
    UHF2_RPRD5_yc,                             //UHF2通道PRPD点5
    UHF2_RPRD6_yc,                             //UHF2通道PRPD点6
    UHF2_RPRD7_yc,                             //UHF2通道PRPD点7
    UHF2_RPRD8_yc,                             //UHF2通道PRPD点8
    UHF2_RPRD9_yc,                             //UHF2通道PRPD点9
    UHF2_RPRD10_yc,                            //UHF2通道PRPD点10
    UHF2_RPRD11_yc,                            //UHF2通道PRPD点11
    UHF2_RPRD12_yc,                            //UHF2通道PRPD点12
    UHF2_RPRD13_yc,                            //UHF2通道PRPD点13
    UHF2_RPRD14_yc,                            //UHF2通道PRPD点14
    UHF2_RPRD15_yc,                            //UHF2通道PRPD点15
    UHF2_RPRD16_yc,                            //UHF2通道PRPD点16
    UHF2_RPRD17_yc,                            //UHF2通道PRPD点17
    UHF2_RPRD18_yc,                            //UHF2通道PRPD点18
    UHF2_RPRD19_yc,                            //UHF2通道PRPD点19
    UHF2_RPRD20_yc,                            //UHF2通道PRPD点20
    UHF2_RPRD21_yc,                            //UHF2通道PRPD点21
    UHF2_RPRD22_yc,                            //UHF2通道PRPD点22
    UHF2_RPRD23_yc,                            //UHF2通道PRPD点23
    UHF2_RPRD24_yc,                            //UHF2通道PRPD点24
    UHF2_RPRD25_yc,                            //UHF2通道PRPD点25
    UHF2_RPRD26_yc,                            //UHF2通道PRPD点26
    UHF2_RPRD27_yc,                            //UHF2通道PRPD点27
    UHF2_RPRD28_yc,                            //UHF2通道PRPD点28
    UHF2_RPRD29_yc,                            //UHF2通道PRPD点29
    UHF2_RPRD30_yc,                            //UHF2通道PRPD点30
    UHF2_RPRD31_yc,                            //UHF2通道PRPD点31
    UHF2_RPRD32_yc,                            //UHF2通道PRPD点32
    UHF2_RPRD33_yc,                            //UHF2通道PRPD点33
    UHF2_RPRD34_yc,                            //UHF2通道PRPD点34
    UHF2_RPRD35_yc,                            //UHF2通道PRPD点35
    UHF2_RPRD36_yc,                            //UHF2通道PRPD点36
    UHF2_RPRD37_yc,                            //UHF2通道PRPD点37
    UHF2_RPRD38_yc,                            //UHF2通道PRPD点38
    UHF2_RPRD39_yc,                            //UHF2通道PRPD点39
    UHF2_RPRD40_yc,                            //UHF2通道PRPD点40

    AA1_RPRD1_yc,                             //AA1通道PRPD点1
    AA1_RPRD2_yc,                             //AA1通道PRPD点2
    AA1_RPRD3_yc,                             //AA1通道PRPD点3
    AA1_RPRD4_yc,                             //AA1通道PRPD点4
    AA1_RPRD5_yc,                             //AA1通道PRPD点5
    AA1_RPRD6_yc,                             //AA1通道PRPD点6
    AA1_RPRD7_yc,                             //AA1通道PRPD点7
    AA1_RPRD8_yc,                             //AA1通道PRPD点8
    AA1_RPRD9_yc,                             //AA1通道PRPD点9
    AA1_RPRD10_yc,                            //AA1通道PRPD点10
    AA1_RPRD11_yc,                            //AA1通道PRPD点11
    AA1_RPRD12_yc,                            //AA1通道PRPD点12
    AA1_RPRD13_yc,                            //AA1通道PRPD点13
    AA1_RPRD14_yc,                            //AA1通道PRPD点14
    AA1_RPRD15_yc,                            //AA1通道PRPD点15
    AA1_RPRD16_yc,                            //AA1通道PRPD点16
    AA1_RPRD17_yc,                            //AA1通道PRPD点17
    AA1_RPRD18_yc,                            //AA1通道PRPD点18
    AA1_RPRD19_yc,                            //AA1通道PRPD点19
    AA1_RPRD20_yc,                            //AA1通道PRPD点20
    AA1_RPRD21_yc,                            //AA1通道PRPD点21
    AA1_RPRD22_yc,                            //AA1通道PRPD点22
    AA1_RPRD23_yc,                            //AA1通道PRPD点23
    AA1_RPRD24_yc,                            //AA1通道PRPD点24
    AA1_RPRD25_yc,                            //AA1通道PRPD点25
    AA1_RPRD26_yc,                            //AA1通道PRPD点26
    AA1_RPRD27_yc,                            //AA1通道PRPD点27
    AA1_RPRD28_yc,                            //AA1通道PRPD点28
    AA1_RPRD29_yc,                            //AA1通道PRPD点29
    AA1_RPRD30_yc,                            //AA1通道PRPD点30
    AA1_RPRD31_yc,                            //AA1通道PRPD点31
    AA1_RPRD32_yc,                            //AA1通道PRPD点32
    AA1_RPRD33_yc,                            //AA1通道PRPD点33
    AA1_RPRD34_yc,                            //AA1通道PRPD点34
    AA1_RPRD35_yc,                            //AA1通道PRPD点35
    AA1_RPRD36_yc,                            //AA1通道PRPD点36
    AA1_RPRD37_yc,                            //AA1通道PRPD点37
    AA1_RPRD38_yc,                            //AA1通道PRPD点38
    AA1_RPRD39_yc,                            //AA1通道PRPD点39
    AA1_RPRD40_yc,                            //AA1通道PRPD点40

    AA2_RPRD1_yc,                             //AA2通道PRPD点1
    AA2_RPRD2_yc,                             //AA2通道PRPD点2
    AA2_RPRD3_yc,                             //AA2通道PRPD点3
    AA2_RPRD4_yc,                             //AA2通道PRPD点4
    AA2_RPRD5_yc,                             //AA2通道PRPD点5
    AA2_RPRD6_yc,                             //AA2通道PRPD点6
    AA2_RPRD7_yc,                             //AA2通道PRPD点7
    AA2_RPRD8_yc,                             //AA2通道PRPD点8
    AA2_RPRD9_yc,                             //AA2通道PRPD点9
    AA2_RPRD10_yc,                            //AA2通道PRPD点10
    AA2_RPRD11_yc,                            //AA2通道PRPD点11
    AA2_RPRD12_yc,                            //AA2通道PRPD点12
    AA2_RPRD13_yc,                            //AA2通道PRPD点13
    AA2_RPRD14_yc,                            //AA2通道PRPD点14
    AA2_RPRD15_yc,                            //AA2通道PRPD点15
    AA2_RPRD16_yc,                            //AA2通道PRPD点16
    AA2_RPRD17_yc,                            //AA2通道PRPD点17
    AA2_RPRD18_yc,                            //AA2通道PRPD点18
    AA2_RPRD19_yc,                            //AA2通道PRPD点19
    AA2_RPRD20_yc,                            //AA2通道PRPD点20
    AA2_RPRD21_yc,                            //AA2通道PRPD点21
    AA2_RPRD22_yc,                            //AA2通道PRPD点22
    AA2_RPRD23_yc,                            //AA2通道PRPD点23
    AA2_RPRD24_yc,                            //AA2通道PRPD点24
    AA2_RPRD25_yc,                            //AA2通道PRPD点25
    AA2_RPRD26_yc,                            //AA2通道PRPD点26
    AA2_RPRD27_yc,                            //AA2通道PRPD点27
    AA2_RPRD28_yc,                            //AA2通道PRPD点28
    AA2_RPRD29_yc,                            //AA2通道PRPD点29
    AA2_RPRD30_yc,                            //AA2通道PRPD点30
    AA2_RPRD31_yc,                            //AA2通道PRPD点31
    AA2_RPRD32_yc,                            //AA2通道PRPD点32
    AA2_RPRD33_yc,                            //AA2通道PRPD点33
    AA2_RPRD34_yc,                            //AA2通道PRPD点34
    AA2_RPRD35_yc,                            //AA2通道PRPD点35
    AA2_RPRD36_yc,                            //AA2通道PRPD点36
    AA2_RPRD37_yc,                            //AA2通道PRPD点37
    AA2_RPRD38_yc,                            //AA2通道PRPD点38
    AA2_RPRD39_yc,                            //AA2通道PRPD点39
    AA2_RPRD40_yc,                            //AA2通道PRPD点40

    AE1_RPRD1_yc,                             //AE1通道PRPD点1
    AE1_RPRD2_yc,                             //AE1通道PRPD点2
    AE1_RPRD3_yc,                             //AE1通道PRPD点3
    AE1_RPRD4_yc,                             //AE1通道PRPD点4
    AE1_RPRD5_yc,                             //AE1通道PRPD点5
    AE1_RPRD6_yc,                             //AE1通道PRPD点6
    AE1_RPRD7_yc,                             //AE1通道PRPD点7
    AE1_RPRD8_yc,                             //AE1通道PRPD点8
    AE1_RPRD9_yc,                             //AE1通道PRPD点9
    AE1_RPRD10_yc,                            //AE1通道PRPD点10
    AE1_RPRD11_yc,                            //AE1通道PRPD点11
    AE1_RPRD12_yc,                            //AE1通道PRPD点12
    AE1_RPRD13_yc,                            //AE1通道PRPD点13
    AE1_RPRD14_yc,                            //AE1通道PRPD点14
    AE1_RPRD15_yc,                            //AE1通道PRPD点15
    AE1_RPRD16_yc,                            //AE1通道PRPD点16
    AE1_RPRD17_yc,                            //AE1通道PRPD点17
    AE1_RPRD18_yc,                            //AE1通道PRPD点18
    AE1_RPRD19_yc,                            //AE1通道PRPD点19
    AE1_RPRD20_yc,                            //AE1通道PRPD点20
    AE1_RPRD21_yc,                            //AE1通道PRPD点21
    AE1_RPRD22_yc,                            //AE1通道PRPD点22
    AE1_RPRD23_yc,                            //AE1通道PRPD点23
    AE1_RPRD24_yc,                            //AE1通道PRPD点24
    AE1_RPRD25_yc,                            //AE1通道PRPD点25
    AE1_RPRD26_yc,                            //AE1通道PRPD点26
    AE1_RPRD27_yc,                            //AE1通道PRPD点27
    AE1_RPRD28_yc,                            //AE1通道PRPD点28
    AE1_RPRD29_yc,                            //AE1通道PRPD点29
    AE1_RPRD30_yc,                            //AE1通道PRPD点30
    AE1_RPRD31_yc,                            //AE1通道PRPD点31
    AE1_RPRD32_yc,                            //AE1通道PRPD点32
    AE1_RPRD33_yc,                            //AE1通道PRPD点33
    AE1_RPRD34_yc,                            //AE1通道PRPD点34
    AE1_RPRD35_yc,                            //AE1通道PRPD点35
    AE1_RPRD36_yc,                            //AE1通道PRPD点36
    AE1_RPRD37_yc,                            //AE1通道PRPD点37
    AE1_RPRD38_yc,                            //AE1通道PRPD点38
    AE1_RPRD39_yc,                            //AE1通道PRPD点39
    AE1_RPRD40_yc,                            //AE1通道PRPD点40

    AE2_RPRD1_yc,                             //AE2通道PRPD点1
    AE2_RPRD2_yc,                             //AE2通道PRPD点2
    AE2_RPRD3_yc,                             //AE2通道PRPD点3
    AE2_RPRD4_yc,                             //AE2通道PRPD点4
    AE2_RPRD5_yc,                             //AE2通道PRPD点5
    AE2_RPRD6_yc,                             //AE2通道PRPD点6
    AE2_RPRD7_yc,                             //AE2通道PRPD点7
    AE2_RPRD8_yc,                             //AE2通道PRPD点8
    AE2_RPRD9_yc,                             //AE2通道PRPD点9
    AE2_RPRD10_yc,                            //AE2通道PRPD点10
    AE2_RPRD11_yc,                            //AE2通道PRPD点11
    AE2_RPRD12_yc,                            //AE2通道PRPD点12
    AE2_RPRD13_yc,                            //AE2通道PRPD点13
    AE2_RPRD14_yc,                            //AE2通道PRPD点14
    AE2_RPRD15_yc,                            //AE2通道PRPD点15
    AE2_RPRD16_yc,                            //AE2通道PRPD点16
    AE2_RPRD17_yc,                            //AE2通道PRPD点17
    AE2_RPRD18_yc,                            //AE2通道PRPD点18
    AE2_RPRD19_yc,                            //AE2通道PRPD点19
    AE2_RPRD20_yc,                            //AE2通道PRPD点20
    AE2_RPRD21_yc,                            //AE2通道PRPD点21
    AE2_RPRD22_yc,                            //AE2通道PRPD点22
    AE2_RPRD23_yc,                            //AE2通道PRPD点23
    AE2_RPRD24_yc,                            //AE2通道PRPD点24
    AE2_RPRD25_yc,                            //AE2通道PRPD点25
    AE2_RPRD26_yc,                            //AE2通道PRPD点26
    AE2_RPRD27_yc,                            //AE2通道PRPD点27
    AE2_RPRD28_yc,                            //AE2通道PRPD点28
    AE2_RPRD29_yc,                            //AE2通道PRPD点29
    AE2_RPRD30_yc,                            //AE2通道PRPD点30
    AE2_RPRD31_yc,                            //AE2通道PRPD点31
    AE2_RPRD32_yc,                            //AE2通道PRPD点32
    AE2_RPRD33_yc,                            //AE2通道PRPD点33
    AE2_RPRD34_yc,                            //AE2通道PRPD点34
    AE2_RPRD35_yc,                            //AE2通道PRPD点35
    AE2_RPRD36_yc,                            //AE2通道PRPD点36
    AE2_RPRD37_yc,                            //AE2通道PRPD点37
    AE2_RPRD38_yc,                            //AE2通道PRPD点38
    AE2_RPRD39_yc,                            //AE2通道PRPD点39
    AE2_RPRD40_yc,                            //AE2通道PRPD点40

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

