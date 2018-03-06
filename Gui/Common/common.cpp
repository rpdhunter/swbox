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
        v_real = code_value * sqlcfg->get_para()->tev1_sql.gain * H_C_FACTOR;
        break;
    case TEV2:
    case TEV2_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->tev2_sql.gain * H_C_FACTOR;
        break;
    case HFCT1:
    case HFCT1_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->hfct1_sql.gain * H_C_FACTOR;
        break;
    case HFCT2:
    case HFCT2_CONTINUOUS:
        v_real = code_value * sqlcfg->get_para()->hfct2_sql.gain * H_C_FACTOR;
        break;
    case AA1:
    case AA1_ENVELOPE:
        v_real = (code_value * 4) * sqlcfg->get_para()->aa1_sql.gain * L_C_FACTOR;
        break;
    case AA2:
    case AA2_ENVELOPE:
        v_real = (code_value * 4) * sqlcfg->get_para()->aa2_sql.gain * L_C_FACTOR;
        break;
    case AE1:
    case AE1_ENVELOPE:
        v_real = (code_value * 4) * sqlcfg->get_para()->ae1_sql.gain * L_C_FACTOR;
        break;
    case AE2:
    case AE2_ENVELOPE:
        v_real = (code_value * 4) * sqlcfg->get_para()->ae2_sql.gain * L_C_FACTOR;
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
        v_code = physical_value / sqlcfg->get_para()->tev1_sql.gain / H_C_FACTOR;
        break;
    case TEV2:
    case TEV2_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->tev2_sql.gain / H_C_FACTOR;
        break;
    case HFCT1:
    case HFCT1_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->hfct1_sql.gain / H_C_FACTOR;
        break;
    case HFCT2:
    case HFCT2_CONTINUOUS:
        v_code = physical_value / sqlcfg->get_para()->hfct2_sql.gain / H_C_FACTOR;
        break;
    case AA1:
        v_code = physical_value / 4 / sqlcfg->get_para()->aa1_sql.gain / L_C_FACTOR;
        break;
    case AA2:
        v_code = physical_value / 4 / sqlcfg->get_para()->aa2_sql.gain / L_C_FACTOR;
        break;
    case AE1:
        v_code = physical_value / 4 / sqlcfg->get_para()->ae1_sql.gain / L_C_FACTOR;
        break;
    case AE2:
        v_code = physical_value / 4 / sqlcfg->get_para()->ae2_sql.gain / L_C_FACTOR;
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
        break;
    case AA1:
    case AA1_ENVELOPE:
        v_real = physical_value(sqlcfg->get_para()->aa1_sql.fpga_threshold, AA1);
        break;
    case AA2:
    case AA2_ENVELOPE:
        v_real = physical_value(sqlcfg->get_para()->aa2_sql.fpga_threshold, AA2);
        break;
    case AE1:
    case AE1_ENVELOPE:
        v_real = physical_value(sqlcfg->get_para()->ae1_sql.fpga_threshold, AE1);
        break;
    case AE2:
    case AE2_ENVELOPE:
        v_real = physical_value(sqlcfg->get_para()->ae2_sql.fpga_threshold, AE2);
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
    case TEV1_CONTINUOUS:
        v_code = sqlcfg->get_para()->tev1_sql.fpga_zero;
        break;
    case TEV2:
    case TEV2_CONTINUOUS:
        v_code = sqlcfg->get_para()->tev2_sql.fpga_zero;
        break;
    case HFCT1:
    case HFCT1_CONTINUOUS:
        v_code = sqlcfg->get_para()->hfct1_sql.fpga_zero;
        break;
    case HFCT2:
    case HFCT2_CONTINUOUS:
        v_code = sqlcfg->get_para()->hfct2_sql.fpga_zero;
        break;
    default:
        break;
    }
    return v_code;
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

void Common::write_fpga_offset_debug(G_PARA *data)
{
    switch (sqlcfg->get_para()->menu_h1) {
    case TEV1:
        data->set_send_para (sp_tev1_zero, 0x8000 + code_value(sqlcfg->get_para()->tev1_sql.fpga_zero,TEV1) );
//        data->set_send_para (sp_tev1_threshold, code_value(sqlcfg->get_para()->tev1_sql.fpga_threshold, TEV1) );
        break;
    case HFCT1:
        data->set_send_para (sp_hfct1_zero, 0x8000 + code_value(sqlcfg->get_para()->hfct1_sql.fpga_zero,HFCT1) );
//        data->set_send_para (sp_hfct1_threshold, code_value(sqlcfg->get_para()->hfct1_sql.fpga_zero,HFCT1) );
        break;
    case UHF1:

        break;
    default:
        break;
    }

    switch (sqlcfg->get_para()->menu_h2) {
    case TEV2:
        data->set_send_para (sp_tev2_zero, 0x8000 + code_value(sqlcfg->get_para()->tev2_sql.fpga_zero,TEV2) );
//        data->set_send_para (sp_tev2_threshold, code_value(sqlcfg->get_para()->tev2_sql.fpga_threshold, TEV2) );
        break;
    case HFCT2:
        data->set_send_para (sp_hfct2_zero, 0x8000 + code_value(sqlcfg->get_para()->hfct2_sql.fpga_zero,HFCT2) );
//        data->set_send_para (sp_hfct2_threshold, code_value(sqlcfg->get_para()->hfct2_sql.fpga_zero,HFCT2) );
        break;
    case UHF2:

        break;
    default:
        break;
    }

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

void Common::calc_aa_value(G_PARA *data, MODE mode, L_CHANNEL_SQL *x_sql, double *aa_val, double *aa_db, int *offset)
{
    int d;
//    AA_SQL *x_sql;
//    switch (mode) {
//    case AA1:
//        d = (int)data->recv_para_normal.ldata0_max - (int)data->recv_para_normal.ldata0_min ;      //最大值-最小值=幅值
//        x_sql = &sqlcfg->get_para()->aa1_sql;
//        break;
//    case AA2:
//        d = (int)data->recv_para_normal.ldata1_max - (int)data->recv_para_normal.ldata1_min ;      //最大值-最小值=幅值
//        x_sql = &sqlcfg->get_para()->aa2_sql;
//        break;
//    case AE1:
//        d = (int)data->recv_para_normal.ldata0_max - (int)data->recv_para_normal.ldata0_min ;      //最大值-最小值=幅值
//        x_sql = &sqlcfg->get_para()->ae1_sql;
//        break;
//    case AE2:
//        d = (int)data->recv_para_normal.ldata1_max - (int)data->recv_para_normal.ldata1_min ;      //最大值-最小值=幅值
//        x_sql = &sqlcfg->get_para()->ae2_sql;
//        break;
//    default:
//        break;
//    }

    if(mode == AA1 || mode == AE1){
        d = ((int)data->recv_para_normal.ldata0_max - (int)data->recv_para_normal.ldata0_min) / 4 ;      //最大值-最小值=幅值
    }
    else {
        d = ((int)data->recv_para_normal.ldata1_max - (int)data->recv_para_normal.ldata1_min) / 4 ;      //最大值-最小值=幅值
    }
//    qDebug()<<"d="<<d<<"\t ph value="<<Common::physical_value(d,mode);
    * offset = ( d - 1 / x_sql->gain / L_C_FACTOR ) / 100;
    * aa_val = (d - x_sql->offset * 100) * x_sql->gain * L_C_FACTOR;
    * aa_db = 20 * log10 (* aa_val);
}

//在给定数组中，根据给定的阈值判定脉冲
QVector<QPoint> Common::calc_pulse_list(QVector<int> datalist, QVector<int> timelist, int threshold)
{
    QVector<QPoint> pulse_list;
    QPoint p;
    for (int i = 1; i < datalist.count()-1; ++i) {
        //注意判定当前点为峰值点的条件为【大于阈值】【大于前点，大于等于后点】，这样避免了方波出现连续波峰的情况，又不会遗漏
        if(datalist.at(i) > threshold && datalist.at(i)-datalist.at(i-1) > 0 && datalist.at(i) - datalist.at(i+1) >= 0){
            p.setX( timelist.at(i/128) + (i%128)*(320000/128) );
            p.setY( datalist.at(i) );
//            p.setX( i );
            pulse_list.append(p);
        }
    }
    return pulse_list;
}

QVector<QPoint> Common::calc_pulse_list(QVector<int> datalist, int threshold)
{
    QVector<QPoint> pulse_list;
    for (int i = 1; i < datalist.count()-1; ++i) {
        //注意判定当前点为峰值点的条件为【大于阈值】【大于前点，大于等于后点】，这样避免了方波出现连续波峰的情况，又不会遗漏
        if(datalist.at(i) > threshold && datalist.at(i)-datalist.at(i-1) > 0 && datalist.at(i) - datalist.at(i+1) >= 0){
            pulse_list.append(QPoint(i, datalist.at(i)));
        }
    }
    return pulse_list;
}

int Common::time_to_phase(int x)
{
    if(sqlcfg->get_para()->freq_val == 50){
        x = x % 2000000;    //取余数
        x = x * 360 /2000000;
    }
    else if(sqlcfg->get_para()->freq_val == 60){
        x = x % 1666667;
        x = x * 360 /1666667;
    }
    return x;
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
    double tmp=0;
    foreach (double p, list) {
        tmp += p;
    }
    return tmp / list.count();
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

















