#ifndef MENU5_H
#define MENU5_H

#include <QFrame>
#include <QLabel>
#include "IO/Key/key.h"
#include "IO/Data/data.h"

class RFCTWidget;

class Menu5 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu5(QWidget *parent = 0, G_PARA *g_data = NULL);

    void working(CURRENT_KEY_VALUE *val);
    void showWaveData(VectorList buf, MODE mod);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);
    void startRecWave(MODE, int);

public slots:
    void trans_key(quint8);


private slots:
    void fresh_table(void);

private:
    CURRENT_KEY_VALUE *key_val;

    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6, *bk_lab;

    RFCTWidget *rfctwidget;

    void fresh_grade1(void);

};

#endif // MENU5_H
