#ifndef MENU3_H
#define MENU3_H

#include <QFrame>
#include <QLabel>
#include "aaultrasonic2.h"
#include "IO/Key/key.h"

class Menu3 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu3(QWidget *parent = 0, G_PARA *g_data = NULL);

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
    AAUltrasonic2 *aaultrasonic;

    QLabel  *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;

    void fresh_table(void);
};

#endif // MENU3_H
