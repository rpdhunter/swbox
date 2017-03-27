#include "pulse.h"

Pulse::Pulse(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    key_val = NULL;
    data = g_data;
    pulse_cnt = 0;
    signal_pulse_cnt = 0;
    degree = 0;

    /* get sql para */
    sql_para = sqlcfg->get_para();

    pulse_sql = new PULSE_SQL;
    memcpy(pulse_sql, &sql_para->pulse_sql, sizeof(PULSE_SQL));

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("Pulse {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    /* view amplitude history value */
    amp_his_val = new QLabel(this);
    amp_his_val->resize(80, 40);
    amp_his_val->move(290, 80);
    amp_his_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:52px;color:white}");
    amp_his_val->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    /* view db */
    db_lab = new QLabel(this);
    db_lab->resize(30, 20);
    db_lab->move(amp_his_val->x() + amp_his_val->width(), amp_his_val->y() + 20);
    db_lab->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:20px;color:white}");
    db_lab->setAlignment(Qt::AlignCenter);
    db_lab->setText("dB");

    /* view content */
    /* pulse */
    pulse_lab = new QLabel(this);
    pulse_lab->resize(200, 25);
    pulse_lab->move(50, 41);

    rect0_lab = new QLabel(pulse_lab);
    rect0_lab->resize(11, 11);
    rect0_lab->move(0, 6);
    rect0_lab->setStyleSheet("QLabel {background-color:rgb(255, 151, 0);}");

    lab0 = new QLabel(pulse_lab);
    lab0->resize(179, 25);
    lab0->move(rect0_lab->x() + rect0_lab->width() + 10, 0);
    lab0->setStyleSheet("QLabel {color:white;}");

    /* signal pulse */
    signal_pulse_lab = new QLabel(this);
    signal_pulse_lab->resize(pulse_lab->width(), pulse_lab->height());
    signal_pulse_lab->move(pulse_lab->x(), pulse_lab->y() + 30);

    rect1_lab = new QLabel(signal_pulse_lab);
    rect1_lab->resize(rect0_lab->width(), rect0_lab->height());
    rect1_lab->move(0, 6);
    rect1_lab->setStyleSheet("QLabel {background-color:rgb(255, 151, 0);}");

    lab1 = new QLabel(signal_pulse_lab);
    lab1->resize(lab0->width(), lab0->height());
    lab1->move(rect1_lab->x() + rect1_lab->width() + 10, 0);
    lab1->setStyleSheet("QLabel {color:white;}");

    /* degree */
    degree_lab = new QLabel(this);
    degree_lab->resize(pulse_lab->width(), pulse_lab->height());
    degree_lab->move(signal_pulse_lab->x(), signal_pulse_lab->y() + 30);

    rect2_lab = new QLabel(degree_lab);
    rect2_lab->resize(rect1_lab->width(), rect1_lab->height());
    rect2_lab->move(0, 6);
    rect2_lab->setStyleSheet("QLabel {background-color:rgb(255, 151, 0);}");

    lab2 = new QLabel(degree_lab);
    lab2->resize(lab1->width(), lab1->height());
    lab2->move(rect2_lab->x() + rect2_lab->width() + 10, 0);
    lab2->setStyleSheet("QLabel {color:white;}");

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
    if (sql_para->pulse_sql.mode == signal) {
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
    child1_val_lab->setText("[" + QString::number(sql_para->pulse_sql.high) + "]dB");

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
    child2_val_lab->setText("[" + QString::number(sql_para->pulse_sql.low) + "]dB");


    set_child3_lab = new QLabel(this);
    set_child3_lab->resize(147, 25);
    set_child3_lab->move(set_lab->x(), set_child2_lab->y() + set_child2_lab->height());
    set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child3_lab->setAlignment(Qt::AlignVCenter);
    set_child3_lab->setText(tr("  脉冲计数时长"));
    set_child3_lab->hide();

    child3_val_lab = new QLabel(set_child3_lab);
    child3_val_lab->resize(47, 25);
    child3_val_lab->move(100, 0);
    child3_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child3_val_lab->setStyleSheet("QLabel {color:gray;}");
    child3_val_lab->setAlignment(Qt::AlignVCenter);
    child3_val_lab->setText("[" + QString::number(sql_para->pulse_sql.time) + "]s");

    timer = new QTimer(this);
    timer->setInterval(1000);
    timer->start();

    connect(timer, &QTimer::timeout, this, &Pulse::fresh_data);
}

void Pulse::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void Pulse::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 2) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if (key_val->grade.val4) {                                              //in setting
            key_val->grade.val4 = 0;
            memcpy(&sql_para->pulse_sql, pulse_sql, sizeof(PULSE_SQL));
            sqlcfg->sql_save(sql_para);                                         //save sql
            if (sql_para->pulse_sql.mode == series) {                           //start timer in series mode
                timer->start();                                                 //and timer no stop
            }
            fresh_setting();
        } else if (!key_val->grade.val4 && sql_para->pulse_sql.mode == signal) {//press OK start in signal mode
            timer->start();
        }
        break;
    case KEY_CANCEL:
        if (!key_val->grade.val4) {
            key_val->grade.val2 = 0;
            key_val->grade.val3 = 0;
            fresh_parent();
        } else if (key_val->grade.val4) {
            key_val->grade.val4 = 0;
            fresh_setting();
        }
        break;
    case KEY_UP:
        if (key_val->grade.val2 && !key_val->grade.val3) {
            key_val->grade.val3 = 1;
        } else if (key_val->grade.val3 && !key_val->grade.val4) {
            memcpy(pulse_sql, &sql_para->pulse_sql, sizeof(PULSE_SQL));
            key_val->grade.val4 = 1;
        } else if (!key_val->grade.val4) {
            break;
        }
        if (key_val->grade.val4 < 2) {
            key_val->grade.val4 = 4;
        } else {
            key_val->grade.val4--;
        }
        fresh_setting();
        break;
    case KEY_DOWN:
        if (key_val->grade.val2 && !key_val->grade.val3) {
            key_val->grade.val3 = 1;
        } else if (key_val->grade.val3 && !key_val->grade.val4) {
            memcpy(pulse_sql, &sql_para->pulse_sql, sizeof(PULSE_SQL));
            key_val->grade.val4 = 0;
        } else if (!key_val->grade.val4) {
            break;
        }
        if (key_val->grade.val4 > 3) {
            key_val->grade.val4 = 1;
        } else {
            key_val->grade.val4++;
        }
        fresh_setting();
        break;
    case KEY_LEFT:
        switch (key_val->grade.val4) {
        case 1:
            if (pulse_sql->mode == signal) {
                pulse_sql->mode = series;
            } else {
                pulse_sql->mode = signal;
            }
            fresh_setting();
            break;
        case 2:
            if (pulse_sql->high > pulse_sql->low) {
                pulse_sql->high--;
            }
            fresh_setting();
            break;
        case 3:
            if (pulse_sql->low > 0) {
                pulse_sql->low--;
            }
            fresh_setting();
            break;
        case 4:
            if (pulse_sql->time > TIME_MIN) {
                pulse_sql->time--;
            }
            fresh_setting();
            break;
        default:
            break;
        }
        break;

    case KEY_RIGHT:
        switch (key_val->grade.val4) {
        case 1:
            if (pulse_sql->mode == signal) {
                pulse_sql->mode = series;
            } else {
                pulse_sql->mode = signal;
            }
            fresh_setting();
            break;
        case 2:
            if (pulse_sql->high < 60) {
                pulse_sql->high++;
            }
            fresh_setting();
            break;
        case 3:
            if (pulse_sql->low < pulse_sql->high) {
                pulse_sql->low++;
            }
            fresh_setting();
            break;
        case 4:
            if (pulse_sql->time < TIME_MAX) {
                pulse_sql->time++;
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

//脉冲核心算法
void Pulse::fresh_data(void)
{
    double t, s;

    /* If signal mode, timer started when press OK button.
       When timeout, stop timer, and fresh gui one time.
    */
    if (sql_para->pulse_sql.mode == signal) {
        timer->stop();
    }

    //qDebug("A:0x%04x", AD_VAL(data->hdata1.ad.ad_max));
    //qDebug("B:0x%04x", AD_VAL(data->hdata1.ad.ad_min));
    //qDebug("MAX:0x%08x", MAX(AD_VAL(data->hdata1.ad.ad_max), AD_VAL(data->hdata1.ad.ad_min)));

    t = ((double)MAX(AD_VAL(data->recv_para.hdata0.ad.ad_max, 0x8000), AD_VAL(data->recv_para.hdata0.ad.ad_min, 0x8000)) * 1000) / 32768;
    //qDebug("%f", t);
    s = fabs(((double)20) * log10(t));
    //qDebug("%f", s);
    amp_his_val->setText(QString::number((quint16)s));

    signal_pulse_cnt = data->recv_para.pulse1.edge.neg + data->recv_para.pulse1.edge.pos;
    pulse_cnt = data->recv_para.hpulse1_totol;
    degree = t * (double)signal_pulse_cnt / 1000;

    lab0->setText("脉冲计数: " + QString::number(pulse_cnt));
    lab1->setText("单周期脉冲数: " + QString::number(signal_pulse_cnt));
    lab2->setText("放电严重程度: " + QString::number(degree, 'f', 2));
}

void Pulse::fresh_setting(void)
{
    if (pulse_sql->mode == signal) {
        child0_val_lab->setText(tr("[单次]"));
    } else {
        child0_val_lab->setText(tr("[连续]"));
    }
    child1_val_lab->setText("[" + QString::number(pulse_sql->high) + "]dB");
    child2_val_lab->setText("[" + QString::number(pulse_sql->low) + "]dB");
    child3_val_lab->setText("[" + QString::number(pulse_sql->time) + "]s");

    if (key_val->grade.val4) {
        set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para2.png);}");
        set_txt_lab->setStyleSheet("QLabel {color:white;}");
        switch (key_val->grade.val4) {
        case 1:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child0_val_lab->setStyleSheet("QLabel {color:white;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
            break;
        case 2:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child1_val_lab->setStyleSheet("QLabel {color:white;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child2_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
            break;
        case 3:
            set_child0_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child0_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child1_val_lab->setStyleSheet("QLabel {color:gray;}");
            set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:white}");
            child2_val_lab->setStyleSheet("QLabel {color:white;}");
            set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
            child3_val_lab->setStyleSheet("QLabel {color:gray;}");
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
            break;
        default:
            break;
        }
        set_child0_lab->show();
        set_child1_lab->show();
        set_child2_lab->show();
        set_child3_lab->show();
    } else if (!key_val->grade.val4) {
        set_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para0.png);}");
        set_txt_lab->setStyleSheet("QLabel {color:gray;}");
        set_child0_lab->hide();
        set_child1_lab->hide();
        set_child2_lab->hide();
        set_child3_lab->hide();
    }
}
