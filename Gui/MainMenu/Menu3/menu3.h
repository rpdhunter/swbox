#ifndef MENU3_H
#define MENU3_H

#include <QFrame>
#include <QLabel>
#include "HFAmplitude/hfamplitude.h"
#include "HFAtlas/hfatlas.h"
#include "HFPrps/hfprps.h"
#include "IO/Key/key.h"

class Menu3 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu3(QWidget *parent = 0);

    void working(CURRENT_KEY_VALUE *val);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;
    HFAmplitude *hfamplitude;
    HFAtlas *hfatlas;
    HFPrps *hfprps;

    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;

    QLabel *amplitude_lab, *atlas_lab, *prps_lab;
    QLabel *amplitude_txt_lab, *atlas_txt_lab, *prps_txt_lab;

    void fresh_grade1(void);
    void fresh_table(void);
};

#endif // MENU3_H
