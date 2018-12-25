#include "aawidget.h"
#include "ui_aawidget.h"
#include <QLineEdit>

#define SETTING_NUM 9           //设置菜单条目数
#define VALUE_MAX 60

AAWidget::AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::AAWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AAWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    recWaveForm->raise();
    reload(-1);
    chart_ini();
    reload(menu_index);


    if(aaultra_sql->camera){
        socket = new Socket;
        socket->open_camera();
        decode = new CameraDecode;
        timer_5000ms = new QTimer();
        timer_5000ms->setInterval(5000);
        timer_5000ms->setSingleShot(true);
        connect(timer_5000ms, SIGNAL(timeout()), this, SLOT(check_camera()));
        connect(socket, SIGNAL(s_camera_packet(QByteArray,int)), decode, SLOT(getOnePacket(QByteArray, int)));      //socket-->解码器
        connect(socket, SIGNAL(s_camera_packet(QByteArray,int)), this, SLOT(change_camera_status()));
        connect(decode,SIGNAL(sigGetOneFrame(QImage)),this,SLOT(slotGetOneFrame(QImage)));                          //解码器-->主界面
        connect(decode,SIGNAL(read_done(int)), socket, SLOT(read_done(int)));
        system("/root/camera/camera &");

        decode->camera_init();      //做一下连接初始化动作（打开AP，打开camera外部程序）
        decode->start();
        camera_fullsize = false;
        camera_hasdata = false;
    }
}

AAWidget::~AAWidget()
{
    delete ui;
}

void AAWidget::save_channel()
{
    PRPDReset();
    ChannelWidget::save_channel();
}

void AAWidget::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();
    if(mode == AA1){
        aaultra_sql = &sql_para.aa1_sql;
        ae_pulse_data = &data->recv_para_envelope1;
    }
    else if(mode == AA2){
        aaultra_sql = &sql_para.aa2_sql;
        ae_pulse_data = &data->recv_para_envelope2;
    }

    //构造函数中计时器不启动
    if(index == menu_index){
        if(!timer_100ms->isActive()){
            timer_100ms->start();
        }
        if(!timer_1000ms->isActive()){
            timer_1000ms->start();
        }

        if(mode == AA1){
            data->set_send_para (sp_vol_l1, aaultra_sql->vol);
            data->set_send_para (sp_aa_record_play, 0);        //耳机送1通道
        }
        else if(mode == AA2){
            data->set_send_para (sp_vol_l2, aaultra_sql->vol);
            data->set_send_para (sp_aa_record_play, 2);        //耳机送2通道
        }
        data->set_send_para(sp_auto_rec, 0);        //关闭自动录波
        ae_pulse_data->readComplete = 1;        //读取完成标志
        fresh_setting();
    }
}

void AAWidget::chart_ini()
{
    //barchart
    Common::set_barchart_style(ui->qwtPlot, VALUE_MAX);
    d_BarChart = new BarChart(ui->qwtPlot, &db, &aaultra_sql->high, &aaultra_sql->low);
    d_BarChart->resize(200, 140);

    //PRPD
    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;
    Common::set_PRPD_style(plot_PRPD,d_PRPD,VALUE_MAX,PRPD_single);
    PRPDReset();

    //频谱图
    plot_Spectra = new QwtPlot(ui->widget);
    plot_Spectra->resize(200, 140);
    d_Spectra = new QwtPlotHistogram;
    Common::set_Spectra_style(plot_Spectra,d_Spectra,0,4,0,30,"");
}

void AAWidget::do_key_ok()
{
    sqlcfg->sql_save(&sql_para);
    reload(menu_index);

    switch (key_val->grade.val2) {
    case 8:             //连续录波
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        emit startRecWave(mode, aaultra_sql->time);        //发送录波信号
        emit show_indicator(true);
        return;
    case 9:             //测量值重置
        maxReset(ui->label_max);
        PRPDReset();
        break;
    default:
        break;
    }

        ChannelWidget::do_key_ok();

    if(aaultra_sql->chart == Camera){
        if(camera_hasdata == false){
            socket->open_camera();
            return;
        }

        if(camera_fullsize == false && mImage.size().width() > 0){       //进入全屏模式
            camera_fullsize = true;
            return;
        }

        if(camera_fullsize == true){
            //插入保存谱图代码
            save_camera_picture();
            return;
        }
    }

}

void AAWidget::do_key_cancel()
{
    if(camera_fullsize == true && aaultra_sql->chart == Camera){    //取消全屏
        camera_fullsize = false;
        this->update();
        return;
    }
    ChannelWidget::do_key_cancel();
}

void AAWidget::do_key_up_down(int d)
{
//    if(camera_fullsize == true && aaultra_sql->chart == Camera){
//        //上下移动焦点框
//        return;
//    }

    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2, d, SETTING_NUM, 1);
}

void AAWidget::do_key_left_right(int d)
{
    if(camera_fullsize == true && aaultra_sql->chart == Camera){
        //左右移动焦点框
        return;
    }

    QList<int> list;
    switch (key_val->grade.val2) {
    case 1:
        aaultra_sql->mode = !aaultra_sql->mode;
        break;
    case 2:
        if(aaultra_sql->camera){
            list << BASIC << PRPD << Spectra << Camera;
        }
        else{
            list << BASIC << PRPD << Spectra;
        }
        Common::change_index(aaultra_sql->chart, d, list);
        break;
    case 3:
        if( (aaultra_sql->gain<9.95 && d>0) || (aaultra_sql->gain<10.15 && d<0) ){
            Common::change_index(aaultra_sql->gain, d * 0.1, 100, 0.1 );
        }
        else{
            Common::change_index(aaultra_sql->gain, d * 10, 100, 0.1 );
        }
        break;
    case 4:
        Common::change_index(aaultra_sql->vol, d, VOL_MAX, VOL_MIN );
        break;
    case 5:
        Common::change_index(aaultra_sql->low, d, aaultra_sql->high, 0 );
        break;
    case 6:
        Common::change_index(aaultra_sql->high, d, 60, aaultra_sql->low );
        break;
    case 7:
        aaultra_sql->fpga_threshold += Common::code_value(1,mode) * d;
        break;
    case 8:
        Common::change_index(aaultra_sql->time, d, TIME_MAX, TIME_MIN );
        break;
    default:
        break;
    }
}

void AAWidget::fresh(bool f)
{

    int offset;
    double val,val_db;

    Compute::calc_aa_value(data,mode,aaultra_sql,&val, &val_db, &offset);

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
            emit beep(menu_index, 2);        //蜂鸣器报警
        } else if (val_db >= aaultra_sql->low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            emit beep(menu_index, 1);
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }
        //        #if 0

        int is_current = 0;
        if((int)key_val->grade.val0 == menu_index){
            is_current = 1;
        }

        if(mode == AA1){
            Common::rdb_set_yc_value(AA1_amplitude_yc,val_db,is_current);
//            Common::rdb_set_yc_value(AA1_severity,0,is_current);
//            Common::rdb_set_yc_value(AA1_gain,aaultra_sql->gain,is_current);
//            Common::rdb_set_yc_value(AA1_biased,aaultra_sql->offset,is_current);
            Common::rdb_set_yc_value(AA1_noise_biased_adv_yc,offset,is_current);
        }
        else if(mode == AA2){
            Common::rdb_set_yc_value(AA2_amplitude_yc,val_db,is_current);
//            Common::rdb_set_yc_value(AA2_severity,0,is_current);
//            Common::rdb_set_yc_value(AA2_gain,aaultra_sql->gain,is_current);
//            Common::rdb_set_yc_value(AA2_biased,aaultra_sql->offset,is_current);
            Common::rdb_set_yc_value(AA2_noise_biased_adv_yc,offset,is_current);
        }
        emit send_log_data(val_db,0,0,is_current,"NOISE");
        //        #endif
    }
    else{   //条件显示
        if(qAbs(val_db-temp_db ) >= aaultra_sql->step){
            ui->label_val->setText(QString::number(val_db, 'f', 1));
            if ( val_db > aaultra_sql->high) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
                emit beep(menu_index, 2);        //蜂鸣器报警
            } else if (val_db >= aaultra_sql->low) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
                emit beep(menu_index, 1);
            } else {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
            }
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


}

void AAWidget::save_camera_picture()
{
    Common::mk_dir(DIR_CAMERASHOTS);
    QString str = QString(DIR_CAMERASHOTS"/camerashot_%1.png").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") );
    int f = mImage.save(str , "PNG");
    qDebug()<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>save pic :"<<f;

    qDebug()<<"save camera picture";
    emit update_statusBar(tr("摄像头截屏已保存"));
}

void AAWidget::slotGetOneFrame(QImage img)
{
    mImage = img.copy();
    //调用update将执行 paintEvent函数
    update();
}

void AAWidget::change_camera_status()
{
    timer_5000ms->start();
    camera_hasdata = true;
}

void AAWidget::check_camera()
{
    camera_hasdata = false;
}

void AAWidget::paintEvent(QPaintEvent *)
{
    QRect rect(ui->widget->x(), ui->widget->y(), ui->widget->width(), ui->widget->height() );
    QPainter painter(this);

    ui->label_val->show();
    ui->label_3->show();
    ui->label_max->show();
    ui->comboBox->show();
    ui->progressBar->show();

    if(aaultra_sql->chart != Camera){
        painter.setBrush(Qt::transparent);
        painter.setPen(Qt::transparent);
        painter.drawRect(rect);
        return;
    }

    if (camera_hasdata == false){

        painter.setPen(Qt::yellow);
        painter.drawText(rect,tr("摄像头未连接，按OK键尝试连接摄像头"));
        return;
    }

    if(camera_fullsize == true && aaultra_sql->chart == Camera){        //摄像头大图显示
        ui->label_val->hide();
        ui->label_3->hide();
        ui->label_max->hide();
        ui->comboBox->hide();
        ui->progressBar->hide();
//        QImage img = mImage.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//        int x = this->width() - img.width();
//        int y = this->height() - img.height();

//        x /= 2;
//        y /= 2;

//        painter.drawImage(QPoint(x,y),img);
        QRect rect_full = this->rect().adjusted(80,0,-80,0);      //修正之后是258×192，约为4×3
        QRect rect_red = rect_full.adjusted(100,60,-100,-60);
        painter.drawImage(rect_full,mImage);

        painter.setPen(Qt::red);
        painter.drawRect(rect_red);
        painter.drawText(rect_red.bottomRight(),ui->label_val->text().append("dbμV"));
//        painter.drawRect(x+img.width()/4, y+img.height()/4, img.width()/2,img.height()/2);
//        painter.drawText(x+img.width()/4, y+img.height()*3/4 + 12,ui->label_val->text().append("dbμV"));
    }
    else if(camera_fullsize == false && aaultra_sql->chart == Camera){  //摄像头小图显示
        painter.drawImage(rect,mImage);
    }

}

void AAWidget::fresh_1000ms()
{
    fresh(true);
    d_BarChart->fresh();
    ui->qwtPlot->replot();
}

void AAWidget::fresh_100ms()
{
    fresh(false);
    pulse_100ms = Common::calc_pulse_list(ae_datalist,aaultra_sql->fpga_threshold);
    do_Spectra_compute();

    int x,y, time;
    double _y;
    for(int i=0; i<pulse_100ms.count(); i++){
        time = pulse_100ms.at(i).x() * 320000 / 128 + ae_timelist.first();  //时标
//        qDebug()<<"ae_timelist clear"<<ae_timelist;
//        ae_timelist.clear();
        x = Common::time_to_phase(time );              //时标(待定)
        x = (x/2)*2;            //去掉奇数项
        _y = Common::physical_value(pulse_100ms.at(i).y(),mode);         //强度
        y = (int)20*log(qAbs(_y) );


        if(x<360 && x>=0 && y<=60 &&y>=-60){
            QwtPoint3D p0(x,y,map[x][y+60]);
            map[x][y+60]++;
            QwtPoint3D p1(x,y,map[x][y+60]);
            if(map[x][y+60]>1){
                int n = prpd_samples.indexOf(p0);
                prpd_samples[n] = p1;
            }
            else{
                prpd_samples.append(p1);
            }
        }
    }

    d_PRPD->setSamples(prpd_samples);
    plot_PRPD->replot();
    ae_timelist.clear();
}

void AAWidget::PRPDReset()
{
    for(int i=0;i<360;i++){
        for(int j=0;j<121;j++){
            map[i][j]=0;
        }
    }
    emit send_PRPD_data(prpd_samples);
    prpd_samples.clear();
}

void AAWidget::fresh_setting()
{
    if (aaultra_sql->mode == single) {
        ui->comboBox->setItemText(0,tr("检测模式\t[单次]"));
        timer_100ms->setSingleShot(true);
        timer_1000ms->setSingleShot(true);
    } else {
        ui->comboBox->setItemText(0,tr("检测模式\t[连续]"));
        timer_100ms->setSingleShot(false);
        timer_1000ms->setSingleShot(false);
    }

    ui->qwtPlot->hide();
    plot_PRPD->hide();
    plot_Spectra->hide();

    switch (aaultra_sql->chart) {
    case BASIC:
        ui->widget->show();
        ui->qwtPlot->show();
        ui->comboBox->setItemText(1,tr("图形显示 \t[时序图]"));
        break;
    case PRPD:
        ui->widget->show();
        plot_PRPD->show();
        ui->comboBox->setItemText(1,tr("图形显示\t[PRPD]"));
        break;
    case Spectra:
        ui->widget->show();
        plot_Spectra->show();
        ui->comboBox->setItemText(1,tr("图形显示  \t[频谱图]"));
        break;
    case Camera:
        ui->widget->hide();
        ui->comboBox->setItemText(1,tr("图形显示  \t[摄像头]"));
        break;
    default:
        break;
    }

    ui->comboBox->setItemText(2,tr("增益调节\t[×%1]").arg(QString::number(aaultra_sql->gain, 'f', 1)) );
    ui->comboBox->setItemText(3,tr("音量调节\t[×%1]").arg(QString::number(aaultra_sql->vol)));
    ui->comboBox->setItemText(4,tr("黄色报警阈值\t[%1]dB").arg(QString::number(aaultra_sql->low)));
    ui->comboBox->setItemText(5,tr("红色报警阈值\t[%1]dB").arg(QString::number(aaultra_sql->high)));
    ui->comboBox->setItemText(6,tr("脉冲触发\t[%1]mV").arg(QString::number((int)Common::physical_value(aaultra_sql->fpga_threshold,mode) )));
    ui->comboBox->setItemText(7,tr("连续录波\t[%1]s").arg(aaultra_sql->time));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0 && isBusy != true) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    emit fresh_parent();
}




