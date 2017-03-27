#ifndef HFPRPS_H
#define HFPRPS_H

#include <QFrame>
#include <QDebug>
#include <QLabel>
#include "IO/Key/key.h"

class HFPrps : public QFrame
{
    Q_OBJECT
public:
    HFPrps(QWidget *parent = 0);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);

signals:
    void fresh_parent(void);

private:
    CURRENT_KEY_VALUE *key_val;

    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab;
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab;
    QLabel *set_txt_lab;
};

#endif // HFPRPS_H
