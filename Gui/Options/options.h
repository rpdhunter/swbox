#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFrame>
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

    void working(CURRENT_KEY_VALUE *val);

public slots:    
    void trans_key(quint8);

signals:
    void fresh_parent();
    void fregChanged(int );
    void closeTimeChanged(int);
    void update_statusBar(QString);

private:
    Ui::OptionUi *ui;

    CURRENT_KEY_VALUE *key_val;
    SQL_PARA sql_para;
    G_PARA *data;

    int _freq;      //频率
    int _backlight; //背光
    QDateTime _datetime;

    void optionIni();
    void updateCheckBox();
    void saveOptions();
    void saveFreq();
    void saveDatetime();
    void setBacklight();
    void saveAutoRec();
    void refresh();
};

#endif // OPTIONS_H
