#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QDateTime>
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Data/data.h"


namespace Ui {
class OptionUi;
}


class Options : public QFrame
{
    Q_OBJECT
public:
    Options(QWidget *parent = 0, G_PARA *g_data = NULL);

    ~Options();

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);
    void refresh();

signals:
    void fresh_parent(void);
    void fregChanged(int );
    void closeTimeChanged(int);
    void maxResetTimeChanged(int);

private:
    CURRENT_KEY_VALUE *key_val;
    SQL_PARA *sql_para;
    G_PARA *data;

    Ui::OptionUi *ui;

    int _freq;      //频率
    int _backlight; //背光
    QDateTime _datetime;
//    int _resettime; //数据重置时间
//    int _closetime; //关机时间

    void optionIni();
    void updateCheckBox();
    void saveOptions();
    void saveFreq();
    void saveDatetime();
    void setBacklight();
    void saveAutoRec();


};

#endif // OPTIONS_H
