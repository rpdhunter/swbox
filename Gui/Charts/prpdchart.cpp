#include "prpdchart.h"
#include <QDateTime>

PRPDChart::PRPDChart(QObject *parent) : BaseChart(parent)
{
}

void PRPDChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    phase_window = 360 / PHASE_NUM;         //目前固定为1
    amp_window = (max_value - min_value)*1.0 / AMP_NUM;

    for(int i=0;i<PHASE_NUM;i++){           //二维数组清零
        for(int j=0;j<AMP_NUM;j++){
            map[i][j]=0;
        }
    }

    plot = new QwtPlot(parent);
    plot->move(10,35);
    plot->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;

    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 360, 180);
    plot->setAxisScale(QwtPlot::yLeft, min_value, max_value, (max_value-min_value) / 2);  //高频双端

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    //    plot_PRPD->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
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
        if(mode == AA1 || mode == AA2 || mode == AE1 || mode == AE2){
            Y.append((max_value/2)*(qSin(2*3.1416*i/360) + 1));
        }
        else{
            Y.append(max_value*qSin(2*3.1416*i/360));
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

/**************************************************************
 * 数据输入函数
 * QVector<QPoint(相位，幅值)>
 * 相位范围：[0,360)
 * 幅值范围：[min_value, max_value]
 * 经量化后，存储于二维数组 map[PHASE_NUM][AMP_NUM]中
 * ************************************************************/
void PRPDChart::add_data(QVector<QPoint> point_list)
{
    if(plot == NULL || point_list.isEmpty())
        return;

    foreach (QPoint P, point_list) {
        if( P.y() < min_value || P.y() > max_value){ //输入校验
//            qDebug()<<Common::mode_to_string(mode)<<" PRPD input out of range, Y = " << P.y();
            continue;
        }

        int x = ( P.x() % 360 ) / phase_window, y = (P.y()-min_value) / amp_window;        //按量化窗口标准化,得到脉冲点的坐标
        int x_s = x * phase_window, y_s = y * amp_window + min_value;            //标准化后的脉冲值

        if(map[x][y] > 0){          //非初次出现，需要替换
            int n = prpd_samples.indexOf(QwtPoint3D(x_s, y_s, map[x][y]) );
            prpd_samples[n] = QwtPoint3D(x_s, y_s, (map[x][y]+1) );
        }
        else{                       //初次出现，添加到末尾
            prpd_samples.append(QwtPoint3D(x_s, y_s, 1));
        }
        map[x][y]++;                //将这次脉冲存储到二维数组中
    }
//    qDebug()<<prpd_samples;

    d_PRPD->setSamples(prpd_samples);
    plot->replot();            //更新数据至图形
}

void PRPDChart::reset_data()
{
    if(plot == NULL)
        return;

    save_data();        //保存文件

    for(int i=0;i<PHASE_NUM;i++){           //二维数组清零
        for(int j=0;j<AMP_NUM;j++){
            map[i][j]=0;
        }
    }
//    emit send_PRPD_data(prpd_samples);

    prpd_samples.clear();           //图形数据清零
    d_PRPD->setSamples(prpd_samples);
    plot->replot();            //更新数据至图形
}

void PRPDChart::save_data()
{
    if(plot == NULL)
        return;

    switch (sqlcfg->get_para()->file_save_standard) {
    case file_save_zdit:
        save_PRPD_zdit();
        break;
    case file_save_stategrid:
        save_PRPD_State_Grid();
        break;
    case file_save_both:
        save_PRPD_zdit();
        save_PRPD_State_Grid();
        break;
    default:
        break;
    }
}

QVector<QwtPoint3D> PRPDChart::samples()
{
    return prpd_samples;
}

void PRPDChart::save_PRPD_zdit()
{
    QFile file;
    bool flag;

    QString file_name = QString(Common::mode_to_string(mode) + "_PRPD_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") + ".log");
    file.setFileName(DIR_PRPDLOG"/" + file_name);

    QString str_src = file.fileName();
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QTextStream out(&file);

        //保存纯数据格式
        foreach (QwtPoint3D p, prpd_samples) {
            out << p.x() << "\t"
                << p.y() << "\t"
                << p.z() << "\n";
        }

        qDebug()<<"ZDIT PRPD file saved! \tnum="<< prpd_samples.length() << "\tmode = "<<Common::mode_to_string(mode);

        file.close();
        Common::create_hard_link(str_src, file_name);           //建立硬连接
    }
}

void PRPDChart::save_PRPD_State_Grid()
{
    QFile file;
    bool flag;

    QString format_str;
    if(mode == HFCT1 || mode == HFCT2){
        format_str = ".HF";
    }
    else if(mode == TEV1 || mode == TEV2){
        format_str = ".TEV";
    }
    else if(mode == UHF1 || mode == UHF2){
        format_str = ".UHF";
    }
    else if(mode == AA1 || mode == AA2){
        format_str = ".AA";
    }
    else if(mode == AE1 || mode == AE2){
        format_str = ".AE";
    }
    QString file_name = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm") + format_str;
    file.setFileName(DIR_PRPDLOG"/" + file_name);

    QString str_src = file.fileName();
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QDataStream out(&file);
        //        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);            //设置浮点数流格式为单精度（float占4字节）

        out << (float)1.0;          //版本号（4字节）
        out << (quint8)0x00;        //谱图类型 0x00——PRPD，0x01——PRPS（1字节）
        out << (int)PHASE_NUM;      //工频相位窗数（4字节）
        out << (qint32)AMP_NUM;     //幅值等分区间数（4字节）
        out << (quint8)0x01;        //幅值单位 0x01——db，0x02——mv，0x03——pC（4字节）
        out << (float)min_value;    //幅值下限（4字节）
        out << (float)max_value;    //幅值上限（4字节）
        out << (qint32)0;           //工频周期数（4字节）

        char t[] = {100,0,0,0,0,0,0,0,0}; //放电概率，8字节对应8种放电类型（8字节）
        out.writeRawData(t,8);

        for (int i = 0; i < PHASE_NUM; ++i) {     //写入
            for (int j = 0; j < AMP_NUM; ++j) {
                out << map[i][j];
            }
        }
        qDebug()<<"State_Grid PRPD file saved! \tnum=" << "\tmode = "<<Common::mode_to_string(mode);

        file.close();
        Common::create_hard_link(str_src, file_name);           //建立硬连接
    }
}


