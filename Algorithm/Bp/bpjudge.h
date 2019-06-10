#ifndef BPJUDGE_H
#define BPJUDGE_H

#include <QObject>
#include "pd.h"
#include <qwt_point_3d.h>
#include "bp.h"
#include "prpd_bp.h"
#include "cable_prpd_bp.h"
#include "input.h"

class BpJudge : public QObject
{
    Q_OBJECT
public:
    explicit BpJudge(QObject *parent = nullptr);

//    void init();            //初始化
    QString add_data(QVector<QPoint> list);


signals:

public slots:

private:
    cable_bp_neural_network *p_cable_prpd_bp;
    QVector<QPoint> _samples;

    QString judge(QVector<QwtPoint3D> samples);         //返回故障类型
    QString judge(QVector<QPoint> samples);

};

#endif // BPJUDGE_H
