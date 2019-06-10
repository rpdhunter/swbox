#ifndef SETTINGMUNU_H
#define SETTINGMUNU_H

#include <QObject>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"

#define SUB_MENU_NUM_BASE   20  //次级菜单起始编号

class SettingMunu : public QObject
{
    Q_OBJECT
public:
    explicit SettingMunu(MODE mode, QWidget *parent = nullptr);

    void fresh(CHANNEL_SQL *sql, int grade);      //按照菜单编号刷新

    int main_menu_num();            //返回主菜单数
    int sub_menu_num();             //返回次菜单数

    void show();
    void hide();

signals:

public slots:

private:
    QListWidget *main_menu, *sub_menu;
    QLineEdit *lineEdit;
    QLabel *label;
    MODE mode;
    int wide;       //菜单宽度
};

#endif // SETTINGMUNU_H
