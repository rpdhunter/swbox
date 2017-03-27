#include "phasespectra.h"

PhaseSpectra::PhaseSpectra(QWidget *parent) : QFrame(parent)
{
    key_val = NULL;

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("PhaseSpectra {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

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
    set_child0_lab->setText(tr("  触发幅值"));

    child0_val_lab = new QLabel(set_child0_lab);
    child0_val_lab->resize(47, 25);
    child0_val_lab->move(100, 0);
    child0_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child0_val_lab->setStyleSheet("QLabel {color:gray;}");
    child0_val_lab->setAlignment(Qt::AlignVCenter);
    child0_val_lab->setText(tr("[  ]"));

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
    child1_val_lab->setText(tr("[44]dB"));

    set_child2_lab = new QLabel(this);
    set_child2_lab->resize(147, 25);
    set_child2_lab->move(set_lab->x(), set_child1_lab->y() + set_child1_lab->height());
    set_child2_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child2_lab->setAlignment(Qt::AlignVCenter);
    set_child2_lab->setText(tr("  相位偏移"));
    child2_val_lab = new QLabel(set_child2_lab);
    child2_val_lab->resize(47, 25);
    child2_val_lab->move(100, 0);
    child2_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child2_val_lab->setStyleSheet("QLabel {color:gray;}");
    child2_val_lab->setAlignment(Qt::AlignVCenter);
    child2_val_lab->setText(tr("[15]"));

    set_child3_lab = new QLabel(this);
    set_child3_lab->resize(147, 25);
    set_child3_lab->move(set_lab->x(), set_child2_lab->y() + set_child2_lab->height());
    set_child3_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    set_child3_lab->setAlignment(Qt::AlignVCenter);
    set_child3_lab->setText(tr("  同步方式"));

    child3_val_lab = new QLabel(set_child3_lab);
    child3_val_lab->resize(47, 25);
    child3_val_lab->move(100, 0);
    child3_val_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    child3_val_lab->setStyleSheet("QLabel {color:gray;}");
    child3_val_lab->setAlignment(Qt::AlignVCenter);
    child3_val_lab->setText(tr("[15]S"));
#if 1
    set_child0_lab->hide();
    set_child1_lab->hide();
    set_child2_lab->hide();
    set_child3_lab->hide();
#endif

    /* view wave */
    plot = new QwtPlot(this);
    //plot->resize(240, 130);
    plot->resize(225, 100);
    plot->move(50, set_lab->y() + 40);
    plot->setStyleSheet("background:transparent;color:gray;font-family:Moonracer;font-size:10px;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 360, 90);
    //plot->axisWidget(QwtPlot::xBottom)->setTitle("相位(度)");
    plot->setAxisMaxMinor( QwtPlot::xBottom, 0);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    //plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    //plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    plot->setAxisScale(QwtPlot::yLeft, 0, 20, 5);
    //plot->setAxisTitle(QwtPlot::yLeft, tr("幅值"));
    plot->setAxisMaxMinor(QwtPlot::yLeft, 0);
    //plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    //plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    //plot->setAxisMaxMajor(QwtPlot::yLeft, 5);

    /* remove gap */
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);


    plot->plotLayout()->setAlignCanvasToScales(true);

    /* display */
    curve = new QwtPlotCurve();
    curve->setPen(QPen(Qt::yellow, 0, Qt::SolidLine, Qt::RoundCap));
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve->attach(plot);

    /* insert wave data */
    wave.clear();
    for (int i = 0; i < 360; i++) {
        wave.push_back(QPointF(i, 10 + 10 * qSin(i * 8 * M_PI / 360)));
    }
    curve->setSamples(wave);
    //plot->replot();
}

void PhaseSpectra::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void PhaseSpectra::trans_key(quint8 key_code)
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

void PhaseSpectra::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPen pen;
    QPainter paint(this);
    QFont font;

    pen.setColor(Qt::gray);
    paint.setPen(pen);

    /* draw color belt */
    color_belt = new QLinearGradient(QPointF(265, 40), QPointF(265, 138));

    /* insert color in postion */
    color_belt->setColorAt(0.0, QColor(244, 114, 121));
    color_belt->setColorAt(1.0, QColor(31, 29, 29));
    paint.setBrush(QBrush(*color_belt));

    /* draw rect */
    color_rect = new QRect(370, 40, 10, 98);
    paint.drawRect(*color_rect);

    paint.drawText(plot->x() - 15, plot->y() + 5, 20, 40, Qt::TextWordWrap, tr("幅值"));
    paint.drawText(color_rect->x() + 20, color_rect->y() + 15, 20, 50, Qt::TextWordWrap, tr("内同步"));

    paint.drawText(plot->x() + plot->width() + 10, plot->y() + plot->height() - 10, tr("相位(度)"));

    font.setFamily("Moonracer");
    font.setPixelSize(10);
    paint.setFont(font);
    paint.drawText(plot->x() - 10, plot->y() - 5, tr("20mV"));
 }
