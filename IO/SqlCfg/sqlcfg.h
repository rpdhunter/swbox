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
enum LANGUAGE {
    CN = 0,
    EN = 1,
};

/* amplitude mode */
enum AMP_MODE {
    signal = 0,
    series = 1,
};

enum AMP_CHART_MODE {
    PRPS = 0,
    PRPD = 1,
    Histogram = 2,
};

typedef struct AMP_SQL {
    bool mode;
    AMP_CHART_MODE mode_chart;
    int high;
    int low;
} AMP_SQL;

#define TIME_MAX            5
#define TIME_MIN            1
/* pulse mode */
typedef struct PULSE_SQL {
    bool mode;
    int high;
    int low;
    int time;
} PULSE_SQL;

/* aaultrasonic mode */
typedef struct AAULTRA_SQL {
#define VOL_MAX                 15
#define VOL_MIN                 0
    bool mode;
    int gain;
    int vol;
    int time;
} AAULTRA_SQL;

typedef struct SYS_INFO {
    char hard_ver[20];
    char soft_ver[20];
    char fpga_ver[20];
    char cpu_ver[20];
    char qt_ver[20];
} SYS_INFO;

/* Sql para */
typedef struct SQL_PARA {
    bool language;          //语言设置
    AMP_SQL amp_sql;        //地电波设置
    PULSE_SQL pulse_sql;
    AAULTRA_SQL aaultra_sql;    //aa超声设置

    int freq_val;           //频率
    int backlight;          //背光
    int reset_time;         //峰值重置时间
    int close_time;         //自动关机时间
    SYS_INFO sys_info;      //系统信息(未使用)

    int tev_offset1;             //TEV偏置1
    int tev_offset2;             //TEV偏置1
    double tev_gain;               //TEV增益

    double aa_step;         //显示幅值变化门槛
    int aa_offset;       //aa超声偏置值
    bool tev_auto_rec;      //自动录波

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

extern pthread_mutex_t sql_mutex;
extern SqlCfg *sqlcfg;
extern sqlite3 *pDB;

void sqlite3_init(void);
#endif // SQLCFG_H
