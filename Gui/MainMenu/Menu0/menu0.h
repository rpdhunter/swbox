#ifndef MENU0_H
#define MENU0_H

#include "Amplitude/amplitude.h"
#include "Pulse/pulse.h"
#include "IO/Key/key.h"
#include <QFrame>
#include <QDebug>
#include <QLabel>

class Menu0 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu0(QWidget *parent = NULL, G_PARA *g_data = NULL);

    void working(CURRENT_KEY_VALUE *val);
    void sysReset();    //重置并显示默认值
    void maxReset();    //最大值清零

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);
    void offset_suggest(int ,int);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;
    Amplitude *amplitude;
//    Pulse *pulse;

    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;
    QLabel *amplitude_lab, *amplitude_txt_lab, *pulse_lab, *pulse_txt_lab;

//    void fresh_grade1(void);
    void fresh_table(void);
};

#endif // MENU0_H
