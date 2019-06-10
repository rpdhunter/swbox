#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "bp.h"
#include "pd.h"
#include "prpd.h"
#include "cable_prpd_bp.h"

c_bp_neural_param cable_prpd_bp_param = {
	N_BUF, O_BUF, C_BUF, 0.05, 0.02, 0.001, 1000000, HFCT_COMPENSATION
};

void cable_bp_neural_network::prpd_bp_calc_discharge_factor (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num)
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
	if (pfv->qf_pos > pfv->qf_neg) {
		pfv->qf = pfv->qf_pos / pfv->qf_neg;
	}
	else {
		pfv->qf = pfv->qf_neg / pfv->qf_pos;
	}
	pdata->x.vect [prpd_qf] = pfv->qf;
}

void cable_bp_neural_network::prpd_bp_calc_sk_ku (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num)
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

void cable_bp_neural_network::prpd_bp_calc_cc (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num)
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

	//printf ("cc: %f\n", cc);
}

void cable_bp_neural_network::prpd_bp_calc_other (c_bp_neural_data * pdata, c_prpd_feature_calc_var * pfv, c_prpd_point points [], int num)
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
	//pdata->x.vect [prpd_u02pos] = u02pos;
	//pdata->x.vect [prpd_u02neg] = u02neg;
}

// 计算PRPD特征
void cable_bp_neural_network::prpd_bp_calc_feature_phase_related (c_bp_neural_data * pdata, c_prpd_point points [], int num)
{
	c_prpd_feature_calc_var fvar;

	prpd_bp_data_clean (points, num);

	prpd_bp_data_normalization (points, num);

	prpd_bp_calc_discharge_factor (pdata, &fvar, points, num);

	prpd_bp_calc_sk_ku (pdata, &fvar, points, num);

	prpd_bp_calc_cc (pdata, &fvar, points, num);

	prpd_bp_calc_other (pdata, &fvar, points, num);

#if 0
	static int count = 0;
	printf ("x[%d]:%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", count++,
		pdata->x.vect [0], pdata->x.vect [1], 
		pdata->x.vect [2], pdata->x.vect [3], 
		pdata->x.vect [4], pdata->x.vect [5], 
		pdata->x.vect [6], pdata->x.vect [7], 
		pdata->x.vect [8], pdata->x.vect [9], pdata->x.vect [10]);
#endif
}

void cable_bp_neural_network::prpd_bp_data_clean (c_prpd_point prpd_points [], int data_num)
{
	int i;

	for (i = 0; i < data_num; i++) {
		/* 检测相位有效 */
		if (prpd_points [i].phase < 0 || prpd_points [i].phase >= 360) {
			//printf ("data clean: points %d, phase %d\n", i, prpd_points [i].phase);
			prpd_points [i].phase = abs (prpd_points [i].phase % 360);
		}

		/* 检测q值有效性 */
		if (prpd_points [i].q < 0) {
			//printf ("data clean: points %d, q %d\n", i, prpd_points [i].q);
			prpd_points [i].q = abs (prpd_points [i].q);
		}
		if (prpd_points [i].q > MAX_Q) {
			//printf ("data clean: points %d, q %d\n", i, prpd_points [i].q);
			prpd_points [i].q = MAX_Q;
		}

		/* 检测n有效性 */
		if (prpd_points [i].n <= 0) {
			//printf ("data clean: points %d, n %d\n", i, prpd_points [i].n);
			prpd_points [i].n = 1;
		}
		if (prpd_points [i].n > MAX_N) {
			//printf ("data clean: points %d, n %d\n", i, prpd_points [i].n);
			prpd_points [i].n = MAX_N;
		}
	}
}

void cable_bp_neural_network::prpd_bp_data_normalization (c_prpd_point prpd_points [], int data_num)
{
	int i;
	int min, max;
	float delta;

	min = max = prpd_points [0].q;
	
	for (i = 0; i < data_num; i++) {
		if (prpd_points [i].q < min) {
			min = prpd_points [i].q;
		}
		if (prpd_points [i].q > max) {
			max = prpd_points [i].q;
		}
	}

	delta = max - min;
	if (delta <= 0) {
		delta = max;
	}

	/* 归一化 */
	for (i = 0; i < data_num; i++) {
		prpd_points [i].q = ((prpd_points [i].q - min) / delta) * MAX_NORMALIZATION;
	}
}

