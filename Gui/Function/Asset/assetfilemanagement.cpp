#include "assetfilemanagement.h"
#include <QtDebug>
#include <QHeaderView>
#include "IO/Other/report.h"

#define DIR_NUM 3           //文件夹菜单条目数
#define FILE_NUM 2          //文件菜单条目数


AssetFileManagement::AssetFileManagement(CURRENT_KEY_VALUE *val, QWidget *parent) : QTreeView(parent)
{
    this->resize(CHANNEL_X, CHANNEL_Y);
//    this->setStyleSheet("AssetWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList list;
    list << tr("展开/收起文件夹") << tr("删除文件夹") << tr("生成测试报告");
    menu_dir = new QListWidget(this);        //右键菜单
    Common::set_contextMenu_style(menu_dir, list, QPoint(250,30));
    list.clear();
    list << tr("查看文件") << tr("删除文件");
    menu_file = new QListWidget(this);        //右键菜单
    Common::set_contextMenu_style(menu_file, list, QPoint(260,30));

    model = new QFileSystemModel;
    key_val = val;
    model->setRootPath(DIR_ASSET);
    this->setModel(model);
    this->setRootIndex(model->index(DIR_ASSET));
    QHeaderView *view = this->header();
    view->resizeSection(0,350);
    view->resizeSection(1,80);
    view->setSectionHidden(2,true);
    view->setSectionHidden(3,true);
    view->hide();
    this->hide();

    connect(model, SIGNAL(directoryLoaded(QString)), this, SLOT(do_some_init()) );      //转换目录后需要等文件系统读取完毕后才能选择
}

void AssetFileManagement::setRootPath(QString path)
{
    model->setRootPath(path);
    this->setRootIndex(model->index(path));
    this->setCurrentIndex(model->index(path));
//    qDebug()<<"rowCount"<<model->rowCount(this->rootIndex());
}

void AssetFileManagement::do_some_init()
{
//    qDebug()<<"loaded: rowCount"<<model->rowCount(this->rootIndex());
    if(model->rowCount(this->rootIndex()) > 0){
        this->setCurrentIndex(rootIndex().child(0,0));

    }
}

void AssetFileManagement::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    bool f = false;

    switch (key_code) {
    case KEY_OK:
        if(menu_dir->isVisible()){
            switch (key_val->grade.val5) {
            case 1:
                Common::expand_collapse(this);
                break;
            case 2:
                f = model->remove(this->currentIndex());
                qDebug()<<"del dir" << f;
                break;
            case 3:
                create_report();
                break;
            default:
                break;
            }
            menu_dir->hide();
        }
        else if(menu_file->isVisible()){
            switch (key_val->grade.val5) {
            case 1:
                qDebug()<<"check file";
                break;
            case 2:
                f = model->remove(this->currentIndex());
                qDebug()<<"del file" << f;
                break;
            default:
                break;
            }
            menu_file->hide();
        }
        else{
            Common::expand_collapse(this);
            qDebug()<<"Common::expand_collapse(this)";
        }
        key_val->grade.val5 = 0;
        break;
    case KEY_CANCEL:
        if(menu_dir->isVisible() || menu_file->isVisible()){
            menu_dir->hide();
            menu_file->hide();
            key_val->grade.val5 = 0;
        }
        else{
            key_val->grade.val4 = 0;
            this->hide();
        }
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        if(menu_dir->isHidden() && menu_file->isHidden() && this->currentIndex() != rootIndex()){
            if(model->isDir(this->currentIndex())){
                menu_dir->show();
                key_val->grade.val5 = 1;
            }
            else{
                menu_file->show();
                key_val->grade.val5 = 1;
            }
        }
        break;
    default:
        break;
    }
    fresh_setting();
}

void AssetFileManagement::create_report()
{
    Report r;
    r.create_report( model->filePath(this->currentIndex()) );
}

void AssetFileManagement::do_key_up_down(int d)
{
    if(menu_dir->isVisible()){
        Common::change_index(key_val->grade.val5,d,DIR_NUM,1);
    }
    else if(menu_file->isVisible()){
        Common::change_index(key_val->grade.val5,d,FILE_NUM,1);
    }
    else{
        if(d<0){
            Common::select_up(this,model);
        }
        else{
            Common::select_down(this,model);
        }
    }
}

void AssetFileManagement::fresh_setting()
{
    if(menu_dir->isVisible()){
        menu_dir->setCurrentRow(key_val->grade.val5 - 1 );
    }
    else if(menu_file->isVisible()){
        menu_file->setCurrentRow(key_val->grade.val5 - 1 );
    }
}


