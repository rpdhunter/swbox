#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>

namespace Ui {
class SystemInfoUi;
}

class SystemInfo : public QFrame
{
    Q_OBJECT
public:
    SystemInfo(QWidget *parent = 0);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);

signals:
    void fresh_parent(void);

private:
    CURRENT_KEY_VALUE *key_val;
    SQL_PARA *sql_para;

    QLabel *lab, *lab_hardv, *lab_hardd, *lab_softv, *lab_softd,
        *lab_fpgav, *lab_fpgad, *lab_cpuv, *lab_cpud, *lab_qtv, *lab_qtd;

    Ui::SystemInfoUi *ui;

};

#endif // SYSTEMINFO_H
