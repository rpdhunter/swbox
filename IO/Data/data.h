#ifndef DATA_H
#define DATA_H

#include <QtGlobal>
#include <QMetaType>
#include <QVector>

//定义一些全局数据类型，和一些全局宏

#define AD_VAL(val, offset)     ((val) >= offset ? (val) - offset : offset - (val))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

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
    HDATA hdata0;           //未使用
    HDATA hdata1;           //地电波使用
    quint32 ldata0_max;     //未使用
    quint32 ldata0_min;     //未使用
    quint32 ldata1_max;     //AA超声使用
    quint32 ldata1_min;     //AA超声使用
    HPULSE_EDGE pulse0;
    HPULSE_EDGE pulse1;
    quint32 hpulse0_totol;
    quint32 hpulse1_totol;

    quint32 recComplete;    //录播完成(1为完成)
    quint32 groupNum;       //组号(0-7)
    quint32 recData[258];   //录播数据
} G_RECV_PARA;

typedef struct RPARA {
    bool flag;
    unsigned int rval;
} RPARA;

/* fifo send data */
typedef struct G_SEND_PARA {
    RPARA freq;
    RPARA blacklight;
    RPARA recstart;     //0为常态,1为开始录播,2为开始上传
    RPARA groupNum;     //传输录波数据的组号
    RPARA aa_vol;       //AA超声音量调节
    RPARA read_fpga;    //
} G_SEND_PARA;

/* total data */
typedef struct G_PARA {
    G_RECV_PARA recv_para;
    G_SEND_PARA send_para;
} G_PARA;
#pragma pack()

enum MODE{
    TEV,                    //0
    AA_Ultrasonic,          //1
    AA_Ultrasonic_End,      //AA超声结束
    AE_Ultrasonic,
    Disable
};


typedef QVector<qint32> VectorList;
//qRegisterMetaType<VectorList>("VectorList");



#endif // DATA_H
