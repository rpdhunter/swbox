#include "recwaveform.h"
//#include "ui_recwaveform.h"
#include <QtDebug>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <QFile>
#include <QScrollBar>
#include "IO/SqlCfg/sqlcfg.h"

#define SHOW_FACTOR 4.0     //保存默认的也是最小的纵轴间距

RecWaveForm::RecWaveForm(QWidget *parent) :
    QWidget(parent)
{
    this->resize(455, 188);
    this->setStyleSheet("background:white;");

    plot = new QwtPlot(this);


    plot->resize(455,188);


    curve = new QwtPlotCurve();
    curve->setPen(QPen(Qt::darkBlue, 0, Qt::SolidLine, Qt::RoundCap));
//    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve->attach(plot);


    x = 0;
    max=0;
    min=0;
    scale = 1.0;
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

    if(key_val->grade.val0 == 6 && key_val->grade.val1 == 2){       //调试模式
        if(key_val->grade.val3 != 4 || key_val->grade.val4 == 0){                                             //not current menu
            return;
        }
    }

    if(key_val->grade.val0 == 6 && key_val->grade.val1 == 3){       //录波管理
        if(key_val->grade.val5 != 1){                                             //not current menu
            return;
        }
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

    if( (key_val->grade.val0 == 0 && mod == TEV1)
            || (key_val->grade.val0 == 1 && mod == TEV2)
            || (key_val->grade.val0 == 3 && mod == AA_Ultrasonic)){                                             //not current menu
        setData(buf,mod);
        this->show();
    }

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
        if(key_val->grade.val0 != 6){
            key_val->grade.val1 = 0;
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
        if(x<wave.length()-400){
            x = x +100;
        }
        else{
            x = wave.length() - 300;
        }

        break;
    default:
        break;
    }

    fresh();
}

void RecWaveForm::setData(QString str)
{
    if(str.contains("TEV1")){
        mode = TEV1;
    }
    else if(str.contains("TEV2")){
        mode = TEV2;
    }
    else if(str.contains("AAUltrasonic")){
        mode = AA_Ultrasonic;
    }

    QFile file;
//    if(str.contains("(SDCard)")){
//        file.setFileName("/mmc/sdcard/WaveForm/"+str.remove("(SDCard)")+".DAT");
//    }
//    else{
    file.setFileName("/root/WaveForm/"+str+".DAT");
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
    wave.clear();
    max=0;
    min=0;
    double v_real = 0;

    while (!in.atEnd()) {
        in >> t1 >> t2 >> v;

        switch (mode) {
        case TEV1:     //TEV1
            v_real = v * sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR;
            break;
        case TEV2:     //TEV2
            v_real = v * sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR ;
            break;
        case AA_Ultrasonic:     //AA超声
            v_real = (v * 4) * sqlcfg->get_para()->aaultra_sql.gain * AA_FACTOR;
            break;
        default:
            break;
        }

        if(mode == TEV1 || mode == TEV2){
            p = QPointF(i*0.01,v_real);
        }
        else if(mode == AA_Ultrasonic){
            p = QPointF(i/320.0,v_real);
        }

        if(v_real>max){
            max = v_real;
        }
        else if(v_real<min){
            min = v_real;
        }

        wave.append(p);

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
    if(mode == TEV1){
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
    wave.clear();
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

        wave.append(p);
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
    if(mode == TEV1){
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
    if(wave.length()-value > 300){
        QVector<QPointF> w;
        w = wave.mid(value,300);
        curve->setSamples(w);

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









