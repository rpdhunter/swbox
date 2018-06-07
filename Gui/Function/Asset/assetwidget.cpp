#include "assetwidget.h"
#include "ui_assetwidget.h"
#include <QLineEdit>
#include <QDateTime>


#define AREA_NUM 5              //设置片区菜单条目数
#define SUBSTATION_NUM 5        //设置站所菜单条目数
#define EQUIPMENT_NUM 5         //设置设备菜单条目数

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
                                "color: white;}"
                                );

//    select_root();
    Common::select_root(ui->treeView, model);
    ui->treeView->expandAll();

    QStringList list;
    list << tr("展开/收起片区") << tr("增加片区") << tr("删除片区") << tr("重命名片区...") << tr("增加子站所");
    menu_area = new QListWidget(this);        //右键菜单
    Common::set_contextMenu_style(menu_area, list, QPoint(200,10));

    list.clear();
    list << tr("展开/收起站所") << tr("增加站所") << tr("删除站所") << tr("重命名站所...") << tr("增加子设备");
    menu_substation = new QListWidget(this);        //右键菜单
    Common::set_contextMenu_style(menu_substation, list, QPoint(250,10));

    list.clear();
    list << tr("设为当前设备") << tr("增加设备") << tr("删除设备") << tr("编辑设备...")<< tr("测量数据管理...");
    menu_equipment = new QListWidget(this);        //右键菜单
    Common::set_contextMenu_style(menu_equipment, list, QPoint(300,10));

    assetFileManagement = new AssetFileManagement(key_val, this);
    connect(this, SIGNAL(send_key(quint8)), assetFileManagement, SLOT(trans_key(quint8)));
}

AssetWidget::~AssetWidget()
{
    delete ui;
}

QString AssetWidget::normal_asset_dir_init()
{
    QString str_new = get_new_asset_dir_name(DIR_ASSET_NORMAL);    //获得新的资产目录（带日期的临时目录）
    Common::mk_dir(str_new);
    return str_new;
}

//key_val->grade.val1表征是否在当前工作页面
//key_val->grade.val2表征右键菜单是否显示
//key_val->grade.val3表征右键菜单选项
//key_val->grade.val4表征历史测量数据窗口
void AssetWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }

    if(inputStatus){        //文字输入状态
        emit send_input_key(key_code);
        return;
    }

    if(key_val->grade.val4 == 1){
        emit send_key(key_code);
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
                case 0:
                case 1:         //展开/收起片区
//                    expand_collapse();
                    Common::expand_collapse(ui->treeView);
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
                case 0:
                case 1:         //展开/收起站所
                    Common::expand_collapse(ui->treeView);
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
                case 0:
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
//                case 5:         //测试报告
//                    create_report();
//                    break;
                case 5:         //浏览测量数据
                    key_val->grade.val4 = 1;
                    assetFileManagement->setRootPath(model->data(index, Node::PathRole).toString());
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
        else if(key_val->grade.val1 != 0){                               //没显示右键菜单，则退出工作页面
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
//            select_down();
            Common::select_down(ui->treeView,model);
        }
        else{
//            select_up();
            Common::select_up(ui->treeView,model);
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

//增加兄弟节点
void AssetWidget::add_node(Node::Type type)
{
    QModelIndex index = ui->treeView->currentIndex();
    if(index.isValid()){
        model->insertRow(index.row() + 1,index.parent());           //插入
        QModelIndex index_new = index.sibling(index.row()+1,index.column());
        set_node(type, index, index_new);
    }
}

//增加子节点
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
        QModelIndex parent = index_new.parent();
        QString new_name = get_new_child_node_name(type,parent);
        model->setData(index_new, new_name, Qt::EditRole );   //设置节点名称
        model->setData(index_new, type, Node::TypeRole );     //设置节点类型
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
            if(model->data(index, Node::CurrentRole) == true){      //删除的节点是当前测量节点
                set_current();                                      //先取消选择
            }
            model->removeRow(index.row(),index.parent());
        }
    }

    //如果是当前节点，需要插入删除代码
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
        if(model->data(index, Node::CurrentRole) == true){      //设置的节点是当前测量节点
            model->setData(index, false, Node::CurrentRole);
            QString str = tr("未指定");
            ui->lab_current->setText(str);
            ui->lab_tips->setText(tr("设置未生效，仪器所有的测量数据都将归档到公有档案下"));
//            emit current_asset_changed(str, QString(DIR_DATA));
            QString str_new = get_new_asset_dir_name(DIR_ASSET_NORMAL);    //获得新的资产目录（带日期的临时目录）

            if(Common::mk_dir(str_new) ){
                emit current_asset_changed(str, str_new);
            }
            else{
                qDebug()<<"make dir failed:" << str_new;
            }
        }
        else{
            ui->lab_current->setText(text);
            ui->lab_tips->setText(tr("设置已生效，仪器所有的测量数据都将归档到此设备档案下"));
            model->setData(index, true, Node::CurrentRole);
            QString str_new = get_new_asset_dir_name(model->data(index, Node::PathRole).toString());    //获得新的资产目录（带日期的临时目录）

            if(Common::mk_dir(str_new) ){
                str_new = Common::str_to_cn(str_new);
                emit current_asset_changed(text, str_new);
            }
            else{
                qDebug()<<"make dir failed:" << str_new;
            }
        }
    }
}

void AssetWidget::set_no_equ()
{
    QString str = tr("未指定");
    ui->lab_current->setText(str);
    ui->lab_tips->setText(tr("设置未生效，仪器所有的测量数据都将归档到公有档案下"));
    emit current_asset_changed(str, QString(DIR_DATA));
}



QString AssetWidget::get_new_child_node_name(Node::Type type, QModelIndex &parent)
{
    QString path = model->data(parent,Node::PathRole).toString();
    qDebug()<<"get_new_name"<<path;
    QDir dir;
    dir.setPath(path);
    QStringList list = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
    QString new_name;
    int i = 0;

    switch (type) {
    case Node::Area:
        do{
            i++;
            new_name = QString("New_Area%1").arg(i);
        }
        while (list.contains(new_name,Qt::CaseInsensitive));
        break;
    case Node::Substation:
        do{
            i++;
            new_name = QString("New_Substation%1").arg(i);
        }
        while (list.contains(new_name,Qt::CaseInsensitive));
        break;
    case Node::Equipment:
        do{
            i++;
            new_name = QString("New_Equipment%1").arg(i);
        }
        while (list.contains(new_name,Qt::CaseInsensitive));
        break;
    default:
        break;
    }
    return new_name;
}

QString AssetWidget::get_new_asset_dir_name(QString path)
{
    QDir dir;
    dir.setPath(path);
    QStringList list = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);    //仅显示文件夹，按名称排列
    int max = 0;                        //文件夹名前四位是编号
    foreach (QString l, list) {         //找到最大编号
        max = MAX(l.left(4).toInt(),max);
    }

    QString str_new = QString("%1/%2#%3").arg(dir.path()).arg(max+1,4,10,QLatin1Char('0'))
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"));
    return str_new;
}

void AssetWidget::fresh_setting()
{
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

    if(key_val->grade.val4 == 1){
        assetFileManagement->show();
    }
    else{
        assetFileManagement->hide();
    }

}











