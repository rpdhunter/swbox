#ifndef SPECTRACHART_H
#define SPECTRACHART_H

#include <QObject>

class SpectraChart : public QObject
{
    Q_OBJECT
public:
    explicit SpectraChart(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SPECTRACHART_H