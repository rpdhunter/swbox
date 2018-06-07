#ifndef ASSETFILEMANAGEMENT_H
#define ASSETFILEMANAGEMENT_H

#include <QTreeView>
#include <QFileSystemModel>
#include <QListWidget>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "Gui/Common/common.h"

class AssetFileManagement : public QTreeView
{
    Q_OBJECT
public:
    explicit AssetFileManagement(CURRENT_KEY_VALUE *val, QWidget *parent = nullptr);
    void setRootPath(QString path);

signals:

public slots:
    void trans_key(quint8 key_code);
    void do_some_init();

private:
    CURRENT_KEY_VALUE *key_val;
    QFileSystemModel *model;

    QListWidget *menu_dir, *menu_file;

    void create_report();           //创建测试报告

    void do_key_up_down(int d);
//    void do_key_left_right(int d);
    void fresh_setting();
};

#endif // ASSETFILEMANAGEMENT_H
