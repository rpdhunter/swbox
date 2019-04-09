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
#define FPGA_THRESHOLD		0x120

#define AA_VOL_DEFAULT				8
#define AA_HIGH				20
#define AA_LOW				7

#define HFCT_HIGH           2000
#define HFCT_LOW			1000

#define SYSTEM_FREQ			50
#define BACK_LIGTH			3
#define SCREEN_DARK_TIME    60
#define SCREEN_CLOSE_TIME   90
#define SHUT_DOWN_TIME		5
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
    Spectra = 7,    //频谱图
    Camera = 8,     //摄像头
};

//滤波器
enum FILTER {
    NONE,
    hp_500k,
    hp_1M,
    hp_1M5,
    hp_1M8,
    hp_2M,
    hp_2M5,
    hp_3M,
    hp_5M,
    hp_8M,
    hp_10M,
    hp_12M,
    hp_15M,
    hp_18M,
    hp_20M,
    hp_22M,
    hp_25M,
    hp_28M,
    hp_30M,
    hp_32M,
    hp_35M,

    lp_100Hz,
    lp_2M,
    lp_5M,
    lp_8M,
    lp_10M,
    lp_12M,
    lp_15M,
    lp_18M,
    lp_20M,
    lp_22M,
    lp_25M,
    lp_28M,
    lp_30M,
    lp_32M,
    lp_35M,
    lp_38M,
    lp_40M,
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
    sync_none = 0,
    sync_vac220_110 = 1,
    sync_light = 2,
    sync_clamp = 3,
    sync_clamp_internal = 4,

    SYNC_INTERNAL,
    SYNC_EXTERNAL,
};

enum SENSOR_FREQ{
    ae_factor_30k,
    ae_factor_40k,
    ae_factor_50k,
    ae_factor_60k,
    ae_factor_70k,
    ae_factor_80k,
    ae_factor_90k,
    ae_factor_30k_v2,
    ae_factor_40k_v2,
};

//计量单位
enum UNITS {
    Units_db = 0,
    Units_pC = 1,
    Units_mV = 2,
    Units_uV = 3,
};

typedef struct CHANNEL_SQL {
    //通用
    bool mode;                      //检测模式
    int chart;                      //图形显示模式
    int high;                       //红色报警阈值
    int low;                        //黄色报警阈值
    double gain;                    //增益
    int pulse_time;                 //脉冲计数时长
    int rec_time;                   //录波时长
    int offset_noise;               //噪声偏置
    int fpga_threshold;             //阈值（高频需要FPGA同步）
    bool mode_recognition;          //模式识别
    int units;                      //计量单位
    bool buzzer;                    //蜂鸣器
    //高频
    int fpga_zero;                  //零点（高频需要FPGA同步）
    int filter_hp;                  //高通滤波器
    int filter_lp;                  //低通滤波器
    bool auto_rec;                  //自动录波（需要FPGA同步）
    bool filter_fir_fpga;           //开启500K-30M的带通滤波（需要FPGA同步）
    bool filter_wavelet;            //录波开启小波变换
    //低频
    int vol;                        //音量（0-15）（需要FPGA同步）
    double step;                    //显示幅值变化门槛
    bool envelope;                  //包络线模式（需要FPGA同步）
    int sensor_freq;                //传感器中心频率
    bool camera;                    //摄像头开关

} CHANNEL_SQL;

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

///* aaultrasonic mode */
//typedef struct H_CHANNEL_SQL {
//    bool mode;                      //检测模式
//    int chart;                      //图形显示模式
//    int high;                       //红色报警阈值
//    int low;                        //黄色报警阈值
//    double gain;                    //增益
//    int vol;                        //音量（0-15）（需要FPGA同步）
//    int rec_time;                       //录波时长（1-60）
//    double step;                    //显示幅值变化门槛
//    int offset_noise;                     //偏置值
//    bool envelope;                  //包络线模式（需要FPGA同步）
//    uint fpga_threshold;            //脉冲阈值
//    int sensor_freq;                //传感器中心频率
//    bool camera;                    //摄像头开关
//} H_CHANNEL_SQL;

enum WIFI_TRANS_MODE{
    wifi_ftp,
    wifi_telnet,
    wifi_104,
};

//文件保存模式
enum FILE_SAVE_MODE{
    file_save_zdit,             //自定义标准
    file_save_stategrid,        //国网标准
    file_save_both,             //我全部都要
};

//测试状态
enum TEST_MODE{
    test_on,
    test_off,
};


/* Sql para */
typedef struct SQL_PARA {
    CHANNEL_SQL tev1_sql, tev2_sql;       //地电波设置
    CHANNEL_SQL hfct1_sql, hfct2_sql;     //高频CT设置
    CHANNEL_SQL uhf1_sql, uhf2_sql;       //特高频设置
    LOCATION_SQL location_sql;              //定位模式设置
    CHANNEL_SQL aa1_sql, aa2_sql;         //AA超声设置
    CHANNEL_SQL ae1_sql, ae2_sql;         //AE超声设置

    bool language;                          //语言设置
    int freq_val;                           //频率（需要FPGA同步,FPGA 0为50, 1为60）
    int backlight;                          //背光（需要FPGA同步）
    int key_backlight;                      //键盘背光（需要FPGA同步）
    int screen_dark_time;                   //屏幕自动变暗时间,单位为秒
    int screen_close_time;                  //屏幕自动关闭时间，单位为秒
    int close_time;                         //自动关机时间,单位为分钟
    int max_rec_num;                        //录波文件保存个数
    bool buzzer_on;                         //蜂鸣器
    int auto_rec_interval;                  //自动录波间隔
    int menu_h1, menu_h2;                   //高速通道模式
    int menu_l1, menu_l2;                   //低速通道模式
    int menu_double, menu_asset;            //其他菜单模式
    int sync_mode;                          //同步模式
    int sync_val;                           //同步补偿值(角度)
    int sync_internal_val;                  //内同步补偿值(角度)
    int sync_external_val;                  //外同步补偿值(角度)
    char current_dir[500];                  //当前工作目录
    int file_save_standard;                 //文件保存标准
    bool test_mode;                         //测试状态

} SQL_PARA;

class SqlCfg
{
public:
    SqlCfg();
    SQL_PARA *get_para();
    void sql_save(SQL_PARA *sql_para);
    SQL_PARA *default_config(void);         //初始设置
    double ae1_factor();             //快捷返回AE_FACTOR
    double ae2_factor();             //快捷返回AE_FACTOR

private:
    SQL_PARA sql_para;
    pthread_mutex_t sql_mutex;
    sqlite3 *pDB = NULL;

    void channel_default(CHANNEL_SQL &sql);
    void tev_default(CHANNEL_SQL &sql);
    void hfct_default(CHANNEL_SQL &sql);
    void uhf_default(CHANNEL_SQL &sql);
    void aa_default(CHANNEL_SQL &sql);
    void ae_default(CHANNEL_SQL &sql);

    void sql_init();                        //每次开机都需要重置的选项（这些选项实际是全局变量的作用）
};

extern SqlCfg * sqlcfg;

#endif // SQLCFG_H
