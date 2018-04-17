#ifndef ASSETWIDGET_H
#define ASSETWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"

#include "../Common/common.h"

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

signals:
    void fresh_parent();

private:
    Ui::AssetWidget *ui;
    CURRENT_KEY_VALUE *key_val;
    int menu_index;

    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh_setting();
};

#endif // ASSETWIDGET_H
