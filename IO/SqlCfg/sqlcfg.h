#ifndef SQLCFG_H
#define SQLCFG_H

#include <QDebug>
#include <sqlite3.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "IO/Data/data.h"

#define SQL_PATH    "/root/sql.db"

#define TEV_HIGH			40
#define TEV_LOW				20
#define FPGA_THRESHOLD		0x80

#define AA_VOL_DEFAULT				8
#define AA_HIGH				40
#define AA_LOW				20

#define HFCT_HIGH           2000
#define HFCT_LOW			1000

#define SYSTEM_FREQ			50
#define BACK_LIGTH			3
#define SCREEN_DARK_TIME    60
#define SCREEN_CLOSE_TIME   90
#define SHUT_DOWN_TIME		5
#define LANGUAGE_DEF		CN
#define MAX_REC_NUM			200
#define MAX_PULSE_CNT       10              //最大脉冲计数时长

enum LANGUAGE {
    CN = 0,
    EN = 1,
};

enum TRIGGER_MODE {
    single = 0,
    continuous = 1,
};

enum DISPLAY {
    BASIC = 0,      //时序图
    PRPD = 1,       //PRPD图
    PRPS = 2,       //PRPS图
    Histogram = 3,  //强度图
    TF = 4,         //时频特性图
    FLY = 5,        //飞行图
    Exponent = 6,   //特征指数图
};

enum FILTER {
    NONE = 0,
    HP_500K = 1,
    HP_1800K = 2,
};

enum LOCATION_TRIGGER_CHANNEL {
    Left = 0,
    Right = 1,
    Double = 2,
};

enum LOCATION_CHART_MODE {
    CURVE = 0,
    COMPASS = 1,
};

enum SYNC_MODE {
    SYNC_NONE = 0,
    SYNC_INTERNAL = 1,
    SYNC_EXTERNAL = 2,
};

enum WIFI_MODE {
    WIFI_NONE = 0,
    WIFI_HOTPOT = 1,
    WIFI_AP = 2,
    WIFI_SYNC = 4,
};

typedef struct H_CHANNEL_SQL {
    bool mode;                      //检测模式
    int chart;                      //图形显示模式
    int high;                       //红色报警阈值
    int low;                        //黄色报警阈值
    double gain;                    //增益
    int pulse_time;                 //脉冲计数时长
    int rec_time;                   //录波时长
    int offset_noise;               //噪声偏置
    int offset_linearity;           //线性度补偿偏置
    int fpga_zero;                  //零点（需要FPGA同步）
    uint fpga_threshold;            //阈值（需要FPGA同步）
    int filter;                     //滤波器（需要FPGA同步）
    bool auto_rec;                  //自动录波（需要FPGA同步）
} H_CHANNEL_SQL;

typedef struct LOCATION_SQL {
    bool mode;                      //检测模式
    int time;                       //触发时长
    int channel;                    //触发通道
    int chart;                      //图形显示模式
} LOCATION_SQL;

#define TIME_MAX                60
#define TIME_MIN                1
#define VOL_MAX                 15
#define VOL_MIN                 0

/* aaultrasonic mode */
typedef struct L_CHANNEL_SQL {
    bool mode;                      //检测模式
    int chart;                      //图形显示模式
    int high;                       //红色报警阈值
    int low;                        //黄色报警阈值
    double gain;                    //增益
    int vol;                        //音量（0-15）（需要FPGA同步）
    int time;                       //录波时长（1-60）
    double step;                    //显示幅值变化门槛
    int offset;                     //偏置值
    bool envelope;                  //包络线模式（需要FPGA同步）
    uint fpga_threshold;            //脉冲阈值（需要FPGA同步）
} L_CHANNEL_SQL;

//typedef struct SYNCMODE_SQL {
//    bool mode;                      //检测模式
//    int time;                       //触发时长
//    int channel;   //触发通道
//    int chart_mode;      //图形显示模式
//} SYNCMODE_SQL;


/* Sql para */
typedef struct SQL_PARA {
    H_CHANNEL_SQL tev1_sql, tev2_sql;       //地电波设置
    H_CHANNEL_SQL hfct1_sql, hfct2_sql;     //高频CT设置
    H_CHANNEL_SQL uhf1_sql, uhf2_sql;       //特高频设置
    LOCATION_SQL location_sql;              //定位模式设置
    L_CHANNEL_SQL aa1_sql, aa2_sql;         //AA超声设置
    L_CHANNEL_SQL ae1_sql, ae2_sql;         //AE超声设置

    bool language;                          //语言设置
    int freq_val;                           //频率（需要FPGA同步）
    int backlight;                          //背光（需要FPGA同步）
    int key_backlight;                      //键盘背光（需要FPGA同步）
    int screen_dark_time;                   //屏幕自动变暗时间,单位为秒
    int screen_close_time;                  //屏幕自动关闭时间，单位为秒
    int close_time;                         //自动关机时间,单位为分钟
    int max_rec_num;                        //录波文件保存个数
    bool buzzer_on;                         //蜂鸣器
    int auto_rec_interval;                  //自动录波间隔
    int menu_h1, menu_h2;                   //高速通道模式
    int menu_double, menu_l1, menu_l2;      //其他菜单模式
    int sync_mode;                          //同步模式
    int sync_internal_val;                  //内同步时间
    int sync_external_val;                  //外同步时间

} SQL_PARA;

class SqlCfg
{
public:
    SqlCfg();
    SQL_PARA *get_para();
    void sql_save(SQL_PARA *sql_para);
    SQL_PARA *default_config(void);      //初始设置

private:
    SQL_PARA sql_para;
    pthread_mutex_t sql_mutex;
    sqlite3 *pDB = NULL;

    void tev_default(H_CHANNEL_SQL &sql);
    void hfct_default(H_CHANNEL_SQL &sql);
    void uhf_default(H_CHANNEL_SQL &sql);
    void aa_default(L_CHANNEL_SQL &sql);
    void ae_default(L_CHANNEL_SQL &sql);

};

extern SqlCfg * sqlcfg;

#endif // SQLCFG_H
