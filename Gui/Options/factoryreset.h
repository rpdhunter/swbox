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

    void working(CURRENT_KEY_VALUE *val);

public slots:    
    void trans_key(quint8 key_code);

signals:
    void fresh_parent();

private:
    CURRENT_KEY_VALUE *key_val;
    QDialogButtonBox *btnBox;
    QLabel *label;
    bool reset;             //false为不重置,true为重置
    void buttonIni(bool f);
};

#endif // FACTORYRESET_H
