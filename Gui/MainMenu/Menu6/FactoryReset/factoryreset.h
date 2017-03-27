#ifndef FACTORYRESET_H
#define FACTORYRESET_H

#include <QFrame>
#include <QDialogButtonBox>
#include <QLabel>
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"

class FactoryReset : public QFrame
{
    Q_OBJECT
public:
    explicit FactoryReset(QWidget *parent = 0);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);

signals:
    void fresh_parent(void);
    void sysReset(void);

private:
    CURRENT_KEY_VALUE *key_val;
    QDialogButtonBox *btnBox;
    QLabel *label;
    int reset;  //0为不重置,1为重置
    void buttonIni(bool f);
};

#endif // FACTORYRESET_H
