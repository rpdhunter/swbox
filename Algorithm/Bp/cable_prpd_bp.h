
#ifndef _BP_RBF_H_
#define _BP_RBF_H_

#include "prpd_bp.h"

#define MAX_Q					3000
#define MAX_N					20
#define MAX_NORMALIZATION		400

#define HFCT_COMPENSATION		0.2			// 补偿系数

#define STUDY_SPEED			0.4 		// 学习速率
#define STUDY_TIMES			1000000		// 最大学习次数
#define ALPHA    			0.2 		// 冲动系数
#define EP       			0.002 		// 误差阈值

#define Q_WIN_NUM			120					// 放电窗数量
#define Q_WIN_WIDTH			(360 / Q_WIN_NUM)	// 放电窗宽度

class c_prpd_feature_calc_var {
public:
	float qs_pos;									// 正向总放电量
	float qs_neg;									// 反向总放电量
	float qf;
	float qf_pos;
	float qf_neg;
	int y_i [Q_WIN_NUM];							// 该相位窗放电量
};

class cable_bp_neural_network : public prpd_bp_neural_network {
public:
	cable_bp_neural_network (c_bp_neural_param * param): prpd_bp_neural_network (param) {}
	~cable_bp_neural_network () {}

public:
	void prpd_bp_calc_feature_phase_related (c_bp_neural_data * pdata, c_prpd_point points [], int num);
	
private:
	void prpd_bp_calc_discharge_factor (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num);
	void prpd_bp_calc_sk_ku (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num);
	void prpd_bp_calc_cc (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num);
	void prpd_bp_calc_other (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num);

	void prpd_bp_data_clean (c_prpd_point prpd_points [], int data_num);
	void prpd_bp_data_normalization (c_prpd_point prpd_points [], int data_num);
};

extern c_bp_neural_param cable_prpd_bp_param;

#endif /* _BP_RBF_H_ */

