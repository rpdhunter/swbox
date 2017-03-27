#ifndef AAULTRASONIC_H
#define AAULTRASONIC_H

#include "IO/Data/fifodata.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <math.h>
#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QProgressBar>

class AAUltrasonic : public QFrame
{
    Q_OBJECT
public:
    AAUltrasonic(QWidget *parent = NULL, G_PARA *g_data = NULL);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);
    void fresh(void);

private:
    void fresh_setting(void);

    SQL_PARA *sql_para;
    AAULTRA_SQL *aaultra_sql;

    //参数设置图标
    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab, *set_child3_lab;
    //参数设置数值
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab, *child3_val_lab;

    QLabel *set_txt_lab;
    //4个数值滚动条，幅值0-100
    QProgressBar *progbar0, *progbar1, *progbar2, *progbar3;
    //4个数值滚动条所对应的4组显示信息
    QLabel *lab0[4], *lab1[4], *lab2[4], *lab3[4];

    CURRENT_KEY_VALUE *key_val;

    QTimer *timer;

    G_PARA *data;
};

#endif // AAULTRASONIC_H
