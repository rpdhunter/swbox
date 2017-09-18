#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFrame>
#include <QLabel>
#include <QDateTime>
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Data/data.h"
#include "IO/Other/wifi.h"
#include "../Common/wifitools.h"


namespace Ui {
class OptionUi;
}


class Options : public QFrame
{
    Q_OBJECT
public:
    Options(QWidget *parent = 0, G_PARA *g_data = NULL);
    ~Options();

    void working(CURRENT_KEY_VALUE *val);

public slots:    
    void trans_key(quint8);

signals:
    void fresh_parent();
    void fregChanged(int );
    void closeTimeChanged(int);
    void update_statusBar(QString);

private slots:
    void refresh_wifilist(QStringList list);

private:
    Ui::OptionUi *ui;

    CURRENT_KEY_VALUE *key_val;
    SQL_PARA sql_para;
    G_PARA *data;
    QDateTime _datetime;
    WifiConfig *wifi_config;
    QLabel *tab0, *tab1, *tab2;

    WifiTools *tools;

    void optionIni();
    void wifiIni();
    void saveOptions();
    void saveDatetime();
    void do_key_left_right(int d);
    void do_key_up_down(int d);

    void refresh();
};

#endif // OPTIONS_H
