
#ifndef _PRPD_BP_H_
#define _PRPD_BP_H_

#include "bp.h"
#include "pd.h"
#include "prpd.h"

#define MAX_PRPD_NUM_PER_FILE	800

#define PHASE_MOV_DEGREE		12
#define PHASE_MOV_MAX_DEG		360

#define N_BUF 					(prpd_feature_num)
#define O_BUF 					(discharge_type_num)
#define C_BUF 					(10)

#define STUDY_SPEED				0.4 		// 学习速率
#define STUDY_TIMES				1000000		// 最大学习次数
#define ALPHA    				0.2 		// 冲动系数
#define EP       				0.002 		// 误差阈值

class prpd_bp_neural_network : public c_bp_neural_network {
public:
	prpd_bp_neural_network (c_bp_neural_param * param) : c_bp_neural_network (param) {}
	virtual ~prpd_bp_neural_network () {}

public:
	virtual void prpd_bp_calc_feature_phase_related (c_bp_neural_data * pdata, c_prpd_point points [], int num) = 0;
	
private:
	int prpd_bp_init_points_file (FILE * fp, c_prpd_point points [], int max_num);
	int prpd_bp_load_data_file (c_bp_neural_data pdata [], c_prpd_discharge_type_file ptf [], int f_num);
	void prpd_bp_translation_phase (c_prpd_point prpd_points [], int data_num, int move_phase);
	void prpd_bp_set_x_file (c_bp_neural_data * pdata, FILE * fp);
	void prpd_bp_set_x_buf (c_bp_neural_data * pdata, c_prpd_point prpd_points [], int data_num);
	void prpd_bp_set_o (c_bp_neural_data * pdata, discharge_type_t dt);
	discharge_type_t prpd_bp_get_o (c_bp_neural_data * pdata);
	
public:
	int prpd_mode_init (const char * weight_file);
	int prpd_study_data (c_prpd_point points [], int data_num, discharge_type_t pd_mode);
	int prpd_study_file (c_prpd_discharge_type_file prpd_file_mode [], int file_num);
	void prpd_result_show (discharge_type_t pd_out, char * prompt);
	int init_pd_study_files (c_prpd_discharge_type_file * study_files, int * study_files_num, int max_files_num, const char * dir_name);
	void show_pd_study_files (c_prpd_discharge_type_file * study_files, int study_files_num);
	int prpd_mode (c_prpd_point prpd_points [], int data_num, discharge_type_t * pd_mode_output);
	int prpd_mode_file (char * prpd_file, discharge_type_t * pd_mode_output);
};

#endif /* _PRPD_BP_H_ */

