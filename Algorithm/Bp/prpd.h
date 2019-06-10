
#ifndef _PRPD_H_
#define _PRPD_H_

#include "pd.h"

#define MAX_FILE_NAME_SIZE		64

enum prpd_features_e {
	prpd_qf = 0,					// 正负半波系数
	prpd_ku_pos,					// 陡峭度
	prpd_ku_neg,					// 陡峭度
	prpd_cc,						// 互相关系数
	prpd_xpos,						// 正半周期x质心
	prpd_ypos,						// 正半周期y质心
	prpd_xneg,						// 负半周期x质心
	prpd_yneg,						// 负半周期y质心
	prpd_u20pos,					// 2阶中心矩
	prpd_u20neg,					// 2阶中心矩
	//prpd_u02pos,					// 2阶中心矩
	//prpd_u02neg,					// 2阶中心矩
	
	prpd_feature_num
};

class c_prpd_point {
public:
	int phase;
	int q;
	int n;
};

class c_prpd_discharge_type_point {
public:
	c_prpd_point point;
	discharge_type_t dsc_type;
};

class c_prpd_discharge_type_file {
public:
	char prpd_file [MAX_FILE_NAME_SIZE];
	discharge_type_t dsc_type;
};

#endif
