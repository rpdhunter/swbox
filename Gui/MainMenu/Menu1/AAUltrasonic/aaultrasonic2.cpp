#include "aaultrasonic2.h"


AAUltrasonic2::AAUltrasonic2(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    data = g_data;

    groupNum = -1;

    temp_db = 0;

    /* get sql para */
    //读取配置信息

    sql_para = sqlcfg->get_para();
    aaultra_sql = new AAULTRA_SQL;
    memcpy(aaultra_sql, &sql_para->aaultra_sql, sizeof(AAULTRA_SQL));

    //开机设置一下音量
    data->send_para.aa_vol.flag = true;
    data->send_para.aa_vol.rval = sql_para->aaultra_sql.vol;
//    qDebug()<<"data->send_para.aa_vol.rval:"<<data->send_para.aa_vol.rval;

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("AAUltrasonic2 {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");


    /* view barchart */
    plot = new QwtPlot(this);
    plot->resize(170, 130);
    plot->move(30, 30);
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 13);
    plot->setAxisScale(QwtPlot::yLeft, 0, 60, 20);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    //plot->setAxisMaxMajor(QwtPlot::yLeft, 2);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    plot->plotLayout()->setAlignCanvasToScales(true);

    /* display */
    curve = new QwtPlotCurve();
    curve->setPen(QPen(Qt::yellow, 0, Qt::SolidLine, Qt::RoundCap));
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve->attach(plot);


    aaVal_lab = new QLabel(this);
    aaVal_lab->resize(150, 40);
    aaVal_lab->move(plot->x()+plot->width()+20, 50);
    aaVal_lab->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:52px;color:white}");
//    aaVal_lab->setAlignment(Qt::AlignCenter);

    aaDb_lab = new QLabel(this);
    aaDb_lab->resize(50, 20);
    aaDb_lab->move(aaVal_lab->x() + aaVal_lab->width(), aaVal_lab->y() + 20);
    aaDb_lab->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:20px;color:white}");
    aaDb_lab->setAlignment(Qt::AlignCenter);
    aaDb_lab->setText("dBμV");

    aaValMax_lab = new QLabel(this);
    aaValMax_lab->resize(90, 20);
    aaValMax_lab->move(aaVal_lab->x()+10, aaVal_lab->y()+aaVal_lab->height()+20);
    aaValMax_lab->setStyleSheet("QLabel {color:white;}");
    aaValMax_lab->setAlignment(Qt::AlignVCenter);


    progbar = new QProgressBar(this);
    progbar->move(aaValMax_lab->x(), aaValMax_lab->y()+aaValMax_lab->height()+20);
    progbar->resize(150, 8);
    progbar->setTextVisible(false);                                            //no show percent
    progbar->setStyleSheet("QProgressBar{border:1px solid gray;"
                            "background:transparent;}"
                            "QProgressBar::chunk{"
                            "background-color:skyblue;}");
    progbar->setRange(0, 100);
    progbar->setValue(0);

    lab_range = new QLabel(this);
    lab_range->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab_range->resize(40, 10);
    lab_range->move(progbar->x() +progbar->width() + 8, progbar->y());
    lab_range->setText("0mV");



    /* set para */
    set_lab = new QLabel(this);
    set_lab->resize(161, 33);
    set_lab->move(1, 1);
    set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para0.png);}");

    /* set para text */
    set_txt_lab = new QLabel(set_lab);
    set_txt_lab->resize(88, 19);
    set_txt_lab->move(10, 0);
    set_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    set_txt_lab->setStyleSheet("QLabel {color:gray;}");
    set_txt_lab->setAlignment(Qt::AlignCenter);
    set_txt_lab->setText(tr("参  数  设  置"));

    /* set para menu list */
    set_child0_lab = new QLabel(this);
    set_child0_lab->resize(147, 25);
    set_child0_lab->move(set_lab->x(), set_lab->y() + 21);
    set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child0_lab->setAlignment(Qt::AlignVCenter);
    set_child0_lab->setText(tr("  检测模式"));

    child0_val_lab = new QLabel(set_child0_lab);
    child0_val_lab->resize(47, 25);
    child0_val_lab->move(100, 0);
    child0_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child0_val_lab->setStyleSheet("QLabel {color:gray;}");
    child0_val_lab->setAlignment(Qt::AlignVCenter);
    if (sql_para->aaultra_sql.mode == signal) {
        child0_val_lab->setText(tr("[单次]"));
    } else {
        child0_val_lab->setText(tr("[连续]"));
    }

    set_child1_lab = new QLabel(this);
    set_child1_lab->resize(147, 25);
    set_child1_lab->move(set_lab->x(), set_child0_lab->y() + set_child0_lab->height());
    set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child1_lab->setAlignment(Qt::AlignVCenter);
    set_child1_lab->setText(tr("  增益调节"));

    child1_val_lab = new QLabel(set_child1_lab);
    child1_val_lab->resize(47, 25);
    child1_val_lab->move(100, 0);
    child1_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child1_val_lab->setStyleSheet("QLabel {color:gray;}");
    child1_val_lab->setAlignment(Qt::AlignVCenter);
    child1_val_lab->setText("[x" + QString::number(sql_para->aaultra_sql.gain) + "]");

    set_child2_lab = new QLabel(this);
    set_child2_lab->resize(147, 25);
    set_child2_lab->move(set_lab->x(), set_child1_lab->y() + set_child1_lab->height());
    set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child2_lab->setAlignment(Qt::AlignVCenter);
    set_child2_lab->setText(tr("  音量调节"));
    child2_val_lab = new QLabel(set_child2_lab);
    child2_val_lab->resize(47, 25);
    child2_val_lab->move(100, 0);
    child2_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child2_val_lab->setStyleSheet("QLabel {color:gray;}");
    child2_val_lab->setAlignment(Qt::AlignVCenter);
    child2_val_lab->setText("[" + QString::number(sql_para->aaultra_sql.vol) + "]");

    set_child3_lab = new QLabel(this);
    set_child3_lab->resize(147, 25);
    set_child3_lab->move(set_lab->x(), set_child2_lab->y() + set_child2_lab->height());
    set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child3_lab->setAlignment(Qt::AlignVCenter);
    set_child3_lab->setText(tr("  脉冲计数时长"));

    child3_val_lab = new QLabel(set_child3_lab);
    child3_val_lab->resize(47, 25);
    child3_val_lab->move(100, 0);
    child3_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child3_val_lab->setStyleSheet("QLabel {color:gray;}");
    child3_val_lab->setAlignment(Qt::AlignVCenter);
    child3_val_lab->setText("[" + QString::number(sql_para->aaultra_sql.time) + "]s");

    set_child4_lab = new QLabel(this);
    set_child4_lab->resize(147, 25);
    set_child4_lab->move(set_lab->x(), set_child3_lab->y() + set_child3_lab->height());
    set_child4_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child4_lab->setAlignment(Qt::AlignVCenter);
    set_child4_lab->setText(tr("  最大值重置"));

    set_child0_lab->hide();
    set_child1_lab->hide();
    set_child2_lab->hide();
    set_child3_lab->hide();
    set_child4_lab->hide();

    timer1 = new QTimer(this);
    timer1->setInterval(100);
    if (sql_para->aaultra_sql.mode == series) {
        timer1->start();
    }
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_2()));   //每0.1秒刷新一次数据状态，明显的变化需要快速显示

    timer2 = new QTimer(this);
    timer2->setInterval(1000);
    if (sql_para->aaultra_sql.mode == series) {
        timer2->start();
    }
    connect(timer2, SIGNAL(timeout()), this, SLOT(fresh_1()));   //每1秒刷新一次数据状态
}

void AAUltrasonic2::sysReset()
{
    if(!this->isHidden())
        qDebug("NULL!!!!!!!!!!!!!!!!!!!!!!!");
    aaultra_sql = &sqlcfg->get_para()->aaultra_sql;
//    fresh_setting();
    fresh_setting_text();
    set_child0_lab->hide();
    set_child1_lab->hide();
    set_child2_lab->hide();
    set_child3_lab->hide();
    set_child4_lab->hide();

    qDebug()<<"AAUltrasonic setting reset!!!!!!!!";
}

void AAUltrasonic2::maxReset()
{
    max_db = 0;
    aaValMax_lab->setText(tr("最大值: ") + QString::number(max_db) + "dBμV");
    qDebug()<<"AAUltrasonic max reset!";
}

void AAUltrasonic2::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void AAUltrasonic2::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 1) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val2 == 5){
            this->maxReset();
        }

        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        if(aaultra_sql->vol != sqlcfg->get_para()->aaultra_sql.vol){   //判断音量调节生效
            data->send_para.aa_vol.flag = true;
            data->send_para.aa_vol.rval = sql_para->aaultra_sql.vol;
            qDebug()<<"vol changed!";
        }
        memcpy(&sql_para->aaultra_sql, aaultra_sql, sizeof(AAULTRA_SQL));
        fresh_setting();


        sqlcfg->sql_save(sql_para);
        timer1->start();                                                         //and timer no stop
        timer2->start();
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        fresh_setting();
        break;
    case KEY_UP:
        if (key_val->grade.val1) {
            if (key_val->grade.val2 < 2) {
                key_val->grade.val2 = 5;
            } else {
                key_val->grade.val2--;
            }
            fresh_setting();
        }
        break;
    case KEY_DOWN:
        if (key_val->grade.val1) {
            if (key_val->grade.val2 > 4) {
                key_val->grade.val2 = 1;
            } else {
                key_val->grade.val2++;
            }
            fresh_setting();
        }
        break;
    case KEY_LEFT:
        switch (key_val->grade.val2) {
        case 1:
            if (aaultra_sql->mode == signal) {
                aaultra_sql->mode = series;
            } else {
                aaultra_sql->mode = signal;
            }
            fresh_setting();
            break;
        case 2:
            if (aaultra_sql->gain < 10) {
                aaultra_sql->gain = 100;
            } else if (aaultra_sql->gain == 10) {
                aaultra_sql->gain = 1;
            } else {
                aaultra_sql->gain -= 10;
            }
            fresh_setting();
            break;
        case 3:
            if (aaultra_sql->vol > VOL_MIN) {
                aaultra_sql->vol--;

                data->send_para.aa_vol.flag = true;
                data->send_para.aa_vol.rval = aaultra_sql->vol;
                qDebug()<<"vol changed!";
            }
            fresh_setting();
            break;
        case 4:
            if (aaultra_sql->time > TIME_MIN) {
                aaultra_sql->time--;
            }
            fresh_setting();
            break;
        default:
            break;
        }
        break;
    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            if (aaultra_sql->mode == signal) {
                aaultra_sql->mode = series;
            } else {
                aaultra_sql->mode = signal;
            }
            fresh_setting();
            break;
        case 2:
            if (aaultra_sql->gain > 90) {
                aaultra_sql->gain = 1;
            } else if (aaultra_sql->gain < 10) {
                aaultra_sql->gain = 10;
            } else {
                aaultra_sql->gain += 10;
            }
            fresh_setting();
            break;
        case 3:
            if (aaultra_sql->vol < VOL_MAX) {
                aaultra_sql->vol++;
            }
            data->send_para.aa_vol.flag = true;
            data->send_para.aa_vol.rval = aaultra_sql->vol;
            qDebug()<<"vol changed!";
            fresh_setting();
            break;
        case 4:
            if (aaultra_sql->time < TIME_MAX) {
                aaultra_sql->time++;
            }
            fresh_setting();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}



void AAUltrasonic2::fresh(bool f)
{
    int d,d2;
    double val, max_val,max_val2;
//    double val_range, max_val_range, val_50_range, val_100_range;

    /* If signal mode, timer started when press OK button.
       When timeout, stop timer, and fresh gui one time.
    */
    if (sql_para->aaultra_sql.mode == signal) {
        timer1->stop();
        timer2->stop();
    }

    d = (int)data->recv_para.ldata1_max - (int)data->recv_para.ldata1_min;      //最大值-最小值=幅值？
//    qDebug()<<"max = "<< data->recv_para.ldata1_max <<"\tmin = "<<data->recv_para.ldata1_min;
//    qDebug()<<"-max = "<< (int)data->recv_para.ldata1_max <<"\t-min = "<<(int)data->recv_para.ldata1_min;


//    d = qAbs(data->recv_para.ldata1_min - data->recv_para.ldata1_max);

    //下面应当是一个自动检测数据数量级的函数
    val = sqrt((d / 2) * 5000 / pow(2, 17));    //有效值?


//    max_val = (double)((d / 2) * 5000) / pow(2, 17);    //最大值
    max_val = (double)((d / 2) * 5000) / qPow(2, 17);    //最大值
//    qDebug()<<"d = \t"<<d<<"max_val ="<<max_val;

    max_val = sql_para->aaultra_sql.gain * (((double)20) * log10(max_val) - sql_para->aa_offset);      //对数运算,再加上增益

//    max_val = max_val - sql_para->aa_offset;    //减去偏置值


//    d2 = (int)data->recv_para.ldata0_max - (int)data->recv_para.ldata0_min;
//    max_val2 = (double)((d2 / 2) * 5000) / pow(2, 17);
//    qDebug()<<"AAUltrasonic2 = "<<max_val2<<"\n";


    //记录并显示最大值
    if (max_db <max_val ) {
        max_db = max_val;
        aaValMax_lab->setText(tr("最大值: ") + QString::number(max_db) + "dBμV");
    }

    if(f){  //直接显示
        aaVal_lab->setText(QString::number(max_val, 'f', 1));
        temp_db = max_val;
    }
    else{   //条件显示
//        qDebug()<<"aa_step = "<<sqlcfg->get_para()->aa_step;
        if(qAbs(max_val-temp_db ) > sqlcfg->get_para()->aa_step){
            aaVal_lab->setText(QString::number(max_val, 'f', 1));
        }
    }






    //实时进度条
//    lab_range->setText("dBμV");
//    if(max_val/sql_para->aaultra_sql.gain <= progbar->maximum()){
//        progbar->setValue(max_val/sql_para->aaultra_sql.gain);
//    }
//    else{
//        progbar->setValue(progbar->maximum());
//        qDebug()<<"AAUltrasonic value out of range!";
//    }


    double max_val_range;
    if (max_val < 1) {
        max_val_range = 1;
        lab_range->setText("mV");                                                //range fresh
    } else if (max_val < 10) {
        max_val_range = 10;
        lab_range->setText("10mV");                                               //range fresh
    } else if (max_val < 100) {
        max_val_range = 100;
        lab_range->setText("100mV");                                              //range fresh
    } else if (max_val < 1000) {
        max_val_range = 1000;
        lab_range->setText("V");                                                 //range fresh
    } else if (max_val < 5000) {
        max_val_range = 5000;
        lab_range->setText("5V");                                                 //range fresh
    } else {
        max_val_range = 10;
        lab_range->setText("10mV");                                               //range fresh
    }
////    lab1[3]->setText(QString::number(max_val, 'f', 1) + "mV");
    progbar->setValue(max_val * 100 / max_val_range);
}

void AAUltrasonic2::fresh_1()
{
    fresh(true);
}

void AAUltrasonic2::fresh_2()       //0.1秒刷新一次
{
    fresh(false);
    fresh_PRPD();
}

void AAUltrasonic2::fresh_PRPD()
{
    if(groupNum != data->recv_para.recData[0]){     //有效数据
        groupNum = data->recv_para.recData[0];
        //处理数据
        for(quint32 i=0;i<data->recv_para.recData[1];i++){
            X.append(data->recv_para.recData[i+2]);
            Y.append(data->recv_para.recData[i+3]);
        }
        qDebug()<<"read " <<data->recv_para.recData[1] <<  " PRPD data !";
        curve->setSamples(X,Y);
        plot->replot();

    }
}

void AAUltrasonic2::fresh_setting()
{
    if (key_val->grade.val2) {
        set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para2.png);}");
        set_txt_lab->setStyleSheet("QLabel {color:white;}");
        switch (key_val->grade.val2) {
        case 1:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child0_val_lab->setStyleSheet("QLabel {color:white;}");
            if (aaultra_sql->mode == signal) {
                child0_val_lab->setText(tr("[单次]"));
            } else {
                child0_val_lab->setText(tr("[连续]"));
            }
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child4_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 2:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child1_val_lab->setStyleSheet("QLabel {color:white;}");
            child1_val_lab->setText("[x" + QString::number(aaultra_sql->gain) + "]");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child4_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 3:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child2_val_lab->setStyleSheet("QLabel {color:white;}");
            child2_val_lab->setText("[" + QString::number(aaultra_sql->vol) + "]");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child4_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 4:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child3_val_lab->setStyleSheet("QLabel {color:white;}");
            child3_val_lab->setText("[" + QString::number(aaultra_sql->time) + "]s");
            set_child4_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 5:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
            child3_val_lab->setText("[" + QString::number(aaultra_sql->time) + "]s");
            set_child4_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            break;
        default:
            break;
        }
        set_child0_lab->show();
        set_child1_lab->show();
        set_child2_lab->show();
        set_child3_lab->show();
        set_child4_lab->show();
    } else if (!key_val->grade.val2) {
        set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para0.png);}");
        set_txt_lab->setStyleSheet("QLabel {color:gray;}");
        set_child0_lab->hide();
        set_child1_lab->hide();
        set_child2_lab->hide();
        set_child3_lab->hide();
        set_child4_lab->hide();
    }
}

void AAUltrasonic2::fresh_setting_text()
{
    if (aaultra_sql->mode == signal) {
        child0_val_lab->setText(tr("[单次]"));
    } else {
        child0_val_lab->setText(tr("[连续]"));
    }
    child1_val_lab->setText("[x" + QString::number(aaultra_sql->gain) + "]");
    child2_val_lab->setText("[" + QString::number(aaultra_sql->vol) + "]");
    child3_val_lab->setText("[" + QString::number(aaultra_sql->time) + "]s");
}
