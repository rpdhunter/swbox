#ifndef PULSE_H
#define PULSE_H

#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <math.h>
#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QTimer>

class Pulse : public QFrame
{
    Q_OBJECT
public:
    Pulse(QWidget *parent = NULL, G_PARA *g_data = NULL);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);
    void fresh_data(void);

signals:
    void fresh_parent(void);

private:
    void fresh_setting(void);

    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    quint32 pulse_cnt;
    quint32 signal_pulse_cnt;
    double degree;

    SQL_PARA *sql_para;
    PULSE_SQL *pulse_sql;

    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab, *set_child3_lab;
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab, *child3_val_lab;
    QLabel *set_txt_lab, *amp_his_val, *db_lab;
    QLabel *pulse_lab, *signal_pulse_lab, *degree_lab;
    QLabel *rect0_lab, *rect1_lab, *rect2_lab;
    QLabel *lab0, *lab1, *lab2;

    QTimer *timer;
};

#endif // PULSE_H
