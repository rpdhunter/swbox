#include "sqlcfg.h"

pthread_mutex_t sql_mutex;
SqlCfg *sqlcfg = NULL;
sqlite3 *pDB = NULL;

SqlCfg::SqlCfg()
{
    bool exit_flag = true;
    char * err_msg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * c_str;
    SQL_PARA * dev;

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
        if (sqlite3_exec (pDB, c_str, NULL, NULL, &err_msg) != SQLITE_OK) {
            printf ("create table device_config error!\n");
            goto exit;
        }

        default_config();

        c_str = "insert into device_config (name, data, is_default) values (:name, :data, :is_default)";
        if (sqlite3_prepare_v2 (pDB , c_str , strlen(c_str) , &stmt , NULL) != SQLITE_OK) {
            printf ("Insert into device_config error!\n");
            goto exit;
        }

        if (sqlite3_bind_text (stmt, 1, "default_config", strlen(c_str), NULL)) {
            printf("Sqlite3_bind_text error!\n");
            goto exit;
        }

        if (sqlite3_bind_blob (stmt, 2, &sql_para, sizeof (sql_para), NULL) != SQLITE_OK) {
            printf ("Sqlite3_bind_blob error!\n");
            goto exit;
        }

        if (sqlite3_bind_int (stmt, 3, 1)) {
            printf ("Sqlite3_bind_int error!\n");
            goto exit;
        }

        if (sqlite3_step (stmt) != SQLITE_DONE) {
            printf ("Sqlite3_bind_blob error!\n");
            goto exit;
        }
    } else {
//        qDebug("Table device_config is exist! [LINE:%d] [FILE:%s]", __LINE__, __FILE__);
        while (sqlite3_step (stmt) == SQLITE_ROW) {
            dev = (SQL_PARA *)(sqlite3_column_blob (stmt, 1));
            memcpy (&sql_para, dev, sizeof (SQL_PARA));
        }        
    }
    exit_flag = false;
exit:
    sqlite3_free(err_msg);
    sqlite3_finalize(stmt);
    sqlite3_close(pDB);

    /* mutex unlock */
    pthread_mutex_unlock (&sql_mutex);
    if (system("sync")) {
        printf("run cmd[sync] failed.");
    }
    if (exit_flag) {

        qDebug()<<"exit AAA";
        exit (-1);
    }
}

SQL_PARA *SqlCfg::get_para(void)
{
    return &sql_para;
}

SQL_PARA *SqlCfg::default_config(void)
{
    /* amplitude mode */
    sql_para.tev1_sql.mode = continuous;					//default series
    sql_para.tev1_sql.mode_chart = PRPS;
    sql_para.tev1_sql.high = TEV_HIGH;						//default high
    sql_para.tev1_sql.low = TEV_LOW;						//default low
    sql_para.tev1_sql.tev_offset1 = 0;						//TEV偏置1
    sql_para.tev1_sql.tev_offset2 = 0;						//TEV偏置2
    sql_para.tev1_sql.gain = 1.0;							//TEV增益
    sql_para.tev1_sql.fpga_zero = 0;
    sql_para.tev1_sql.fpga_threshold = FPGA_THRESHOLD;
    sql_para.tev1_sql.auto_rec = false;						//自动录波默认关闭

    sql_para.tev2_sql.mode = continuous;					//default series
    sql_para.tev2_sql.mode_chart = PRPS;
    sql_para.tev2_sql.high = TEV_HIGH;						//default high
    sql_para.tev2_sql.low = TEV_LOW;						//default low
    sql_para.tev2_sql.tev_offset1 = 0;						//TEV偏置1
    sql_para.tev2_sql.tev_offset2 = 0;						//TEV偏置2
    sql_para.tev2_sql.gain = 1.0;							//TEV增益
    sql_para.tev2_sql.fpga_zero = 0;
    sql_para.tev2_sql.fpga_threshold = FPGA_THRESHOLD;
    sql_para.tev2_sql.auto_rec = false;						//自动录波默认关闭

    /* aaultrasonic mode */
    sql_para.aaultra_sql.mode = continuous;
    sql_para.aaultra_sql.vol = AA_VOL_DEFAULT;
    sql_para.aaultra_sql.gain = 1.0;
    sql_para.aaultra_sql.high = AA_HIGH;					//default high
    sql_para.aaultra_sql.low = AA_LOW;						//default low
    sql_para.aaultra_sql.time = TIME_MIN;					//default time length
    sql_para.aaultra_sql.aa_step = 2;
    sql_para.aaultra_sql.aa_offset = 0;

    /* setting para */
    sql_para.freq_val = SYSTEM_FREQ;						//default 50Hz

    /* setting backlight */
    sql_para.backlight = BACK_LIGTH;						//backlight 0~7

    sql_para.close_time = SHUT_DOWN_TIME;

    /* Local default */
    sql_para.language = LANGUAGE_DEF;

    sql_para.max_rec_num = MAX_REC_NUM;

    sql_para.full_featured = 0;

    return &sql_para;
}

void sqlite3_init(void)
{
//    SQL_PARA *dev_para;

    pthread_mutex_init(&sql_mutex, NULL);       //互斥锁初始化

    sqlcfg = new SqlCfg();

    /* Load default para */
//    dev_para = sqlcfg->get_para();


}

void SqlCfg::sql_save(SQL_PARA *sql_para)
{
    bool exit_flag = true;
    sqlite3_stmt * stmt = NULL;
    const char * c_str;

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
        goto exit;
    }

    if (sqlite3_bind_blob(stmt, 1, sql_para, sizeof(SQL_PARA), NULL) != SQLITE_OK) {
        printf("sqlite3_bind_blob error!\n");
        goto exit;
    }

    if (sqlite3_bind_int(stmt, 2, 1)) {
        printf("sqlite3_bind_int error!\n");
        goto exit;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        printf("sqlite3_bind_blob error!\n");
        goto exit;
    }

	exit_flag = false;

exit:
    sqlite3_finalize (stmt);
    sqlite3_close (pDB);

    /* mutex unlock */
    pthread_mutex_unlock (&sql_mutex);
    if (system("sync")){
        printf("run cmd[sync] failed.");
    }
    if (exit_flag) {
        exit (-1);
    }
}
