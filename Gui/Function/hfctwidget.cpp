#include "hfctwidget.h"
#include "ui_hfctwidget.h"
#include "IO/File/testtools.h"

HFCTWidget::HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::HFCTWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("HFCTWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    ui->layout_val->parentWidget()->setGeometry(260, 40, 160, 81);
    ui->layout_max->parentWidget()->setGeometry(280, 112, 122, 71);

    historic_chart->chart_init(this, mode);
    prps_chart->chart_init(this, mode);
    prpd_chart->chart_init(this, mode);
    tf_chart->chart_init(this, mode);
    spectra_chart->chart_init(this, mode);

    h_fun = new HChannelFunction(data, mode);
    fun = qobject_cast<BaseChannlFunction*>(h_fun);
    connect(h_fun, SIGNAL(fresh_100ms()), this, SLOT(fresh_100ms()));
    connect(h_fun, SIGNAL(fresh_1000ms()), this, SLOT(fresh_1000ms()));
    h_fun->channel_start();

    settingMenu = new SettingMunu(mode,this);
    recWaveForm->raise();
    bpjudge = NULL;
    judge_multimachine();

    fresh_setting();
//    emit startRecWave(mode,0);      //开机录一次
}

HFCTWidget::~HFCTWidget()
{
    delete ui;
}

void HFCTWidget::fresh_100ms()
{
    if(fun->is_in_test_window()){
        prpd_chart->add_data(h_fun->pulse_100ms());
        tf_chart->add_data(h_fun->pclist_100ms());
        spectra_chart->add_data(h_fun->spectra_100ms());
    }
    prps_chart->add_data(h_fun->pulse_100ms());

    h_fun->clear_100ms();
}

void HFCTWidget::fresh_1000ms()
{
    if(fun->is_in_test_window()){
        historic_chart->add_data(h_fun->val() );

        ui->label_val->setText(QString("%1").arg(h_fun->val()));
        ui->label_pluse->setText(tr("脉冲数: ") + Common::secton_three(h_fun->pulse_cnt()) );//按三位分节法显示脉冲计数
        ui->label_degree->setText(tr("严重度: %1").arg(h_fun->degree()));

        if(h_fun->sql()->mode_recognition){
            if(bpjudge == NULL){
                bpjudge = new BpJudge;
            }
    //        ui->label_judge->setText(tr("模式判断: %1").arg(bpjudge->judge(prpd_chart->samples())));
            ui->label_judge->setText(tr("模式判断: %1").arg(bpjudge->add_data(fun->pulse_1000ms())));
            ui->label_judge->show();
        }
        else{
            ui->label_judge->hide();
        }

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

        ui->label_max->setText(tr("最大值: ") + QString::number(h_fun->max_val()) + "pC");

        emit send_log_data(h_fun->val(),h_fun->pulse_cnt(),h_fun->degree(),h_fun->is_current(), "NOISE");

        h_fun->save_rdb_data();

        do_multimachine();          //作为客户端贡献第三通道数据
    }

    h_fun->clear_1000ms();
}

void HFCTWidget::do_key_ok()
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
        return;
    default:
        break;
    }

    ChannelWidget::do_key_ok();
}

void HFCTWidget::do_key_left_right(int d)
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
    case 7:         //修改高通滤波
        h_fun->change_HF(d);
        break;
    case 8:         //修改低通滤波
        h_fun->change_LF(d);
        break;
    case 9:         //修改单位
        h_fun->toggle_units();
        break;
    default:
        break;
    }
    ChannelWidget::do_key_left_right(d);
}

void HFCTWidget::data_reset()
{
    prpd_chart->reset_data();
    tf_chart->reset_data();    
    h_fun->click_reset();

    ui->label_val->setText(QString("%1").arg(0));
    ui->label_pluse->setText(tr("脉冲数: ") + Common::secton_three(0) );//按三位分节法显示脉冲计数
    ui->label_degree->setText(tr("严重度: %1").arg(0));
    ui->label_judge->setText(tr("模式判断: "));
    ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    ui->label_max->setText(tr("最大值: ") + QString::number(0) + "pC");

    qDebug()<<"HFCT data reset!";
}

void HFCTWidget::judge_multimachine()
{
    qDebug()<<"judge_multimachine";
    switch (sqlcfg->get_para()->multimachine_mode) {
    case multimachine_server:                           //服务器,建立socket server
        system("ifconfig eth0 192.168.20.1 up ");       //修改服务器ip地址
        server = new MultiMachineServer(this);
        break;
    case multimachine_client1:                          //客户端1,数据发送端
        system("ifconfig eth0 192.168.20.17 up ");      //修改修改客户机1ip地址
        client = new MultiMachineClient(this);
        break;
    case multimachine_client2:                          //客户端2,数据发送端
        system("ifconfig eth0 192.168.20.18 up ");      //修改客户机2ip地址
        client = new MultiMachineClient(this);
        break;
    default:
        break;
    }
}

void HFCTWidget::do_multimachine()
{
    if(sqlcfg->get_para()->multimachine_mode == multimachine_client1 || sqlcfg->get_para()->multimachine_mode == multimachine_client2){  //客户端,需要发送数据
        QByteArray block;                               //用于暂存我们要发送的数据
        QDataStream out(&block,QIODevice::ReadWrite);
        out.setVersion(QDataStream::Qt_5_9);
        switch (sqlcfg->get_para()->multimachine_mode) {
        case multimachine_client1:
            switch (mode) {
            case HFCT1:
                out << 3;                                //通道
                break;
            case HFCT2:
                out << 4;                                //通道
                break;
            default:
                break;
            }
            break;
        case multimachine_client2:
            switch (mode) {
            case HFCT1:
                out << 5;                                //通道
                break;
            case HFCT2:
                out << 6;                                //通道
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        out << fun->val() << fun->pulse_cnt();      //幅值,脉冲
        foreach (QPoint P, fun->pulse_1000ms()) {   //1s的脉冲
            out << P;
        }
        client->write(block);
    }
}


