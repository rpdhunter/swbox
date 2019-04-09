#ifndef DATA_H
#define DATA_H

#include <QtGlobal>
#include <QVector>
#include "zynq.h"

//定义一些全局数据类型，和一些全局宏
#define DIR_PROGRAM         "/root"
#define DIR_USB             "/mmc/mmc2"
#define DIR_DATA            DIR_USB"/data"

#define DIR_WAVE            DIR_DATA"/WaveForm"
#define DIR_FAVORITE        DIR_WAVE"/favorite"
#define DIR_DATALOG         DIR_DATA"/DataLog"
#define DIR_PRPDLOG         DIR_DATA"/PRPDLog"
#define DIR_ASSET           DIR_DATA"/asset"
#define DIR_TEST            DIR_DATA"/Test"
#define DIR_ASSET_NORMAL    DIR_ASSET"/Normal"

#define DIR_SCREENSHOTS     DIR_DATA"/ScreenShots"
#define DIR_CAMERASHOTS     DIR_DATA"/CameraShots"

#define SDCARD_DIR          "/mmc/sdcard"
#define WAVE_DIR_SD         SDCARD_DIR "/WaveForm"
#define FAVORITE_DIR_SD     SDCARD_DIR "/WaveForm/favorite"
#define DATALOG_DIR_SD      SDCARD_DIR "/DataLog"
#define PRPDLOG_DIR_SD      SDCARD_DIR "/PRPDLog"

#define AD_VAL(val, offset)		((val) >= offset ? (val) - offset : offset - (val))
#define MAX(a, b)				((a) > (b) ? (a) : (b))
#define MIN(a, b)				((a) < (b) ? (a) : (b))
#define BUFF_DATA_SIZE			520

#define AMP_FACTOR_J27_680P     22560.0f
#define AMP_FACTOR_J27_1N       12200.0f
#define AMP_FACTOR_J27_470P     8131.0f
#define RESOLUTION_AD_LOW       (10.0/262144.0)
#define AA_FACTOR       ( RESOLUTION_AD_LOW * 1000000 / AMP_FACTOR_J27_470P )
#define AE_FACTOR_30K   ( RESOLUTION_AD_LOW * 1000000 / 12670.0f )
#define AE_FACTOR_40K   ( RESOLUTION_AD_LOW * 1000000 / 13900.0f )
#define AE_FACTOR_50K   ( RESOLUTION_AD_LOW * 1000000 / 13560.0f )
#define AE_FACTOR_60K   ( RESOLUTION_AD_LOW * 1000000 / 14530.0f )
#define AE_FACTOR_70K   ( RESOLUTION_AD_LOW * 1000000 / 13700.0f )
#define AE_FACTOR_80K   ( RESOLUTION_AD_LOW * 1000000 / 13240.0f )
#define AE_FACTOR_90K   ( RESOLUTION_AD_LOW * 1000000 / 10210.0f )
#define AE_FACTOR_30K_V2   ( RESOLUTION_AD_LOW * 1000000 / 5809.0f )
#define AE_FACTOR_40K_V2   ( RESOLUTION_AD_LOW * 1000000 / 8131.0f )

#define H_C_FACTOR  (2.0*1000/65536)

#define TOKEN_MAX   50          //最大允许一次处理的数据量(TEV 和 HFCT通道使用)

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

/********************************************
 *  普通数据
 *  适用通道：所有
 *  更新时间：0.1s
 * *****************************************/
typedef struct G_RECV_PARA_NOMAL {
    HDATA hdata0;               //高频第1通道最大最小值
    HDATA hdata1;               //高频第2通道最大最小值
    quint32 hpulse0_totol;      //高频第1通道脉冲数
    quint32 hpulse1_totol;      //高频第2通道脉冲数
    quint32 ldata0_max;         //低频第1通道最大值
    quint32 ldata0_min;         //低频第1通道最小值
    quint32 ldata1_max;         //低频第2通道最大值
    quint32 ldata1_min;         //低频第2通道最小值
    quint32 playvoice_enable;   //是否能否播放声音标志位,0为不能播放,1为可以播放
    HDATA version;              //高16位为版本号(0x0300 = 3.0版本),低16位为结束符0x55AA
} G_RECV_PARA_NOMAL;

/********************************************
 *  短录波数据
 *  适用通道：高频
 *  更新时间：立刻更新
 * *****************************************/
struct G_RECV_PARA_SHORT {
    quint32 data_flag;              //第一个为0有数据，1无数据
    quint32 time;                   //时标（0-2M对应0-360°）
    quint32 data[BUFF_DATA_SIZE];   //数据,0x55AA结束,实际为240-3个有效数据
};

/********************************************
 *  录波数据
 *  适用通道：所有
 *  更新时间：立刻更新
 * *****************************************/
struct G_RECV_PARA_REC {
    quint32 recComplete;            //录播完成(1为完成)
    quint32 groupNum;               //组号(0-15)
    quint32 data[BUFF_DATA_SIZE];   //录播数据256
};

/********************************************
 *  包络线数据
 *  适用通道：低频
 *  更新时间：3.2ms
 * *****************************************/
struct G_RECV_PARA_ENVELOPE {
    quint32 data_flag;              //数据标志位(1还有数据,2为读取完成)
    quint32 groupNum;               //组号(0-7)
    quint32 time;                   //时标（0-10M对应0-5×360°）
    quint32 data[BUFF_DATA_SIZE];   //数据实际长度128
    quint32 readComplete;           //处理完成(由UI侧操作)
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
    G_RECV_PARA_NOMAL recv_para_normal;         //常规数据
    G_RECV_PARA_REC recv_para_rec;              //录波数据
    G_RECV_PARA_SHORT recv_para_short1;         //短波形数据(高频1)
    G_RECV_PARA_SHORT recv_para_short2;         //短波形数据(高频2)
    G_RECV_PARA_ENVELOPE recv_para_envelope1;   //包络线数据,共128位有效数据（低频1）
    G_RECV_PARA_ENVELOPE recv_para_envelope2;   //包络线数据,共128位有效数据（低频2）
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
    AA1,
    AA2,
    AE1,
    AE2,
    Double_Channel,
    TEV_CONTINUOUS1,        //连续录波
    TEV_CONTINUOUS2,        //连续录波
    HFCT_CONTINUOUS1,        //连续录波
    HFCT_CONTINUOUS2,        //连续录波
    UHF_CONTINUOUS1,        //连续录波
    UHF_CONTINUOUS2,        //连续录波
    AA_ENVELOPE1,       //包络线
    AA_ENVELOPE2,       //包络线
    AE_ENVELOPE1,       //包络线
    AE_ENVELOPE2,       //包络线
    ASSET,              //资产管理
    Options_Mode,
};

enum CHANNEL{
    CHANNEL_H1,     //高频通道1
    CHANNEL_H2,     //高频通道2
    CHANNEL_L1,     //低频通道1
    CHANNEL_L2,     //低频通道2
};

enum WIFI_MODE {
    WIFI_NONE,
    WIFI_STA,
    WIFI_AP,
    WIFI_APSTA,
    WIFI_SYNC,
};

struct PC_DATA {
    double pc_value;    //脉冲放电强度
    double mV_value;    //脉冲毫伏值
    int phase;          //相位
    double rise_time;   //上升时间
    double fall_time;   //下降时间
};

typedef QVector<qint32> VectorList;
typedef QVector<quint32> VectorUList;

#endif // DATA_H




