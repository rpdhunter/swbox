#include "aewidget.h"
#include "ui_aewidget.h"

#define SETTING_NUM 6           //设置菜单条目数

AEWidget::AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::AEWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AEWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    ui->layout_main->parentWidget()->setGeometry(230, 40, 196, 141);

    historic_chart->chart_init(this, mode);
    prpd_chart->chart_init(this, mode);
    prps_chart->chart_init(this, mode);
    fly_chart->chart_init(this, mode);
    histogram_chart->chart_init(this, mode);
    spectra_chart->chart_init(this, mode);

    l_fun = new LChannelFunction(data, mode);
    fun = qobject_cast<BaseChannlFunction*>(l_fun);
    connect(l_fun, SIGNAL(fresh_100ms()), this, SLOT(fresh_100ms()));
    connect(l_fun, SIGNAL(fresh_gusty()), this, SLOT(fresh_gusty()));
    connect(l_fun, SIGNAL(fresh_1000ms()), this, SLOT(fresh_1000ms()));
    l_fun->channel_start();

    settingMenu = new SettingMunu(mode,this);
    recWaveForm->raise();
    fresh_setting();
}

AEWidget::~AEWidget()
{
    delete ui;
}

void AEWidget::fresh_1000ms()
{
    historic_chart->add_data(l_fun->val());
    ui->label_50Hz->setText(QString("%1μV").arg(l_fun->v_50Hz()));
    ui->label_100Hz->setText(QString("%1μV").arg(l_fun->v_100Hz()));
    ui->progressBar_50Hz->setValue(l_fun->v_50Hz());
    if(l_fun->v_50Hz() > ui->progressBar_50Hz->maximum() ){
        ui->progressBar_50Hz->setValue(ui->progressBar_50Hz->maximum());
    }
    ui->progressBar_100Hz->setValue(l_fun->v_100Hz());
    if(l_fun->v_100Hz() > ui->progressBar_100Hz->maximum() ){
        ui->progressBar_100Hz->setValue(ui->progressBar_100Hz->maximum());
    }
    fresh_gusty();
}

void AEWidget::fresh_gusty()
{
    ui->label_val->setText(QString::number(l_fun->val()) );
    if ( l_fun->val() >= l_fun->sql()->high) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
        emit beep(menu_index,2);        //蜂鸣器报警
    } else if (l_fun->val() >= l_fun->sql()->low) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
        emit beep(menu_index,1);
    } else {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    }

    ui->label_max->setText(tr("最大值: ") + QString::number(l_fun->max_val()) + "dB");
//    ui->label_pluse->setText(tr("脉冲数: ") + Common::secton_three(l_fun->pulse_cnt()) );//按三位分节法显示脉冲计数
//    ui->label_degree->setText(tr("严重度: ") + QString::number(l_fun->degree(), 'f', 2));

    emit send_log_data(l_fun->val(),l_fun->pulse_cnt(),l_fun->degree(),l_fun->is_current(),"NOISE");
    l_fun->save_rdb_data();
}

void AEWidget::fresh_100ms()
{


    prpd_chart->add_data(l_fun->pulse_100ms());
    prps_chart->add_data(l_fun->pulse_100ms());
    histogram_chart->add_data(l_fun->pulse_100ms());
    fly_chart->add_data(l_fun->pulse_100ms());
    spectra_chart->add_data(l_fun->spectra_100ms());
    l_fun->clear_100ms();
}

void AEWidget::do_key_ok()
{
    l_fun->save_sql();
    l_fun->channel_start();

    switch (key_val->grade.val2) {
    case 1:         //启动/停止测试
        break;
    case 2:         //测量值重置
        data_reset();
        break;
    case 5:         //连续录波
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        emit startRecWave(mode, l_fun->sql()->rec_time);        //发送录波信号
        manual = true;
        emit show_indicator(true);
        return;
    case 6:         //展开次级设置菜单
        key_val->grade.val2 = SUB_MENU_NUM_BASE;
        return;
    default:
        break;
    }

    ChannelWidget::do_key_ok();
}

void AEWidget::do_key_cancel()
{
    if(key_val->grade.val2 >= SUB_MENU_NUM_BASE){
        key_val->grade.val2 = SETTING_NUM;
    }
    else{
        ChannelWidget::do_key_cancel();
    }
}

void AEWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    if(key_val->grade.val2 >= SUB_MENU_NUM_BASE){
        Common::change_index(key_val->grade.val2,d,SUB_MENU_NUM_BASE + 6, SUB_MENU_NUM_BASE);       //控制次级菜单
    }
    else{
        Common::change_index(key_val->grade.val2,d,SETTING_NUM,1);          //控制主菜单
    }
}

void AEWidget::do_key_left_right(int d)
{
    switch (key_val->grade.val2) {
    case 3:         //修改图形显示
        l_fun->change_chart(d);
        break;
    case 4:         //修改音量
        l_fun->change_vol(d);
        break;
    case 5:         //连续录波时长
        l_fun->change_rec_cons_time(d);
        break;
    case SETTING_NUM:                   //展开次级设置菜单
        key_val->grade.val2 = SUB_MENU_NUM_BASE;
        return;
    default:
        break;
    }
    ChannelWidget::do_key_left_right(d);
}

void AEWidget::data_reset()
{
    prpd_chart->reset_data();
    histogram_chart->reset_data();
    fly_chart->reset_data();

    l_fun->click_reset();
    qDebug()<<"AE data reset!";
}
