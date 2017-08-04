#ifndef MENU1_H
#define MENU1_H

#include <QFrame>
#include <QLabel>
#include "../Menu0/tevwidget.h"
#include "IO/Data/data.h"
#include "IO/Key/key.h"

class Menu1 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu1(QWidget *parent = 0, G_PARA *g_data = NULL);

    void working(CURRENT_KEY_VALUE *val);

    void showWaveData(VectorList buf, MODE mod);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);
    void offset_suggest(int,int);
    void origin_pluse_points(QVector<QPoint> p, int group);
    void startRecWave(MODE, int);
    void tev_modbus_data(int,int);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;
    TEVWidget *tevWidget;

    QLabel  *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;

    void fresh_table(void);
};

#endif // MENU1_H
