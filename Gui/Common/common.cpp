﻿#include "common.h"
#include <QtDebug>

#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_plot.h>
#include <QTime>
#include "Algorithm/fir.h"
#include "Algorithm/Wavelet/wavelet.h"

Common::Common(QObject *parent) : QObject(parent)
{

}

void Common::change_index(unsigned char &index, int d_index, int max_index, int min_index)
{
    if(max_index <= min_index){
        qDebug()<<"warning in Common::change_index()";
    }

    int t = (int)index;
    t += d_index;
    if(t > max_index) {
        t = min_index;
    }
    else if(t < min_index){
        t = max_index;
    }

    index = (unsigned char)t;
}

void Common::change_index(double &index, double d_index, double max_index, double min_index)
{
    if(max_index <= min_index){
        qDebug()<<"warning in Common::change_index()";
    }

    index += d_index;
    if(index > max_index) {
        index = min_index;
    }
    else if(index < min_index){
        index = max_index;
    }
}

void Common::change_index(int &index, int d_index, int max_index, int min_index)
{
    if(max_index <= min_index){
        qDebug()<<"warning in Common::change_index()";
    }

    index += d_index;
    if(index > max_index) {
        index = min_index;
    }
    else if(index < min_index){
        index = max_index;
    }
}

void Common::change_index(int &index, int d_index, QList<int> list)
{
    int current_index = list.indexOf(index);
    if(current_index != -1){
        current_index += d_index;
        if(current_index >= list.length()){
            current_index = 0;
        }
        if(current_index < 0){
            current_index = list.length()-1;
        }
        index = list.at(current_index);
    }
    else{
        qDebug()<<"input error in Common::change_index()"<<list;
        index = list.first();
    }
}

void Common::change_value(int &value, int value_a, int value_b)
{
    if(value == value_a){
        value = value_b;
    }
    else{
        value = value_a;
    }
}

void Common::change_value(bool &value, bool value_a, bool value_b)
{
    if(value == value_a){
        value = value_b;
    }
    else{
        value = value_a;
    }
}

void Common::set_comboBox_style(QComboBox *comboBox)
{
    if(sqlcfg->get_para()->language == CN){
        comboBox->resize(150,22);
    }
    else{
        comboBox->resize(240,22);
    }
    QLineEdit *lineEdit = new QLineEdit;
    comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/bk/para_child.png);color:gray; }");
    comboBox->setLineEdit(lineEdit);
    comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    comboBox->lineEdit()->setReadOnly(true);
    comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/bk/para_child.png);color:gray}");
    comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
//    comboBox->view()->setFrameShadow(QFrame::Plain);
    comboBox->view()->setFrameShape(QFrame::NoFrame);
    //    QComboBox QAbstractItemView::item { min-height: 40px; min-width: 60px; };
//    comboBox->setFrame(true);
}

void Common::set_contextMenu_style(QListWidget *w, QStringList list, QPoint pos)
{
    w->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    w->addItems(list);
    int n = list.count();
    if(sqlcfg->get_para()->language == CN){
        w->resize(110, n * 20);
    }
    else{
        w->resize(150, n * 20);
    }

    w->move(pos);
    w->setSpacing(2);
    w->hide();
}

void Common::set_barchart_style(QwtPlot *plot, int v_max)
{
    plot->setStyleSheet("background:transparent;color:gray;");
    plot->setAxisScale(QwtPlot::xBottom, 0, 13);
    plot->setAxisScale(QwtPlot::yLeft, 0, v_max, v_max/3);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    plot->plotLayout()->setAlignCanvasToScales(true);


    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
}

void Common::set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value, PRPD_MODE mode)
{
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 360, 180);
    if(mode == PRPD_double){
        plot->setAxisScale(QwtPlot::yLeft, -max_value, max_value, max_value);
    }
    else{
        plot->setAxisScale(QwtPlot::yLeft, 0, max_value, max_value/2);
    }


    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    //    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    QwtPlotCurve *curve_grid = new QwtPlotCurve();
    curve_grid->setPen(QPen(Qt::gray, 0, Qt::SolidLine, Qt::RoundCap));
    curve_grid->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve_grid->attach(plot);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( false );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach(plot);

    QVector<double> X,Y;
    for(int i=0;i<360;i++){
        X.append(i);
        if(mode == PRPD_double){
            Y.append(max_value*qSin(2*3.1416*i/360));
        }
        else{
            Y.append((max_value/2)*(qSin(2*3.1416*i/360) + 1));
        }
    }
    curve_grid->setSamples(X,Y);

    QwtLinearColorMap *colorMap = new QwtLinearColorMap;
    colorMap->setColorInterval(Qt::blue,Qt::red);
    colorMap->addColorStop(0.3,Qt::green);
    colorMap->addColorStop(0.6,Qt::yellow);
    d_PRPD->setColorMap(colorMap);
    d_PRPD->setColorRange(QwtInterval(1,6));
    QwtScaleWidget *rightAxis = plot->axisWidget( QwtPlot::yRight );
    rightAxis->setColorBarEnabled( true );
    rightAxis->setColorMap(QwtInterval(1,6),colorMap);

    plot->setAxisScale( QwtPlot::yRight, 1, 6 );
    plot->enableAxis( QwtPlot::yRight );

    plot->plotLayout()->setAlignCanvasToScales( true );
    d_PRPD->attach(plot);
}

void Common::set_fly_style(QwtPlot *plot, QwtPlotSpectroCurve *d_fly, int max_value)
{
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 200);
    plot->setAxisScale(QwtPlot::yLeft, 0, max_value);
    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    d_fly->attach(plot);
}

void Common::set_TF_style(QwtPlot *plot, QwtPlotSpectroCurve *d_TF, int max_value)
{
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 360, 180);
    plot->setAxisScale(QwtPlot::yLeft, -max_value, max_value, max_value);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    //    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    QwtLinearColorMap *colorMap = new QwtLinearColorMap;
    colorMap->setColorInterval(Qt::blue,Qt::red);
    colorMap->addColorStop(0.3,Qt::green);
    colorMap->addColorStop(0.6,Qt::yellow);
    d_TF->setColorMap(colorMap);
    d_TF->setColorRange(QwtInterval(1,6));
    QwtScaleWidget *rightAxis = plot->axisWidget( QwtPlot::yRight );
    rightAxis->setColorBarEnabled( true );
    rightAxis->setColorMap(QwtInterval(1,6),colorMap);

    plot->setAxisScale( QwtPlot::yRight, 1, 6 );
    plot->enableAxis( QwtPlot::yRight );

    plot->plotLayout()->setAlignCanvasToScales( true );
    d_TF->attach(plot);
}

void Common::set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram, int xBottom_min, int xBottom_max,
                                 int yLeft_min, int yLeft_max, QString title)
{
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, xBottom_min, xBottom_max);
    plot->setAxisScale(QwtPlot::yLeft, yLeft_min, yLeft_max);
    //    plot->setAxisScale(QwtPlot::xBottom, 0, 6);
    //    plot->setAxisScale(QwtPlot::yLeft, 0.01, 10);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::xBottom)->setTitle(title);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    //    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    d_histogram->setStyle( QwtPlotHistogram::Columns );
    d_histogram->setBrush(Qt::green);
    d_histogram->setPen(Qt::green);
    d_histogram->attach(plot);
}

void Common::set_Spectra_style(QwtPlot *plot, QwtPlotHistogram *d_histogram, int xBottom_min, int xBottom_max, int yLeft_min, int yLeft_max, QString title)
{
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, xBottom_min, xBottom_max);
    plot->setAxisScale(QwtPlot::yLeft, yLeft_min, yLeft_max);
    //    plot->setAxisScale(QwtPlot::xBottom, 0, 6);
    //    plot->setAxisScale(QwtPlot::yLeft, 0.01, 10);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::xBottom)->setTitle(title);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    //    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    //    d_histogram->setStyle( QwtPlotHistogram::Outline );
    d_histogram->setStyle( QwtPlotHistogram::Columns );
    d_histogram->setBrush(Qt::yellow);
    d_histogram->setPen(Qt::yellow);
    d_histogram->attach(plot);
}

void Common::setTab(QLabel *label)
{
    label->resize(70, 28);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

double Common::physical_value(int code_value, MODE mode)
{
    double v_real=0;
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        v_real = code_value * sqlcfg->get_para()->tev1_sql.gain * H_C_FACTOR;
        break;
    case TEV2:
    case TEV_CONTINUOUS2:
        v_real = code_value * sqlcfg->get_para()->tev2_sql.gain * H_C_FACTOR;
        break;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        v_real = code_value * sqlcfg->get_para()->hfct1_sql.gain * H_C_FACTOR;
        break;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        v_real = code_value * sqlcfg->get_para()->hfct2_sql.gain * H_C_FACTOR;
        break;
    case UHF1:
    case UHF_CONTINUOUS1:
        v_real = code_value * sqlcfg->get_para()->uhf1_sql.gain * H_C_FACTOR;
        break;
    case UHF2:
    case UHF_CONTINUOUS2:
        v_real = code_value * sqlcfg->get_para()->uhf2_sql.gain * H_C_FACTOR;
        break;
    case AA1:
    case AA_ENVELOPE1:
        v_real = (code_value * 4) * sqlcfg->get_para()->aa1_sql.gain * AA_FACTOR;
        v_real = Compute::l_channel_modify(v_real);
        break;
    case AA2:
    case AA_ENVELOPE2:
        v_real = (code_value * 4) * sqlcfg->get_para()->aa2_sql.gain * AA_FACTOR;
        v_real = Compute::l_channel_modify(v_real);
        break;
    case AE1:
    case AE_ENVELOPE1:
        v_real = (code_value * 4) * sqlcfg->get_para()->ae1_sql.gain * sqlcfg->ae1_factor();
        v_real = Compute::l_channel_modify(v_real);
        break;
    case AE2:
    case AE_ENVELOPE2:
        v_real = (code_value * 4) * sqlcfg->get_para()->ae2_sql.gain * sqlcfg->ae2_factor();
        v_real = Compute::l_channel_modify(v_real);
        break;
    default:
        break;
    }
    return v_real;
}

double Common::physical_value(int code_value, double gain, MODE mode)
{
    double v_real=0;
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
    case TEV2:
    case TEV_CONTINUOUS2:
    case HFCT1:
    case HFCT_CONTINUOUS1:
    case HFCT2:
    case HFCT_CONTINUOUS2:
    case UHF1:
    case UHF_CONTINUOUS1:
    case UHF2:
    case UHF_CONTINUOUS2:
        v_real = code_value * gain * H_C_FACTOR;
        break;
    case AA1:
    case AA_ENVELOPE1:
    case AA2:
    case AA_ENVELOPE2:
        v_real = (code_value * 4) * gain * AA_FACTOR;
        v_real = Compute::l_channel_modify(v_real);
        break;
    case AE1:
    case AE_ENVELOPE1:
        v_real = (code_value * 4) * gain * sqlcfg->ae1_factor();
        v_real = Compute::l_channel_modify(v_real);
        break;
    case AE2:
    case AE_ENVELOPE2:
        v_real = (code_value * 4) * gain * sqlcfg->ae2_factor();
        v_real = Compute::l_channel_modify(v_real);
        break;
    default:
        break;
    }
    return v_real;
}

int Common::code_value(double physical_value, MODE mode)
{
    int v_code=0;
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        v_code = physical_value / sqlcfg->get_para()->tev1_sql.gain / H_C_FACTOR;
        break;
    case TEV2:
    case TEV_CONTINUOUS2:
        v_code = physical_value / sqlcfg->get_para()->tev2_sql.gain / H_C_FACTOR;
        break;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        v_code = physical_value / sqlcfg->get_para()->hfct1_sql.gain / H_C_FACTOR;
        break;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        v_code = physical_value / sqlcfg->get_para()->hfct2_sql.gain / H_C_FACTOR;
        break;
    case UHF1:
    case UHF_CONTINUOUS1:
        v_code = physical_value / sqlcfg->get_para()->uhf1_sql.gain / H_C_FACTOR;
        break;
    case UHF2:
    case UHF_CONTINUOUS2:
        v_code = physical_value / sqlcfg->get_para()->uhf2_sql.gain / H_C_FACTOR;
        break;
    case AA1:
        v_code = physical_value / 4 / sqlcfg->get_para()->aa1_sql.gain / AA_FACTOR;
        break;
    case AA2:
        v_code = physical_value / 4 / sqlcfg->get_para()->aa2_sql.gain / AA_FACTOR;
        break;
    case AE1:
        v_code = physical_value / 4 / sqlcfg->get_para()->ae1_sql.gain / sqlcfg->ae1_factor();
        break;
    case AE2:
        v_code = physical_value / 4 / sqlcfg->get_para()->ae2_sql.gain / sqlcfg->ae2_factor();
        break;
    default:
        break;
    }
    return v_code;
}

double Common::physical_threshold(MODE mode)
{
    double v_real=0;
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        v_real = sqlcfg->get_para()->tev1_sql.fpga_threshold;
        break;
    case TEV2:
    case TEV_CONTINUOUS2:
        v_real = sqlcfg->get_para()->tev2_sql.fpga_threshold;
        break;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        v_real = sqlcfg->get_para()->hfct1_sql.fpga_threshold;
        break;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        v_real = sqlcfg->get_para()->hfct2_sql.fpga_threshold;
        break;
    case UHF1:
    case UHF_CONTINUOUS1:
        v_real = sqlcfg->get_para()->uhf1_sql.fpga_threshold;
        break;
    case UHF2:
    case UHF_CONTINUOUS2:
        v_real = sqlcfg->get_para()->uhf2_sql.fpga_threshold;
        break;
    case Double_Channel:        //to be
        v_real = sqlcfg->get_para()->tev1_sql.fpga_threshold;
        break;
    case AA1:
    case AA_ENVELOPE1:
        v_real = sqlcfg->get_para()->aa1_sql.fpga_threshold;
        break;
    case AA2:
    case AA_ENVELOPE2:
        v_real = sqlcfg->get_para()->aa2_sql.fpga_threshold;
        break;
    case AE1:
    case AE_ENVELOPE1:
        v_real = sqlcfg->get_para()->ae1_sql.fpga_threshold;
        break;
    case AE2:
    case AE_ENVELOPE2:
        v_real = sqlcfg->get_para()->ae2_sql.fpga_threshold;
        break;
    default:
        break;
    }
    return v_real;
}

qint32 Common::offset_zero_code(MODE mode)
{
    qint32 v_code=0;
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        v_code = sqlcfg->get_para()->tev1_sql.fpga_zero;
        break;
    case TEV2:
    case TEV_CONTINUOUS2:
        v_code = sqlcfg->get_para()->tev2_sql.fpga_zero;
        break;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        v_code = sqlcfg->get_para()->hfct1_sql.fpga_zero;
        break;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        v_code = sqlcfg->get_para()->hfct2_sql.fpga_zero;
        break;
    case UHF1:
    case UHF_CONTINUOUS1:
        v_code = sqlcfg->get_para()->uhf1_sql.fpga_zero;
        break;
    case UHF2:
    case UHF_CONTINUOUS2:
        v_code = sqlcfg->get_para()->uhf2_sql.fpga_zero;
        break;
    default:
        break;
    }
    return v_code;
}

QString Common::mode_to_string(MODE val)
{
    QString tmpStr;
    switch (val) {
    case Disable:
        tmpStr = "Disable";
        break;
    case TEV1:
        tmpStr = "TEV1";
        break;
    case TEV2:
        tmpStr = "TEV2";
        break;
    case HFCT1:
        tmpStr = "HFCT1";
        break;
    case HFCT2:
        tmpStr = "HFCT2";
        break;
    case UHF1:
        tmpStr = "UHF1";
        break;
    case UHF2:
        tmpStr = "UHF2";
        break;
    case TEV_CONTINUOUS1:
        tmpStr = "TEV1_CONTINUOUS";
        break;
    case TEV_CONTINUOUS2:
        tmpStr = "TEV2_CONTINUOUS";
        break;
    case HFCT_CONTINUOUS1:
        tmpStr = "HFCT1_CONTINUOUS";
        break;
    case HFCT_CONTINUOUS2:
        tmpStr = "HFCT2_CONTINUOUS";
        break;
    case UHF_CONTINUOUS1:
        tmpStr = "UHF1_CONTINUOUS";
        break;
    case UHF_CONTINUOUS2:
        tmpStr = "UHF2_CONTINUOUS";
        break;
    case AA1:
        tmpStr = "AA1";
        break;
    case AA2:
        tmpStr = "AA2";
        break;
    case AE1:
        tmpStr = "AE1";
        break;
    case AE2:
        tmpStr = "AE2";
        break;
    case AA_ENVELOPE1:
        tmpStr = "AA1_ENVELOPE";
        break;
    case AA_ENVELOPE2:
        tmpStr = "AA2_ENVELOPE";
        break;
    case AE_ENVELOPE1:
        tmpStr = "AE1_ENVELOPE";
        break;
    case AE_ENVELOPE2:
        tmpStr = "AE2_ENVELOPE";
        break;
    case Double_Channel:
        tmpStr = "Double_Channel";
        break;
    case Options_Mode:
        tmpStr = "Options_Mode";
        break;
    default:
        break;
    }
    return tmpStr;
}

MODE Common::string_to_mode(QString str)
{
    if(str.contains("TEV1"))
        return TEV1;
    if(str.contains("TEV2"))
        return TEV2;
    if(str.contains("HFCT1"))
        return HFCT1;
    if(str.contains("HFCT2"))
        return HFCT2;
    if(str.contains("UHF1"))
        return UHF1;
    if(str.contains("UHF2"))
        return UHF2;
    if(str.contains("AA1"))
        return AA1;
    if(str.contains("AA2"))
        return AA2;
    if(str.contains("AE1"))
        return AE1;
    if(str.contains("AE2"))
        return AE2;
    if(str.contains("TEV_CONTINUOUS1"))
        return TEV_CONTINUOUS1;
    if(str.contains("TEV_CONTINUOUS2"))
        return TEV_CONTINUOUS2;
    if(str.contains("HFCT_CONTINUOUS1"))
        return HFCT_CONTINUOUS1;
    if(str.contains("HFCT_CONTINUOUS2"))
        return HFCT_CONTINUOUS2;
    if(str.contains("UHF_CONTINUOUS1"))
        return UHF_CONTINUOUS1;
    if(str.contains("UHF_CONTINUOUS2"))
        return UHF_CONTINUOUS2;
    if(str.contains("AA_ENVELOPE1"))
        return AA_ENVELOPE1;
    if(str.contains("AA_ENVELOPE2"))
        return AA_ENVELOPE2;
    if(str.contains("AE_ENVELOPE1"))
        return AE_ENVELOPE1;
    if(str.contains("AE_ENVELOPE2"))
        return AE_ENVELOPE2;
    return Disable;
}

void Common::write_fpga_offset_debug(G_PARA *data)
{
    int h1=0, h2=0;
    switch (sqlcfg->get_para()->menu_h1) {
    case TEV1:
        data->set_send_para (sp_h1_zero, 0x8000 + sqlcfg->get_para()->tev1_sql.fpga_zero );
        h1 = sqlcfg->get_para()->tev1_sql.filter_fir_fpga;
        break;
    case HFCT1:
        data->set_send_para (sp_h1_zero, 0x8000 + sqlcfg->get_para()->hfct1_sql.fpga_zero );
        h1 = sqlcfg->get_para()->hfct1_sql.filter_fir_fpga;
        break;
    case UHF1:
        data->set_send_para (sp_h1_zero, 0x8000 + sqlcfg->get_para()->uhf1_sql.fpga_zero );
        h1 = sqlcfg->get_para()->uhf1_sql.filter_fir_fpga;
        break;
    default:
        break;
    }

    switch (sqlcfg->get_para()->menu_h2) {
    case TEV2:
        data->set_send_para (sp_h2_zero, 0x8000 + sqlcfg->get_para()->tev2_sql.fpga_zero );
        h2 = sqlcfg->get_para()->tev2_sql.filter_fir_fpga;
        break;
    case HFCT2:
        data->set_send_para (sp_h2_zero, 0x8000 + sqlcfg->get_para()->hfct2_sql.fpga_zero );
        h2 = sqlcfg->get_para()->hfct2_sql.filter_fir_fpga;
        break;
    case UHF2:
        data->set_send_para (sp_h2_zero, 0x8000 + sqlcfg->get_para()->uhf2_sql.fpga_zero );
        h2 = sqlcfg->get_para()->uhf2_sql.filter_fir_fpga;
        break;
    default:
        break;
    }
    data->set_send_para(sp_filter_mode, h1 + (h2<<8) );

    switch (sqlcfg->get_para()->menu_l1) {
    case AA1:
        //        data->set_send_para (sp_vol_l1, sqlcfg->get_para()->aa1_sql.vol);
        data->set_send_para (sp_l1_channnel_mode, sqlcfg->get_para()->aa1_sql.envelope);
        break;
    case AE1:
        //        data->set_send_para (sp_vol_l1, sqlcfg->get_para()->ae1_sql.vol);
        data->set_send_para (sp_l1_channnel_mode, sqlcfg->get_para()->ae1_sql.envelope);
        break;
    default:
        break;
    }

    switch (sqlcfg->get_para()->menu_l2) {
    case AA2:
        //        data->set_send_para (sp_vol_l2, sqlcfg->get_para()->aa2_sql.vol);
        data->set_send_para (sp_l2_channnel_mode, sqlcfg->get_para()->aa2_sql.envelope);
        break;
    case AE2:
        //        data->set_send_para (sp_vol_l2, sqlcfg->get_para()->ae2_sql.vol);
        data->set_send_para (sp_l2_channnel_mode, sqlcfg->get_para()->ae2_sql.envelope);
        break;
    default:
        break;
    }
}

//在给定数组中，根据给定的阈值判定脉冲, 每个脉冲点x坐标为相位,y坐标为物理db值,max_num为脉冲计算最大数量
QVector<QPoint> Common::calc_pulse_list(QVector<int> datalist, QVector<int> timelist, int threshold , MODE mode, int max_num)
{
    QVector<QPoint> pulse_list;
    int code_v = code_value(threshold, mode);       //根据物理阈值计算出码值
    int x, y;
    if(datalist.count() < 5){
        return pulse_list;
    }
    for (int i = 2; i < datalist.count()-4; ++i) {
        //注意判定当前点为峰值点的条件为【大于阈值】【大于前点，大于等于后点】，这样避免了方波出现连续波峰的情况，又不会遗漏
        if(datalist.at(i) > code_v
                && datalist.at(i-1)-datalist.at(i-2) > 0
                && datalist.at(i)-datalist.at(i-1) > 0
                && datalist.at(i) - datalist.at(i+1) >= 0
                && datalist.at(i+1) - datalist.at(i+2) >= 0){
            x = time_to_phase(timelist.at(i/128) + (i%128)*(320000/128) );
            y = physical_value(datalist.at(i),mode);        //转换成物理值
            y = (int)20*log10(qAbs(y) );                    //转换成DB值
            pulse_list.append(QPoint(x,y));
            if(pulse_list.count() > max_num  && max_num != 0){
                break;
            }
            i+=3;
        }
    }
    return pulse_list;
}

int Common::time_to_phase(quint32 x)
{
    double y = 0;
    if(sqlcfg->get_para()->freq_val == 50){
        //        x = x % 2000000;          //取余数
        y = x * 360.0 / 2000000;     //使用浮点数防止溢出
    }
    else if(sqlcfg->get_para()->freq_val == 60){
        //        x = x % 1666667;
        y = x * 360.0 / 1666667;
    }
    return (int)y;
}

QVector<int> Common::smooth(QVector<int> datalist, int n)
{
    QVector<int> templist;
    int t;
    if(datalist.count() > n){

        for (int i = 0; i < datalist.count() - n; ++i) {
            t = 0;
            for (int j = 0; j < n; ++j) {
                t += datalist.at(i + j);
            }
            templist.append(t / n);
        }
    }
    return templist;
}

QVector<int> Common::smooth_2(QVector<int> datalist, int n)
{
    return smooth( smooth(datalist,n), n );
}

float Common::kalman_filter_core(float ResrcData, float ProcessNiose_Q, float MeasureNoise_R, float &x_last, float &p_last)
{
    float R = MeasureNoise_R;
    float Q = ProcessNiose_Q;

    //    static float x_last;
    float x_mid = x_last;
    float x_now;

    //    static float p_last;
    float p_mid ;
    float p_now;

    float kg;

    x_mid=x_last;                       //x_last=x(k-1|k-1),x_mid=x(k|k-1)
    p_mid=p_last+Q;                     //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=??

    /*
     *  卡尔曼滤波的五个重要公式
     */
    kg=p_mid/(p_mid+R);                 //kg为kalman filter，R 为噪声
    x_now=x_mid+kg*(ResrcData-x_mid);   //估计出的最优值
    p_now=(1-kg)*p_mid;                 //最优值对应的covariance
    p_last = p_now;                     //更新covariance 值
    x_last = x_now;                     //更新系统状态值
    //    qDebug()<<"kg = "<<kg;

    return x_now;

}

QVector<int> Common::kalman_filter(QVector<int> wave)
{
    QVector<int> out;
    float x_last = 0, p_last = 0;
    foreach (int w, wave) {
        out.append( kalman_filter_core(w, KALMAN_Q, KALMAN_R, x_last, p_last) );

    }
    //        qDebug()<<"p_last="<<p_last;
    return out;
}

double Common::avrage(QVector<double> list)
{
    if(list.isEmpty())
        return 0;

    double tmp=0;
    foreach (double p, list) {
        tmp += p;
    }
    return tmp / list.count();
}

int Common::avrage(QVector<int> list)
{
    if(list.isEmpty())
        return 0;

    int tmp=0;
    foreach (int p, list) {
        tmp += p;
    }
    return tmp / list.count();
}

float Common::avrage(QList<float> list)
{
    if(list.isEmpty())
        return 0;

    float tmp=0;
    foreach (float p, list) {
        tmp += p;
    }
    return tmp / list.count();
}

double Common::sum(QVector<double> list)
{
    double tmp=0;
    foreach (double p, list) {
        tmp += p;
    }
    return tmp;
}

int Common::sum(QVector<int> list)
{
    int tmp=0;
    foreach (int p, list) {
        tmp += p;
    }
    return tmp;
}

double Common::tev_freq_compensation(int pulse_num)
{
    int f = pulse_num / 2000000;     //单位是M
    double k = 1;
    if(f>=3 && f<8){
        k = -0.04 * f + 1.12;
    }
    else if(f>=8 && f<13){
        k = 0.04 * f + 0.48;
    }
    else if(f>=13 && f<18){
        k = 0.1 * f - 0.3;
    }
    else if(f>=18 && f<30){
        k = -0.1 * f + 3.3;
    }
    else if(f>=30){
        k = 0.2;
    }
    return k;
}

QString Common::secton_three(int n)
{
    QString tmpstr = QString::number(n);
    QStringList slist;
    while(tmpstr.count() > 3){
        slist<<tmpstr.mid(tmpstr.count()-3,3);
        tmpstr.remove(tmpstr.count()-3,3);
    }
    slist<<tmpstr;
    tmpstr.clear();
    for (int i = slist.count()-1; i >=0; i--) {
        tmpstr.append(slist.at(i));
        tmpstr.append(" ");
    }
    return tmpstr;
}

//使用硬连接，可以做到直接拷贝
void Common::create_hard_link(QString str_src, QString file_name){
    QString str_dst = QString(sqlcfg->get_global()->current_dir);
    if(str_dst.contains("asset")){
        QString cmd = QString("ln \"%1\" \"%2\"").arg(str_src).arg(str_dst + "/" + file_name);      //文件名加入双引号，为了应对带空格的文件名
        //        printf("create_hard_link %s \n", cmd.toLocal8Bit().data());
        system(cmd.toLocal8Bit().data());
    }
}

QLabel *Common::set_mainwindow_lab(QLabel *lab, int n, QTabWidget *widget)
{
    lab->resize(41, 24);
    widget->tabBar()->setTabButton(n,QTabBar::LeftSide,lab);
    return lab;
}

QString Common::str_to_cn(QString str)
{
    QByteArray byte = str.toUtf8();
    char *c = byte.data();
    return QString().fromLocal8Bit(c);
}

//实现创建文件夹操作
//这里之所以用标准字符串中介一下，是因为直接使用QString会造成乱码，原因未知
bool Common::mk_dir(QString path/*, QDir &dir*/)
{
    QDir dir;
    path = str_to_cn(path);

    if(!dir.exists(path) ){
        dir.mkdir(path );
    }
    if(dir.exists(path)){
        //        printf("check dir: %s  succeed!\n",path.toLocal8Bit().data());
        return true;
    }
    else{
        printf("check dir: %s  failed!\n",path.toLocal8Bit().data());
        return false;
    }
}

bool Common::del_dir(QString path)
{
    QDir dir;
    dir.setPath(Common::str_to_cn(path) );
    return dir.removeRecursively();        //递归删除文件夹及其所有子内容
}

bool Common::rename_dir(QString old_path, QString new_path)
{
    QDir dir;
    old_path = str_to_cn(old_path);
    new_path = str_to_cn(new_path);
    dir.setPath(old_path);
    return dir.rename(old_path,new_path);
}

int Common::max_at(QVector<double> list)
{
    int max_n = 0;
    for (int i = 0; i < list.count(); ++i) {
        if(list.at(max_n) < list.at(i)){
            max_n = i;
        }
    }

    return max_n;
}

int Common::max_at(QVector<int> list)
{
    int max_n = 0;
    for (int i = 0; i < list.count(); ++i) {
        if(list.at(max_n) < list.at(i)){
            max_n = i;
        }
    }

    return max_n;
}

int Common::max(QVector<int> list)
{
    if(list.isEmpty()){
        return 0;
    }
    return list.at(max_at(list));
}

double Common::max(QVector<double> list)
{
    if(list.isEmpty()){
        return 0;
    }
    return list.at(max_at(list));
}

void Common::rdb_set_yc_value(uint yc_no, double val, uint qc)
{
    yc_data_type temp_data;
    temp_data.f_val = val;



//    if(qc == 1){
//        yc_set_value(yc_no, &temp_data, QDS_FO, NULL, 0, 1);
//    }
//    else{
//        yc_set_value(yc_no, &temp_data, QDS_BA, NULL, 0, 1);
//    }

//    qDebug()<<"get_value"<<yc_no <<  ":"<< rdb_get_yc_value(yc_no);

    time_type ts;
    ts.seconds = QTime::currentTime().second();
    ts.msec = QTime::currentTime().msec();
    if(qc == 1){
        yc_set_value(yc_no, &temp_data, QDS_FO, &ts, 0, 1);
    }
    else{
        yc_set_value(yc_no, &temp_data, QDS_BA, &ts, 0, 1);
    }
}

double Common::rdb_get_yc_value(uint yc_no)
{
    yc_data_type temp_data;
    unsigned char a[1],b[1];
    yc_get_value(0,yc_no,1, &temp_data, b, a);
    return temp_data.f_val;
}

void Common::rdb_set_yx_value(uint yc_no, uint val)
{
    time_type ts;
    ts.seconds = QTime::currentTime().second();
    ts.msec = QTime::currentTime().msec();
    yx_set_value(yc_no, &val, &ts, 1, 1);
}

bool Common::rdb_check_test_start()
{
    if(rdb_udp != NULL){
        int r = rdb_udp->yk_operate.yk_result;      //初始化为0,成功FF,不成功0x55
        if(r == 0xFF){
            qDebug()<<"get test start signal!";
            rdb_udp->yk_operate.yk_result = 0;
//            send_path((unsigned char*)sqlcfg->get_para()->current_dir, strlen(sqlcfg->get_para()->current_dir));
            char path[] = "/mmc/mmc2/data/asset/Normal/0219#2018-06-29-08-44-40-193/";
            send_path((unsigned char*)path, strlen(path));
            return true;
        }
    }
    return false;
}

void Common::rdb_set_dz_value(uint dz_no, char val)
{
    rdb_dz_param_t dz;
    dz.dz_no = dz_no;
    dz.tag = DZ_CHAR;
    dz.data_len = sizeof(char);
    dz.data_buf[0] = val;
    dz_set_value(&dz, 1);
}

void Common::rdb_dz_init()
{
    SQL_PARA *sql = sqlcfg->get_para();

    rdb_set_dz_value(HF_Channel1, (char)sql->menu_h1);
    rdb_set_dz_value(HF_Channel2, (char)sql->menu_h2);
    rdb_set_dz_value(LF_Channel1, (char)sql->menu_l1);
    rdb_set_dz_value(LF_Channel2, (char)sql->menu_l2);
    rdb_set_dz_value(Dual_Channel, (char)sql->menu_double);
    switch (sql->menu_h1) {
    case TEV1:
        rdb_set_dz_value(TEV1_test_mode_dz, (char)sql->tev1_sql.mode);
        rdb_set_dz_value(TEV1_show_mode_dz, (char)sql->tev1_sql.chart);
        rdb_set_dz_value(TEV1_gain_dz, (char)sql->tev1_sql.gain);
        rdb_set_dz_value(TEV1_low_threshold_dz, (char)sql->tev1_sql.low);
        rdb_set_dz_value(TEV1_high_threshold_dz, (char)sql->tev1_sql.high);
        rdb_set_dz_value(TEV1_pulse_trigger_dz, (char)sql->tev1_sql.fpga_threshold);
        rdb_set_dz_value(TEV1_pulse_counttime_dz, (char)sql->tev1_sql.pulse_time);
        rdb_set_dz_value(TEV1_waverec_auto_dz, (char)sql->tev1_sql.auto_rec);
        rdb_set_dz_value(TEV1_waverec_time_dz, (char)sql->tev1_sql.rec_time);
        rdb_set_dz_value(TEV1_center_biased_dz, (char)sql->tev1_sql.fpga_zero);
        rdb_set_dz_value(TEV1_noise_biased_dz, (char)sql->tev1_sql.offset_noise);
        break;
    case HFCT1:
        rdb_set_dz_value(HFCT1_test_mode_dz, (char)sql->hfct1_sql.mode);
        rdb_set_dz_value(HFCT1_show_mode_dz, (char)sql->hfct1_sql.chart);
        rdb_set_dz_value(HFCT1_gain_dz, (char)sql->hfct1_sql.gain);
        rdb_set_dz_value(HFCT1_passband_down_limit_dz, (char)sql->hfct1_sql.filter_hp);
        rdb_set_dz_value(HFCT1_passband_up_limit_dz, (char)sql->hfct1_sql.filter_lp);
        rdb_set_dz_value(HFCT1_pulse_trigger_dz, (char)sql->hfct1_sql.fpga_threshold);
        rdb_set_dz_value(HFCT1_pulse_counttime_dz, (char)sql->hfct1_sql.pulse_time);
        rdb_set_dz_value(HFCT1_waverec_auto_dz, (char)sql->hfct1_sql.auto_rec);
        rdb_set_dz_value(HFCT1_waverec_time_dz, (char)sql->hfct1_sql.rec_time);
        rdb_set_dz_value(HFCT1_center_biased_dz, (char)sql->hfct1_sql.fpga_zero);
        rdb_set_dz_value(HFCT1_noise_biased_dz, (char)sql->hfct1_sql.offset_noise);
        break;
    case UHF1:
        rdb_set_dz_value(UHF1_test_mode_dz, (char)sql->uhf1_sql.mode);
        rdb_set_dz_value(UHF1_show_mode_dz, (char)sql->uhf1_sql.chart);
        rdb_set_dz_value(UHF1_gain_dz, (char)sql->uhf1_sql.gain);
        rdb_set_dz_value(UHF1_low_threshold_dz, (char)sql->uhf1_sql.low);
        rdb_set_dz_value(UHF1_high_threshold_dz, (char)sql->uhf1_sql.high);
        rdb_set_dz_value(UHF1_pulse_trigger_dz, (char)sql->uhf1_sql.fpga_threshold);
        rdb_set_dz_value(UHF1_pulse_counttime_dz, (char)sql->uhf1_sql.pulse_time);
        rdb_set_dz_value(UHF1_waverec_auto_dz, (char)sql->uhf1_sql.auto_rec);
        rdb_set_dz_value(UHF1_waverec_time_dz, (char)sql->uhf1_sql.rec_time);
        rdb_set_dz_value(UHF1_center_biased_dz, (char)sql->uhf1_sql.fpga_zero);
        rdb_set_dz_value(UHF1_noise_biased_dz, (char)sql->uhf1_sql.offset_noise);
        break;
    default:
        break;
    }

    switch (sql->menu_h2) {
    case TEV2:
        rdb_set_dz_value(TEV2_test_mode_dz, (char)sql->tev2_sql.mode);
        rdb_set_dz_value(TEV2_show_mode_dz, (char)sql->tev2_sql.chart);
        rdb_set_dz_value(TEV2_gain_dz, (char)sql->tev2_sql.gain);
        rdb_set_dz_value(TEV2_low_threshold_dz, (char)sql->tev2_sql.low);
        rdb_set_dz_value(TEV2_high_threshold_dz, (char)sql->tev2_sql.high);
        rdb_set_dz_value(TEV2_pulse_trigger_dz, (char)sql->tev2_sql.fpga_threshold);
        rdb_set_dz_value(TEV2_pulse_counttime_dz, (char)sql->tev2_sql.pulse_time);
        rdb_set_dz_value(TEV2_waverec_auto_dz, (char)sql->tev2_sql.auto_rec);
        rdb_set_dz_value(TEV2_waverec_time_dz, (char)sql->tev2_sql.rec_time);
        rdb_set_dz_value(TEV2_center_biased_dz, (char)sql->tev2_sql.fpga_zero);
        rdb_set_dz_value(TEV2_noise_biased_dz, (char)sql->tev2_sql.offset_noise);
        break;
    case HFCT2:
        rdb_set_dz_value(HFCT2_test_mode_dz, (char)sql->hfct2_sql.mode);
        rdb_set_dz_value(HFCT2_show_mode_dz, (char)sql->hfct2_sql.chart);
        rdb_set_dz_value(HFCT2_gain_dz, (char)sql->hfct2_sql.gain);
        rdb_set_dz_value(HFCT2_passband_down_limit_dz, (char)sql->hfct2_sql.filter_hp);
        rdb_set_dz_value(HFCT2_passband_up_limit_dz, (char)sql->hfct2_sql.filter_lp);
        rdb_set_dz_value(HFCT2_pulse_trigger_dz, (char)sql->hfct2_sql.fpga_threshold);
        rdb_set_dz_value(HFCT2_pulse_counttime_dz, (char)sql->hfct2_sql.pulse_time);
        rdb_set_dz_value(HFCT2_waverec_auto_dz, (char)sql->hfct2_sql.auto_rec);
        rdb_set_dz_value(HFCT2_waverec_time_dz, (char)sql->hfct2_sql.rec_time);
        rdb_set_dz_value(HFCT2_center_biased_dz, (char)sql->hfct2_sql.fpga_zero);
        rdb_set_dz_value(HFCT2_noise_biased_dz, (char)sql->hfct2_sql.offset_noise);
        break;
    case UHF2:
        rdb_set_dz_value(UHF2_test_mode_dz, (char)sql->uhf2_sql.mode);
        rdb_set_dz_value(UHF2_show_mode_dz, (char)sql->uhf2_sql.chart);
        rdb_set_dz_value(UHF2_gain_dz, (char)sql->uhf2_sql.gain);
        rdb_set_dz_value(UHF2_low_threshold_dz, (char)sql->uhf2_sql.low);
        rdb_set_dz_value(UHF2_high_threshold_dz, (char)sql->uhf2_sql.high);
        rdb_set_dz_value(UHF2_pulse_trigger_dz, (char)sql->uhf2_sql.fpga_threshold);
        rdb_set_dz_value(UHF2_pulse_counttime_dz, (char)sql->uhf2_sql.pulse_time);
        rdb_set_dz_value(UHF2_waverec_auto_dz, (char)sql->uhf2_sql.auto_rec);
        rdb_set_dz_value(UHF2_waverec_time_dz, (char)sql->uhf2_sql.rec_time);
        rdb_set_dz_value(UHF2_center_biased_dz, (char)sql->uhf2_sql.fpga_zero);
        rdb_set_dz_value(UHF2_noise_biased_dz, (char)sql->uhf2_sql.offset_noise);
        break;
    default:
        break;
    }

    switch (sql->menu_l1) {
    case AA1:
        rdb_set_dz_value(AA1_test_mode_dz, (char)sql->aa1_sql.mode);
        rdb_set_dz_value(AA1_show_mode_dz, (char)sql->aa1_sql.chart);
        rdb_set_dz_value(AA1_gain_dz, (char)sql->aa1_sql.gain);
        rdb_set_dz_value(AA1_voice_dz, (char)sql->aa1_sql.vol);
        rdb_set_dz_value(AA1_low_threshold_dz, (char)sql->aa1_sql.low);
        rdb_set_dz_value(AA1_high_threshold_dz, (char)sql->aa1_sql.high);
        //    rdb_set_dz_value(AA1_waverec_auto_dz, (char)sql->aa1_sql.auto_rec);
        rdb_set_dz_value(AA1_waverec_time_dz, (char)sql->aa1_sql.rec_time);
        rdb_set_dz_value(AA1_niose_biased_dz, (char)sql->aa1_sql.offset_noise);
        break;
    case AE1:
        rdb_set_dz_value(AE1_test_mode_dz, (char)sql->ae1_sql.mode);
        rdb_set_dz_value(AE1_show_mode_dz, (char)sql->ae1_sql.chart);
        rdb_set_dz_value(AE1_gain_dz, (char)sql->ae1_sql.gain);
        rdb_set_dz_value(AE1_voice_dz, (char)sql->ae1_sql.vol);
        rdb_set_dz_value(AE1_low_threshold_dz, (char)sql->ae1_sql.low);
        rdb_set_dz_value(AE1_high_threshold_dz, (char)sql->ae1_sql.high);
        //    rdb_set_dz_value(AE1_waverec_auto_dz, (char)sql->ae1_sql.auto_rec);
        rdb_set_dz_value(AE1_waverec_time_dz, (char)sql->ae1_sql.rec_time);
        rdb_set_dz_value(AE1_niose_biased_dz, (char)sql->ae1_sql.offset_noise);
        break;
    default:
        break;
    }

    switch (sql->menu_l2) {
    case AA2:
        rdb_set_dz_value(AA2_test_mode_dz, (char)sql->aa2_sql.mode);
        rdb_set_dz_value(AA2_show_mode_dz, (char)sql->aa2_sql.chart);
        rdb_set_dz_value(AA2_gain_dz, (char)sql->aa2_sql.gain);
        rdb_set_dz_value(AA2_voice_dz, (char)sql->aa2_sql.vol);
        rdb_set_dz_value(AA2_low_threshold_dz, (char)sql->aa2_sql.low);
        rdb_set_dz_value(AA2_high_threshold_dz, (char)sql->aa2_sql.high);
        //    rdb_set_dz_value(AA2_waverec_auto_dz, (char)sql->aa2_sql.auto_rec);
        rdb_set_dz_value(AA2_waverec_time_dz, (char)sql->aa2_sql.rec_time);
        rdb_set_dz_value(AA2_niose_biased_dz, (char)sql->aa2_sql.offset_noise);
        break;
    case AE2:
        rdb_set_dz_value(AE2_test_mode_dz, (char)sql->ae2_sql.mode);
        rdb_set_dz_value(AE2_show_mode_dz, (char)sql->ae2_sql.chart);
        rdb_set_dz_value(AE2_gain_dz, (char)sql->ae2_sql.gain);
        rdb_set_dz_value(AE2_voice_dz, (char)sql->ae2_sql.vol);
        rdb_set_dz_value(AE2_low_threshold_dz, (char)sql->ae2_sql.low);
        rdb_set_dz_value(AE2_high_threshold_dz, (char)sql->ae2_sql.high);
        //    rdb_set_dz_value(AE2_waverec_auto_dz, (char)sql->ae2_sql.auto_rec);
        rdb_set_dz_value(AE2_waverec_time_dz, (char)sql->ae2_sql.rec_time);
        rdb_set_dz_value(AE2_niose_biased_dz, (char)sql->ae2_sql.offset_noise);
        break;
    default:
        break;
    }
}

void Common::select_root(QTreeView *v, QAbstractItemModel *model)
{
    QModelIndex index = model->index(0,0,v->rootIndex());        //选择第一个可选节点（非不可见的根节点）
    v->setCurrentIndex(index);
}

void Common::select_up(QTreeView *v, QAbstractItemModel *model)
{
    QModelIndex index = v->currentIndex();
    QModelIndex index_next;

    if(index.row()>0){      //有哥哥
        index_next = index.sibling(index.row()-1,0);            //先找本节点的第一个哥哥
        if(index_next.isValid() && v->isExpanded(index_next)){       //有哥哥，如果是展开的，先找哥哥的小儿子
            int rowCount = model->rowCount(index_next);
            if(rowCount > 0){
                index_next = model->index(rowCount-1,0,index_next);     //找到哥哥的小儿子
                if(index_next.isValid() && v->isExpanded(index_next) ){       //有孙子并且是展开的，再找哥哥的小孙子
                    rowCount = model->rowCount(index_next);
                    if(rowCount > 0){
                        index_next = model->index(rowCount-1,0,index_next);     //找到哥哥的小孙子（循环次数视叔深度而定，不定深度可能需要递归）
                    }
                }
            }
        }
    }
    else{           //无哥哥
        if(index.parent() != v->rootIndex()){
            index_next = index.parent();        //找爹
        }
    }

    if(index_next.isValid()){
        v->setCurrentIndex(index_next);
    }
}

void Common::select_down(QTreeView *v, QAbstractItemModel *model)
{
    QModelIndex index = v->currentIndex();
    QModelIndex index_next;

    if(v->isExpanded(index) || index == v->rootIndex()){                   //如果当前节点是展开的
        index_next = model->index(0,0,index);                   //第一顺位是本节点的第一个儿子
    }


    if(!index_next.isValid()){
        index_next = model->index(index.row()+1,index.column(),index.parent());     //第二顺位是本节点的下一个兄弟
    }

    if(!index_next.isValid()){
        int row = index.parent().row();
        index_next = model->index(row + 1,0,index.parent().parent());       //第三顺位是本节点的下一个叔叔
    }

    if(!index_next.isValid()){
        int row = index.parent().parent().row();
        index_next = model->index(row + 1,0,index.parent().parent().parent());      //第四顺位是叔爷爷
    }
    if(index_next.isValid()){
        v->setCurrentIndex(index_next);
    }
}

//展开/收起节点
void Common::expand_collapse(QTreeView *v)
{
    QModelIndex index = v->currentIndex();
    if(v->isExpanded(index)){
        v->collapse(index);
    }
    else{
        v->expand(index);
    }
}

void Common::check_base_dir()
{
    Common::mk_dir(DIR_USB);
    Common::mk_dir(DIR_DATA);
    Common::mk_dir(DIR_DATALOG);
    Common::mk_dir(DIR_PRPDLOG);
    Common::mk_dir(DIR_ASSET);
    Common::mk_dir(DIR_ASSET_NORMAL);
}

void Common::messagebox_show_and_init(QMessageBox *box)
{
    box->move(135,100);
    //    box->raise();
    box->show();
    box->button(QMessageBox::Ok)->setStyleSheet("QPushButton {background-color:gray;}");
    box->button(QMessageBox::Cancel)->setStyleSheet("");
    box->setDefaultButton(QMessageBox::Ok);
}

void Common::messagebox_switch(QMessageBox *box)
{
    if(box->defaultButton() == box->button(QMessageBox::Ok)){
        box->button(QMessageBox::Ok)->setStyleSheet("");
        box->button(QMessageBox::Cancel)->setStyleSheet("QPushButton {background-color:gray;}");
        box->setDefaultButton(QMessageBox::Cancel);
    }
    else{
        box->button(QMessageBox::Ok)->setStyleSheet("QPushButton {background-color:gray;}");
        box->button(QMessageBox::Cancel)->setStyleSheet("");
        box->setDefaultButton(QMessageBox::Ok);
    }
}

QString Common::filter_to_string(int f)
{
    switch (f) {
    case NONE:
        return "None";
    case hp_500k:
        return "500K";
    case hp_1M:
        return "1M";
    case hp_1M5:
        return "1.5M";
    case hp_1M8:
        return "1.8M";
    case hp_2M:
        return "2M";
    case hp_2M5:
        return "2.5M";
    case hp_3M:
        return "3M";
    case hp_5M:
        return "5M";
    case hp_8M:
        return "8M";
    case hp_10M:
        return "10M";
    case hp_12M:
        return "12M";
    case hp_15M:
        return "15M";
    case hp_18M:
        return "18M";
    case hp_20M:
        return "20M";
    case hp_22M:
        return "22M";
    case hp_25M:
        return "25M";
    case hp_28M:
        return "28M";
    case hp_30M:
        return "30M";
    case hp_32M:
        return "32M";
    case hp_35M:
        return "35M";
    case lp_100Hz:
        return "100Hz";
    case lp_2M:
        return "2M";
    case lp_5M:
        return "5M";
    case lp_8M:
        return "8M";
    case lp_10M:
        return "10M";
    case lp_12M:
        return "12M";
    case lp_15M:
        return "15M";
    case lp_18M:
        return "18M";
    case lp_20M:
        return "20M";
    case lp_22M:
        return "22M";
    case lp_25M:
        return "25M";
    case lp_28M:
        return "28M";
    case lp_30M:
        return "30M";
    case lp_32M:
        return "32M";
    case lp_35M:
        return "35M";
    case lp_38M:
        return "38M";
    case lp_40M:
        return "40M";
    default:
        return "None";
    }
}

double Common::filter_to_number(int f)
{
    switch (f) {
    case NONE:
        return 0;
    case hp_500k:
        return 0.5;
    case hp_1M:
        return 1;
    case hp_1M5:
        return 1.5;
    case hp_1M8:
        return 1.8;
    case hp_2M:
        return 2;
    case hp_2M5:
        return 2.5;
    case hp_3M:
        return 3;
    case hp_5M:
        return 5;
    case hp_8M:
        return 8;
    case hp_10M:
        return 10;
    case hp_12M:
        return 12;
    case hp_15M:
        return 15;
    case hp_18M:
        return 18;
    case hp_20M:
        return 20;
    case hp_22M:
        return 22;
    case hp_25M:
        return 25;
    case hp_28M:
        return 28;
    case hp_30M:
        return 30;
    case hp_32M:
        return 32;
    case hp_35M:
        return 35;
    case lp_100Hz:
        return 100e-6;
    case lp_2M:
        return 2;
    case lp_5M:
        return 5;
    case lp_8M:
        return 8;
    case lp_10M:
        return 10;
    case lp_12M:
        return 12;
    case lp_15M:
        return 15;
    case lp_18M:
        return 18;
    case lp_20M:
        return 20;
    case lp_22M:
        return 22;
    case lp_25M:
        return 25;
    case lp_28M:
        return 28;
    case lp_30M:
        return 30;
    case lp_32M:
        return 32;
    case lp_35M:
        return 35;
    case lp_38M:
        return 38;
    case lp_40M:
        return 40;
    default:
        return 0;
    }
}

void Common::adjust_filter_list(QList<int> &list, double cut_off_low, double cut_off_high)
{
    if(cut_off_high == 0){
        return;
    }
    while(list.count() > 0 && list.first() == 0){
        list.removeFirst();
    }
    while(list.count() > 0 && filter_to_number(list.first()) <= cut_off_low){
        list.removeFirst();
    }
    while(list.count() > 0 && filter_to_number(list.last()) >= cut_off_high){
        list.removeLast();
    }
    list.prepend(0);
}

QVector<int> Common::set_filter(QVector<int> wave, MODE mode)
{
    Fir fir;
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->tev1_sql.filter_hp);
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->tev1_sql.filter_lp);
        if(sqlcfg->get_para()->tev1_sql.filter_wavelet){
            wave = Wavelet::set_filter(wave,3);
        }
        break;
    case TEV2:
    case TEV_CONTINUOUS2:
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->tev2_sql.filter_hp);
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->tev2_sql.filter_lp);
        if(sqlcfg->get_para()->tev2_sql.filter_wavelet){
            wave = Wavelet::set_filter(wave,3);
        }
        break;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct1_sql.filter_hp);
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct1_sql.filter_lp);
        if(sqlcfg->get_para()->hfct1_sql.filter_wavelet){
            wave = Wavelet::set_filter(wave,3);
        }
        break;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct2_sql.filter_hp);
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct2_sql.filter_lp);
        if(sqlcfg->get_para()->hfct2_sql.filter_wavelet){
            wave = Wavelet::set_filter(wave,3);
        }
        break;
    case UHF1:
    case UHF_CONTINUOUS1:
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->uhf1_sql.filter_hp);
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->uhf1_sql.filter_lp);
        if(sqlcfg->get_para()->uhf1_sql.filter_wavelet){
            wave = Wavelet::set_filter(wave,3);
        }
        break;
    case UHF2:
    case UHF_CONTINUOUS2:
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->uhf2_sql.filter_hp);
        wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->uhf2_sql.filter_lp);
        if(sqlcfg->get_para()->uhf2_sql.filter_wavelet){
            wave = Wavelet::set_filter(wave,3);
        }
        break;
    default:
        break;
    }
    return wave;
}

int Common::time_interval(timeval start_time, timeval stop_time)
{
    int interval_usec = stop_time.tv_usec - start_time.tv_usec;
    int interval_sec = stop_time.tv_sec - start_time.tv_sec;
    return interval_sec*1000000 + interval_usec;
}

void Common::time_addusec(timeval &time, int usec)
{
    int tmp = time.tv_usec + usec;
    int s = tmp / 1000000;
    int u = tmp % 1000000;
    if(u < 0 ){
        u += 1000000;
        s -= 1;
    }
    time.tv_sec += s;
    time.tv_usec = u;
}

void Common::change_rbt_status(bool flag, QRadioButton *b0, QRadioButton *b1)
{
    if(flag){
        b0->setStyleSheet("QRadioButton {background-color:darkGray;}");
    }
    else{
        b1->setStyleSheet("QRadioButton {background-color:darkGray;}");
    }
}

void Common::check_restart_file()
{
    QFile file("/root/restart");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        return;
    QByteArray t = file.readAll();
    if(!t.isEmpty()){
        qDebug()<<"read restart file: "<<t;
        QTextStream out(&file);
        out << 0;
    }

    file.close();
}

CHANNEL_SQL *Common::channel_sql(MODE mode)
{
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        return &sqlcfg->get_para()->tev1_sql;
    case TEV2:
    case TEV_CONTINUOUS2:
        return &sqlcfg->get_para()->tev2_sql;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        return &sqlcfg->get_para()->hfct1_sql;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        return &sqlcfg->get_para()->hfct2_sql;
    case UHF1:
    case UHF_CONTINUOUS1:
        return &sqlcfg->get_para()->uhf1_sql;
    case UHF2:
    case UHF_CONTINUOUS2:
        return &sqlcfg->get_para()->uhf2_sql;
    case AA1:
    case AA_ENVELOPE1:
        return &sqlcfg->get_para()->aa1_sql;
    case AA2:
    case AA_ENVELOPE2:
        return &sqlcfg->get_para()->aa2_sql;
    case AE1:
    case AE_ENVELOPE1:
        return &sqlcfg->get_para()->ae2_sql;
    case AE2:
    case AE_ENVELOPE2:
        return &sqlcfg->get_para()->ae2_sql;
    default:
        qDebug()<<"Common::channel_sql() error!" << mode_to_string(mode);
        return NULL;
    }
}

int Common::mode_to_channel(MODE mode)
{
    switch (mode) {
    case TEV1:
    case TEV_CONTINUOUS1:
        return 0;
    case TEV2:
    case TEV_CONTINUOUS2:
        return 1;
    case HFCT1:
    case HFCT_CONTINUOUS1:
        return 0;
    case HFCT2:
    case HFCT_CONTINUOUS2:
        return 1;
    case UHF1:
    case UHF_CONTINUOUS1:
        return 0;
    case UHF2:
    case UHF_CONTINUOUS2:
        return 1;
    case AA1:
    case AA_ENVELOPE1:
        return 3;
    case AA2:
    case AA_ENVELOPE2:
        return 4;
    case AE1:
    case AE_ENVELOPE1:
        return 3;
    case AE2:
    case AE_ENVELOPE2:
        return 4;
    default:
        qDebug()<<"Common::mode_to_channel() error!" << mode_to_string(mode);
        return 0;
    }
}

QString Common::sensor_freq_to_string(int s)
{
    switch (s) {
    case ae_factor_30k:
        return "30KHz";
    case ae_factor_40k:
        return "40KHz";
    case ae_factor_50k:
        return "50KHz";
    case ae_factor_60k:
        return "60KHz";
    case ae_factor_70k:
        return "70KHz";
    case ae_factor_80k:
        return "80KHz";
    case ae_factor_90k:
        return "90KHz";
    case ae_factor_30k_v2:
        return tr("30KHz(硬件版本2)");
    case ae_factor_40k_v2:
        return tr("40KHz(硬件版本2)");
    default:
        return "40KHz";
    }
}














