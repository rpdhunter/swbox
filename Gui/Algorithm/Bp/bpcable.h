#ifndef BPCABLE_H
#define BPCABLE_H

#include <QObject>
#include "bp.h"
#include "qwt_point_3d.h"

#define MAX_PRPD_NUM_PER_FILE	200
#define MAX_FILE_NAME_SIZE		64
#define PD_FILE_DIR				"/root/bp"

typedef enum discharge_type_e {
    discharge_internal = 0,			// 内部放电
    discharge_surface,				// 表明(沿面)放电
    discharge_corona,				// 电晕放电
    discharge_floating_potential,	// 悬浮电位放电

    discharge_type_num
} discharge_type_t;

enum prpd_features_e {
    prpd_ku_pos = 0,				// 陡峭度
    prpd_ku_neg,					// 陡峭度
    prpd_cc,						// 互相关系数
    prpd_xpos,						// 正半周期x质心
    prpd_ypos,						// 正半周期y质心
    prpd_xneg,						// 负半周期x质心
    prpd_yneg,						// 负半周期y质心
    prpd_u20pos,					// 2阶中心矩
    prpd_u20neg,					// 2阶中心矩
    prpd_u02pos,					// 2阶中心矩
    prpd_u02neg,					// 2阶中心矩

    prpd_feature_num
};

#define N_BUF 				(prpd_feature_num)
#define O_BUF 				(discharge_type_num)
#define C_BUF 				(10)

#define STUDY_SPEED			0.4 		// 学习速率
#define STUDY_TIMES			1000000		// 最大学习次数
#define ALPHA    			0.2 		// 冲动系数
#define EP       			0.01 		// 误差阈值

#define TEST_SETS   		8
#define EACH_LINE_OFFSET	40

#define Q_WIN_NUM			120					// 放电窗数量
#define Q_WIN_WIDTH			(360 / Q_WIN_NUM)	// 放电窗宽度

typedef struct prpd_point_s {
    int phase;
    int q;
    int n;
} prpd_point_t;

typedef struct prpd_discharge_type_point_s {
    prpd_point_t point;
    discharge_type_t dsc_type;
} prpd_discharge_type_point_t;

typedef struct prpd_discharge_type_file_s {
    char prpd_file [MAX_FILE_NAME_SIZE];
    discharge_type_t dsc_type;
} prpd_discharge_type_file_t;

typedef struct prpd_feature_calc_var_s {
    float qs_pos;									// 正向总放电量
    float qs_neg;									// 反向总放电量
    float qf;
    float qf_pos;
    float qf_neg;
    int y_i [Q_WIN_NUM];							// 该相位窗放电量
} prpd_feature_calc_var_t;

class BpCable : public QObject
{
    Q_OBJECT
public:
    explicit BpCable(QObject *parent = nullptr);

    /**********************************************************************************
     * 放电模式学习:从数据
     * ********************************************************************************/
    int cable_prpd_study_data (prpd_point_t points [], int data_num, discharge_type_t pd_mode);

    /**********************************************************************************
     * 放电模式学习:从文件
     * ********************************************************************************/
    int cable_prpd_study_file (prpd_discharge_type_file_t prpd_file_mode [], int file_num);

    /**********************************************************************************
     * 放电模式判断:数据
     * ********************************************************************************/
    int cable_prpd_mode (prpd_point_t prpd_points [], int data_num, discharge_type_t * pd_mode_output);
    QString cable_prpd_mode(QVector<QwtPoint3D> list);

    /**********************************************************************************
     * 放电模式判断:文件
     * ********************************************************************************/
    int cable_prpd_mode_file (char * prpd_file, discharge_type_t * pd_mode_output);
    QString cable_prpd_mode_file(char * prpd_file);
    void test();        //测试

    /**********************************************************************************
     * 放电模式学习:读取原先学习结果函数
     * 返回值0：读取失败（文件不存在），返回值1：读取成功
     * ********************************************************************************/
    int cable_prpd_weight_load (char * weight_file);

    /**********************************************************************************
     * 放电模式学习:保存学习结果函数
     * 返回值0：保存文件失败，返回值1：保存成功
     * ********************************************************************************/
    int cable_prpd_weight_save (char * weight_file);

signals:

public slots:

private:
    bp_neural_param_t prpd_bp_param;
    bp_neural_network_t *cable_prpd_bp;

    void prpd_bp_calc_discharge_factor (bp_neural_data_t * pdata, prpd_feature_calc_var_t * pfv, prpd_point_t points [], int num);

    void prpd_bp_calc_sk_ku (bp_neural_data_t * pdata, prpd_feature_calc_var_t * pfv, prpd_point_t points [], int num);

    void prpd_bp_calc_cc (bp_neural_data_t * pdata, prpd_feature_calc_var_t * pfv, prpd_point_t points [], int num);

    void prpd_bp_calc_other (bp_neural_data_t * pdata, prpd_feature_calc_var_t * pfv, prpd_point_t points [], int num);

    // 计算PRPD特征
    void prpd_bp_calc_feature_phase_related (bp_neural_data_t * pdata, prpd_point_t points [], int num);

    // 从文件输入phase, q, n
    int prpd_bp_init_points_file (FILE * fp, prpd_point_t points [], int max_num);

    void prpd_bp_set_x_file (bp_neural_data_t * pdata, FILE * fp, int data_num);

    void prpd_bp_set_x_buf (bp_neural_data_t * pdata, prpd_point_t prpd_points [], int data_num);

    void prpd_bp_set_o (bp_neural_data_t * pdata, discharge_type_t dt);

    discharge_type_t prpd_bp_get_o (bp_neural_data_t * pdata);

    int prpd_bp_load_data_file (bp_neural_network_t * bpnn, bp_neural_data_t pdata [], prpd_discharge_type_file_t ptf [], int f_num);

    int prpd_bp_study (bp_neural_network_t * bpnn, bp_neural_data_t pdata [], int data_num);

    int prpd_bp_w_save (bp_neural_network_t * bpnn, char * prpd_w_file);

    int prpd_bp_w_load (bp_neural_network_t * bpnn, char * prpd_w_file);

    void prpd_bp_show (bp_neural_network_t * bpnn, int level);

    void cable_prpd_result_show (discharge_type_t pd_out, char * prompt);

    QString mode_2_string(discharge_type_t pd_out);
    QString mode_2_tr_string(discharge_type_t pd_out);
};

#endif // BPCABLE_H
