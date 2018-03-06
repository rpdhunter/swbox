#include "sqlcfg.h"

//pthread_mutex_t sql_mutex;
SqlCfg *sqlcfg = NULL;
//sqlite3 *pDB = NULL;

SqlCfg::SqlCfg()
{
    pthread_mutex_init(&sql_mutex, NULL);       //互斥锁初始化

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
    tev_default(sql_para.tev1_sql);
    tev_default(sql_para.tev2_sql);
    hfct_default(sql_para.hfct1_sql);
    hfct_default(sql_para.hfct2_sql);
    uhf_default(sql_para.uhf1_sql);
    uhf_default(sql_para.uhf2_sql);
    aa_default(sql_para.aa1_sql);
    aa_default(sql_para.aa2_sql);
    ae_default(sql_para.ae1_sql);
    ae_default(sql_para.ae2_sql);

    /* location mode */
    sql_para.location_sql.mode = continuous;
    sql_para.location_sql.time = 2;
    sql_para.location_sql.channel = Double;
    sql_para.location_sql.chart = CURVE;

    /* setting para */
    sql_para.freq_val = SYSTEM_FREQ;						//default 50Hz
    sql_para.backlight = BACK_LIGTH;						//backlight 0~7
    sql_para.key_backlight = 0;                             //默认键盘背光关闭
    sql_para.screen_dark_time = SCREEN_DARK_TIME;
    sql_para.screen_close_time = SCREEN_CLOSE_TIME;
    sql_para.close_time = SHUT_DOWN_TIME;
    sql_para.language = LANGUAGE_DEF;

    sql_para.max_rec_num = MAX_REC_NUM;
    sql_para.buzzer_on = false;
    sql_para.auto_rec_interval = 1;

    sql_para.menu_h1 = TEV1;
    sql_para.menu_h2 = HFCT2;
    sql_para.menu_double = Double_Channel;
    sql_para.menu_l1 = AA1;
    sql_para.menu_l2 = AE2;

    sql_para.sync_mode = SYNC_NONE;
    sql_para.sync_internal_val = 0;
    sql_para.sync_external_val = 0;

    return &sql_para;
}

void SqlCfg::tev_default(H_CHANNEL_SQL &sql)
{
    sql.mode = continuous;
    sql.chart = BASIC;
    sql.high = TEV_HIGH;
    sql.low = TEV_LOW;
    sql.gain = 1.0;
    sql.fpga_zero = 0;
    sql.fpga_threshold = FPGA_THRESHOLD;
    sql.auto_rec = false;
    sql.pulse_time = 1;
    sql.rec_time = 5;
    sql.offset_noise = 0;
    sql.offset_linearity = 0;
    sql.filter = NONE;
}

void SqlCfg::hfct_default(H_CHANNEL_SQL &sql)
{
    sql.mode = continuous;
    sql.chart = BASIC;
    sql.high = HFCT_HIGH;
    sql.low = HFCT_LOW;
    sql.gain = 1.0;
    sql.fpga_zero = 0;
    sql.fpga_threshold = FPGA_THRESHOLD;
    sql.auto_rec = false;
    sql.pulse_time = 1;
    sql.rec_time = 5;
    sql.offset_noise = 0;
    sql.offset_linearity = 0;
    sql.filter = NONE;
}

void SqlCfg::uhf_default(H_CHANNEL_SQL &sql)
{
    sql.mode = continuous;
    sql.chart = BASIC;
    sql.high = TEV_HIGH;
    sql.low = TEV_LOW;
    sql.gain = 1.0;
    sql.fpga_zero = 0;
    sql.fpga_threshold = FPGA_THRESHOLD;
    sql.auto_rec = false;
    sql.pulse_time = 1;
    sql.rec_time = 5;
    sql.offset_noise = 0;
    sql.offset_linearity = 0;
    sql.filter = NONE;
}

void SqlCfg::aa_default(L_CHANNEL_SQL &sql)
{
    sql.mode = continuous;
    sql.chart = BASIC;
    sql.vol = AA_VOL_DEFAULT;
    sql.gain = 1.0;
    sql.high = AA_HIGH;					//default high
    sql.low = AA_LOW;						//default low
    sql.time = TIME_MIN;					//default time length
    sql.step = 2;
    sql.offset = 0;
    sql.envelope = 1;                      //默认使用包络线
    sql.fpga_threshold = FPGA_THRESHOLD;
}

void SqlCfg::ae_default(L_CHANNEL_SQL &sql)
{
    sql.mode = continuous;
    sql.chart = BASIC;
    sql.vol = AA_VOL_DEFAULT;
    sql.gain = 1.0;
    sql.high = AA_HIGH;					//default high
    sql.low = AA_LOW;						//default low
    sql.time = TIME_MIN;					//default time length
    sql.step = 2;
    sql.offset = 0;
    sql.envelope = 1;                      //默认使用包络线
    sql.fpga_threshold = FPGA_THRESHOLD;
}

void SqlCfg::sql_save(SQL_PARA *sql_para)
{
    this->sql_para = *sql_para;      //新加的，实现保存变量功能

    //以下为原代码，实现写入数据库功能
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
