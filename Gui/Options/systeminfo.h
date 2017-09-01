#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QFrame>

#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"

namespace Ui {
class SystemInfoUi;
}

class SystemInfo : public QFrame
{
    Q_OBJECT
public:
    SystemInfo(QWidget *parent = 0);

    void working(CURRENT_KEY_VALUE *val);

public slots:    
    void trans_key(quint8);

signals:
    void fresh_parent();

private:
    CURRENT_KEY_VALUE *key_val;

    Ui::SystemInfoUi *ui;
};

#endif // SYSTEMINFO_H
