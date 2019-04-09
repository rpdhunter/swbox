#ifndef OPTIONWIDGET_H
#define OPTIONWIDGET_H

#include <QTabWidget>
#include <QLabel>
#include "Option/optionbasic.h"
#include "Option/optionexpert.h"
#include "Option/optionstorage.h"
#include "Option/optionwifi.h"
#include "../Common/basewidget.h"
#include "IO/Data/data.h"

namespace Ui {
class OptionWidget;
}

class OptionWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit OptionWidget(G_PARA *g_data, QWidget *parent = nullptr);
    ~OptionWidget();

    void working(CURRENT_KEY_VALUE *val);


signals:

public slots:
    void trans_key(quint8 key_code);

private slots:
    void do_tab_quit();     //处理子页面退出
    void save_options();                    //保存sql

private:
    Ui::OptionWidget *ui;

    OptionBasic *option_basic;
    OptionExpert *option_expert;
    OptionStorage *option_storage;
    OptionWifi *option_wifi;

//    QTabWidget *tab_wdiget;
    G_PARA *data;
    SQL_PARA sql_para;

    QLabel *tab0, *tab1, *tab2, *tab3;

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);

    void fresh();
};

#endif // OPTIONWIDGET_H
