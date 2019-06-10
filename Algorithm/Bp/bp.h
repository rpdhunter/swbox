
#ifndef _BP_H_
#define _BP_H_

#define MAX_LAYER   		4 			// 网络层数
#define MAX_NODE   			(30 + 1) 	// 层中最大的节点数

enum trans_fx_e {
	trans_fx_sigmoid_unipolar = 0,
	trans_fx_sigmoid_bipolar,

	trans_fx_num
};

class c_bp_neural_param {
public:
	int 	in_dim;									// 输入维数
	int 	out_dim;								// 输出维数
	int 	hid_dim;								// 隐层维数
	float 	study_spd;								// 学习速率
	float 	alpha;									// 冲动系数
	float 	ep;										// 误差阈值
	int 	max_study_time;							// 学习迭代次数
	float	out_compensation;						// 输出补偿系数
};

class c_bp_neural_input {
public:
	float 	vect [MAX_NODE];
};

class c_bp_neural_output {
public:
	int 	vect [MAX_NODE];
};

class c_bp_neural_data {
public:
	c_bp_neural_input x;							// 输入向量
	c_bp_neural_output o;							// 输出向量
};

class c_bp_neural_network {
public:
	c_bp_neural_network (c_bp_neural_param * param);
	~c_bp_neural_network ();

	int bp_study (c_bp_neural_data bp_data [], int x_num);
	void bp_judge (c_bp_neural_data * pdata);
	int bp_w_save (const char * wfile);
	int bp_w_load (const char * wfile);
	void bp_show (int level);
	int bp_init (c_bp_neural_param * param);

private:
	float randnumber ();
	float sigmoid_unipolar (float x);
	float sigmoid_bipolar (float x);
	float bp_output (int layer, int node);
	float bp_gf (int layer, int node);
	void bp_inout_init (c_bp_neural_data * bp_data);
	void bp_feed_forward ();
	void bp_calc_gradient ();
	void bp_update_weights ();
	float bp_error ();
	void bp_get_result (c_bp_neural_data * pdata);
	
private:
	int 	nodes_per_layer [MAX_LAYER];			// 各层的节点数
	float 	w [MAX_LAYER][MAX_NODE][MAX_NODE];		// 权值
	float 	o [MAX_LAYER][MAX_NODE];				// 某一层某一节点的输出
	int 	d [MAX_NODE];							// 期望输出向量
	float 	error;									// 误差
	//float error_1;								// 误差
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
	float	o_compensation;							// 输出补偿系数
};

#endif /* _BP_H_ */

