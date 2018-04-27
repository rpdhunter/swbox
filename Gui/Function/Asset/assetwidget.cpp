#include "assetwidget.h"
#include "ui_assetwidget.h"
#include <QLineEdit>
//#include <QScrollBar>

#define AREA_NUM 5           //设置菜单条目数
#define SUBSTATION_NUM 5           //设置菜单条目数
#define EQUIPMENT_NUM 4           //设置菜单条目数

AssetWidget::AssetWidget(CURRENT_KEY_VALUE *val, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AssetWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AssetWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->key_val = val;
    this->menu_index = menu_index;

    inputStatus = false;

    model = new AssetModel;
    view = new AssetView;

    ui->treeView->setModel(model);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->treeView->setStyleSheet("QTreeView{ "
                                "border-image: url(:/widgetphoto/bk/bk2.png);"
//                                "background-color: #5B677A;"
//                                font-size:17px;
                                "color: white;}"
                                );

    select_root();
    ui->treeView->expandAll();

    QStringList list;
    list << tr("展开/收起片区") << tr("增加片区") << tr("删除片区") << tr("重命名片区") << tr("增加子站所");
    menu_area = new QListWidget(this);        //右键菜单
    menu_area->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    menu_area->addItems(list);
    menu_area->resize(100, 100);
    menu_area->move(200,10);
    menu_area->setSpacing(2);
    menu_area->hide();

    list.clear();
    list << tr("展开/收起站所") << tr("增加站所") << tr("删除站所") << tr("重命名站所") << tr("增加子设备");
    menu_substation = new QListWidget(this);        //右键菜单
    menu_substation->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    menu_substation->addItems(list);
    menu_substation->resize(100, 100);
    menu_substation->move(250,10);
    menu_substation->setSpacing(2);
    menu_substation->hide();

    list.clear();
    list << tr("设为当前设备") << tr("增加设备") << tr("删除设备") << tr("编辑设备");
    menu_equipment = new QListWidget(this);        //右键菜单
    menu_equipment->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    menu_equipment->addItems(list);
    menu_equipment->resize(100, 80);
    menu_equipment->move(300,10);
    menu_equipment->setSpacing(2);
    menu_equipment->hide();

}

AssetWidget::~AssetWidget()
{
    delete ui;
}

//    key_val->grade.val1            //是否在当前工作页面
//    key_val->grade.val2            //右键菜单是否显示
//    key_val->grade.val3             //右键菜单选项
void AssetWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }

    if(inputStatus){        //文字输入状态
        emit send_input_key(key_code);
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val1 == 0){       //不在工作页面则进入工作页面
            key_val->grade.val1 = 1;
        }
        else{                               //在工作页面，则进入对应功能
            QModelIndex index = ui->treeView->currentIndex();
            QString str = model->data(model->index(index.row(),1,index.parent()),Qt::DisplayRole).toString();
            if(str == "Area"){
                switch (key_val->grade.val3) {
                case 1:         //展开/收起片区
                    expand_collapse();
                    break;
                case 2:         //增加片区
                    add_node(Node::Area);
                    break;
                case 3:         //删除片区
                    del_node();
                    break;
                case 4:         //重命名片区
                    edit_node(tr("输入新的片区名"));
                    break;
                case 5:         //增加子站所
                    add_child(Node::Substation);
                    break;
                default:
                    break;
                }
            }
            else if(str == "Substation"){
                switch (key_val->grade.val3) {
                case 1:         //展开/收起站所
                    expand_collapse();
                    break;
                case 2:         //增加站所
                    add_node(Node::Substation);
                    break;
                case 3:         //删除站所
                    del_node();
                    break;
                case 4:         //重命名站所
                    edit_node(tr("输入新的站所名"));
                    break;
                case 5:         //增加子设备
                    add_child(Node::Equipment);
                    break;
                default:
                    break;
                }
            }
            else if(str == "Equipment"){
                switch (key_val->grade.val3) {
                case 1:         //设为当前设备
                    set_current();
                    break;
                case 2:         //增加设备
                    add_node(Node::Equipment);
                    break;
                case 3:         //删除设备
                    del_node();
                    break;
                case 4:         //编辑设备
                    edit_node(tr("输入新的设备名"));
                    break;
                default:
                    break;
                }
            }
            key_val->grade.val2 = 0;
            key_val->grade.val3 = 0;
        }
        break;
    case KEY_CANCEL:
        if(key_val->grade.val2 != 0){       //右键菜单激活则退出右键菜单
            key_val->grade.val2 = 0;
        }
        else{                               //没显示右键菜单，则退出工作页面
            key_val->grade.val1 = 0;
        }
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    default:
        break;
    }

    emit fresh_parent();
    fresh_setting();
}

void AssetWidget::input_finished(QString str)
{
    if(inputStatus){            //保证不会收到非此界面的键盘输入
        inputStatus = false;
        if(!str.isEmpty()){
            QModelIndex index = ui->treeView->currentIndex();
            model->setData(index, str, Qt::EditRole );
            model->save_node(index);        //保存至sql
        }
    }
}

void AssetWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;            //进入资产操作页面

    if(key_val->grade.val2 == 1){       //右键显示
        QModelIndex index = ui->treeView->currentIndex();
        QString str = model->data(model->index(index.row(),1,index.parent()),Qt::DisplayRole).toString();
        if(str == "Area"){
            Common::change_index(key_val->grade.val3, d, AREA_NUM, 1);
        }
        else if(str == "Substation"){
            Common::change_index(key_val->grade.val3, d, SUBSTATION_NUM, 1);
        }
        else if(str == "Equipment"){
            Common::change_index(key_val->grade.val3, d, EQUIPMENT_NUM, 1);
        }
    }
    else{                               //树操作
        if(d > 0){
            select_down();
        }
        else{
            select_up();
        }
    }
}

void AssetWidget::do_key_left_right(int d)
{    
    if(key_val->grade.val1 == 0){
        return;
    }
    if(key_val->grade.val2 == 1){       //右键已显示

    }
    else{                               //树操作
        if(d > 0){
            key_val->grade.val2 = 1;        //右键菜单显示
            key_val->grade.val3 = 1;        //自动选择第一个菜单
        }
        else{                               //返回上一级树
            QModelIndex index = ui->treeView->currentIndex();
            QModelIndex index_next = model->index(0,0,index.parent().parent());
            if(index_next.isValid()){
                ui->treeView->setCurrentIndex(index_next);
            }
        }
    }
}

void AssetWidget::select_root()
{
    QModelIndex index = model->index(0,0,ui->treeView->rootIndex());        //选择第一个可选节点（非不可见的根节点）
    ui->treeView->setCurrentIndex(index);
}

void AssetWidget::select_up()
{
    QModelIndex index = ui->treeView->currentIndex();
    QModelIndex index_next;

    if(index.row()>0){      //有哥哥
        index_next = index.sibling(index.row()-1,0);            //先找本节点的第一个哥哥
        if(index_next.isValid() && ui->treeView->isExpanded(index_next)){       //有哥哥，如果是展开的，先找哥哥的小儿子
            int rowCount = model->rowCount(index_next);
            if(rowCount > 0){
                index_next = model->index(rowCount-1,0,index_next);     //找到哥哥的小儿子
                if(index_next.isValid() && ui->treeView->isExpanded(index_next) ){       //有孙子并且是展开的，再找哥哥的小孙子
                    rowCount = model->rowCount(index_next);
                    if(rowCount > 0){
                        index_next = model->index(rowCount-1,0,index_next);     //找到哥哥的小孙子（循环次数视叔深度而定，不定深度可能需要递归）
                    }
                }
            }
        }
    }
    else{           //无哥哥
        index_next = index.parent();        //找爹
    }

    if(index_next.isValid()){
        ui->treeView->setCurrentIndex(index_next);
    }
}

void AssetWidget::select_down()
{
    QModelIndex index = ui->treeView->currentIndex();
    QModelIndex index_next;

    if(ui->treeView->isExpanded(index) ){                   //如果当前节点是展开的
        index_next = model->index(0,0,index);                   //第一顺位是本节点的第一个儿子
    }


    if(!index_next.isValid()){
        index_next = model->index(index.row()+1,index.column(),index.parent());     //第二顺位是本节点的下一个兄弟
    }

    if(!index_next.isValid()){
        int row = index.parent().row();
        index_next = model->index(row + 1,0,index.parent().parent());       //第三顺位是本节点的下一个叔叔
    }

    if(!index_next.isValid()){
        int row = index.parent().parent().row();
        index_next = model->index(row + 1,0,index.parent().parent().parent());      //第四顺位是叔爷爷
    }
    if(index_next.isValid()){
        ui->treeView->setCurrentIndex(index_next);
    }
}

//展开/收起节点
void AssetWidget::expand_collapse()
{
    QModelIndex index = ui->treeView->currentIndex();
    if(ui->treeView->isExpanded(index)){
        ui->treeView->collapse(index);
    }
    else{
        ui->treeView->expand(index);
    }
}

void AssetWidget::add_node(Node::Type type)
{
    QModelIndex index = ui->treeView->currentIndex();
    if(index.isValid()){
        model->insertRow(index.row() + 1,index.parent());           //插入
        QModelIndex index_new = index.sibling(index.row()+1,index.column());
        set_node(type, index, index_new);
    }
}

void AssetWidget::add_child(Node::Type type)
{
    QModelIndex index = ui->treeView->currentIndex();
    if(index.isValid()){
        model->insertRow(model->rowCount(index),index);
        QModelIndex index_new = model->index(model->rowCount(index)-1,0,index);
        set_node(type, index, index_new);
    }
}

void AssetWidget::set_node(Node::Type type, QModelIndex &index, QModelIndex &index_new)
{
    if(index.isValid() && index_new.isValid()){
        switch (type) {
        case Node::Area:
            model->setData(index_new, QString("New Area%1").arg(index.row()), Qt::EditRole );   //设置节点名称
            model->setData(index_new, Node::Area, Qt::UserRole );                               //设置节点类型
            break;
        case Node::Substation:
            model->setData(index_new, QString("New Substation%1").arg(index.row()), Qt::EditRole );
            model->setData(index_new, Node::Substation, Qt::UserRole );
            break;
        case Node::Equipment:
            model->setData(index_new, QString("New Equipment%1").arg(index.row()), Qt::EditRole );
            model->setData(index_new, Node::Equipment, Qt::UserRole );
            break;
        default:
            break;
        }

        model->save_node(index_new);        //保存至sql
        ui->treeView->setCurrentIndex(index_new);
    }
}

void AssetWidget::del_node()
{
    QModelIndex index = ui->treeView->currentIndex();
    if(index.isValid()){
        if(model->rowCount(index) >0 ){
            qDebug()<<"the node has at least one child yet, please delete it first!";
        }
        else{
            model->removeRow(index.row(),index.parent());
        }
    }
}

void AssetWidget::edit_node(QString hint_str)
{
    emit show_input(model->data(ui->treeView->currentIndex(),Qt::DisplayRole).toString(), hint_str);
    inputStatus = true;
}

void AssetWidget::set_current()
{
    QModelIndex index = ui->treeView->currentIndex();
    if(index.isValid()){
        QString text = model->data(index,Qt::DisplayRole).toString();
        if(ui->lab_current->text() == text){
            ui->lab_current->setText(tr("未设置"));
            ui->lab_tips->setText(tr("设置未生效，仪器所有的测量数据都将归档到公有档案下"));
        }
        else{
            ui->lab_current->setText(text);
            ui->lab_tips->setText(tr("设置已生效，仪器所有的测量数据都将归档到此设备档案下"));
        }
    }
}

void AssetWidget::fresh_setting()
{
    //    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    //    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
    //        ui->comboBox->showPopup();
    //    }
    //    else{
    //        ui->comboBox->hidePopup();
    //    }

    //    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    if(key_val->grade.val2 == 1){       //右键显示
        QModelIndex index = ui->treeView->currentIndex();
        QString str = model->data(model->index(index.row(),1,index.parent()),Qt::DisplayRole).toString();
        if(str == "Area"){
            menu_area->show();
            menu_area->setCurrentRow(key_val->grade.val3-1);
        }
        else if(str == "Substation"){
            menu_substation->show();
            menu_substation->setCurrentRow(key_val->grade.val3-1);
        }
        else if(str == "Equipment"){
            menu_equipment->show();
            menu_equipment->setCurrentRow(key_val->grade.val3-1);
        }
    }
    else{
        menu_area->hide();
        menu_substation->hide();
        menu_equipment->hide();
    }

}











