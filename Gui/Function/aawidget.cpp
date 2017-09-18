#include "aawidget.h"
#include "ui_aawidget.h"
#include <QLineEdit>
#include <QTimer>
#include "IO/rdb/rdb.h"

#define SETTING_NUM 7           //设置菜单条目数
#define VALUE_MAX 60

AAWidget::AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AAWidget)
{
    ui->setupUi(this);

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->setStyleSheet("AAWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    this->move(3, 3);
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->menu_index = menu_index;

    reload(-1);

    temp_db = 0;
    db = 0;

    //图形设置
    Common::set_barchart_style(ui->qwtPlot, VALUE_MAX);
    chart = new BarChart(ui->qwtPlot, &db, &aaultra_sql->high, &aaultra_sql->low);

    timer1 = new QTimer(this);
    timer1->setInterval(100);
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_2()));   //每0.1秒刷新一次数据状态，明显的变化需要快速显示

    timer2 = new QTimer(this);
    timer2->setInterval(1000);
    connect(timer2, SIGNAL(timeout()), this, SLOT(fresh_1()));   //每1秒刷新一次数据状态
    connect(timer2, &QTimer::timeout, chart, &BarChart::fresh);

    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));

    logtools = new LogTools(MODE::AA_Ultrasonic);      //日志保存模块
    connect(this,SIGNAL(aa_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));

    reload(menu_index);
}

AAWidget::~AAWidget()
{
    delete ui;
}

void AAWidget::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();
    aaultra_sql = &sql_para.aaultra_sql;

    //构造函数中计时器不启动
    if(index == menu_index){
        if(!timer1->isActive()){
            timer1->start();
        }
        if(!timer2->isActive()){
            timer2->start();
        }
        data->set_send_para (sp_aa_vol, aaultra_sql->vol);  //重置声音
//        qDebug()<<"vol changed!";
    }
}

void AAWidget::trans_key(quint8 key_code)
{    
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }
//    qDebug()<<"val0 = "<<key_val->grade.val0 <<"\nval1 = "<<key_val->grade.val1 <<"\nval2 = "<<key_val->grade.val2 ;
    switch (key_code) {
    case KEY_OK:
        sqlcfg->sql_save(&sql_para);
        reload(menu_index);
        switch (key_val->grade.val2) {
        case 6:
            key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
            emit startRecWave(AA_Ultrasonic, aaultra_sql->time);        //发送录波信号
            return;
        case 7:
            this->maxReset();
            break;
        default:
            break;
        }
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        reload(menu_index);        //重置默认数据
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    default:
        break;
    }
    emit fresh_parent();
    fresh_setting();
}

void AAWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2, d, SETTING_NUM, 1);
}

void AAWidget::do_key_left_right(int d)
{
    switch (key_val->grade.val2) {
    case 1:
        aaultra_sql->mode = !aaultra_sql->mode;
        break;
    case 2:
        Common::change_index(aaultra_sql->gain, d * 0.1, 20, 0.1 );
        break;
    case 3:
        Common::change_index(aaultra_sql->vol, d, VOL_MAX, VOL_MIN );
        break;
    case 4:
        Common::change_index(aaultra_sql->low, d, aaultra_sql->high, 0 );
        break;
    case 5:
        Common::change_index(aaultra_sql->high, d, 60, aaultra_sql->low );
        break;
    case 6:
        Common::change_index(aaultra_sql->time, d, TIME_MAX, TIME_MIN );
        break;
    default:
        break;
    }
}

void AAWidget::showWaveData(VectorList buf, MODE mod)
{
    if(key_val->grade.val0 == menu_index){       //在超声界面，可以显示
        ui->comboBox->hidePopup();
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        key_val->grade.val5 = 1;
        emit fresh_parent();
        ui->comboBox->hidePopup();
        recWaveForm->working(key_val,buf,mod);
    }
}

void AAWidget::calc_aa_value (double * aa_val, double * aa_db, int * offset)
{
    int d;
    d = (int)data->recv_para_normal.ldata1_max - (int)data->recv_para_normal.ldata1_min ;      //最大值-最小值=幅值
    * offset = ( d - 1 / sqlcfg->get_para()->aaultra_sql.gain / AA_FACTOR ) / 100;
    * aa_val = (d - sqlcfg->get_para()->aaultra_sql.aa_offset * 100) * sqlcfg->get_para()->aaultra_sql.gain * AA_FACTOR;
    if(* aa_val < 1){
        * aa_val = 1;
    }
    * aa_db = 20 * log10 (* aa_val);
}

void AAWidget::fresh(bool f)
{
    int offset;
    double val,val_db;

//    if (aaultra_sql->mode == single) {
//        timer1->stop();
//        timer2->stop();
//    }

	calc_aa_value (&val, &val_db, &offset);


    if(db < int(val_db)){
        db = int(val_db);      //每秒的最大值
    }

    //记录并显示最大值
    if (max_db <val_db ) {
        max_db = val_db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
    }

    if(f){  //直接显示（1s一次）
        ui->label_val->setText(QString::number(val_db, 'f', 1));
        temp_db = val_db;
        //彩色显示
        if ( val_db >= aaultra_sql->high) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
        } else if (val_db >= aaultra_sql->low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }
        emit aa_log_data(val_db,0,0);

        yc_data_type temp_data;
        temp_data.f_val = val_db;
        yc_set_value(AA_amplitude, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(AA_severity, &temp_data, 0, NULL,0);
        temp_data.f_val = aaultra_sql->gain;
        yc_set_value(AA_gain, &temp_data, 0, NULL,0);
        temp_data.f_val = aaultra_sql->aa_offset;
        yc_set_value(AA_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = offset;
        yc_set_value(AA_biased_adv, &temp_data, 0, NULL,0);
    }
    else{   //条件显示
        if(qAbs(val_db-temp_db ) >= sqlcfg->get_para()->aaultra_sql.aa_step){
            ui->label_val->setText(QString::number(val_db, 'f', 1));
            if ( val_db > aaultra_sql->high) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            } else if (val_db >= aaultra_sql->low) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            } else {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
            }
            yc_data_type temp_data;
            temp_data.f_val = val_db;
            yc_set_value(AA_amplitude, &temp_data, 0, NULL,0);
        }
    }

    double max_val_range;
    if (val < 1) {
        max_val_range = 1;
        ui->label_range->setText("μV");                                                //range fresh
    } else if (val < 10) {
        max_val_range = 10;
        ui->label_range->setText("10μV");                                              //range fresh
    } else if (val < 100) {
        max_val_range = 100;
        ui->label_range->setText("100μV");                                              //range fresh
    } else if (val < 1000) {
        max_val_range = 1000;
        ui->label_range->setText("mV");                                                 //range fresh
    } else {
        max_val_range = 10000;
        ui->label_range->setText("10mV");                                               //range fresh
    }
    ui->progressBar->setValue(val * 100 / max_val_range);

//    ui->label_range->setText(QString("%1").arg(data->recv_para.ldata1_max * 4 * aaultra_sql->gain * AA_FACTOR));
}

void AAWidget::fresh_1()
{
    fresh(true);
    ui->qwtPlot->replot();
}

void AAWidget::fresh_2()
{
    fresh(false);
}

void AAWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
    qDebug()<<"AAUltrasonic max reset!";
}

void AAWidget::fresh_setting()
{
    if (aaultra_sql->mode == single) {
        ui->comboBox->setItemText(0,tr("检测模式\t[单次]"));
        timer1->setSingleShot(true);
        timer2->setSingleShot(true);
    } else {
        ui->comboBox->setItemText(0,tr("检测模式\t[连续]"));
        timer1->setSingleShot(false);
        timer2->setSingleShot(false);
    }

    ui->comboBox->setItemText(1,tr("增益调节\t[×%1]").arg(QString::number(aaultra_sql->gain, 'f', 1)) );
    ui->comboBox->setItemText(2,tr("音量调节\t[×%1]").arg(QString::number(aaultra_sql->vol)));
    ui->comboBox->setItemText(3,tr("黄色报警阈值\t[%1]dB").arg(QString::number(aaultra_sql->low)));
    ui->comboBox->setItemText(4,tr("红色报警阈值\t[%1]dB").arg(QString::number(aaultra_sql->high)));
    ui->comboBox->setItemText(5,tr("连续录波\t[%1]s").arg(aaultra_sql->time));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}


