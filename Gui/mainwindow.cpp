#include "mainwindow.h"

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

    modbus = new Modbus(g_data);


    connect(keydetect, &KeyDetect::sendkey, mainmenu, &MainMenu::trans_key);    //trans key value
#endif
    connect(this, &MainWindow::sendkey, mainmenu, &MainMenu::trans_key);        //trans key value

    //录波信号
    connect(mainmenu,SIGNAL(startRecWv(int)),fifodata,SLOT(startRecWave(int)));
    connect(fifodata,SIGNAL(waveData(quint32*,int,int)),mainmenu,SLOT(showWaveData(quint32*,int,int)));

    //系统重启
    rebootTimer = new QTimer;
    rebootTimer->setInterval(sqlcfg->get_para()->close_time *60 *1000 );
    rebootTimer->start();   //永远开启
    connect(rebootTimer,SIGNAL(timeout()),this,SLOT(system_reboot()));
    connect(keydetect,SIGNAL(sendkey(quint8)),rebootTimer,SLOT(start()));
    connect(mainmenu,SIGNAL(closeTimeChanged(int)),this,SLOT(setCloseTime(int)));

    //状态栏显示
    showTimer = new QTimer;
    showTimer->start(1000);
    connect(showTimer,SIGNAL(timeout()),this,SLOT(showTime()));

}

void MainWindow::showTime()
{
    int s = rebootTimer->remainingTime() / 1000;    //自动关机秒数
    if(s<50){
        mainmenu->showReminTime(rebootTimer->remainingTime() / 1000 , tr("自动关机"));
    }
    else{
        mainmenu->showMaxResetTime();   //借机显示最大值重置时间
    }
}

void MainWindow::system_reboot()
{
    system("reboot");
}

void MainWindow::setCloseTime(int m)
{

    rebootTimer->setInterval(m*60 *1000);
    rebootTimer->start();   //永远开启
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
