#include "camerachart.h"
#include <QPicture>
#include <QDateTime>

CameraChart::CameraChart(QObject *parent) : BaseChart(parent)
{
    camera_label = NULL;
}

void CameraChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    camera_hasdata = false;
    camera_fullsize = false;

    camera_label = new QLabel(parent);
//    camera_label->setStyleSheet("background:red;");
    camera_label->move(10,35);
    camera_label->resize(200, 140);
    camera_label->hide();

    fps_label = new QLabel(parent);
    fps_label->setText("FPS: 0");
    fps_label->resize(60,20);
    fps_label->setStyleSheet("QLabel{color:white;}");
    fps_label->move(15,155);
    fps_label->hide();

    tips_label = new QLabel(parent);
    fps_label->setText("按OK键连接摄像头");
    fps_label->resize(200,60);
    fps_label->setStyleSheet("QLabel{color:white;}");
    fps_label->move(15,80);

    camera_socket = new CameraSocket1;

//    socket = new CameraSocket;

    decode = new CameraDecode;
    timer_5000ms = new QTimer();
    timer_5000ms->setInterval(5000);
    timer_5000ms->setSingleShot(true);
    connect(timer_5000ms, SIGNAL(timeout()), this, SLOT(check_camera()));
    connect(camera_socket, SIGNAL(sendOnePacket(QByteArray)), decode, SLOT(getOnePacket(QByteArray)));      //socket-->解码器
    connect(camera_socket, SIGNAL(sendOnePacket(QByteArray)), this, SLOT(change_camera_status()));
    connect(decode,SIGNAL(sigGetOneFrame(QImage)),this,SLOT(slotGetOneFrame(QImage)),Qt::QueuedConnection);      //解码器-->主界面

    decode->camera_init();      //做一下连接初始化动作（打开AP，打开camera外部程序）
    decode->start();

    timer_1000ms = new QTimer;
    timer_1000ms->setInterval(1000);
    frame_count = 0;
    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(show_frame_count()));
    timer_1000ms->start();
}

void CameraChart::do_key_ok()
{
    if(camera_hasdata == false){        //如果摄像头卡顿,则尝试重连
        camera_socket->connect_camera();
//        socket->connect_camera();
        return;
    }

    if(camera_fullsize == false /*&& mImage.size().width() > 0*/){       //进入全屏模式
        enterFullScreen();
        return;
    }

    if(camera_fullsize == true){            //如果全屏且有数据,则进行截屏
        //插入保存谱图代码
        save_picture();
        return;
    }
}

void CameraChart::do_key_cancel()
{
    if(camera_fullsize == true){
        cancelFullScreen();
    }
}

bool CameraChart::isFullScreen(){
    return camera_fullsize;
}

void CameraChart::cancelFullScreen()
{
    camera_fullsize = false;
    camera_label->setGeometry(10,35,200, 140);
    fps_label->hide();
}

void CameraChart::enterFullScreen()
{
    camera_fullsize = true;
    camera_label->setGeometry(0,0,CHANNEL_X, CHANNEL_Y);
    fps_label->show();
}

void CameraChart::save_picture()
{
    Common::mk_dir(DIR_CAMERASHOTS);
    QString str = QString(DIR_CAMERASHOTS"/camerashot_%1.png").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") );
    int f = mImage.save(str , "PNG");
    qDebug()<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>save pic :"<<f;

    qDebug()<<"save camera picture";
    emit update_statusBar(tr("摄像头截屏已保存"));
}

void CameraChart::hide()
{
    if(camera_label != NULL){
        camera_label->hide();
        fps_label->hide();
    }
}

void CameraChart::show()
{
    if(camera_label != NULL){
        camera_label->show();
        if(camera_fullsize)
            fps_label->show();
    }
}

void CameraChart::slotGetOneFrame(QImage img)
{
    frame_count++;
    if(camera_fullsize){
        camera_label->setPixmap(QPixmap::fromImage(img).scaledToWidth(camera_label->width()));
    }
    else{
        camera_label->setPixmap(QPixmap::fromImage(img).scaled(camera_label->size()));
    }
}

void CameraChart::change_camera_status()
{
    timer_5000ms->start();
    camera_hasdata = true;
}

void CameraChart::check_camera()
{
    camera_hasdata = false;
}

void CameraChart::show_frame_count()
{
    fps_label->setText(QString("FPS: %1").arg(frame_count));
//    qDebug()<<"frame_count:\t"<<frame_count;
    frame_count = 0;
}



