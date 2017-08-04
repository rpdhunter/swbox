#include "mainwindow.h"
#include <QBuffer>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QSplashScreen *sp, QWidget *parent)
    : QFrame(parent)
{
    /* GUI */
    this->setGeometry (0, 0, RESOLUTION_X, RESOLUTION_Y);

    /* get data from fpga fifo */
    g_data = new G_PARA;

	if (g_data != NULL) {
	    //memset是一个c++函数，用于快速清零一块内存区域
		memset(g_data, 0, sizeof(G_PARA));
	}
	else {
		exit (0);
	}

    sp->showMessage(tr("正在设置定值..."),Qt::AlignBottom|Qt::AlignLeft);
#ifdef ARM
    //开启新线程，持续监听FPGA的数据，
    fifodata = new FifoData(g_data);
#endif

    sp->showMessage(tr("正在初始化菜单..."),Qt::AlignBottom|Qt::AlignLeft);
    /* main menu */
    mainmenu = new MainMenu(sp, this, g_data);




#ifdef ARM
    /* detect key */
    sp->showMessage(tr("正在开启子线程..."),Qt::AlignBottom|Qt::AlignLeft);
    //开启键盘监测线程，持续监听键盘输入
    keydetect = new KeyDetect(this);


    modbus = new Modbus(this,g_data);

    //注册两个自定义类型
    qRegisterMetaType<VectorList>("VectorList");
    qRegisterMetaType<MODE>("MODE");


    connect(keydetect, &KeyDetect::sendkey, mainmenu, &MainMenu::trans_key);    //trans key value
#endif
    connect(this, &MainWindow::sendkey, mainmenu, &MainMenu::trans_key);        //trans key value

    //录波信号
    connect(mainmenu,SIGNAL(startRecWv(MODE,int)),fifodata,SLOT(startRecWave(MODE,int)));
    connect(fifodata,SIGNAL(waveData(VectorList,MODE)),mainmenu,SLOT(showWaveData(VectorList,MODE)));

    //系统重启
    rebootTimer = new QTimer;
    setCloseTime(sqlcfg->get_para()->close_time);
    connect(keydetect,SIGNAL(sendkey(quint8)),this,SLOT(resetTimerFromKey()));
    connect(mainmenu,SIGNAL(closeTimeChanged(int)),this,SLOT(setCloseTime(int)));
    connect(rebootTimer,SIGNAL(timeout()),this,SLOT(system_reboot()));

    //状态栏显示
    showTimer = new QTimer;
    showTimer->start(1000);
    connect(showTimer,SIGNAL(timeout()),this,SLOT(showTime()));

//    connect(showTimer,SIGNAL(timeout()),this,SLOT(printSc()));  //截屏

    //modbus相关
    connect(mainmenu,SIGNAL(tev_modbus_data(int,int)),modbus,SLOT(tev_modbus_data(int,int)));
    connect(mainmenu,SIGNAL(aa_modbus_data(int)),modbus,SLOT(aa_modbus_data(int)));
    connect(modbus,SIGNAL(closeTimeChanged(int)),this,SLOT(setCloseTime(int)));
    connect(mainmenu,SIGNAL(modbus_tev_offset_suggest(int,int)),modbus,SLOT(tev_modbus_suggest(int,int)) );
    connect(mainmenu,SIGNAL(modbus_aa_offset_suggest(int)),modbus,SLOT(aa_modbus_suggest(int)) );

    //声音播放
    connect(mainmenu,SIGNAL(play_voice(VectorList)),fifodata,SLOT(playVoiceData(VectorList)));
    connect(fifodata,SIGNAL(playVoiceProgress(int,int,bool)),mainmenu,SLOT(playVoiceProgress(int,int,bool)));
    connect(mainmenu,SIGNAL(stop_play_voice()),fifodata,SLOT(stop_play_voice()));

    //高频CT模式
    connect(mainmenu,SIGNAL(switch_rfct_mode(int)),fifodata,SLOT(switch_rfct_mode(int)));
}

void MainWindow::showTime()
{
    int s = rebootTimer->remainingTime() / 1000;    //自动关机秒数
    if(rebootTimer->isActive() && s<60){
        mainmenu->showReminTime(rebootTimer->remainingTime() / 1000 , tr("自动关机"));
    }
}

void MainWindow::printSc()
{
//    QPixmap fullScreenPixmap = this->grab(this->rect());          //老的截屏方式，只能截取指定Wdiget及其子类
    QPixmap fullScreenPixmap = QGuiApplication::primaryScreen()->grabWindow(0);     //新截屏方式更加完美
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
        rebootFlag = true;
        rebootTimer->setInterval(m*60 *1000);
        rebootTimer->start();   //永远开启
        qDebug()<<"reboot timer started!  interval is :"<<m*60<<"sec";
    }
    else{
        rebootFlag = false;
        rebootTimer->stop();
        qDebug()<<"reboot timer stoped!";
    }
}

void MainWindow::resetTimerFromKey()
{
    if(rebootFlag){
        rebootTimer->start();
    }
    else{
        rebootTimer->stop();
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
