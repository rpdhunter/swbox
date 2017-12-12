#include "common.h"
#include <QtDebug>

#include <QLineEdit>
#include <QListView>
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
    comboBox->resize(155,25);
    QLineEdit *lineEdit = new QLineEdit;
    comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/bk/para_child.png);color:gray; }");
    comboBox->setLineEdit(lineEdit);
    comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    comboBox->lineEdit()->setReadOnly(true);
    comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/bk/para_child.png);color:gray}");
    comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    comboBox->view()->setFrameShadow(QFrame::Plain);
    comboBox->setFrame(false);
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

void Common::set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value)
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
        Y.append(max_value*qSin(2*3.1416*i/360));
    }
    curve_grid->setSamples(X,Y);

//    d_PRPD = new QwtPlotSpectroCurve;
//    d_curve->setPenWidth(2);

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

void Common::set_TF_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value)
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

void Common::set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram)
{
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, -60, 60);
    plot->setAxisScale(QwtPlot::yLeft, 0, 50);
    //    plot->setAxisScale(QwtPlot::xBottom, 0, 6);
    //    plot->setAxisScale(QwtPlot::yLeft, 0.01, 10);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::xBottom)->setTitle("dB");
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
    case TEV1_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR;
        break;
    case TEV2:
    case TEV2_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR;
        break;
    case AA_Ultrasonic:
        v_real = (code_value * 4) * sqlcfg->get_para()->aaultra_sql.gain * AA_FACTOR;
        break;
    case HFCT1:
    case HFCT1_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->hfct1_sql.gain * TEV_FACTOR;
        break;
    case HFCT2:
    case HFCT2_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->hfct2_sql.gain * TEV_FACTOR;
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
    case TEV1_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->tev1_sql.gain / TEV_FACTOR;
        break;
    case TEV2:
    case TEV2_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->tev2_sql.gain / TEV_FACTOR;
        break;
    case AA_Ultrasonic:
        v_code = physical_value / 4 / sqlcfg->get_para()->aaultra_sql.gain / AA_FACTOR;
        break;
    case HFCT1:
    case HFCT1_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->hfct1_sql.gain / TEV_FACTOR;
        break;
    case HFCT2:
    case HFCT2_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->hfct2_sql.gain / TEV_FACTOR;
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
    case TEV1_CONTINUOUS:
        v_real = physical_value(sqlcfg->get_para()->tev1_sql.fpga_threshold, TEV1);
        break;
    case TEV2:
    case TEV2_CONTINUOUS:
        v_real = physical_value(sqlcfg->get_para()->tev2_sql.fpga_threshold, TEV2);
        break;
    case HFCT1:
    case HFCT1_CONTINUOUS:
        v_real = physical_value(sqlcfg->get_para()->hfct1_sql.fpga_threshold, HFCT1);
        break;
    case HFCT2:
    case HFCT2_CONTINUOUS:
        v_real = physical_value(sqlcfg->get_para()->hfct2_sql.fpga_threshold, HFCT2);
        break;
    case Double_Channel:        //to be
        v_real = physical_value(sqlcfg->get_para()->tev1_sql.fpga_threshold, TEV1);
    default:
        break;
    }
    return v_real;
}

QString Common::MODE_toString(MODE val)
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
    case AA_Ultrasonic:
        tmpStr = "AA_Ultrasonic";
        break;
    case AE_Ultrasonic:
        tmpStr = "AE_Ultrasonic";
        break;
    case Double_Channel:
        tmpStr = "Double_Channel";
        break;
    case TEV1_CONTINUOUS:
        tmpStr = "TEV1_CONTINUOUS";
        break;
    case TEV2_CONTINUOUS:
        tmpStr = "TEV2_CONTINUOUS";
        break;
    case HFCT1_CONTINUOUS:
        tmpStr = "HFCT1_CONTINUOUS";
        break;
    case HFCT2_CONTINUOUS:
        tmpStr = "HFCT2_CONTINUOUS";
        break;
    case Options_Mode:
        tmpStr = "Options_Mode";
        break;
    default:
        break;
    }
    return tmpStr;
}
