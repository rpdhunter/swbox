#ifndef HFAMPLITUDE_H
#define HFAMPLITUDE_H

#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <qwt_plot.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_histogram.h>
#include <qwt_scale_widget.h>
#include <qwt_series_store.h>
#include <qwt_series_data.h>
#include <qwt_samples.h>
#include "hfbarchart.h"
#include "IO/Key/key.h"

class HFAmplitude : public QFrame
{
    Q_OBJECT
public:
    HFAmplitude(QWidget *parent = 0);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);

signals:
    void fresh_parent(void);

private:
    QwtPlot *plot;
    HFBarChart *d_chart;
    CURRENT_KEY_VALUE *key_val;

    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab;
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab;
    QLabel *set_txt_lab;
    QLabel *tev_lab, *max_val_lab, *yellow_lab, *green_lab, *red_lab,
        *amp_his_val, *db_lab;
};

#endif // HFAMPLITUDE_H
