#ifndef HISTOGRAMCHART_H
#define HISTOGRAMCHART_H

#include <QObject>

class HistogramChart : public QObject
{
    Q_OBJECT
public:
    explicit HistogramChart(QObject *parent = nullptr);

signals:

public slots:
};

#endif // HISTOGRAMCHART_H