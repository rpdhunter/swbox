#ifndef SQLCFG_H
#define SQLCFG_H

#include <QDebug>
#include <sqlite3.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#ifdef ARM
#define SQL_PATH    "/root/sql.db"
#else
#define SQL_PATH    "./sql.db"
#endif

#define TEV_HIGH			40
#define TEV_LOW				20
#define FPGA_THRESHOLD		0x200

#define AA_VOL_DEFAULT				8
#define AA_HIGH				40
#define AA_LOW				20

#define SYSTEM_FREQ			50
#define BACK_LIGTH			4
#define SHUT_DOWN_TIME		5
#define LANGUAGE_DEF		CN
#define MAX_REC_NUM			200

enum LANGUAGE {
    CN = 0,
    EN = 1,
};

/* amplitude mode */
enum TRIGGER_MODE {
    single = 0,
    continuous = 1,
};

enum TEV_CHART_MODE {
    PRPS = 0,
    PRPD = 1,
    Histogram = 2,
};

typedef struct TEV_SQL {
    bool mode;                      //脉冲触发模式
    TEV_CHART_MODE mode_chart;      //图形显示模式
    int high;                       //红色报警阈值
    int low;                        //黄色报警阈值
    int tev_offset1;                //TEV偏置1，目前用作噪声偏置
    int tev_offset2;                //TEV偏置2，目前未使用，备用
    double gain;                    //TEV增益
    int fpga_zero;                  //TEV零点（需要FPGA同步）
    uint fpga_threshold;            //TEV阈值（需要FPGA同步）
    bool auto_rec;                  //自动录波（需要FPGA同步）
} TEV_SQL;

#define TIME_MAX                60
#define TIME_MIN                1
#define VOL_MAX                 15
#define VOL_MIN                 0

/* aaultrasonic mode */
typedef struct AAULTRA_SQL {
    bool mode;                      //脉冲触发模式
    double gain;                    //AA超声增益
    int vol;                        //AA超声音量（0-15）（需要FPGA同步）
    int high;                       //红色报警阈值
    int low;                        //黄色报警阈值
    int time;                       //录波时长（1-60）
    double aa_step;                 //显示幅值变化门槛
    int aa_offset;                  //AA超声偏置值
} AAULTRA_SQL;


/* Sql para */
typedef struct SQL_PARA {
    TEV_SQL tev1_sql, tev2_sql;     //地电波设置(通道1和2)
    AAULTRA_SQL aaultra_sql;        //AA超声设置

    bool language;                  //语言设置
    int freq_val;                   //频率（需要FPGA同步）
    int backlight;                  //背光（需要FPGA同步）
    int close_time;                 //自动关机时间
    int max_rec_num;                //录波文件保存个数

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
};

extern SqlCfg * sqlcfg;

void sqlite3_init(void);
#endif // SQLCFG_H
