#include "aawidget.h"
#include "ui_aawidget.h"

AAWidget::AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::AAWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AAWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    ui->layout_main->parentWidget()->setGeometry(230, 40, 200, 141);

    historic_chart->chart_init(this, mode);
    prpd_chart->chart_init(this, mode);
    prps_chart->chart_init(this, mode);
    spectra_chart->chart_init(this, mode);
    camera_chart->chart_init(this, mode);

    l_fun = new LChannelFunction(data, mode);
    fun = qobject_cast<BaseChannlFunction*>(l_fun);
    connect(l_fun, SIGNAL(fresh_100ms()), this, SLOT(fresh_100ms()));
    connect(l_fun, SIGNAL(fresh_gusty()), this, SLOT(fresh_gusty()));
    connect(l_fun, SIGNAL(fresh_1000ms()), this, SLOT(fresh_1000ms()));
    l_fun->channel_start();

    ui->progressBar_50Hz->setMaximum(FREQ50_MAX);
    ui->progressBar_100Hz->setMaximum(FREQ100_MAX);
    ui->progressBar_effective->setMaximum(EFFECTIVE_MAX);
    ui->progressBar_peak->setMaximum(PEAK_MAX);

    if(fun->sql()->camera){
        sqlcfg->get_global()->wifi_working = true;
    }

    settingMenu = new SettingMunu(mode,this);
    recWaveForm->raise();
    fresh_setting();
}

AAWidget::~AAWidget()
{
    delete ui;
}

void AAWidget::do_key_ok()
{
    if(key_val->grade.val2 == settingMenu->main_menu_num()){        //展开次级设置菜单
        key_val->grade.val2 = SUB_MENU_NUM_BASE;
        return;
    }

    l_fun->save_sql();

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
    case 5:         //连续录波
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        emit startRecWave(mode, l_fun->sql()->rec_time);        //发送录波信号
        manual = true;
        emit show_indicator(true);
        return;
    default:
        break;
    }

    ChannelWidget::do_key_ok();

    if(l_fun->sql()->chart == Camera){      //摄像头操作(额外的键盘逻辑)
        camera_chart->do_key_ok();
        if(camera_chart->isFullScreen()){
            settingMenu->hide();
        }
        key_val->grade.val1 = 1;
    }
}

void AAWidget::do_key_cancel()
{
    if(camera_chart->isFullScreen()){    //取消全屏
        camera_chart->do_key_cancel();
        settingMenu->show();
        key_val->grade.val1 = 0;
    }
    else{
        ChannelWidget::do_key_cancel();
    }
}

void AAWidget::do_key_up_down(int d)
{
    if(camera_chart->isFullScreen()){
        //上下移动焦点框
        return;
    }

    ChannelWidget::do_key_up_down(d);
}

void AAWidget::do_key_left_right(int d)
{
    if(camera_chart->isFullScreen()){
        //左右移动焦点框
        return;
    }

    if(key_val->grade.val2 == settingMenu->main_menu_num()){        //展开次级设置菜单
        key_val->grade.val2 = SUB_MENU_NUM_BASE;
        return;
    }

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
    default:
        break;
    }
    ChannelWidget::do_key_left_right(d);
}

void AAWidget::data_reset()
{
    prpd_chart->reset_data();
    l_fun->click_reset();

    ui->label_50Hz->setText(QString("%1/%2μV").arg(QString::number(0, 'f', 2)).arg(FREQ50_MAX));
    ui->label_100Hz->setText(QString("%1/%2μV").arg(QString::number(0, 'f', 2)).arg(FREQ100_MAX));
    ui->label_effective->setText(QString("%1/%2μV").arg(QString::number(0, 'f', 2)).arg(EFFECTIVE_MAX));
    ui->label_peak->setText(QString("%1/%2μV").arg(QString::number(0, 'f', 2)).arg(PEAK_MAX));
    ui->progressBar_50Hz->setValue(0);
    ui->progressBar_100Hz->setValue(0);
    ui->progressBar_effective->setValue(0);
    ui->progressBar_peak->setValue(0);
    ui->label_val->setText("-20" );
    ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    ui->label_max->setText(tr("最大值: ") + "-20dB");

    qDebug()<<"AA data reset!";
}

void AAWidget::fresh_1000ms()
{
    if(fun->is_in_test_window()){
        historic_chart->add_data(l_fun->val());
        ui->label_50Hz->setText(QString("%1/%2μV").arg(QString::number(l_fun->v_50Hz(), 'f', 2)).arg(FREQ50_MAX));
        ui->label_100Hz->setText(QString("%1/%2μV").arg(QString::number(l_fun->v_100Hz(), 'f', 2)).arg(FREQ100_MAX));
        ui->label_effective->setText(QString("%1/%2μV").arg(QString::number(l_fun->v_effective(), 'f', 2)).arg(EFFECTIVE_MAX));
        ui->label_peak->setText(QString("%1/%2μV").arg(QString::number(l_fun->v_peak(), 'f', 2)).arg(PEAK_MAX));
        ui->progressBar_50Hz->setValue(l_fun->v_50Hz());
        if(l_fun->v_50Hz() > ui->progressBar_50Hz->maximum() ){
            ui->progressBar_50Hz->setValue(ui->progressBar_50Hz->maximum());
        }
        ui->progressBar_100Hz->setValue(l_fun->v_100Hz());
        if(l_fun->v_100Hz() > ui->progressBar_100Hz->maximum() ){
            ui->progressBar_100Hz->setValue(ui->progressBar_100Hz->maximum());
        }
        ui->progressBar_effective->setValue(l_fun->v_effective());
        if(l_fun->v_effective() > ui->progressBar_effective->maximum() ){
            ui->progressBar_effective->setValue(ui->progressBar_effective->maximum());
        }
        ui->progressBar_peak->setValue(l_fun->v_peak());
        if(l_fun->v_peak() > ui->progressBar_peak->maximum() ){
            ui->progressBar_peak->setValue(ui->progressBar_peak->maximum());
        }

        fresh_gusty();
    }
    l_fun->clear_1000ms();
}

void AAWidget::fresh_gusty()
{
    if(fun->is_in_test_window()){
        ui->label_val->setText(QString::number(l_fun->val()) );
        if ( l_fun->val() >= l_fun->sql()->high) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            if(fun->is_current()){
                emit beep(menu_index,2);        //蜂鸣器报警
            }
        } else if (l_fun->val() >= l_fun->sql()->low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            if(fun->is_current()){
                emit beep(menu_index,1);
            }
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }

        ui->label_max->setText(tr("最大值: ") + QString::number(l_fun->max_val()) + "dB");

        emit send_log_data(l_fun->val(),l_fun->pulse_cnt(),l_fun->degree(),l_fun->is_current(),"NOISE");
        l_fun->save_rdb_data();
    }
}

void AAWidget::fresh_100ms()
{
    if(fun->is_in_test_window()){
        prpd_chart->add_data(l_fun->pulse_100ms());
        spectra_chart->add_data(l_fun->spectra_100ms());
    }
    prps_chart->add_data(l_fun->pulse_100ms());    

    l_fun->clear_100ms();
}
