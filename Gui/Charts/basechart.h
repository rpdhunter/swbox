#ifndef BASECHART_H
#define BASECHART_H

#include <QObject>

class BaseChart : public QObject
{
    Q_OBJECT
public:
    explicit BaseChart(QObject *parent = nullptr);

signals:

public slots:
};

#endif // BASECHART_H