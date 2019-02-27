#ifndef FLYCHART_H
#define FLYCHART_H

#include <QObject>

class FlyChart : public QObject
{
    Q_OBJECT
public:
    explicit FlyChart(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FLYCHART_H