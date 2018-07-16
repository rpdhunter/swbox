#ifndef BP_H
#define BP_H

#include <QObject>

#define MAX_LAYER   		4 			// 网络层数
#define MAX_NODE   			(30 + 1) 	// 层中最大的节点数

typedef struct bp_neural_network_s {
    int 	nodes_per_layer [MAX_LAYER];			// 各层的节点数
    float 	w [MAX_LAYER][MAX_NODE][MAX_NODE];		// 权值
    float 	o [MAX_LAYER][MAX_NODE];				// 某一层某一节点的输出
    int 	d [MAX_NODE];							// 期望输出向量
    float 	error;									// 误差
    int		bp_study_time;							// 当前学习次数

    float 	e [MAX_LAYER][MAX_NODE];				// 误差函数对o的导数
    float 	g [MAX_LAYER][MAX_NODE][MAX_NODE];		// 误差函数对w的导数，即梯度
    float 	delta_w [MAX_LAYER][MAX_NODE][MAX_NODE];// 动量因子

    int 	in_dim;									// 输入维数
    int 	out_dim;								// 输出维数
    int 	hid_dim;								// 隐层维数
    float 	study_spd;								// 学习速率
    float 	alpha;									// 动量系数
    float 	ep;										// 误差阈值
    int 	max_study_time;							// 学习迭代次数
} bp_neural_network_t;

typedef struct bp_neural_input_s {
    float 	vect [MAX_NODE];
} bp_neural_input_t;

typedef struct bp_neural_output_s {
    int 	vect [MAX_NODE];
} bp_neural_output_t;

typedef struct bp_neural_data_s {
    bp_neural_input_t x;							// 输入向量
    bp_neural_output_t o;							// 输出向量
} bp_neural_data_t;

typedef struct bp_neural_param_s {
    int 	in_dim;									// 输入维数
    int 	out_dim;								// 输出维数
    int 	hid_dim;								// 隐层维数
    float 	study_spd;								// 学习速率
    float 	alpha;									// 冲动系数
    float 	ep;										// 误差阈值
    int 	max_study_time;							// 学习迭代次数
} bp_neural_param_t;

class Bp : public QObject
{
    Q_OBJECT
public:
    explicit Bp(QObject *parent = nullptr);

    static int bp_init (bp_neural_network_t * bpnn, bp_neural_param_t * param);
    static int bp_study (bp_neural_network_t * bpnn, bp_neural_data_t bp_data [], int x_num);
    static void bp_judge (bp_neural_network_t * bpnn, bp_neural_data_t * pdata);
    static int bp_w_save (bp_neural_network_t * bpnn, char * wfile);
    static int bp_w_load (bp_neural_network_t * bpnn, char * wfile);
    static void bp_show (bp_neural_network_t * bpnn, int level);

signals:

public slots:

private:
    // -0.1~0.1的随机函数
    static float randnumber ();

    static float sigmoid_unipolar (float x);

    static float sigmoid_bipolar (float x);

    // 求某个节点的输出值
    static float bp_output (bp_neural_network_t * bpnn, int layer, int node);

    // 求误差函数对o的导数
    static float bp_gf (bp_neural_network_t * bpnn, int layer, int node);

    static void bp_inout_init (bp_neural_network_t * bpnn, bp_neural_data_t * bp_data);

    // 前馈输出值
    static void bp_feed_forward (bp_neural_network_t * bpnn);

    // 求梯度
    static void bp_calc_gradient (bp_neural_network_t * bpnn);

    // 修改权值
    static void bp_update_weights (bp_neural_network_t * bpnn);

    // 误差
    static float bp_error (bp_neural_network_t * bpnn);

    static void bp_get_result (bp_neural_network_t * bpnn, bp_neural_data_t * pdata);
};

#endif // BP_H
