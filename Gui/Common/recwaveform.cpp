#include "recwaveform.h"
#include <QtDebug>
#include <QFile>
#include <QScrollBar>

#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>

#include "IO/SqlCfg/sqlcfg.h"

#define SHOW_FACTOR 4.0     //保存默认的也是最小的纵轴间距

RecWaveForm::RecWaveForm(int menu_index, QWidget *parent) :
    QWidget(parent)
{
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->setStyleSheet("background:white;");

    this->menu_index = menu_index;
    plot = new QwtPlot(this);


    plot->resize(CHANNEL_X,CHANNEL_Y);

//    plot->insertLegend( new QwtLegend(),  QwtPlot::RightLegend, -2 );
//    plot->legend()->deleteLater();


    curve1 = new QwtPlotCurve();
    curve1->setPen(QPen(Qt::darkBlue, 0, Qt::SolidLine, Qt::RoundCap));
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve1->setTitle("Channel 1");
    curve1->attach(plot);

    curve2 = new QwtPlotCurve();
    curve2->setPen(QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap));
    curve2->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve2->setTitle("Channel 2");
    curve2->attach(plot);


    x = 0;
    max=0;
    min=0;
    scale = 1.0;

    this->hide();
}

RecWaveForm::~RecWaveForm()
{
}

void RecWaveForm::working(CURRENT_KEY_VALUE *val, QString str)
{
    if (val == NULL) {
        return;
    }

    key_val = val;

    if(key_val->grade.val0 != menu_index){
        return;
    }

    setData(str);
    this->show();
}

void RecWaveForm::working(CURRENT_KEY_VALUE *val,VectorList buf, MODE mod)
{
    if (val == NULL) {
        return;
    }

    key_val = val;

    if(key_val->grade.val0 != menu_index){
        return;
    }

    setData(buf,mod);
    this->show();
}

void RecWaveForm::trans_key(quint8 key_code)
{
    //    qDebug()<<"BBBBB"<<key_val->grade.val5;
    if (key_val == NULL) {
        return;
    }

    if(key_val->grade.val5 == 0 ){
        return;
    }
    if(this->isHidden()){
        return;
    }

    switch (key_code) {
    case KEY_OK:

        break;
    case KEY_CANCEL:
        this->hide();
        key_val->grade.val5 = 0;
        if(key_val->grade.val0 != 5){
            key_val->grade.val1 = 0;
            key_val->grade.val2 = 0;
            emit fresh_parent();
        }
        if(mode == Double_Channel){
            plot->legend()->deleteLater();
        }
        break;
    case KEY_UP:
        scale = scale * 1.2;
        break;
    case KEY_DOWN:
        scale = scale / 1.2;
        break;
    case KEY_LEFT:
        if(x<100){
            x = 0;
        }
        else{
            x = x -100;
        }

        break;

    case KEY_RIGHT:
        if(x<wave1.length()-400){
            x = x +100;
        }
        else{
            x = wave1.length() - 300;
        }

        break;
    default:
        break;
    }

    fresh();
}

void RecWaveForm::setData(QString str)
{
    if(str.contains("TEV1_")){
        mode = TEV1;
//        qDebug()<<"tev1";
    }
    else if(str.contains("TEV2_")){
        mode = TEV2;
//        qDebug()<<"tev2";
    }
    else if(str.contains("AAUltrasonic_")){
        mode = AA_Ultrasonic;
//        qDebug()<<"aa";
    }
    else if(str.contains("TEVDouble_")){
        mode = Double_Channel;
//        qDebug()<<"tev_double";
        plot->insertLegend( new QwtLegend(),  QwtPlot::RightLegend, -2 );
    }
    else if(str.contains("HFCT1_")){
        mode = HFCT1;
//        qDebug()<<"HFCT";
    }


    QFile file;
//    if(str.contains("(SDCard)")){
//        file.setFileName("/mmc/sdcard/WaveForm/"+str.remove("(SDCard)")+".DAT");
//    }
//    else{
    file.setFileName(WAVE_DIR"/"+str+".DAT");
//    }

    if (!file.open(QIODevice::ReadOnly)){
        qDebug()<<"file open failed!";
        return;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    QPointF p;
    int i = 0;
    quint32 t1,t2;
    qint16 v;
    curve2->detach();
    wave1.clear();
    wave2.clear();
    max=0;
    min=0;
    double v_real = 0, temp;

    while (!in.atEnd()) {
        in >> t1 >> t2 >> v;

        switch (mode) {
        case TEV1:     //TEV1
            v_real = v * sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,v_real);
            wave1.append(p);
            break;
        case TEV2:     //TEV2
            v_real = v * sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,v_real);
            wave1.append(p);
            break;
        case AA_Ultrasonic:     //AA超声
            v_real = (v * 4) * sqlcfg->get_para()->aaultra_sql.gain * AA_FACTOR;
            p = QPointF(i/320.0,v_real);
            wave1.append(p);
            break;
        case Double_Channel:
            v_real = v * sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,v_real);
            wave1.append(p);
            in >> v;
            temp = v * sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,temp);
            wave2.append(p);
            curve2->attach(plot);
            v_real = MAX(v_real,temp);
            break;
        case HFCT1:     //HFCT
        case HFCT_CONTINUOUS:
            v_real = v * sqlcfg->get_para()->hfct1_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,v_real);
            wave1.append(p);
            break;
        default:
            break;
        }

        if(v_real>max){
            max = v_real;
        }
        else if(v_real<min){
            min = v_real;
        }

        i++;
    }

    if(min > -SHOW_FACTOR){
        min = -SHOW_FACTOR;
    }
    if(max < SHOW_FACTOR){
        max = SHOW_FACTOR;
    }

    plot->setAxisScale(QwtPlot::yLeft, min, max);
    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setSpacing(10);
    if(mode == TEV1 || mode == TEV2 || mode == Double_Channel || mode == HFCT1 || mode == HFCT_CONTINUOUS){
        plot->axisWidget(QwtPlot::xBottom)->setTitle("us");
        plot->axisWidget(QwtPlot::yLeft)->setTitle("V m");
    }
    else if(mode == AA_Ultrasonic){
        plot->axisWidget(QwtPlot::xBottom)->setTitle("ms");
        plot->axisWidget(QwtPlot::yLeft)->setTitle("V u");
    }

    plot->setTitle(str);

    x = 0;   //开始显示最左边数据
    scale = 1.0;    //纵坐标拉伸因子为1

    fresh();

    file.close();

}

void RecWaveForm::setData(VectorList buf, MODE mod)
{
    mode = mod;
    QPointF p;
    wave1.clear();
    max=0;
    min=0;
    double v_real = 0;
    for (int i = 0; i < buf.length(); ++i) {

        switch (mode) {
        case TEV1:     //TEV1
            v_real = sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
//            p = QPointF(i*0.01, buf.at(i));
            break;
        case TEV2:     //TEV2
        case HFCT1:
        case HFCT_CONTINUOUS:
            v_real = sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
//            p = QPointF(i*0.01, buf.at(i));
            break;
        case AA_Ultrasonic:     //AA超声
            v_real = (buf.at(i) <<  2 ) * sqlcfg->get_para()->aaultra_sql.gain * AA_FACTOR;
            p = QPointF(i/320.0, v_real);
//            p = QPointF(i/320.0, buf.at(i));
            break;
        default:
            break;
        }

        if(v_real>max){
            max = v_real;
        }
        else if(v_real<min){
            min = v_real;
        }

        wave1.append(p);
    }

    if(min > -SHOW_FACTOR){
        min = -SHOW_FACTOR;
    }
    if(max < SHOW_FACTOR ){
        max = SHOW_FACTOR;
    }

    plot->setAxisScale(QwtPlot::yLeft, min, max);
    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setSpacing(10);
    if(mode == TEV1 || mode == TEV2 || mode == Double_Channel || mode == HFCT1 || mode == HFCT_CONTINUOUS){
        plot->axisWidget(QwtPlot::xBottom)->setTitle("us");
        plot->axisWidget(QwtPlot::yLeft)->setTitle("V m");
    }
    else if(mode == AA_Ultrasonic){
        plot->axisWidget(QwtPlot::xBottom)->setTitle("ms");
        plot->axisWidget(QwtPlot::yLeft)->setTitle("V u");
    }

    x = 0;   //开始显示最左边数据
    scale = 1.0;    //纵坐标拉伸因子为1

    fresh();
}


void RecWaveForm::setScroll(int value)
{
    if(wave1.length()-value > 300){
        QVector<QPointF> w;
        w = wave1.mid(value,300);
        curve1->setSamples(w);

        if(mode == Double_Channel){
            curve2->setSamples(wave2.mid(value,300));
        }

        //        qDebug()<<"w.first().x() = "<<w.first().x()<<"\tw.last().x() = "<<w.last().x();

        plot->setAxisScale(QwtPlot::xBottom, w.first().x(), w.last().x());
        plot->replot();
    }
}



void RecWaveForm::fresh()
{
    this->setScroll(x);
    plot->setAxisScale(QwtPlot::yLeft, min * scale , max * scale);

    plot->replot();
}









