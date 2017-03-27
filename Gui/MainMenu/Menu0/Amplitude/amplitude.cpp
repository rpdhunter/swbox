#include "amplitude.h"

Amplitude::Amplitude(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    key_val = NULL;

    /* get sql para */
    sql_para = sqlcfg->get_para();
//    amp_sql = new AMP_SQL;
//    memcpy(amp_sql, &sql_para->amp_sql, sizeof(AMP_SQL));
    amp_sql = &sql_para->amp_sql;

    data = g_data;
    db = 0;
    max_db = 0;

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("Amplitude {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    /* view TEV */
    tev_lab = new QLabel(this);
    tev_lab->resize(30, 20);
    tev_lab->move(370, 15);
    tev_lab->setStyleSheet("QLabel {color:white;}");
    tev_lab->setAlignment(Qt::AlignVCenter);
    tev_lab->setText(tr("TEV"));
    tev_lab->hide();

    /* view max value */
    max_val_lab = new QLabel(this);
    max_val_lab->resize(90, 20);
    max_val_lab->move(350, 50);
    max_val_lab->setStyleSheet("QLabel {color:white;}");
    max_val_lab->setAlignment(Qt::AlignVCenter);

    pulse_cnt_lab = new QLabel(this);
    pulse_cnt_lab->resize(90, 20);
    pulse_cnt_lab->move(max_val_lab->x(), max_val_lab->y() + 35);
    pulse_cnt_lab->setStyleSheet("QLabel {color:white;}");
    pulse_cnt_lab->setAlignment(Qt::AlignVCenter);

    degree_lab = new QLabel(this);
    degree_lab->resize(90, 20);
    degree_lab->move(pulse_cnt_lab->x(), pulse_cnt_lab->y() + 35);
    degree_lab->setStyleSheet("QLabel {color:white;}");
    degree_lab->setAlignment(Qt::AlignVCenter);

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

    d_chart = new BarChart(plot, &db, sql_para);


    /* view color block */
    green_lab = new QLabel(this);
    green_lab->resize(22, 22);
//    green_lab->move(max_val_lab->x(), max_val_lab->y() + 30);
    green_lab->move(plot->x()+plot->width()+25, 100);
    green_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:yellow;}");

    yellow_lab = new QLabel(this);
    yellow_lab->resize(22, 22);
    yellow_lab->move(green_lab->x() + 20, green_lab->y());
    yellow_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:green;}");

    red_lab = new QLabel(this);
    red_lab->resize(22, 22);
    red_lab->move(yellow_lab->x() + 20, green_lab->y());
    red_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:red;}");

    /* view amplitude history value */
    amp_his_val = new QLabel(this);
    amp_his_val->resize(75, 40);
//    amp_his_val->move(green_lab->x(), green_lab->y() + 50);
    amp_his_val->move(green_lab->x()-20, green_lab->y() - 60);
    amp_his_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:52px;color:white}");
    amp_his_val->setAlignment(Qt::AlignCenter);

    /* view db */
    db_lab = new QLabel(this);
    db_lab->resize(30, 20);
    db_lab->move(amp_his_val->x() + amp_his_val->width(), amp_his_val->y() + 20);
    db_lab->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:20px;color:white}");
    db_lab->setAlignment(Qt::AlignCenter);
    db_lab->setText("dB");

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
    set_child0_lab->hide();

    child0_val_lab = new QLabel(set_child0_lab);
    child0_val_lab->resize(47, 25);
    child0_val_lab->move(100, 0);
    child0_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child0_val_lab->setStyleSheet("QLabel {color:gray;}");
    child0_val_lab->setAlignment(Qt::AlignVCenter);
    if (sql_para->amp_sql.mode == signal) {
        child0_val_lab->setText(tr("[单次]"));
    } else {
        child0_val_lab->setText(tr("[连续]"));
    }

    set_child1_lab = new QLabel(this);
    set_child1_lab->resize(147, 25);
    set_child1_lab->move(set_lab->x(), set_child0_lab->y() + set_child0_lab->height());
    set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child1_lab->setAlignment(Qt::AlignVCenter);
    set_child1_lab->setText(tr("  红色报警阀值"));
    set_child1_lab->hide();

    child1_val_lab = new QLabel(set_child1_lab);
    child1_val_lab->resize(47, 25);
    child1_val_lab->move(100, 0);
    child1_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child1_val_lab->setStyleSheet("QLabel {color:gray;}");
    child1_val_lab->setAlignment(Qt::AlignVCenter);
    child1_val_lab->setText("[" + QString::number(sql_para->amp_sql.high) + "]dB");

    set_child2_lab = new QLabel(this);
    set_child2_lab->resize(147, 25);
    set_child2_lab->move(set_lab->x(), set_child1_lab->y() + set_child1_lab->height());
    set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child2_lab->setAlignment(Qt::AlignVCenter);
    set_child2_lab->setText(tr("  黄色报警阀值"));
    set_child2_lab->hide();

    child2_val_lab = new QLabel(set_child2_lab);
    child2_val_lab->resize(47, 25);
    child2_val_lab->move(100, 0);
    child2_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child2_val_lab->setStyleSheet("QLabel {color:gray;}");
    child2_val_lab->setAlignment(Qt::AlignVCenter);
    child2_val_lab->setText("[" + QString::number(sql_para->amp_sql.low) + "]dB");

    set_child3_lab = new QLabel(this);
    set_child3_lab->resize(147, 25);
    set_child3_lab->move(set_lab->x(), set_child2_lab->y() + set_child2_lab->height());
    set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child3_lab->setAlignment(Qt::AlignVCenter);
    set_child3_lab->setText(tr("  最大值重置"));
    set_child3_lab->hide();

    timer = new QTimer(this);
    timer->setInterval(1000);
    if (sql_para->amp_sql.mode == series) {
        timer->start();
    }

    //每隔1秒，刷新
    connect(timer, &QTimer::timeout, this, &Amplitude::fresh_plot);
    connect(timer, &QTimer::timeout, d_chart, &BarChart::fresh);


}

void Amplitude::sysReset()
{
    amp_sql = &sqlcfg->get_para()->amp_sql;

    fresh_setting();
    set_child0_lab->hide();
    set_child1_lab->hide();
    set_child2_lab->hide();
    set_child3_lab->hide();

    qDebug()<<"Amplitude setting reset!!!!!!!!";
}

void Amplitude::fresh_plot(void)
{
    double t, s;

    /* If signal mode, timer started when press OK button.
       When timeout, stop timer, and fresh gui one time.
    */
    if (sql_para->amp_sql.mode == signal) {
        timer->stop();      //如果单次模式，停止计时器
    }

    double a = AD_VAL(data->recv_para.hdata0.ad.ad_max, 0x8000 );
    double b = AD_VAL(data->recv_para.hdata0.ad.ad_min, 0x8000 );

    a = data->recv_para.hdata0.ad.ad_max - 0x8000;
    b = data->recv_para.hdata0.ad.ad_min - 0x8000;
    emit offset_suggest((int)(a/10),(int)(b/10));
//    qDebug()<<"[1]a = "<<a <<"\tb = "<<b<<"\tsql_para->tev_offset1 = "<<sql_para->tev_offset1<<"\tsql_para->tev_offset2 = "<<sql_para->tev_offset2;

    a = AD_VAL(data->recv_para.hdata0.ad.ad_max, (0x8000+sql_para->tev_offset1*10) );
    b = AD_VAL(data->recv_para.hdata0.ad.ad_min, (0x8000+sql_para->tev_offset2*10) );

//    offset1 = (int)(a/10);
//    offset2 = (int)(b/10);



    t = ((double)MAX(a, b) * 1000) / 32768;



//    s = fabs(((double)20) * log10(t));      //对数运算，来自工具链的函数
    s = ((double)20) * log10(t);      //对数运算，来自工具链的函数

//    qDebug()<<"[2]a = "<<a <<"\tb = "<<b <<"\tt= "<<t<<"\ts= "<<s;

    s = sql_para->tev_gain * s;     //设置增益系数

    //记录并显示最大值
    if (max_db < s) {
        max_db = s;
        max_val_lab->setText(tr("最大值: ") + QString::number(max_db) + "dB");
    }

    //脉冲计数和严重度

//    signal_pulse_cnt = data->recv_para.pulse1.edge.neg + data->recv_para.pulse1.edge.pos;
    pulse_cnt = data->recv_para.hpulse1_totol;
    degree = s * (double)pulse_cnt / 1000;

    pulse_cnt_lab->setText(tr("脉冲数: ") + QString::number(pulse_cnt));
    degree_lab->setText(tr("严重度: ") + QString::number(degree, 'f', 2));

    amp_his_val->setText(QString::number((qint16)s));
    db = (int)s;

    if (db > sql_para->amp_sql.high) {
        green_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:transparent;}");
        yellow_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:transparent;}");
        red_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:red;}");
    } else if (db >= sql_para->amp_sql.low) {
        green_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:transparent;}");
        yellow_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:yellow;}");
        red_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:transparent;}");
    } else {
        green_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:green;}");
        yellow_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:transparent;}");
        red_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:transparent;}");
    }
    plot->replot();
}

void Amplitude::maxReset()
{
    max_db = 0;
    max_val_lab->setText(tr("最大值: ") + QString::number(max_db) + "dB");
    qDebug()<<"max value reset!";
}

void Amplitude::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void Amplitude::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 0) {
        return;
    }
//    printf("\nkey_val->grade.val1 is : %d",key_val->grade.val1);
//    printf("\tkey_val->grade.val2 is : %d",key_val->grade.val2);
//    printf("\tkey_val->grade.val3 is : %d",key_val->grade.val3);

    switch (key_code) {
    case KEY_OK:
//        if (key_val->grade.val2) {                                              //in setting

//            key_val->grade.val2 = 0;

//            memcpy(&sql_para->amp_sql, amp_sql, sizeof(AMP_SQL));
//        fresh_setting();
//            sqlcfg->sql_save(sql_para);                                         //save sql
//            if (sql_para->amp_sql.mode == series) {                             //start timer in series mode
//                timer->start();                                                 //and timer no stop
//            }

//        } else if (!key_val->grade.val2 && sql_para->amp_sql.mode == signal) {  //press OK start in signal mode
//            timer->start();
//        }



        memcpy(&sql_para->amp_sql, amp_sql, sizeof(AMP_SQL));

        sqlcfg->sql_save(sql_para);
        timer->start();                                                         //and timer no stop
        if(key_val->grade.val2 == 4){
            this->maxReset();
        }

        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        fresh_setting();

        break;
    case KEY_CANCEL:
//        if (!key_val->grade.val2) {
//            key_val->grade.val2 = 0;
//            fresh_parent();
//        } else if (key_val->grade.val3) {
//            key_val->grade.val3 = 0;
//            fresh_setting();
//        }
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        fresh_setting();
        break;
    case KEY_UP:
//        if (key_val->grade.val2 && !key_val->grade.val3) {
//            key_val->grade.val3 = 1;
//        } else if (key_val->grade.val3 && !key_val->grade.val4) {
//            memcpy(amp_sql, &sql_para->amp_sql, sizeof(AMP_SQL));
//            key_val->grade.val4 = 1;
//        } else if (!key_val->grade.val4) {
//            break;
//        }
        if (key_val->grade.val2 < 2) {
            key_val->grade.val2 = 4;
        } else {
            key_val->grade.val2--;
        }
        fresh_setting();
        break;
    case KEY_DOWN:
//        if (key_val->grade.val2 && !key_val->grade.val3) {
//            key_val->grade.val3 = 1;
//        } else if (key_val->grade.val3 && !key_val->grade.val4) {
//            memcpy(amp_sql, &sql_para->amp_sql, sizeof(AMP_SQL));
//            key_val->grade.val4 = 0;
//        } else if (!key_val->grade.val4) {
//            break;
//        }
        if (key_val->grade.val2 > 3) {
            key_val->grade.val2 = 1;
        } else {
            key_val->grade.val2++;
        }
        fresh_setting();
        break;
    case KEY_LEFT:
        switch (key_val->grade.val2) {
        case 1:
            if (amp_sql->mode == signal) {
                amp_sql->mode = series;
            } else {
                amp_sql->mode = signal;
            }
            fresh_setting();
            break;
        case 2:
            if (amp_sql->high > amp_sql->low) {
                amp_sql->high--;
            }
            fresh_setting();
            break;
        case 3:
            if (amp_sql->low > 0) {
                amp_sql->low--;
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
            if (amp_sql->mode == signal) {
                amp_sql->mode = series;
            } else {
                amp_sql->mode = signal;
            }
            fresh_setting();
            break;
        case 2:
            if (amp_sql->high < 60) {
                amp_sql->high++;
            }
            fresh_setting();
            break;
        case 3:
            if (amp_sql->low < amp_sql->high) {
                amp_sql->low++;
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

void Amplitude::fresh_setting(void)
{

//    printf("\nkey_val->grade.val1 is : %d",key_val->grade.val1);
//    printf("\tkey_val->grade.val2 is : %d",key_val->grade.val2);
//    printf("\tkey_val->grade.val3 is : %d",key_val->grade.val3);

    if (amp_sql->mode == signal) {
        child0_val_lab->setText(tr("[单次]"));
    } else {
        child0_val_lab->setText(tr("[连续]"));
    }
    child1_val_lab->setText("[" + QString::number(amp_sql->high) + "]dB");
    child2_val_lab->setText("[" + QString::number(amp_sql->low) + "]dB");

    if (key_val->grade.val2) {
        set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para2.png);}");
        set_txt_lab->setStyleSheet("QLabel {color:white;}");
        switch (key_val->grade.val2) {
        case 1:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child0_val_lab->setStyleSheet("QLabel {color:white;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 2:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child1_val_lab->setStyleSheet("QLabel {color:white;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 3:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child2_val_lab->setStyleSheet("QLabel {color:white;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            break;
        case 4:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            break;
        default:
            break;
        }
        set_child0_lab->show();
        set_child1_lab->show();
        set_child2_lab->show();
        set_child3_lab->show();
    } else if (!key_val->grade.val2) {
        set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para0.png);}");
        set_txt_lab->setStyleSheet("QLabel {color:gray;}");
        set_child0_lab->hide();
        set_child1_lab->hide();
        set_child2_lab->hide();
        set_child3_lab->hide();
    }
}
