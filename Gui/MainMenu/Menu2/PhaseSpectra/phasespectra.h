#ifndef PHASESPECTRA_H
#define PHASESPECTRA_H

#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QtMath>
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
#include <qwt_abstract_scale_draw.h>
#include <qwt_scale_div.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_curve.h>
#include "IO/Key/key.h"

class PhaseSpectra : public QFrame
{
    Q_OBJECT
public:
    PhaseSpectra(QWidget *parent = 0);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);

signals:
    void fresh_parent(void);

private:
    QwtPlot *plot;
    QVector<QPointF> wave;
    QwtPlotCurve *curve;
    CURRENT_KEY_VALUE *key_val;

    QLinearGradient *color_belt;
    QRect *color_rect;
    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab, *set_child3_lab;
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab, *child3_val_lab;

protected:
    void paintEvent(QPaintEvent *event);
    QLabel *set_txt_lab;
};

#endif // PHASESPECTRA_H
