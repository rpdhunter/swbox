#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "bp.h"

static c_bp_neural_param loc_bp_param = {
	MAX_NODE, MAX_NODE, MAX_LAYER - 2, 0.05, 0.02, 0.001, 1000000, 0.5
};

// -0.1~0.1的随机函数
float c_bp_neural_network::randnumber ()
{
	return (sin (rand ()) / 10);
}

float c_bp_neural_network::sigmoid_unipolar (float x)
{
	return (1.0 / (1 + exp (-x)));
}

float c_bp_neural_network::sigmoid_bipolar (float x)
{
	return (1 - exp (-x)) / (1 + exp (-x));
}

// 求某个节点的输出值
float c_bp_neural_network::bp_output (int layer, int node)
{
	float sum = 0.0;
	int node2;
	
	for (node2 = 0; node2 < nodes_per_layer [layer - 1]; node2++) {
		sum = sum + w [layer][node][node2] * o [layer - 1][node2];
		//printf ("w[%d][%d][%d]=%f, o[%d][%d]=%f\n", 
		//	layer, node, node2, w [layer][node][node2],
		//	layer - 1, node2, o [layer - 1][node2]);
	}
	//printf ("sum %f\n", sum);

	return sigmoid_unipolar (sum);
}

// 求误差函数对o的导数
float c_bp_neural_network::bp_gf (int layer, int node)
{
	float sum = 0.0;
	int m;
	
	for (m = 1; m < nodes_per_layer [layer + 1]; m++) {
		sum = sum + e [layer + 1][m] * w [layer + 1][m][node] * o [layer + 1][m] * (1 - o [layer + 1][m]);
	}

	return sum;
}

void c_bp_neural_network::bp_inout_init (c_bp_neural_data * bp_data)
{
	int i;

	/* 设置输入x1, x2... */
	//printf ("o[0]:");
	for (i = 0; i < in_dim; i++) {
		o [0][i + 1] = bp_data->x.vect [i];
	//	printf ("%f,", o[0][i + 1]);
	}
	//printf ("\n");
	/* 设置期望输出d1, d2... */
	for (i = 0; i < out_dim; i++) {
		d [i + 1] = bp_data->o.vect [i];
	}
}

// 前馈输出值
void c_bp_neural_network::bp_feed_forward ()
{
	int layer, node;
	
	for (layer = 1; layer < MAX_LAYER; layer++) {
		for (node = 1; node < nodes_per_layer [layer]; node++) {
			o [layer][node] = bp_output (layer, node);
			//printf ("o[%d][%d]=%f\n", layer, node, o [layer][node]);
		}
	}
}

// 求梯度
void c_bp_neural_network::bp_calc_gradient ()
{
	int layer, node, j;
	
	for (layer = (MAX_LAYER - 1); layer > 0; layer--) {
		for (node = 1; node < nodes_per_layer [layer]; node++) {
			if (layer == (MAX_LAYER - 1)) {
				/* 输出层 */
				e [layer][node] = o [layer][node] - d [node];
			}
			else {
				/* 隐层 */
				e [layer][node] = bp_gf (layer, node);
			}

			for (j = 0; j < nodes_per_layer [layer - 1]; j++) {
				g [layer][node][j] = e [layer][node] * o [layer][node] * (1.0 - o [layer][node]) * o [layer - 1][j];
			}
		}
	}
}

// 修改权值
void c_bp_neural_network::bp_update_weights ()
{
	int layer, i, j;
	float w_k_1;	// w(k-1)
	float w_k;		// w(k)

	for (layer = 1; layer < MAX_LAYER; layer++) {
		for (i = 0; i < nodes_per_layer [layer]; i++) {
			for (j = 0; j < nodes_per_layer [layer - 1]; j++) {
				w_k_1 = w [layer][i][j];
				w [layer][i][j] = w [layer][i][j] - study_spd * g [layer][i][j] + alpha * delta_w [layer][i][j];
				w_k = w [layer][i][j];
				delta_w [layer][i][j] = w_k - w_k_1;
			}
		}
	}
}

// 误差
float c_bp_neural_network::bp_error ()
{
	float sum = 0.0;
	int i;

	for (i = 1; i < out_dim; i++) {
		sum += pow (fabs (o [MAX_LAYER - 1][i] - d [i]), 2);
	}

	error = sqrt (sum);

	return sum;
}

void c_bp_neural_network::bp_get_result (c_bp_neural_data * pdata)
{
	int i;

	for (i = 0; i < out_dim - 1; i++) {
		pdata->o.vect [i] = (int)floor (o [MAX_LAYER - 1][i + 1] + o_compensation);
        //printf ("o[%d][%d]=%f\n", MAX_LAYER - 1, i, o [MAX_LAYER - 1][i + 1]);
	}
}

int c_bp_neural_network::bp_init (c_bp_neural_param * param)
{
	int layer, i, j;

	in_dim = param->in_dim + 1;		// 输入维数 + 1
	out_dim = param->out_dim + 1;		// 输出维数 + 1
	hid_dim = param->hid_dim + 1;		// 隐藏层维数 + 1
	study_spd = param->study_spd;
	alpha = param->alpha;
	ep = param->ep;
	max_study_time = param->max_study_time;
	o_compensation = param->out_compensation;
	
	nodes_per_layer [0] = in_dim;
	nodes_per_layer [MAX_LAYER - 1] = out_dim;
	for (layer = 1; layer < MAX_LAYER - 1; layer++) {
		nodes_per_layer [layer] = hid_dim;
	}

	// 设置随机数种子
	srand ((unsigned int)time (NULL));
	// 对权值初始化为-0.1~0.1之间的随机数
	for (layer = 0; layer < MAX_LAYER; layer++) {
		for (i = 0; i < MAX_NODE; i++) {
			for (j = 0; j < MAX_NODE; j++) {
				w [layer][i][j] = randnumber ();
			}
		}
	}

	/* 输入层和隐层的第0个元素(x0, y00, y10...)都设成-1 */
	for (layer = 0; layer < MAX_LAYER; layer++) {
		o [layer][0] = -1.0;
	}

	return 0;
}

c_bp_neural_network::c_bp_neural_network (c_bp_neural_param * param = NULL)
{
	if (param != NULL) {
		bp_init (param);
	}
	else {
		bp_init (&loc_bp_param);
	}
}

c_bp_neural_network::~c_bp_neural_network ()
{
}

int c_bp_neural_network::bp_w_load (const char * wfile)
{
	int layer, i, j, ret;
	FILE * wfp;

	wfp = fopen (wfile, "r");
	if (wfp == NULL) {
		printf ("fail to open %s for read\n", wfile);
		return -1;
	}

	for (layer = 0; layer < MAX_LAYER; layer++) {
		for (i = 0; i < MAX_NODE; i++) {
			for (j = 0; j < MAX_NODE; j++) {
				ret = fscanf (wfp, "%f", &w [layer][i][j]);
				if (ret < 1) {
					break;
				}
			}
		}
	}

	fclose (wfp);

	if (ret < 1) {
		return -1;
	}
	else {
		return 0;
	}
}

int c_bp_neural_network::bp_w_save (const char * wfile)
{
	int layer, i, j;
	FILE * wfp;

	wfp = fopen (wfile, "w");
	if (wfp == NULL) {
		printf ("fail to open %s for write\n", wfile);
		return -1;
	}

	for (layer = 0; layer < MAX_LAYER; layer++) {
		for (i = 0; i < MAX_NODE; i++) {
			for (j = 0; j < MAX_NODE; j++) {
				fprintf (wfp, "%f\n", w [layer][i][j]);
			}
		}
	}

	fclose (wfp);

	return 0;
}

int c_bp_neural_network::bp_study (c_bp_neural_data bp_data [], int x_num)
{
	int p = 0, complete = 0;
	float min_err = 1.0;

	if (x_num <= 0) {
		return -1;
	}

	bp_study_time = 0;

	do {
		bp_inout_init (&bp_data [p]);
		bp_feed_forward ();
		bp_calc_gradient ();
		bp_update_weights ();
		bp_error ();

		if (++p >= x_num) {
			p = 0;
			complete = 1;
		}
		bp_study_time++;
		if (error < min_err) {
			min_err = error;
		}
		if (!(bp_study_time & 0x1ff)) {
			printf ("bp_study_time %d, error %f, min_err %f\n", bp_study_time, error, min_err);
		}
	} while ((!complete) || ((error > ep) && (bp_study_time < max_study_time)));

	if (bp_study_time >= max_study_time) {
		return -1;
	}
	else {
		return 0;
	}
}

void c_bp_neural_network::bp_judge (c_bp_neural_data * pdata)
{
	bp_inout_init (pdata);
	bp_feed_forward ();
	bp_get_result (pdata);
}

void c_bp_neural_network::bp_show (int level)
{
	int i, j, layer;

	printf ("BP Params:\n");
	
	for (i = 0; i < MAX_LAYER; i++) {
		printf ("  layer %d node num: %d\n", i, nodes_per_layer [i]);
	}
	printf ("  study_spd       : %f\n", study_spd);
	printf ("  alpha           : %f\n", alpha);
	printf ("  ep              : %f\n", ep);
	printf ("  last e          : %f\n", error);
	printf ("  max study times : %d\n", max_study_time);
	printf ("  act study times : %d\n", bp_study_time);

	if (level >= 1) {
		for (layer = 1; layer < MAX_LAYER; layer++) {
			printf ("layer %d\n", layer);
			for (i = 0; i < nodes_per_layer [layer - 1]; i++) {
				for (j = 0; j < nodes_per_layer [layer]; j++) {
					printf ("w [%d][%d] = %f\n", i, j, w [layer][i][j]);
				}
			}
		}
	}
}
