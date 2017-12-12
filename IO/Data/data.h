#ifndef DATA_H
#define DATA_H

#include <QtGlobal>
#include <QVector>
#include "zynq.h"

//定义一些全局数据类型，和一些全局宏
#define PROGRAM_DIR     "/root"
#define WAVE_DIR        PROGRAM_DIR "/WaveForm"
#define FAVORITE_DIR    PROGRAM_DIR "/WaveForm/favorite"
#define DATALOG_DIR     PROGRAM_DIR "/DataLog"
#define PRPDLOG_DIR     PROGRAM_DIR "/PRPDLog"

#define SDCARD_DIR          "/mmc/sdcard"
#define WAVE_DIR_SD         SDCARD_DIR "/WaveForm"
#define FAVORITE_DIR_SD     SDCARD_DIR "/WaveForm/favorite"
#define DATALOG_DIR_SD      SDCARD_DIR "/DataLog"
#define PRPDLOG_DIR_SD      SDCARD_DIR "/PRPDLog"

#define AD_VAL(val, offset)		((val) >= offset ? (val) - offset : offset - (val))
#define MAX(a, b)				((a) > (b) ? (a) : (b))
#define MIN(a, b)				((a) < (b) ? (a) : (b))
#define BUFF_DATA_SIZE			1030

#define AMP_FACTOR_J27_680P 22560.0f
#define AMP_FACTOR_J27_1N   12200.0f
#define RESOLUTION_AD_LOW   (5.0/262144.0)
#define AA_FACTOR   ( RESOLUTION_AD_LOW * 1000000 / AMP_FACTOR_J27_680P )

#define TEV_FACTOR  (2.0*1000/65536)

#define VERSION_MAJOR   1       //软件版本号
#define VERSION_MINOR   4

//屏幕分辨率
#define RESOLUTION_X		480
#define RESOLUTION_Y		272

//通道分辨率
#define CHANNEL_X   458
#define CHANNEL_Y   192

#pragma pack(1)     //以1字节为单位，设置内存对齐
typedef union HDATA {
    quint32 ad_data;
    struct {
        quint16 ad_min;
        quint16 ad_max;
    } ad;
} HDATA;

/* fifo recieve data */
//普通数据
//适用通道：TEV1，TEV2，AA
//更新时间：0.1-1s
typedef struct G_RECV_PARA_NOMAL {
	HDATA hdata0;           //TEV0
	HDATA hdata1;           //TEV1
    quint32 hpulse0_totol;  //TEV0脉冲数
    quint32 hpulse1_totol;  //TEV1脉冲数
	quint32 ldata0_max;     //未使用
	quint32 ldata0_min;     //未使用
	quint32 ldata1_max;     //AA超声最大值
	quint32 ldata1_min;     //AA超声最小值
} G_RECV_PARA_NOMAL;

//PRPD数据
//适用通道：TEV1，TEV2
//更新时间：0.2s
struct G_RECV_PARA_PRPD {
//    quint32 zero;           //第一个为0，无意义
    quint32 groupNum;       //组号(0-3)
    quint32 totol;
    quint32 data [BUFF_DATA_SIZE];   //数据
};

//HFCT数据
//适用通道：HFCT1，HFCT2
//更新时间：立刻更新
struct G_RECV_PARA_HFCT {
    quint32 empty;          //第一个为0有数据，1无数据
    quint32 time;           //时标（0-2M对应0-360°）
    quint32 data [BUFF_DATA_SIZE];   //数据,250
};

//录波数据
//适用通道：TEV1，TEV2，HFCT1，HFCT2，AA
//更新时间：立刻更新
struct G_RECV_PARA_REC {
    quint32 recComplete;    //录播完成(1为完成)
    quint32 groupNum;       //组号(0-15)
    quint32 data [BUFF_DATA_SIZE];   //录播数据256
};


/* fifo send data */
//G_SEND_PARA.data_changed参数引入是为了保证有需要写入FPGA的数据时，才调用写操作
//RPARA.flag参数是为了精确控制写某一项参数
typedef struct RPARA {
    bool flag;
    unsigned int rval;
} RPARA;

typedef struct G_SEND_PARA {
	RPARA send_params [sp_num];		
    bool  data_changed;			// some pararmeters changed
} G_SEND_PARA;

/* total data */
typedef struct G_PARA {
    G_RECV_PARA_NOMAL recv_para_normal;
    G_RECV_PARA_REC recv_para_rec;
    G_RECV_PARA_PRPD recv_para_prpd1;
    G_RECV_PARA_PRPD recv_para_prpd2;
    G_RECV_PARA_HFCT recv_para_hfct1;
    G_RECV_PARA_HFCT recv_para_hfct2;
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
    Disable,
    TEV1,
    TEV2,
    HFCT1,
    HFCT2,
    UHF1,
    UHF2,
    AA_Ultrasonic,
    AE_Ultrasonic,
    Double_Channel,
    TEV1_CONTINUOUS,        //连续录波
    TEV2_CONTINUOUS,        //连续录波
    HFCT1_CONTINUOUS,        //连续录波
    HFCT2_CONTINUOUS,        //连续录波
    Options_Mode,
};


typedef QVector<qint32> VectorList;


#endif // DATA_H




