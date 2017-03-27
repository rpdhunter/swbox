#ifndef HFCATLAS_H
#define HFCATLAS_H

#include <QFrame>
#include <QDebug>
#include <QLabel>
#include "IO/Key/key.h"

class HFCAtlas : public QFrame
{
    Q_OBJECT
public:
    HFCAtlas(QWidget *parent = 0);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);

signals:
    void fresh_parent(void);

private:
    CURRENT_KEY_VALUE *key_val;
    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab, *set_child3_lab;
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab, *child3_val_lab;
    QLabel *set_txt_lab;
    QLabel *curve_lab;
};

#endif // HFCATLAS_H
