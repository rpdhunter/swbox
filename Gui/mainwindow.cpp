#include "mainwindow.h"
#include <QBuffer>

MainWindow::MainWindow(QWidget *parent)
    : QFrame(parent)
{
    /* GUI */
    this->setGeometry(0, 0, 480, 272);

    /* get data from fpga fifo */
    g_data = new G_PARA;

    //memset是一个c++函数，用于快速清零一块内存区域
    memset(g_data, 0, sizeof(G_PARA));


#ifdef ARM
    //开启新线程，持续监听FPGA的数据，
    fifodata = new FifoData(g_data);
#endif

    /* main menu */
    mainmenu = new MainMenu(this, g_data);



#ifdef ARM
    /* detect key */
    //开启键盘监测线程，持续监听键盘输入
    keydetect = new KeyDetect(this);


//    modbus = new Modbus(this,g_data);

    //注册两个自定义类型
    qRegisterMetaType<VectorList>("VectorList");
    qRegisterMetaType<MODE>("MODE");


    connect(keydetect, &KeyDetect::sendkey, mainmenu, &MainMenu::trans_key);    //trans key value
#endif
    connect(this, &MainWindow::sendkey, mainmenu, &MainMenu::trans_key);        //trans key value

    //录波信号
    connect(mainmenu,SIGNAL(startRecWv(int,int)),fifodata,SLOT(startRecWave(int,int)));
    connect(fifodata,SIGNAL(waveData(VectorList,MODE)),mainmenu,SLOT(showWaveData(VectorList,MODE)));

    //系统重启
    rebootTimer = new QTimer;
    setCloseTime(sqlcfg->get_para()->close_time);
    connect(rebootTimer,SIGNAL(timeout()),this,SLOT(system_reboot()));

    connect(mainmenu,SIGNAL(closeTimeChanged(int)),this,SLOT(setCloseTime(int)));

    //状态栏显示
    showTimer = new QTimer;
    showTimer->start(1000);
    connect(showTimer,SIGNAL(timeout()),this,SLOT(showTime()));

//    connect(showTimer,SIGNAL(timeout()),this,SLOT(printSc()));  //截屏

}

void MainWindow::showTime()
{
    int s = rebootTimer->remainingTime() / 1000;    //自动关机秒数
    if(rebootTimer->isActive() && s<50){
        mainmenu->showReminTime(rebootTimer->remainingTime() / 1000 , tr("自动关机"));
    }
    else{
        mainmenu->showMaxResetTime();   //借机显示最大值重置时间
    }
}

void MainWindow::printSc()
{
    QPixmap fullScreenPixmap = this->grab(this->rect());
//    bool flag = fullScreenPixmap.save(QString("./ScreenShots/%1.png").arg(QTime::currentTime().toString("-hh-mm-ss")),"PNG");
    bool flag = fullScreenPixmap.save(QString("./ScreenShots/ScreenShots-%1.png").arg(QTime::currentTime().toString("hh-mm-ss")),"PNG");
    if(flag)
        qDebug()<<"fullScreen saved!";
    else
        qDebug()<<"fullScreen failed!";
}

void MainWindow::system_reboot()
{
    system("reboot");
}

void MainWindow::setCloseTime(int m)
{
    if(m!=0){
        connect(keydetect,SIGNAL(sendkey(quint8)),rebootTimer,SLOT(start()));
        rebootTimer->setInterval(m*60 *1000);
        rebootTimer->start();   //永远开启
        qDebug()<<"reboot timer started!  interval is :"<<m*60<<"sec";
    }
    else{
        disconnect(keydetect,SIGNAL(sendkey(quint8)),rebootTimer,SLOT(start()));
        rebootTimer->stop();
        qDebug()<<"reboot timer stoped!";
    }
}


//主窗口的按键事件，程序中有实际用处
void MainWindow::keyPressEvent(QKeyEvent *event)
{
//    qDebug("key_val = %02x [FILE:%s LINE:%d]", event->key(), __FILE__, __LINE__);

    rebootTimer->start();   //重置关机计时器
    qDebug()<< "rebootTimer reset!\n";

    switch (event->key()) {
    case Qt::Key_Escape:
        emit sendkey(KEY_CANCEL);
        break;
    case Qt::Key_Return:
        emit sendkey(KEY_OK);
        break;
    case Qt::Key_Up:
        emit sendkey(KEY_UP);
        break;
    case Qt::Key_Down:
        emit sendkey(KEY_DOWN);
        break;
    case Qt::Key_Left:
        emit sendkey(KEY_LEFT);
        break;
    case Qt::Key_Right:
        emit sendkey(KEY_RIGHT);
        break;
    default:
        break;
    }
}
