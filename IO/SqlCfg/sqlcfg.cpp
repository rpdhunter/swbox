#include "sqlcfg.h"
#include <QNetworkInterface>

SqlCfg *sqlcfg = NULL;

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
    //打开数据库，并关联句柄pDB。文件名不需要一定存在，如果此文件不存在，sqlite会自动建立它。如果它存在，就尝试把它当数据库文件来打开。
    if (sqlite3_open(SQL_PATH, &pDB) != SQLITE_OK) {
        /* mutex unlock */
        pthread_mutex_unlock(&sql_mutex);
        printf("Sqlite3 open %s error!\n", SQL_PATH);
        exit(-1);
    }

    //查询是否有is_default=1的条目，有则读取，没有则新建
    c_str = "SELECT * FROM device_config where is_default=1";
    if (sqlite3_prepare_v2(pDB , c_str , strlen(c_str) , &stmt , NULL) != SQLITE_OK) {
        //新建数据库device_config，数据库存在3个字段：name,data,is_default
        printf("Table device_config is no exist and create!\n");
        c_str = "create table if not exists device_config (name text, data blob, is_default int primary key);";
        if (sqlite3_exec (pDB, c_str, NULL, NULL, &err_msg) != SQLITE_OK) {
            printf ("create table device_config error!\n");
            goto exit;
        }

        //数据库初始化
        default_config();
        c_str = "insert into device_config (name, data, is_default) values (:name, :data, :is_default)";
        if (sqlite3_prepare_v2 (pDB , c_str , strlen(c_str) , &stmt , NULL) != SQLITE_OK) {
            printf ("Insert into device_config error!\n");
            goto exit;
        }
        //插入第一字段:name
        if (sqlite3_bind_text (stmt, 1, "default_config", strlen(c_str), NULL)) {
            printf("Sqlite3_bind_text error!\n");
            goto exit;
        }
        //插入第二字段:data
        if (sqlite3_bind_blob (stmt, 2, &sql_para, sizeof (sql_para), NULL) != SQLITE_OK) {
            printf ("Sqlite3_bind_blob error!\n");
            goto exit;
        }
        //插入第三字段:is_default
        if (sqlite3_bind_int (stmt, 3, 1)) {
            printf ("Sqlite3_bind_int error!\n");
            goto exit;
        }

        if (sqlite3_step (stmt) != SQLITE_DONE) {
            printf ("Sqlite3_bind_blob error!\n");
            goto exit;
        }
    }
    else {
//        qDebug("Table device_config is exist! [LINE:%d] [FILE:%s]", __LINE__, __FILE__);
        while (sqlite3_step (stmt) == SQLITE_ROW) {
            dev = (SQL_PARA *)(sqlite3_column_blob (stmt, 1));      //读取sqlite3中的blob数据
            memcpy (&sql_para, dev, sizeof (SQL_PARA));
        }        
    }
    exit_flag = false;
exit:
    sqlite3_free(err_msg);
    sqlite3_finalize(stmt);
    sqlite3_close(pDB);

    sql_init();         //每次开机初始化

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
    qDebug()<<"sql_para.freq_val"<<sql_para.freq_val;
    sql_para.backlight = BACK_LIGTH;						//backlight 0~7
    sql_para.key_backlight = 0;                             //默认键盘背光关闭
    sql_para.screen_dark_time = SCREEN_DARK_TIME;
    sql_para.screen_close_time = SCREEN_CLOSE_TIME;
    sql_para.close_time = SHUT_DOWN_TIME;
    sql_para.language = CN;

    sql_para.max_rec_num = MAX_REC_NUM;
    sql_para.buzzer_on = false;
    sql_para.auto_rec_interval = 2;         //默认2秒一次录波(防止卡顿)


    sql_para.sync_mode = sync_none;
    sql_para.sync_val = 0;
    sql_para.file_save_standard = file_save_zdit;       //采用自定义标准保存文件
    sql_para.multimachine_mode = multimachine_close;    //默认关闭
    sql_para.time_zone = 8;                 //默认为东8区
    sql_para.ethernet = false;              //默认关闭以太网口
    sql_para.temp_compensation = temp_auto;     //默认智能温度补偿

    return &sql_para;
}

SQL_GLOBAL *SqlCfg::get_global()
{
    return &sql_global;
}

double SqlCfg::ae1_factor()
{
    switch (sql_para.ae1_sql.sensor_freq) {
    case ae_factor_30k:
        return AE_FACTOR_30K;
    case ae_factor_40k:
        return AE_FACTOR_40K;
    case ae_factor_50k:
        return AE_FACTOR_50K;
    case ae_factor_60k:
        return AE_FACTOR_60K;
    case ae_factor_70k:
        return AE_FACTOR_70K;
    case ae_factor_80k:
        return AE_FACTOR_80K;
    case ae_factor_90k:
        return AE_FACTOR_90K;
    case ae_factor_30k_v2:
        return AE_FACTOR_30K_V2;
    case ae_factor_40k_v2:
        return AE_FACTOR_40K_V2;
    default:
        return AE_FACTOR_30K;
    }
}

double SqlCfg::ae2_factor()
{
    switch (sql_para.ae2_sql.sensor_freq) {
    case ae_factor_30k:
        return AE_FACTOR_30K;
    case ae_factor_40k:
        return AE_FACTOR_40K;
    case ae_factor_50k:
        return AE_FACTOR_50K;
    case ae_factor_60k:
        return AE_FACTOR_60K;
    case ae_factor_70k:
        return AE_FACTOR_70K;
    case ae_factor_80k:
        return AE_FACTOR_80K;
    case ae_factor_90k:
        return AE_FACTOR_90K;
    case ae_factor_30k_v2:
        return AE_FACTOR_30K_V2;
    case ae_factor_40k_v2:
        return AE_FACTOR_40K_V2;
    default:
        return AE_FACTOR_30K;
    }
}

void SqlCfg::sql_init()
{
    sql_para.tev1_sql.auto_rec = false;
    sql_para.tev2_sql.auto_rec = false;
    sql_para.hfct1_sql.auto_rec = false;
    sql_para.hfct2_sql.auto_rec = false;
    sql_para.uhf1_sql.auto_rec = false;
    sql_para.uhf2_sql.auto_rec = false;

    sql_para.backlight = BACK_LIGTH;                //开机重置屏幕亮度
    sql_global.test_mode = false;                   //开机关闭测试模式
    strcpy(sql_global.current_dir, DIR_DATA);       //开机时需要重置当前目录
    sql_global.mac_code = 1;                        //特征码默认为1
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    foreach (QNetworkInterface i, nets) {
        if(i.name() == "eth0"){
            sql_global.mac = i.hardwareAddress();
            sql_global.mac_code = sql_global.mac.mid(sql_global.mac.count()-2,2).toInt(NULL,10);       //取mac后两位
            break;
        }
    }
    if(sql_global.mac.mid(sql_global.mac.count()-5,2) == "01"){
        sql_global.wifi_enable = false;
    }
    else{
        sql_global.wifi_enable = true;
    }
    sql_global.wifi_working = false;
    sql_global.color = Qt::black;
}

void SqlCfg::tev_default(CHANNEL_SQL &sql)
{
    channel_default(sql);
    sql.high = TEV_HIGH;
    sql.low = TEV_LOW;
    sql.fpga_threshold = 8;       //8mv对应18db
    sql.units = Units_db;
}

void SqlCfg::hfct_default(CHANNEL_SQL &sql)
{
    channel_default(sql);
    sql.high = HFCT_HIGH;
    sql.low = HFCT_LOW;
    sql.fpga_threshold = 8;       //8mv对应18db
    sql.units = Units_pC;
}

void SqlCfg::uhf_default(CHANNEL_SQL &sql)
{
    channel_default(sql);
    sql.high = TEV_HIGH;
    sql.low = TEV_LOW;
    sql.fpga_threshold = 8;       //8mv对应18db
    sql.units = Units_db;
}

void SqlCfg::aa_default(CHANNEL_SQL &sql)
{
    channel_default(sql);
    sql.high = AA_HIGH;
    sql.low = AA_LOW;
    sql.fpga_threshold = 2;        //2uV对应6db
    sql.units = Units_db;
}

void SqlCfg::ae_default(CHANNEL_SQL &sql)
{
    channel_default(sql);
    sql.high = AA_HIGH;
    sql.low = AA_LOW;
    sql.fpga_threshold = 2;        //2uV对应6db
    sql.units = Units_db;
}

void SqlCfg::channel_default(CHANNEL_SQL &sql)
{
    //通用
    sql.mode = continuous;
    sql.chart = BASIC;
//    sql.high = TEV_HIGH;
//    sql.low = TEV_LOW;
    sql.gain = 1.0;
    sql.pulse_time = 2;
    sql.rec_time = 3;
    sql.offset_noise = 0;
//    sql.fpga_threshold = FPGA_THRESHOLD;
    sql.mode_recognition = false;
//    sql.units = Units_db;
    sql.buzzer = false;
    //高频
    sql.fpga_zero = 0;
    sql.filter_hp = NONE;
    sql.filter_lp = NONE;
    sql.auto_rec = false;
    sql.filter_fir_fpga = true;
    sql.filter_wavelet = false;
    //低频
    sql.vol = AA_VOL_DEFAULT;
    sql.step = 2;
    sql.envelope = 1;                      //默认使用包络线
    sql.sensor_freq = ae_factor_40k;
    sql.camera = false;
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
