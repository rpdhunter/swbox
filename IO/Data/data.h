#ifndef DATA_H
#define DATA_H

#include <QtGlobal>
//#include <QMetaType>
#include <QVector>

//定义一些全局数据类型，和一些全局宏

#define AD_VAL(val, offset)		((val) >= offset ? (val) - offset : offset - (val))
#define MAX(a, b)				((a) > (b) ? (a) : (b))
#define REC_DATA_SIZE			259

enum send_params {
	sp_freq = 0,				//频率（FREQ_REG）
	sp_backlight,				//背光（BACKLIGHT_REG）
	sp_recstart,				//0为常态,1为开始录播,2为开始上传（REC_START）
	sp_groupNum,				//传输录波数据的组号（GROUP_NUM）
	sp_aa_vol,					//AA超声音量调节（AA_VOL）
	sp_read_fpga,				//读数据标志位（READ_FPGA）
	sp_tev_auto_rec,			//TEV自动录波（TEV_AUTO_REC）
	sp_aa_record_play,			//播放声音标志(AA_RECORD_PLAY)
	sp_tev1_zero,				//TEV参考零点(TEV1_ZERO)
	sp_tev1_threshold,			//TEV脉冲阈值(TEV1_THRESHOLD)
	sp_tev2_zero,				//TEV参考零点(TEV2_ZERO)
	sp_tev2_threshold,			//TEV脉冲阈值(TEV2_THRESHOLD)
	
	sp_num
};

#pragma pack(1)     //以1字节为单位，设置内存对齐
typedef union HDATA {
    quint32 ad_data;
    struct {
        quint16 ad_min;
        quint16 ad_max;
    } ad;
} HDATA;

typedef union HPULSE_EDGE {
    quint32 hpulse_edge;
    struct {
        quint16 pos;
        quint16 neg;
    } edge;
} HPULSE_EDGE;

/* fifo recieve data */
typedef struct G_RECV_PARA {
	HDATA hdata0;           //TEV0
	HDATA hdata1;           //TEV1
	quint32 ldata0_max;     //未使用
	quint32 ldata0_min;     //未使用
	quint32 ldata1_max;     //AA超声最大值
	quint32 ldata1_min;     //AA超声最小值
	HPULSE_EDGE pulse0;     //TEV0上升下降沿
	HPULSE_EDGE pulse1;     //TEV1上升下降沿
	quint32 hpulse0_totol;  //TEV0脉冲数
	quint32 hpulse1_totol;  //TEV1脉冲数

	quint32 recComplete;    //录播完成(1为完成)
	quint32 groupNum;       //组号(0-7)
	quint32 recData [REC_DATA_SIZE];   //录播数据
} G_RECV_PARA;

typedef struct RPARA {
    bool flag;
    unsigned int rval;
} RPARA;

/* fifo send data */
typedef struct G_SEND_PARA {
	RPARA send_params [sp_num];		
	bool  data_changed;			// some parameters changed
} G_SEND_PARA;

/* total data */
typedef struct G_PARA {
    G_RECV_PARA recv_para;
    G_SEND_PARA send_para;

public:
	void set_send_para (enum send_params p_idx, unsigned int val)
	{
		if (p_idx >= sp_num) {
			return;
		}

		send_para.send_params [p_idx].rval = val;
		if (!send_para.send_params [p_idx].flag) {
			send_para.send_params [p_idx].flag = true;
		}
		if (!send_para.data_changed) {
			send_para.data_changed = true;
		}
	}

} G_PARA;
#pragma pack()

enum MODE{
    TEV1,                    //0
    TEV2,
    AA_Ultrasonic,          //1
    AA_Ultrasonic_End,      //AA超声结束
    AE_Ultrasonic,
    Disable
};


typedef QVector<qint32> VectorList;


#endif // DATA_H
