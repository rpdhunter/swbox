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
#include <QListWidget>


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
    void input_finished(QString str);

signals:
    void fresh_parent();
    void fregChanged(int );
    void closeTimeChanged(int);
    void update_statusBar(QString);

    void show_input(QString);
    void send_input_key(quint8);    //专门的虚拟键盘事件

    void show_indicator(bool);      //显示菊花
    void show_wifi_icon(int);

private slots:
    void refresh_wifilist(QStringList list, WifiConfig *con);
    void wifi_hotpot_finished();

private:
    Ui::OptionUi *ui;

    CURRENT_KEY_VALUE *key_val;
    SQL_PARA sql_para;
    G_PARA *data;
    QDateTime _datetime;
    WifiConfig *wifi_config;
    QLabel *tab0, *tab1, *tab2, *tab3;

    WifiTools *tools;

    QListWidget *contextMenu;

    bool inputStatus;       //记录当前是否在虚拟键盘输入状态
    bool isBusy;            //菊花状态
    int contextMenu_num;

    void optionIni();
    void wifi_connect();
    void wifi_connect(QString name, QString password);
    void saveOptions();
    void saveDatetime();
    void do_key_up_down(int d);
    void do_key_left_right(int d);    
    void refresh();
};

#endif // OPTIONS_H
