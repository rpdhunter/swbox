#include "aaultrasonic.h"

AAUltrasonic::AAUltrasonic(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    data = g_data;

    /* get sql para */
    //读取配置信息
    sql_para = sqlcfg->get_para();
    aaultra_sql = new AAULTRA_SQL;
    memcpy(aaultra_sql, &sql_para->aaultra_sql, sizeof(AAULTRA_SQL));


    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("AAUltrasonic {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    /* bar0 */
    progbar0 = new QProgressBar(this);
    progbar0->move(100, 47);
    progbar0->resize(208, 8);
    progbar0->setTextVisible(false);                                            //no show percent
    progbar0->setStyleSheet("QProgressBar{border:1px solid gray;"
                            "background:transparent;}"
                            "QProgressBar::chunk{"
                            "background-color:skyblue;}");
    progbar0->setRange(0, 100);
    progbar0->setValue(0);

    lab0[0] = new QLabel(this);
    lab0[0]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab0[0]->resize(40, 10);
    lab0[0]->move(progbar0->x() - 8, progbar0->y() - 12);
    lab0[0]->setText("0mV");

    lab0[1] = new QLabel(this);
    lab0[1]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab0[1]->resize(40, 10);
    lab0[1]->move(progbar0->x() + progbar0->width() - 12, progbar0->y() - 12);
    lab0[1]->setText("10mV");

    lab0[2] = new QLabel(this);
    lab0[2]->setStyleSheet("QLabel {color:skyblue;}");
    lab0[2]->resize(40, 12);
    lab0[2]->move(progbar0->x() + progbar0->width() / 2 - lab0[2]->width() / 2, progbar0->y() - lab0[2]->height() - 4);
    lab0[2]->setAlignment(Qt::AlignCenter);
    lab0[2]->setText(tr("有效值"));

    lab0[3] = new QLabel(this);
    lab0[3]->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:12px;color:skyblue;}");
    lab0[3]->resize(100, 14);
    lab0[3]->move(progbar0->x() + progbar0->width() + 16, progbar0->y() - 2);
    lab0[3]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab0[3]->setText("0mV");

    /* bar1 */
    progbar1 = new QProgressBar(this);
    progbar1->move(progbar0->x(), progbar0->y() + 34);
    progbar1->resize(208, 8);
    progbar1->setTextVisible(false);                                            //no show percent
    progbar1->setStyleSheet("QProgressBar{border:1px solid gray;"
                            "background:transparent;}"
                            "QProgressBar::chunk{"
                            "background-color:cyan;}");
    progbar1->setRange(0, 100);
    progbar1->setValue(0);

    lab1[0] = new QLabel(this);
    lab1[0]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab1[0]->resize(40, 10);
    lab1[0]->move(progbar1->x() - 8, progbar1->y() - 12);
    lab1[0]->setText("0mV");

    lab1[1] = new QLabel(this);
    lab1[1]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab1[1]->resize(40, 10);
    lab1[1]->move(progbar1->x() + progbar1->width() - 12, progbar1->y() - 12);
    lab1[1]->setText("20mV");

    lab1[2] = new QLabel(this);
    lab1[2]->setStyleSheet("QLabel {color:cyan;}");
    lab1[2]->resize(80, 12);
    lab1[2]->move(progbar1->x() + progbar1->width() / 2 - lab1[2]->width() / 2, progbar1->y() - lab1[2]->height() - 4);
    lab1[2]->setAlignment(Qt::AlignCenter);
    lab1[2]->setText(tr("周期最大值"));

    lab1[3] = new QLabel(this);
    lab1[3]->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:12px;color:cyan;}");
    lab1[3]->resize(100, 14);
    lab1[3]->move(progbar1->x() + progbar1->width() + 16, progbar1->y() - 2);
    lab1[3]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab1[3]->setText("0mV");

    /* bar2 */
    progbar2 = new QProgressBar(this);
    progbar2->move(progbar1->x(), progbar1->y() + 34);
    progbar2->resize(208, 8);
    progbar2->setTextVisible(false);                                            //no show percent
    progbar2->setStyleSheet("QProgressBar{border:1px solid gray;"
                            "background:transparent;}"
                            "QProgressBar::chunk{"
                            "background-color:green;}");
    progbar2->setRange(0, 100);
    progbar2->setValue(0);

    lab2[0] = new QLabel(this);
    lab2[0]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab2[0]->resize(40, 10);
    lab2[0]->move(progbar2->x() - 8, progbar2->y() - 12);
    lab2[0]->setText("0mV");

    lab2[1] = new QLabel(this);
    lab2[1]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab2[1]->resize(40, 10);
    lab2[1]->move(progbar2->x() + progbar2->width() - 12, progbar2->y() - 12);
    lab2[1]->setText("2mV");

    lab2[2] = new QLabel(this);
    lab2[2]->setStyleSheet("QLabel {color:green;}");
    lab2[2]->resize(110, 12);
    lab2[2]->move(progbar2->x() + progbar2->width() / 2 - lab2[2]->width() / 2, progbar2->y() - lab2[2]->height() - 4);
    lab2[2]->setAlignment(Qt::AlignCenter);
    lab2[2]->setText(tr("频率成分1[50Hz]"));

    lab2[3] = new QLabel(this);
    lab2[3]->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:12px;color:green;}");
    lab2[3]->resize(100, 14);
    lab2[3]->move(progbar2->x() + progbar2->width() + 16, progbar2->y() - 2);
    lab2[3]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab2[3]->setText("0mV");

    /* bar3 */
    progbar3 = new QProgressBar(this);
    progbar3->move(progbar2->x(), progbar2->y() + 34);
    progbar3->resize(208, 8);
    progbar3->setTextVisible(false);                                            //no show percent
    progbar3->setStyleSheet("QProgressBar{border:1px solid gray;"
                            "background:transparent;}"
                            "QProgressBar::chunk{"
                            "background-color:yellow;}");
    progbar3->setRange(0, 100);
    progbar3->setValue(0);

    lab3[0] = new QLabel(this);
    lab3[0]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab3[0]->resize(40, 10);
    lab3[0]->move(progbar3->x() - 8, progbar3->y() - 12);
    lab3[0]->setText("0mV");

    lab3[1] = new QLabel(this);
    lab3[1]->setStyleSheet("QLabel {font-family:Moonracer;font-size:10px;color:white;}");
    lab3[1]->resize(40, 10);
    lab3[1]->move(progbar3->x() + progbar3->width() - 12, progbar3->y() - 12);
    lab3[1]->setText("2mV");

    lab3[2] = new QLabel(this);
    lab3[2]->setStyleSheet("QLabel {color:yellow;}");
    lab3[2]->resize(110, 12);
    lab3[2]->move(progbar3->x() + progbar3->width() / 2 - lab3[2]->width() / 2, progbar3->y() - lab3[2]->height() - 4);
    lab3[2]->setAlignment(Qt::AlignCenter);
    lab3[2]->setText(tr("频率成分2[100Hz]"));

    lab3[3] = new QLabel(this);
    lab3[3]->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:12px;color:yellow;}");
    lab3[3]->resize(100, 14);
    lab3[3]->move(progbar3->x() + progbar3->width() + 16, progbar3->y() - 2);
    lab3[3]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lab3[3]->setText("0mV");

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

    set_child0_lab->hide();
    set_child1_lab->hide();
    set_child2_lab->hide();
    set_child3_lab->hide();

    timer = new QTimer(this);
    timer->setInterval(100);
    if (sql_para->aaultra_sql.mode == series) {
        timer->start();
    }
    connect(timer, &QTimer::timeout, this, &AAUltrasonic::fresh);   //每0.1秒刷新一次数据状态
}

void AAUltrasonic::trans_key(unsigned char key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 1) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        memcpy(&sql_para->aaultra_sql, aaultra_sql, sizeof(AAULTRA_SQL));
        fresh_setting();
        sqlcfg->sql_save(sql_para);
        timer->start();                                                         //and timer no stop
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        fresh_setting();
        break;
    case KEY_UP:
        if (key_val->grade.val1) {
            if (key_val->grade.val2 < 2) {
                key_val->grade.val2 = 4;
            } else {
                key_val->grade.val2--;
            }
            fresh_setting();
        }
        break;
    case KEY_DOWN:
        if (key_val->grade.val1) {
            if (key_val->grade.val2 > 3) {
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

void AAUltrasonic::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

//配合键盘，设定参数
void AAUltrasonic::fresh_setting(void)
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

//数据处理显示函数
//有诸多疑点，前两个数据计算公式从何而来？
//后两个数据几乎肯定不对，因为没做频率变换
void AAUltrasonic::fresh(void)
{
    int d;
    double val, max_val, val_50, val_100;
    double val_range, max_val_range, val_50_range, val_100_range;

    /* If signal mode, timer started when press OK button.
       When timeout, stop timer, and fresh gui one time.
    */
    if (sql_para->aaultra_sql.mode == signal) {
        timer->stop();
    }

    d = (int)data->recv_para.ldata1_max - (int)data->recv_para.ldata1_min;      //最大值-最小值=幅值？
    //下面应当是一个自动检测数据数量级的函数
    val = sqrt((d / 2) * 5000 / pow(2, 17));



    if (val < 1) {
        val_range = 1;
        lab0[1]->setText("1mV");                                                //range fresh
    } else if (val < 10) {
        val_range = 10;
        lab0[1]->setText("10mV");                                               //range fresh
    } else if (val < 100) {
        val_range = 100;
        lab0[1]->setText("100mV");                                              //range fresh
    } else if (val < 1000) {
        val_range = 1000;
        lab0[1]->setText("1V");                                                 //range fresh
    } else if (val < 5000) {
        val_range = 5000;
        lab0[1]->setText("5V");                                                 //range fresh
    } else {
        val_range = 10;
        lab0[1]->setText("10mV");                                               //range fresh
    }
    lab0[3]->setText(QString::number(val, 'f', 1) + "mV");
    progbar0->setValue(val * 100 / val_range);

    max_val = (double)((d / 2) * 5000) / pow(2, 17);

//    printf("\n max= %u , min = %u  ",(int)data->recv_para.ldata1_max,(int)data->recv_para.ldata1_min);
    //qDebug("max_val = %f", max_val);
    if (max_val < 1) {
        max_val_range = 1;
        lab1[1]->setText("1mV");                                                //range fresh
    } else if (max_val < 10) {
        max_val_range = 10;
        lab1[1]->setText("10mV");                                               //range fresh
    } else if (max_val < 100) {
        max_val_range = 100;
        lab1[1]->setText("100mV");                                              //range fresh
    } else if (max_val < 1000) {
        max_val_range = 1000;
        lab1[1]->setText("1V");                                                 //range fresh
    } else if (max_val < 5000) {
        max_val_range = 5000;
        lab1[1]->setText("5V");                                                 //range fresh
    } else {
        max_val_range = 10;
        lab1[1]->setText("10mV");                                               //range fresh
    }
    lab1[3]->setText(QString::number(max_val, 'f', 1) + "mV");
    progbar1->setValue(max_val * 100 / max_val_range);

    val_50 = val * 0.5;
    //qDebug("val_50 = %f", val_50);
    if (val_50 < 1) {
        val_50_range = 1;
        lab2[1]->setText("1mV");                                                //range fresh
    } else if (val_50 < 10) {
        val_50_range = 10;
        lab2[1]->setText("10mV");                                               //range fresh
    } else if (val_50 < 100) {
        val_50_range = 100;
        lab2[1]->setText("100mV");                                              //range fresh
    } else if (val_50 < 1000) {
        val_50_range = 1000;
        lab2[1]->setText("1V");                                                 //range fresh
    } else if (val_50 < 5000) {
        val_50_range = 5000;
        lab2[1]->setText("5V");                                                 //range fresh
    } else {
        val_50_range = 10;
        lab2[1]->setText("10mV");                                               //range fresh
    }
    lab2[3]->setText(QString::number(val_50, 'f', 1) + "mV");
    progbar2->setValue(val_50 * 100 / val_50_range);

    val_100 = val * 0.6;
    //qDebug("val_100 = %f", val_100);
    if (val_100 < 1) {
        val_100_range = 1;
        lab3[1]->setText("1mV");                                                //range fresh
    } else if (val_100 < 10) {
        val_100_range = 10;
        lab3[1]->setText("10mV");                                               //range fresh
    } else if (val_100 < 100) {
        val_100_range = 100;
        lab3[1]->setText("100mV");                                              //range fresh
    } else if (val_100 < 1000) {
        val_100_range = 1000;
        lab3[1]->setText("1V");                                                 //range fresh
    } else if (val_100 < 5000) {
        val_100_range = 5000;
        lab3[1]->setText("5V");                                                 //range fresh
    } else {
        val_100_range = 10;
        lab3[1]->setText("10mV");                                               //range fresh
    }
    lab3[3]->setText(QString::number(val_100, 'f', 1) + "mV");
    progbar3->setValue(val_100 * 100 / val_100_range);
}
