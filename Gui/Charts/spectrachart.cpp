#include "spectrachart.h"

SpectraChart::SpectraChart(QObject *parent) : BaseChart(parent)
{
}

void SpectraChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    plot = new QwtPlot(parent);
    plot->move(10,35);
    plot->resize(200, 140);
    plot->setStyleSheet("background:transparent;color:gray;");

    d_Spectra = new QwtPlotHistogram;


    if(mode == AA1 || mode == AA2 || mode == AE1 || mode == AE2){   //低频
        plot->setAxisScale(QwtPlot::xBottom, 0, 4);                 //0-4k
        plot->setAxisScale(QwtPlot::yLeft, 0, 30);                  //0-30μV
        plot->axisWidget(QwtPlot::xBottom)->setTitle("kHz");        //此处存疑
    }
    else{                                                           //高频
        plot->setAxisScale(QwtPlot::xBottom, 0, 40);                //0-40M
        plot->setAxisScale(QwtPlot::yLeft, 0, 60);
        plot->axisWidget(QwtPlot::xBottom)->setTitle("MHz");        //此处存疑
    }

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);

    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

//    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    d_Spectra->setStyle( QwtPlotHistogram::Columns );
    d_Spectra->setBrush(Qt::yellow);
    d_Spectra->setPen(Qt::yellow);
    d_Spectra->attach(plot);

    fft = new FFT;
}

void SpectraChart::add_data(QVector<int> datalist)
{
    if(mode == AA1 || mode == AA2 || mode == AE1 || mode == AE2){   //低频
        if(datalist.count() > 2048){
            QVector<qint32> fft_result = fft->fft2048(datalist.mid(0,2048));
            QVector<double> fft_result_p;       //先转成物理值,再累加
            foreach (int c, fft_result) {
                fft_result_p.append(Common::physical_value(c, mode));
            }
            for (int i = 0; i <41 ; ++i) {
                Spectra_map[i] = Common::sum(fft_result_p.mid(i*5 + 1, 5) );
            }

            Spectra_data.clear();
            for(int i=0;i<40;i++){
                QwtInterval interval( 0.1*(i + 0.2) , 0.1*(i + 0.8) );      //单位为0-4K
                interval.setBorderFlags( QwtInterval::ExcludeMaximum );
                Spectra_data.append( QwtIntervalSample( Spectra_map[i], interval ) );
            }

            d_Spectra->setData(new QwtIntervalSeriesData( Spectra_data ));
            plot->replot();
        }
    }
    else{                                                           //高频
//        qDebug()<<"SpectraChart:"<<datalist.count() << Common::mode_to_string(mode);
        if(datalist.count() > 128){         //计算128点fft
            QVector<qint32> fft_result = fft->fft128(datalist.mid(100,128));        //修改
            for (int i = 0; i < 64 ; ++i) {
                Spectra_map[i] = Common::physical_value(fft_result.at(i+1), mode);
            }
        }
        else{
            for (int i = 0; i < 64 ; ++i) {
                Spectra_map[i] = 0;
            }
        }

        Spectra_data.clear();

        for(int i=0;i<64;i++){
            QwtInterval interval( 0.78125*(i + 0.2) , 0.78125*(i + 0.8) );    //单位为0-50M
            interval.setBorderFlags( QwtInterval::ExcludeMaximum );
            Spectra_data.append( QwtIntervalSample( Spectra_map[i], interval ) );
        }

        d_Spectra->setData(new QwtIntervalSeriesData( Spectra_data ));
        plot->replot();
    }
}
