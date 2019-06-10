#include "uhfwidget.h"
#include "ui_uhfwidget.h"

UHFWidget::UHFWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::UHFWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("UHFWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    ui->layout_val->parentWidget()->setGeometry(269, 40, 131, 81);
    ui->layout_max->parentWidget()->setGeometry(280, 112, 122, 71);

    historic_chart->chart_init(this, mode);
    prps_chart->chart_init(this, mode);
    prpd_chart->chart_init(this, mode);
    histogram_chart->chart_init(this, mode);

    h_fun = new HChannelFunction(data, mode);
    fun = qobject_cast<BaseChannlFunction*>(h_fun);
    connect(h_fun, SIGNAL(fresh_100ms()), this, SLOT(fresh_100ms()));
    connect(h_fun, SIGNAL(fresh_1000ms()), this, SLOT(fresh_1000ms()));
    h_fun->channel_start();

    settingMenu = new SettingMunu(mode,this);
    recWaveForm->raise();
    fresh_setting();

//    emit startRecWave(mode,0);      //开机录一次
}

UHFWidget::~UHFWidget()
{
    delete ui;
}

void UHFWidget::fresh_100ms()
{
    if(fun->is_in_test_window()){
        prpd_chart->add_data(h_fun->pulse_100ms());
        histogram_chart->add_data(h_fun->pulse_100ms());
    }
    prps_chart->add_data(h_fun->pulse_100ms());

    h_fun->clear_100ms();
}

void UHFWidget::fresh_1000ms()
{
    if(fun->is_in_test_window()){
        historic_chart->add_data(h_fun->val());

        ui->label_val->setText(QString::number(h_fun->val()) );
        if ( h_fun->val() >= h_fun->sql()->high) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            if(fun->is_current()){
                emit beep(menu_index,2);        //蜂鸣器报警
            }
        } else if (h_fun->val() >= h_fun->sql()->low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            if(fun->is_current()){
                emit beep(menu_index,1);
            }
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }

        ui->label_max->setText(tr("最大值: ") + QString::number(h_fun->max_val()) + "dB");
        ui->label_pluse->setText(tr("脉冲数: ") + Common::secton_three(h_fun->pulse_cnt()) );//按三位分节法显示脉冲计数
        ui->label_degree->setText(tr("严重度: ") + QString::number(h_fun->degree(), 'f', 2));

        emit send_log_data(h_fun->val(),h_fun->pulse_cnt(),h_fun->degree(),h_fun->is_current(),"NOISE");
        h_fun->save_rdb_data();
    }
    h_fun->clear_1000ms();
}

void UHFWidget::do_key_ok()
{
    if(key_val->grade.val2 == settingMenu->main_menu_num()){        //展开次级设置菜单
        key_val->grade.val2 = SUB_MENU_NUM_BASE;
        return;
    }

    h_fun->save_sql();
//    h_fun->channel_start();

    switch (key_val->grade.val2) {
    case 0:
        if(fun->sql()->mode == single){         //在单次测量模式下,开启测量窗口
            fun->start_single_test();
            qDebug()<<"single !";
        }
        break;
    case 1:         //启动/停止测试
        fun->toggle_test_status();
        break;
    case 2:         //测量值重置
        data_reset();
        break;
    case 4:         //单次录波
        emit startRecWave(mode,0);
        manual = true;
        break;
    case 5:         //连续录波
        emit startRecWave(mode,h_fun->sql()->rec_time);     //开始连续录波
        key_val->grade.val1 = 1;
        manual = true;
        emit show_indicator(true);
        break;
    default:
        break;
    }

    ChannelWidget::do_key_ok();
}

void UHFWidget::do_key_left_right(int d)
{
    if(key_val->grade.val2 == settingMenu->main_menu_num()){        //展开次级设置菜单
        key_val->grade.val2 = SUB_MENU_NUM_BASE;
        return;
    }

    switch (key_val->grade.val2) {
    case 3:         //修改图形显示
        h_fun->change_chart(d);
        break;
    case 5:         //连续录波时长
        h_fun->change_rec_cons_time(d);
        break;
    case 6:         //修改自动录波
        h_fun->toggle_auto_rec();
        break;
    default:
        break;
    }
    ChannelWidget::do_key_left_right(d);
}

void UHFWidget::data_reset()
{
    prpd_chart->reset_data();
    histogram_chart->reset_data();
    h_fun->click_reset();

    ui->label_val->setText(QString::number(0) );
    ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    ui->label_max->setText(tr("最大值: ") + QString::number(0) + "dB");
    ui->label_pluse->setText(tr("脉冲数: ") + Common::secton_three(0) );
    ui->label_degree->setText(tr("严重度: ") + QString::number(0, 'f', 2));
    qDebug()<<"TEV data reset!";
}
