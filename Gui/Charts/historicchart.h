#ifndef HISTORICCHART_H
#define HISTORICCHART_H

#include <QObject>

class HistoricChart : public QObject
{
    Q_OBJECT
public:
    explicit HistoricChart(QObject *parent = nullptr);

signals:

public slots:
};

#endif // HISTORICCHART_H