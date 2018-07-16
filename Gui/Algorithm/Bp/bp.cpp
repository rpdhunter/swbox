#include "bp.h"

Bp::Bp(QObject *parent) : QObject(parent)
{
}

int Bp::bp_init(bp_neural_network_t *bpnn, bp_neural_param_t *param)
{
    int layer, i, j;

    memset (bpnn, 0, sizeof (* bpnn));

    bpnn->in_dim = param->in_dim + 1;		// 输入维数 + 1
    bpnn->out_dim = param->out_dim + 1;		// 输出维数 + 1
    bpnn->hid_dim = param->hid_dim + 1;		// 隐藏层维数 + 1
    bpnn->study_spd = param->study_spd;
    bpnn->alpha = param->alpha;
    bpnn->ep = param->ep;
    bpnn->max_study_time = param->max_study_time;

    bpnn->nodes_per_layer [0] = bpnn->in_dim;
    bpnn->nodes_per_layer [MAX_LAYER - 1] = bpnn->out_dim;
    for (layer = 1; layer < MAX_LAYER - 1; layer++) {
        bpnn->nodes_per_layer [layer] = bpnn->hid_dim;
    }

    // 设置随机数种子
    srand ((unsigned int)time (NULL));
    // 对权值初始化为-0.1~0.1之间的随机数
    for (layer = 0; layer < MAX_LAYER; layer++) {
        for (i = 0; i < MAX_NODE; i++) {
            for (j = 0; j < MAX_NODE; j++) {
                bpnn->w [layer][i][j] = randnumber ();
            }
        }
    }

    /* 输入层和隐层的第0个元素(x0, y00, y10...)都设成-1 */
    for (layer = 0; layer < MAX_LAYER; layer++) {
        bpnn->o [layer][0] = -1.0;
    }

    return 0;
}

int Bp::bp_study(bp_neural_network_t *bpnn, bp_neural_data_t bp_data[], int x_num)
{
    int p = 0, complete = 0;

    if (x_num <= 0) {
        return -1;
    }

    bpnn->bp_study_time = 0;

    do {
        bp_inout_init (bpnn, &bp_data [p]);
        bp_feed_forward (bpnn);
        bp_calc_gradient (bpnn);
        bp_update_weights (bpnn);
        bp_error (bpnn);

        if (++p >= x_num) {
            p = 0;
            complete = 1;
        }
        //printf ("i %d, error %lf\n", i, bpnn->error);
    } while ((!complete) || ((bpnn->error > bpnn->ep) && (++bpnn->bp_study_time < bpnn->max_study_time)));

    if (bpnn->bp_study_time >= bpnn->max_study_time) {
        return -1;
    }
    else {
        return 0;
    }
}

void Bp::bp_judge(bp_neural_network_t *bpnn, bp_neural_data_t *pdata)
{
    bp_inout_init (bpnn, pdata);
    bp_feed_forward (bpnn);
    bp_get_result (bpnn, pdata);
}

int Bp::bp_w_save(bp_neural_network_t *bpnn, char *wfile)
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
                fprintf (wfp, "%f\n", bpnn->w [layer][i][j]);
            }
        }
    }

    fclose (wfp);

    return 0;
}

int Bp::bp_w_load(bp_neural_network_t *bpnn, char *wfile)
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
                ret = fscanf (wfp, "%f", &bpnn->w [layer][i][j]);
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

void Bp::bp_show(bp_neural_network_t *bpnn, int level)
{
    int i, j, layer;

    printf ("BP Params:\n");

    for (i = 0; i < MAX_LAYER; i++) {
        printf ("  layer %d node num: %d\n", i, bpnn->nodes_per_layer [i]);
    }
    printf ("  study_spd       : %f\n", bpnn->study_spd);
    printf ("  alpha           : %f\n", bpnn->alpha);
    printf ("  ep              : %f\n", bpnn->ep);
    printf ("  last e          : %f\n", bpnn->error);
    printf ("  max study times : %d\n", bpnn->max_study_time);
    printf ("  act study times : %d\n", bpnn->bp_study_time);

    if (level >= 1) {
        for (layer = 1; layer < MAX_LAYER; layer++) {
            printf ("layer %d\n", layer);
            for (i = 0; i < bpnn->nodes_per_layer [layer - 1]; i++) {
                for (j = 0; j < bpnn->nodes_per_layer [layer]; j++) {
                    printf ("w [%d][%d] = %f\n", i, j, bpnn->w [layer][i][j]);
                }
            }
        }
    }
}

float Bp::randnumber()
{
    return (sin (rand ()) / 10);
}

float Bp::sigmoid_unipolar(float x)
{
    return (1.0 / (1 + exp (-x)));
}

float Bp::sigmoid_bipolar(float x)
{
    return (1 - exp (-x)) / (1 + exp (-x));
}

float Bp::bp_output(bp_neural_network_t *bpnn, int layer, int node)
{
    float sum = 0.0;
    int node2;

    for (node2 = 0; node2 < bpnn->nodes_per_layer [layer - 1]; node2++) {
        sum = sum + bpnn->w [layer][node][node2] * bpnn->o [layer - 1][node2];
    }

//    return (trans_fx [trans_fx_sigmoid_unipolar]) (sum);
    return sigmoid_unipolar(sum);
}

float Bp::bp_gf(bp_neural_network_t *bpnn, int layer, int node)
{
    float sum = 0.0;
    int m;

    for (m = 1; m < bpnn->nodes_per_layer [layer + 1]; m++) {
        sum = sum + bpnn->e [layer + 1][m] * bpnn->w [layer + 1][m][node] * bpnn->o [layer + 1][m] * (1 - bpnn->o [layer + 1][m]);
    }

    return sum;
}

void Bp::bp_inout_init(bp_neural_network_t *bpnn, bp_neural_data_t *bp_data)
{
    int i;

    /* 设置输入x1, x2... */
    for (i = 0; i < bpnn->in_dim; i++) {
        bpnn->o [0][i + 1] = bp_data->x.vect [i];
    }
    /* 设置期望输出d1, d2... */
    for (i = 0; i < bpnn->out_dim; i++) {
        bpnn->d [i + 1] = bp_data->o.vect [i];
    }
}

void Bp::bp_feed_forward(bp_neural_network_t *bpnn)
{
    int layer, node;

    for (layer = 1; layer < MAX_LAYER; layer++) {
        for (node = 1; node < bpnn->nodes_per_layer [layer]; node++) {
            bpnn->o [layer][node] = bp_output (bpnn, layer, node);
        }
    }
}

void Bp::bp_calc_gradient(bp_neural_network_t *bpnn)
{
    int layer, node, j;

    for (layer = (MAX_LAYER - 1); layer > 0; layer--) {
        for (node = 1; node < bpnn->nodes_per_layer [layer]; node++) {
            if (layer == (MAX_LAYER - 1)) {
                /* 输出层 */
                bpnn->e [layer][node] = bpnn->o [layer][node] - bpnn->d [node];
            }
            else {
                /* 隐层 */
                bpnn->e [layer][node] = bp_gf (bpnn, layer, node);
            }

            for (j = 0; j < bpnn->nodes_per_layer [layer - 1]; j++) {
                bpnn->g [layer][node][j] = bpnn->e [layer][node] * bpnn->o [layer][node] * (1.0 - bpnn->o [layer][node]) * bpnn->o [layer - 1][j];
            }
        }
    }
}

void Bp::bp_update_weights(bp_neural_network_t *bpnn)
{
    int layer, i, j;
    float w_k_1;	// w(k-1)
    float w_k;		// w(k)

    for (layer = 1; layer < MAX_LAYER; layer++) {
        for (i = 0; i < bpnn->nodes_per_layer [layer]; i++) {
            for (j = 0; j < bpnn->nodes_per_layer [layer - 1]; j++) {
                w_k_1 = bpnn->w [layer][i][j];
                bpnn->w [layer][i][j] = bpnn->w [layer][i][j] - bpnn->study_spd * bpnn->g [layer][i][j] + bpnn->alpha * bpnn->delta_w [layer][i][j];
                w_k = bpnn->w [layer][i][j];
                bpnn->delta_w [layer][i][j] = w_k - w_k_1;
            }
        }
    }
}

float Bp::bp_error(bp_neural_network_t *bpnn)
{
    float sum = 0.0;
    int i;

    for (i = 1; i < bpnn->out_dim; i++) {
        sum += pow (fabs (bpnn->o [MAX_LAYER - 1][i] - bpnn->d [i]), 2);
    }

    bpnn->error = sqrt (sum);

    return sum;
}

void Bp::bp_get_result(bp_neural_network_t *bpnn, bp_neural_data_t *pdata)
{
    int i;

    for (i = 0; i < bpnn->out_dim - 1; i++) {
        pdata->o.vect [i] = (int)floor (bpnn->o [MAX_LAYER - 1][i + 1] + 0.5);
        printf ("o[%d][%d]=%f\n", MAX_LAYER - 1, i, bpnn->o [MAX_LAYER - 1][i + 1]);
    }
}
