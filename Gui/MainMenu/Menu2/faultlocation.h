#ifndef FAULTLOCATION_H
#define FAULTLOCATION_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"

class QTimer;

namespace Ui {
class FaultLocation;
}

class FaultLocation : public QFrame
{
    Q_OBJECT

public:
    explicit FaultLocation(G_PARA *data, QWidget *parent = 0);
    ~FaultLocation();

    void get_origin_points(QVector<QPoint> p,int group);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);

signals:
    void fresh_parent(void);

private slots:
    void setCompassValue();
    void setCompassValue(int c);
    void setMiniCompassValue();

private:
    Ui::FaultLocation *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE *key_val;
//    int db;
//    int max_db;
    SQL_PARA *sql_para;

    QTimer *timer, *timer1;

    int from,to;
    int speed;
    int process;

    int groupNum_left,groupNum_right;
    QVector<QPoint> points_left, points_right;

    void compare();
    void fresh_setting();
};

#endif // FAULTLOCATION_H
