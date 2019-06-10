#ifndef CAMERACHART_H
#define CAMERACHART_H

#include "basechart.h"
#include "Thread/Camera/cameradecode.h"
#include "Thread/Camera/camerasocket.h"

class CameraChart : public BaseChart
{
    Q_OBJECT
public:
    explicit CameraChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);  //初始化
    void hide();
    void show();

    void do_key_ok();           //完成OK一系列操作
    void do_key_cancel();       //完成cancel一系列操作
    bool isFullScreen();        //返回是否全屏模式

signals:
    void update_statusBar(QString);
    void connect_camera();      //连接摄像头

public slots:
    void slotGetOneFrame(QImage img);
    void change_camera_status();
    void check_camera();        //重新评估摄像头状态

    void show_frame_count();

private:
    QLabel *camera_label;

    CameraDecode *decode;

    CameraSocket *camera_socket;

    QImage mImage;
//    bool ap_init_flag;          //网络连接标志位
    bool camera_fullsize;       //摄像头全屏标志位
    bool camera_hasdata;        //摄像头状态正常
    QTimer *timer_5000ms;        //5秒检测摄像头状态

    int frame_count;            //帧计数器
    QTimer *timer_1000ms;
    QLabel *fps_label;          //帧数显示
    QLabel *tips_label;         //显示提示信息

    void cancelFullScreen();    //退出全屏模式
    void enterFullScreen();     //进入全屏模式
    void save_picture();        //保存摄像头数据


};

#endif // CAMERACHART_H
