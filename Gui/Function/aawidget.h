#ifndef AAWIDGET_H
#define AAWIDGET_H

#include "channelwidget.h"
#include "IO/Com/socket.h"
#include "IO/TempThread/cameradecode.h"

namespace Ui {
class AAWidget;
}

class AAWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~AAWidget();

public slots:


private slots:
    void fresh_1000ms();
    void fresh_100ms();
    void reload(int index);
    void slotGetOneFrame(QImage img);

private:
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void do_key_ok();
    void fresh_setting();

    void chart_ini();
    void fresh(bool f); //刷新数据核

    Ui::AAWidget *ui;
    L_CHANNEL_SQL *aaultra_sql;

    double temp_db; //显示值缓冲区，用于减缓刷新

    Socket *socket;
    CameraDecode *decode;

    QImage mImage;

    void paintEvent(QPaintEvent *);

};

#endif // AAWIDGET_H
