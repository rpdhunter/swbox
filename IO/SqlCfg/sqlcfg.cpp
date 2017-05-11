#include "sqlcfg.h"

pthread_mutex_t sql_mutex;
SqlCfg *sqlcfg = NULL;
sqlite3 *pDB = NULL;

SqlCfg::SqlCfg()
{
    bool exit_flag = false;
    char *err_msg = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *c_str;
    SQL_PARA *dev;

    memset(&sql_para, 0, sizeof(SQL_PARA));

    /* mutex lock sql opt */
    pthread_mutex_lock(&sql_mutex);
    if (sqlite3_open(SQL_PATH, &pDB) != SQLITE_OK) {
        /* mutex unlock */
        pthread_mutex_unlock(&sql_mutex);
        printf("Sqlite3 open %s error!\n", SQL_PATH);
        exit(-1);
    }

    c_str = "SELECT * FROM device_config where is_default=1";
    if (sqlite3_prepare_v2(pDB , c_str , strlen(c_str) , &stmt , NULL) != SQLITE_OK) {
        printf("Table device_config is no exist and create!\n");
        c_str = "create table if not exists device_config (name text, data blob, is_default int primary key);";
        if (sqlite3_exec(pDB, c_str, NULL, NULL, &err_msg) != SQLITE_OK) {
            printf("create table device_config error!\n");
            exit_flag = true;
            goto exit;
        }

        default_config();

        c_str = "insert into device_config (name, data, is_default) values (:name, :data, :is_default)";
        if (sqlite3_prepare_v2(pDB , c_str , strlen(c_str) , &stmt , NULL) != SQLITE_OK) {
            printf("Insert into device_config error!\n");
            exit_flag = true;
            goto exit;
        }

        if (sqlite3_bind_text(stmt, 1, "default_config", strlen(c_str), NULL)) {
            printf("Sqlite3_bind_text error!\n");
            exit_flag = true;
            goto exit;
        }

        if (sqlite3_bind_blob(stmt, 2, &sql_para, sizeof(sql_para), NULL) != SQLITE_OK) {
            printf("Sqlite3_bind_blob error!\n");
            exit_flag = true;
            goto exit;
        }

        if(sqlite3_bind_int(stmt, 3, 1))
        {
            printf("Sqlite3_bind_int error!\n");
            exit_flag = true;
            goto exit;
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            printf("Sqlite3_bind_blob error!\n");
            exit_flag = true;
            goto exit;
        }
    } else {
//        qDebug("Table device_config is exist! [LINE:%d] [FILE:%s]", __LINE__, __FILE__);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            dev = (SQL_PARA *)(sqlite3_column_blob(stmt, 1));
            memcpy(&sql_para, dev, sizeof(SQL_PARA));
        }
    }
exit:
    sqlite3_free(err_msg);
    sqlite3_finalize(stmt);
    sqlite3_close(pDB);

    /* mutex unlock */
    pthread_mutex_unlock(&sql_mutex);
    if (system("sync")){
        printf("run cmd[sync] failed.");
    }
    if(exit_flag)
        exit(-1);
}

SQL_PARA *SqlCfg::get_para(void)
{
    return &sql_para;
}

SQL_PARA *SqlCfg::default_config(void)
{
    char str[20];

    /* amplitude mode */
    sql_para.amp_sql1.mode = series;                                             //default series
    sql_para.amp_sql1.mode_chart = PRPS;
    sql_para.amp_sql1.high = 40;                                                 //default high
    sql_para.amp_sql1.low = 20;                                                  //default low
    sql_para.amp_sql1.tev_offset1 = 0;             //TEV偏置1
    sql_para.amp_sql1.tev_offset2 = 0;             //TEV偏置1
    sql_para.amp_sql1.tev_gain = 1.0;               //TEV增益

    sql_para.amp_sql2.mode = series;                                             //default series
    sql_para.amp_sql2.mode_chart = PRPS;
    sql_para.amp_sql2.high = 40;                                                 //default high
    sql_para.amp_sql2.low = 20;                                                  //default low
    sql_para.amp_sql2.tev_offset1 = 0;             //TEV偏置1
    sql_para.amp_sql2.tev_offset2 = 0;             //TEV偏置1
    sql_para.amp_sql2.tev_gain = 1.0;               //TEV增益

    /* pulse mode */
    sql_para.pulse_sql.mode = series;                                           //default series
    sql_para.pulse_sql.high = 40;                                               //default high
    sql_para.pulse_sql.low = 20;                                                //default low
    sql_para.pulse_sql.time = TIME_MIN;                                         //default time length

    /* aaultrasonic mode */
    sql_para.aaultra_sql.mode = series;
    sql_para.aaultra_sql.vol = 8;
    sql_para.aaultra_sql.gain = 1;
    sql_para.aaultra_sql.time = TIME_MIN;                                       //default time length

    /* setting para */
    sql_para.freq_val = 50;                                                     //default 50Hz

    /* setting backlight */
    sql_para.backlight = 7;                                                     /* backlight 0~7 */

    sql_para.reset_time = 0;

    sql_para.close_time = 5;

    /* Local default */
    sql_para.language = CN;
    strcpy(sql_para.sys_info.hard_ver, (char *)"Ver 0.1");
    strcpy(sql_para.sys_info.soft_ver, (char *)"Ver 0.1");
    strcpy(sql_para.sys_info.fpga_ver, (char *)"Ver 0.1");
    strcpy(sql_para.sys_info.cpu_ver, (char *)"Cortex A9x2");
    sprintf(str, "Qt-%d.%d.%d",
            QT_VERSION >> 16,
            0xf & (QT_VERSION >> 8),
            0xf & QT_VERSION);
    strcpy(sql_para.sys_info.qt_ver, str);

//    sql_para.tev_offset1 =0;
//    sql_para.tev_offset2 =0;

//    sql_para.tev_gain = 1;

    sql_para.aa_step = 2;

    sql_para.aa_offset = 0;

    sql_para.tev_auto_rec = false;       //自动录波默认关闭

    sql_para.max_rec_num = 200;

    return &sql_para;
}

void sqlite3_init(void)
{
    SQL_PARA *dev_para;

    pthread_mutex_init(&sql_mutex, NULL);

    sqlcfg = new SqlCfg();

    /* Load default para */
    dev_para = sqlcfg->get_para();


}

void SqlCfg::sql_save(SQL_PARA *sql_para)
{
    bool exit_flag = false;
    sqlite3_stmt *stmt = NULL;
    const char *c_str;

    /* mutex lock sql opt */
    pthread_mutex_lock(&sql_mutex);
    if (sqlite3_open(SQL_PATH, &pDB) != SQLITE_OK) {
        /* mutex unlock */
        pthread_mutex_unlock(&sql_mutex);
        printf("sqlite3 open %s error!\n", SQL_PATH);
        exit(-1);
    }

    c_str = "replace into device_config (data, is_default) values (:data, :is_default)";
    if (sqlite3_prepare_v2(pDB , c_str , strlen(c_str) , &stmt , NULL) != SQLITE_OK) {
        printf("sqlite3_prepare_v2 error!\n");
        exit_flag = true;
        goto exit;
    }

    if (sqlite3_bind_blob(stmt, 1, sql_para, sizeof(SQL_PARA), NULL) != SQLITE_OK) {
        printf("sqlite3_bind_blob error!\n");
        exit_flag = true;
        goto exit;
    }

    if (sqlite3_bind_int(stmt, 2, 1)) {
        printf("sqlite3_bind_int error!\n");
        exit_flag = true;
        goto exit;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("sqlite3_bind_blob error!\n");
        exit_flag = true;
        goto exit;
    }

exit:
    sqlite3_finalize(stmt);
    sqlite3_close(pDB);

    /* mutex unlock */
    pthread_mutex_unlock(&sql_mutex);
    if (system("sync")){
        printf("run cmd[sync] failed.");
    }
    if(exit_flag)
        exit(-1);
}
