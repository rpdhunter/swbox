#ifndef MENU2_H
#define MENU2_H

#include <QFrame>
#include <QLabel>
#include "IO/Key/key.h"
#include "IO/Data/data.h"
#include "faultlocation.h"

class Menu2 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu2(QWidget *parent = 0, G_PARA *g_data = NULL);

    void working(CURRENT_KEY_VALUE *val);

    void sysReset();
    void maxReset();

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;
    FaultLocation *faultlocation;

    QLabel  *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;

    void fresh_table(void);
};

#endif // MENU2_H
