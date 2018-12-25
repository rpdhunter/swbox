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
    void save_channel();        //保存通道数据

private slots:
    void fresh_1000ms();
    void fresh_100ms();
    void reload(int index);
    void slotGetOneFrame(QImage img);
    void change_camera_status();
    void check_camera();        //重新评估摄像头状态

private:
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void do_key_ok();
    void do_key_cancel();
    void fresh_setting();
    void PRPDReset();

    void chart_ini();
    void fresh(bool f); //刷新数据核

    void save_camera_picture();

    Ui::AAWidget *ui;
    L_CHANNEL_SQL *aaultra_sql;

    double temp_db; //显示值缓冲区，用于减缓刷新

    Socket *socket;
    CameraDecode *decode;

    QImage mImage;

    void paintEvent(QPaintEvent *);

    bool camera_fullsize;       //摄像头全屏标志位
    bool camera_hasdata;        //摄像头状态正常
    QTimer *timer_5000ms;        //5秒检测摄像头状态

};

#endif // AAWIDGET_H
