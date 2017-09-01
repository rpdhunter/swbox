#ifndef AEWIDGET_H
#define AEWIDGET_H

#include <QFrame>

#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"

#include "../Common/barchart.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"

namespace Ui {
class AEWidget;
}

class AEWidget : public QFrame
{
    Q_OBJECT

public:
    explicit AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, int menu_index, QWidget *parent = 0);
    ~AEWidget();

public slots:
    void trans_key(quint8 key_code);

signals:
    void send_key(quint8);
    void fresh_parent();

private:
    Ui::AEWidget *ui;    
    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    int menu_index;
    SQL_PARA sql_para;

    void fresh_setting();
};

#endif // AEWIDGET_H
