#ifndef MENU4_H
#define MENU4_H

#include <QFrame>
#include <QLabel>
#include "HFCAmplitude/hfcamplitude.h"
#include "HFCAtlas/hfcatlas.h"
#include "IO/Key/key.h"

class Menu4 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu4(QWidget *parent = 0);

    void working(CURRENT_KEY_VALUE *val);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;
    HFCAmplitude *hfcamplitude;
    HFCAtlas *hfcatlas;

    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;

    QLabel *amplitude_lab, *atlas_lab;
    QLabel *amplitude_txt_lab, *atlas_txt_lab;

    void fresh_grade1(void);
    void fresh_table(void);
};

#endif // MENU4_H
