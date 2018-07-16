#include "bpcable.h"
#include <QtDebug>

BpCable::BpCable(QObject *parent) : QObject(parent)
{
    cable_prpd_bp = new bp_neural_network_t;

    prpd_bp_param.in_dim = N_BUF;
    prpd_bp_param.out_dim = O_BUF;
    prpd_bp_param.hid_dim = C_BUF;
    prpd_bp_param.study_spd = STUDY_SPEED;
    prpd_bp_param.alpha = ALPHA;
    prpd_bp_param.ep = EP;
    prpd_bp_param.max_study_time = STUDY_TIMES;

    /* 初始化BP结构 */
    Bp::bp_init (cable_prpd_bp, &prpd_bp_param);
    /* 从权值文件载入w */
    Bp::bp_w_load (cable_prpd_bp, PD_FILE_DIR"/weight.txt");
}



int BpCable::cable_prpd_study_data(prpd_point_t points[], int data_num, discharge_type_t pd_mode)
{
    bp_neural_data_t prpd_bp_data;

    prpd_bp_set_x_buf (&prpd_bp_data, points, data_num);
    prpd_bp_set_o (&prpd_bp_data, pd_mode);

    /* 从PRPD训练神经网络 */
    return prpd_bp_study (cable_prpd_bp, &prpd_bp_data, 1);
}

int BpCable::cable_prpd_study_file(prpd_discharge_type_file_t prpd_file_mode[], int file_num)
{
    bp_neural_data_t * prpd_bp_data;
    int data_num, ret;

    if (file_num <= 0) {
        return -1;
    }

    prpd_bp_data = (bp_neural_data_t *)malloc (sizeof (bp_neural_data_t) * file_num);
    if (prpd_bp_data == NULL) {
        return -1;
    }

    /* 从文件读取PRPD数据并提取特征 */
    data_num = prpd_bp_load_data_file (cable_prpd_bp, prpd_bp_data, prpd_file_mode, file_num);

    /* 从PRPD训练神经网络 */
    ret = prpd_bp_study (cable_prpd_bp, prpd_bp_data, data_num);

    free (prpd_bp_data);

    return ret;
}

int BpCable::cable_prpd_mode(prpd_point_t prpd_points[], int data_num, discharge_type_t *pd_mode_output)
{
    bp_neural_data_t pd_data;

    prpd_bp_set_x_buf (&pd_data, prpd_points, data_num);
    Bp::bp_judge (cable_prpd_bp, &pd_data);
    * pd_mode_output = prpd_bp_get_o (&pd_data);

    return 0;
}

QString BpCable::cable_prpd_mode(QVector<QwtPoint3D> list)
{
    prpd_point_t *prpd_points = new prpd_point_t[ list.count() ];
    for (int i = 0; i < list.count(); ++i) {
        prpd_points[i].phase = list.at(i).x();
        prpd_points[i].q = list.at(i).y();
        prpd_points[i].n = list.at(i).z();
    }
    discharge_type_t pd_out;
    cable_prpd_mode(prpd_points, list.count(), &pd_out);
    qDebug()<<mode_2_string(pd_out);
//    return mode_2_string(pd_out);
    return mode_2_tr_string(pd_out);
}

int BpCable::cable_prpd_mode_file(char *prpd_file, discharge_type_t *pd_mode_output)
{
    bp_neural_data_t pd_data;
    FILE * fp;

    fp = fopen (prpd_file, "r");
    if (fp == NULL) {
        return -1;
    }

    prpd_bp_set_x_file (&pd_data, fp, 0);
    fclose (fp);
    Bp::bp_judge (cable_prpd_bp, &pd_data);
    * pd_mode_output = prpd_bp_get_o (&pd_data);

    return 0;
}

QString BpCable::cable_prpd_mode_file(char *prpd_file)
{
    discharge_type_t pd_out;
    cable_prpd_mode_file (prpd_file, &pd_out);
    qDebug()<<mode_2_string(pd_out);
    return mode_2_string(pd_out);
}

void BpCable::test()
{
    qDebug()<<"begin bp test=============";
    cable_prpd_mode_file("/root/bp/t50.txt");
}

int BpCable::cable_prpd_weight_load(char *weight_file)
{
    return prpd_bp_w_load (cable_prpd_bp, weight_file);
}

int BpCable::cable_prpd_weight_save(char *weight_file)
{
    return prpd_bp_w_save (cable_prpd_bp, weight_file);
}


/***********************************************************
 * private函数
 * *********************************************************/
void BpCable::prpd_bp_calc_discharge_factor(bp_neural_data_t *pdata, prpd_feature_calc_var_t *pfv, prpd_point_t points[], int num)
{
    int i, Nneg, Npos;

    pfv->qs_neg = pfv->qs_pos = 0.0;
    Nneg = Npos = 0;
    for (i = 0 ; i < num ; i++) {
        if (points [i].phase >= 180) {
            pfv->qs_neg = pfv->qs_neg + abs (points [i].q * points [i].n);
            Nneg = Nneg + points [i].n;
        }
        else {
            pfv->qs_pos = pfv->qs_pos + abs (points [i].q * points [i].n);
            Npos = Npos + points [i].n;
        }
    }
    pfv->qf_neg = (pfv->qs_neg / Nneg);
    pfv->qf_pos = (pfv->qs_pos / Npos);
    pfv->qf = pfv->qf_neg / pfv->qf_pos;
}

void BpCable::prpd_bp_calc_sk_ku(bp_neural_data_t *pdata, prpd_feature_calc_var_t *pfv, prpd_point_t points[], int num)
{
    int i, w;
    float p_i [Q_WIN_NUM];	// 落在当前相位窗概率
    long sum_y_i_pos = 0;	// 正半周总放电量
    long sum_y_i_neg = 0;	// 负半周总放电量
    float u_pos = 0;		// 均值
    float d_pos = 0;		// 标准差
    float u_neg = 0;		// 均值
    float d_neg = 0;		// 标准差
    float sk_pos = 0;		// 偏斜度
    float ku_pos = 0;		// 陡峭度
    float sk_neg = 0;		// 偏斜度
    float ku_neg = 0;		// 陡峭度
    int temp_q;

    memset (pfv->y_i, 0, sizeof (pfv->y_i));

    for (i = 0; i < num; i++) {
        w = points [i].phase / Q_WIN_WIDTH;
        temp_q = abs (points [i].q) * points [i].n;
        pfv->y_i [w] += temp_q;
        if (w < (Q_WIN_NUM / 2)) {
            sum_y_i_pos = sum_y_i_pos + temp_q;
        }
        else {
            sum_y_i_neg = sum_y_i_neg + temp_q;
        }
    }

    for (i = 0; i < (Q_WIN_NUM / 2); i++) {
        p_i [i] = pfv->y_i [i] / (float)sum_y_i_pos;
        u_pos += (i * 3 + 1.5) * p_i [i];
    }

    for (i = 0; i < (Q_WIN_NUM / 2); i++) {
        d_pos += p_i [i] * (((i * 3 + 1.5) - u_pos) * ((i * 3 + 1.5) - u_pos));
    }
    d_pos = sqrt (d_pos);

    for (i = 0; i < (Q_WIN_NUM / 2); i++) {
        sk_pos += pow ((i * 3 + 1.5 - u_pos), 3) * p_i [i];
        ku_pos += pow ((i * 3 + 1.5 - u_pos), 4) * p_i [i];
    }
    sk_pos = sk_pos / pow (d_pos, 3);
    ku_pos = ku_pos / pow (d_pos, 4) - 3;

    for (i = (Q_WIN_NUM / 2); i < Q_WIN_NUM; i++) {
        p_i [i] = pfv->y_i [i] / (float)sum_y_i_neg;
        u_neg += (i * 3 + 1.5) * p_i [i];
    }

    for (i = (Q_WIN_NUM / 2); i < Q_WIN_NUM; i++) {
        d_neg += p_i [i] * (((i * 3 + 1.5) - u_neg) * ((i * 3 + 1.5) - u_neg));
    }
    d_neg = sqrt (d_neg);

    for (i = (Q_WIN_NUM / 2); i < Q_WIN_NUM; i++) {
        sk_neg += pow ((i * 3 + 1.5 - u_neg), 3) * p_i [i];
        ku_neg += pow ((i * 3 + 1.5 - u_neg), 4) * p_i [i];
    }
    sk_neg = sk_neg / pow (d_neg, 3);
    ku_neg = ku_neg / pow (d_neg, 4) - 3;

    pdata->x.vect [prpd_ku_pos] = ku_pos;
    pdata->x.vect [prpd_ku_neg] = ku_neg;
}

void BpCable::prpd_bp_calc_cc(bp_neural_data_t *pdata, prpd_feature_calc_var_t *pfv, prpd_point_t points[], int num)
{
    int i;
    float cc = 0	;	//互相关系数
    float sumxy = 0;
    float sumx = 0;
    float sumy = 0;
    float sumpowx = 0;
    float sumpowy = 0;

    for (i = 0 ; i < (Q_WIN_NUM / 2); i++) {
        sumxy += pfv->y_i [i] * pfv->y_i [i + (Q_WIN_NUM / 2)];
        sumx += pfv->y_i [i];
        sumy += pfv->y_i [i + (Q_WIN_NUM / 2)];
        sumpowx += pfv->y_i [i] * pfv->y_i [i];
        sumpowy += pfv->y_i [i + (Q_WIN_NUM / 2)] * pfv->y_i [i + (Q_WIN_NUM / 2)];
    }

    cc = (sumxy - sumx * sumy / (Q_WIN_NUM / 2)) /
            (sqrt((sumpowx - sumx * sumx / (Q_WIN_NUM / 2)) * (sumpowy - sumy * sumy / (Q_WIN_NUM / 2))));

    pdata->x.vect [prpd_cc] = cc;
}

void BpCable::prpd_bp_calc_other(bp_neural_data_t *pdata, prpd_feature_calc_var_t *pfv, prpd_point_t points[], int num)
{
    int i;
    float xpos = 0;		//正半周期x质心
    float ypos = 0;		//正半周期y质心
    float xneg = 0;		//负半周期x质心
    float yneg = 0;		//负半周期y质心
    float Mxpos = 0;		//正半周横坐标一阶矩
    float Mxneg = 0;		//
    float M00pos = 0;		//零阶矩
    float M00neg = 0;
    float u20pos = 0;		//2阶中心矩
    float u20neg = 0;
    float u02pos = 0;
    float u02neg = 0;
    float u11pos = 0;
    float u11neg = 0;
    float u30pos = 0;		//3阶中心矩
    float u30neg = 0;
    float u03pos = 0;
    float u03neg = 0;
    float u21pos = 0;
    float u21neg = 0;
    float u12pos = 0;
    float u12neg = 0;
    //float A2pos = 0;
    //float A2neg = 0;

    for (i = 0 ; i < num ; i++) {
        if (points [i].phase >= 180) {
            Mxneg = Mxneg + abs (points [i].phase * points [i].n);
            M00neg = M00neg + points [i].n;
        }
        else {
            Mxpos = Mxpos + abs (points [i].phase *points [i].n);
            M00pos = M00pos + points [i].n;
        }
    }
    xpos = Mxpos / M00pos;
    xneg = Mxneg / M00neg;
    ypos = pfv->qs_pos / M00pos;
    yneg = pfv->qs_neg / M00neg;

    for (i = 0 ; i < num; i++) {
        if (points [i].phase >= 180) {
            u20neg = u20neg + ((points [i].phase - xneg) / 180) * ((points [i].phase - xneg) / 180) * points [i].n;
            u02neg = u02neg + (abs (points [i].q) - yneg) * ((abs (points [i].q) - yneg) / 3600) * points [i].n;
            u11neg = u11neg + ((points [i].phase - xneg) / 30) * ((abs (points [i].q) - yneg)/60) * points [i].n;
            u21neg = u21neg + ((points [i].phase - xneg) / 5) * ((points [i].phase - xneg) / 180) * ((abs (points [i].q) - yneg) / 60)* points [i].n;
            u12neg = u12neg + (abs (points [i].q) - yneg) * ((abs (points [i].q) - yneg) / 3600) * ((points [i].phase - xneg) / 6) * points [i].n;
            u30neg = u30neg + pow ((points [i].phase - xneg),3) * (points [i].n / (180 * 180 * 3));
            u03neg = u03neg + pow ((abs (points [i].q) - yneg),3) * (points [i].n / (60 * 60 * 6));
        }
        else {
            u20pos = u20pos + ((points [i].phase - xpos) / 180) * ((points [i].phase - xpos) / 180) * points [i].n;
            u02pos = u02pos + (abs (points [i].q) - ypos) * ((abs (points [i].q) - ypos) / 3600) * points [i].n;
            u11pos = u11pos + ((points [i].phase - xpos) / 30) * ((abs (points [i].q) - ypos) / 60) * points [i].n;
            u21pos = u21pos + ((points [i].phase - xpos) / 5) * ((points [i].phase - xpos) / 180) * ((abs (points [i].q) - ypos) / 60) * points [i].n;
            u12pos = u12pos + (abs (points [i].q) - ypos) * ((abs (points [i].q) - ypos) / 3600) * ((points [i].phase - xpos) / 6) * points [i].n;
            u30pos = u30pos + pow ((points [i].phase - xpos),3) * (points [i].n / (180 *180 * 3));
            u03pos = u03pos + pow ((abs (points [i].q) - ypos),3) * (points [i].n / (60 * 60 * 6));
        }
    }
    //A2pos = u20pos / u02pos;
    //A2neg = u20neg / u02neg;

    pdata->x.vect [prpd_xpos] = xpos / 360;
    pdata->x.vect [prpd_ypos] = ypos / 60;
    pdata->x.vect [prpd_xneg] = xneg / 360;
    pdata->x.vect [prpd_yneg] = yneg / 60;
    pdata->x.vect [prpd_u20pos] = u20pos;
    pdata->x.vect [prpd_u20neg] = u20neg;
    pdata->x.vect [prpd_u02pos] = u02pos;
    pdata->x.vect [prpd_u02neg] = u02neg;
}

void BpCable::prpd_bp_calc_feature_phase_related(bp_neural_data_t *pdata, prpd_point_t points[], int num)
{
    prpd_feature_calc_var_t fvar;

    prpd_bp_calc_discharge_factor (pdata, &fvar, points, num);

    prpd_bp_calc_sk_ku (pdata, &fvar, points, num);

    prpd_bp_calc_cc (pdata, &fvar, points, num);

    prpd_bp_calc_other (pdata, &fvar, points, num);
}

int BpCable::prpd_bp_init_points_file(FILE *fp, prpd_point_t points[], int max_num)
{
    int i = 0;

    if (max_num <= 0) {
        max_num = MAX_PRPD_NUM_PER_FILE;
    }

    if (fscanf (fp, "%d", &points [i].phase) <= 0) {
        return 0;
    }

    do {
        fscanf (fp, "%d", &points [i].q);
        fscanf (fp, "%d", &points [i].n);
        i++;
    }
    while ((fscanf (fp, "%d", &points [i].phase) > 0) && (i < max_num));

    return i;
}

void BpCable::prpd_bp_set_x_file(bp_neural_data_t *pdata, FILE *fp, int data_num)
{
    int N;
    prpd_point_t prpd_points [MAX_PRPD_NUM_PER_FILE];

    N = prpd_bp_init_points_file (fp, prpd_points, data_num);

    prpd_bp_calc_feature_phase_related (pdata, prpd_points, N);

#if 0
    int i;
    printf ("features: ");
    for (i = 0; i < prpd_feature_num; i++) {
        printf ("%f, ", pdata->x.vect [i]);
    }
    printf ("\n");
#endif
}

void BpCable::prpd_bp_set_x_buf(bp_neural_data_t *pdata, prpd_point_t prpd_points[], int data_num)
{
    prpd_bp_calc_feature_phase_related (pdata, prpd_points, data_num);
}

void BpCable::prpd_bp_set_o(bp_neural_data_t *pdata, discharge_type_t dt)
{
    memset (pdata->o.vect, 0, sizeof (pdata->o.vect));
    pdata->o.vect [dt] = 1;
}

discharge_type_t BpCable::prpd_bp_get_o(bp_neural_data_t *pdata)
{
    int i;

    for (i = discharge_internal; i < discharge_type_num; i++) {
        if (pdata->o.vect [i] != 0) {
            break;
        }
    }

    return (discharge_type_t)i;
}

int BpCable::prpd_bp_load_data_file(bp_neural_network_t *bpnn, bp_neural_data_t pdata[], prpd_discharge_type_file_t ptf[], int f_num)
{
    int file_index, data_index;
    FILE * fp;

    data_index = 0;
    for (file_index = 0; file_index < f_num; file_index++) {
        fp = fopen (ptf [file_index].prpd_file, "r");
        if (fp != NULL) {
            prpd_bp_set_x_file (&pdata [data_index], fp, 0);
            fclose (fp);
            prpd_bp_set_o (&pdata [data_index], ptf [file_index].dsc_type);
            data_index++;
        }
        else {
            printf ("fail to open %s\n", ptf [file_index].prpd_file);
        }
    }

    return data_index;
}

int BpCable::prpd_bp_study(bp_neural_network_t *bpnn, bp_neural_data_t pdata[], int data_num)
{
    return Bp::bp_study (bpnn, pdata, data_num);
}

int BpCable::prpd_bp_w_save(bp_neural_network_t *bpnn, char *prpd_w_file)
{
    return Bp::bp_w_save (bpnn, prpd_w_file);
}

int BpCable::prpd_bp_w_load(bp_neural_network_t *bpnn, char *prpd_w_file)
{
    return Bp::bp_w_load (bpnn, prpd_w_file);
}

void BpCable::prpd_bp_show(bp_neural_network_t *bpnn, int level)
{
    return Bp::bp_show (bpnn, level);
}

void BpCable::cable_prpd_result_show(discharge_type_t pd_out, char *prompt)
{
    switch ((int)pd_out) {
    case discharge_internal:
        printf ("%s: discharge_internal\n", prompt);
        break;
    case discharge_surface:
        printf ("%s: discharge_surface\n", prompt);
        break;
    case discharge_corona:
        printf ("%s: discharge_corona\n", prompt);
        break;
    case discharge_floating_potential:
        printf ("%s: discharge_floating_potential\n", prompt);
        break;
    default:
        printf ("%s: Can't recognizied discharge\n", prompt);
        break;
    }
}

QString BpCable::mode_2_string(discharge_type_t pd_out)
{
    switch ((int)pd_out) {
    case discharge_internal:
        return "discharge_internal";
    case discharge_surface:
        return "discharge_surface";
    case discharge_corona:
        return "discharge_corona";
    case discharge_floating_potential:
        return "discharge_floating_potential";
    default:
        return "Can't recognizied discharge";
    }
}

QString BpCable::mode_2_tr_string(discharge_type_t pd_out)
{
    switch ((int)pd_out) {
    case discharge_internal:
        return tr("内部放电");
    case discharge_surface:
        return tr("表面放电");
    case discharge_corona:
        return tr("电晕放电");
    case discharge_floating_potential:
        return tr("悬浮电位");
    default:
        return tr("白噪声");
    }
}
