#ifndef ASSETWIDGET_H
#define ASSETWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"

#include "Gui/Common/common.h"
#include "assetmodel.h"
#include "assetview.h"
#include "assetsql.h"
#include <QListWidget>

namespace Ui {
class AssetWidget;
}

class AssetWidget : public QFrame
{
    Q_OBJECT

public:
    explicit AssetWidget(CURRENT_KEY_VALUE *val, int menu_index, QWidget *parent = 0);
    ~AssetWidget();

public slots:
    void trans_key(quint8 key_code);
    void input_finished(QString str);   //软键盘输入结束


signals:
    void fresh_parent();

    void show_input(QString initial_str, QString hint_str);       //开启软键盘(初始值，提示值)
    void send_input_key(quint8);    //专门的虚拟键盘事件

private:
    Ui::AssetWidget *ui;
    CURRENT_KEY_VALUE *key_val;
    int menu_index;

    AssetModel *model;
    AssetView *view;
    AssetSql *sql;

    QListWidget *menu_area, *menu_substation, *menu_equipment;       //右键菜单

    bool inputStatus;       //记录当前是否在虚拟键盘输入状态


    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh_setting();

    void select_root();
    void select_up();
    void select_down();

    void expand_collapse();
    void add_node(Node::Type type);
    void add_child(Node::Type type);        //参数type是添加子节点的类型
    void set_node(Node::Type type, QModelIndex &index, QModelIndex &index_new);
    void del_node();
    void edit_node(QString hint_str);
    void set_current();
};

#endif // ASSETWIDGET_H
