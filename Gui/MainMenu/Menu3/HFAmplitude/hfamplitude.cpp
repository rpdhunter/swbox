#include "hfamplitude.h"

HFAmplitude::HFAmplitude(QWidget *parent) : QFrame(parent)
{
    key_val = NULL;

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("HFAmplitude {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

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
    child0_val_lab->setText(tr("[连续]"));

    set_child1_lab = new QLabel(this);
    set_child1_lab->resize(147, 25);
    set_child1_lab->move(set_lab->x(), set_child0_lab->y() + set_child0_lab->height());
    set_child1_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child1_lab->setAlignment(Qt::AlignVCenter);
    set_child1_lab->setText(tr("  红色报警阀值"));

    child1_val_lab = new QLabel(set_child1_lab);
    child1_val_lab->resize(47, 25);
    child1_val_lab->move(100, 0);
    child1_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child1_val_lab->setStyleSheet("QLabel {color:gray;}");
    child1_val_lab->setAlignment(Qt::AlignVCenter);
    child1_val_lab->setText(tr("[44]dB"));

    set_child2_lab = new QLabel(this);
    set_child2_lab->resize(147, 25);
    set_child2_lab->move(set_lab->x(), set_child1_lab->y() + set_child1_lab->height());
    set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child2_lab->setAlignment(Qt::AlignVCenter);
    set_child2_lab->setText(tr("  黄色报警阀值"));

    child2_val_lab = new QLabel(set_child2_lab);
    child2_val_lab->resize(47, 25);
    child2_val_lab->move(100, 0);
    child2_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child2_val_lab->setStyleSheet("QLabel {color:gray;}");
    child2_val_lab->setAlignment(Qt::AlignVCenter);
    child2_val_lab->setText(tr("[15]dB"));
#if 1
    set_child0_lab->hide();
    set_child1_lab->hide();
    set_child2_lab->hide();
#endif

    /* view TEV */
    tev_lab = new QLabel(this);
    tev_lab->resize(30, 20);
    tev_lab->move(370, 5);
    tev_lab->setStyleSheet("QLabel {color:white;}");
    tev_lab->setAlignment(Qt::AlignVCenter);
    tev_lab->setText(tr("TEV"));

    /* view max value */
    max_val_lab = new QLabel(this);
    max_val_lab->resize(90, 20);
    max_val_lab->move(tev_lab->x() - 80, 30);
    max_val_lab->setStyleSheet("QLabel {color:white;}");
    max_val_lab->setAlignment(Qt::AlignVCenter);
    max_val_lab->setText(tr("最大值: 45dB"));

    /* view color block */
    yellow_lab = new QLabel(this);
    yellow_lab->resize(22, 22);
    yellow_lab->move(max_val_lab->x(), max_val_lab->y() + 30);
    yellow_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:yellow;}");

    green_lab = new QLabel(this);
    green_lab->resize(22, 22);
    green_lab->move(yellow_lab->x() + 20, yellow_lab->y());
    green_lab->setStyleSheet("QLabel {border:2px solid gray;background:transparent;}");

    red_lab = new QLabel(this);
    red_lab->resize(22, 22);
    red_lab->move(green_lab->x() + 20, yellow_lab->y());
    red_lab->setStyleSheet("QLabel {border:2px solid gray;background-color:red;}");

    /* view amplitude history value */
    amp_his_val = new QLabel(this);
    amp_his_val->resize(75, 40);
    amp_his_val->move(yellow_lab->x(), yellow_lab->y() + 50);
    amp_his_val->setStyleSheet("QLabel {font-family:Moonracer;font-size:60px;color:white}");
    amp_his_val->setAlignment(Qt::AlignCenter);
    amp_his_val->setText("45");

    /* view db */
    db_lab = new QLabel(this);
    db_lab->resize(30, 20);
    db_lab->move(amp_his_val->x() + amp_his_val->width(), amp_his_val->y() + 20);
    db_lab->setStyleSheet("QLabel {font-family:SimSun;font-size:20px;color:white}");
    db_lab->setAlignment(Qt::AlignCenter);
    db_lab->setText("dB");

    /* view barchart */
    plot = new QwtPlot(this);
    plot->resize(170, 130);
    plot->move(50, set_lab->y() + 30);
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

    d_chart = new HFBarChart(plot);
}

void HFAmplitude::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void HFAmplitude::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 1) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        break;
    case KEY_CANCEL:
        if (!key_val->grade.val3) {
            key_val->grade.val2 = 0;
            fresh_parent();
        }
        break;
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:

        break;

    case KEY_RIGHT:

        break;
    default:
        break;
    }
}
