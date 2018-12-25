#include "wifi.h"
#include <QtDebug>
#include <QDateTime>
#include <QThread>

Wifi::Wifi(QObject *parent) : QObject(parent)
{
}

//这个类全部依附于一个次线程，所以初始化不能在构造函数中执行，只能在次线程启动完毕之后再初始化
//这样做的好处是类的内部对象全在一个线程中，无需顾忌线程同步
void Wifi::wifi_init()
{
    process1 = new QProcess;        //显示列表

    process2 = new QProcess;        //尝试连接
    connect(process2, SIGNAL(readyRead()), this, SLOT(ap_connect_output()));


    process3 = new QProcess;        //状态监视
//    process3->setProcessChannelMode(QProcess::ForwardedChannels);        //这条语句可以将所有输出强制定向到主进程

    timer1 = new QTimer;
    timer1->setInterval(1000);
    connect(timer1, SIGNAL(timeout()), this, SLOT(ap_refresh_info()));

    timer2 = new QTimer;
    timer2->setSingleShot(true);
    timer2->setInterval(20000);     //10秒钟
    connect(process2, SIGNAL(started()), timer2, SLOT(start()));
    connect(timer2, SIGNAL(timeout()), this, SLOT(ap_connect_failed()));

}

//调用系统命令刷新AP列表
void Wifi::ap_fresh_list()
{
    system("ifconfig  wlan0 up");                           //打开wifi设备
    process1->start("iwlist  wlan0  scan");                 //扫描
    process1->waitForFinished();
    QByteArray output = process1->readAllStandardOutput();
    if(output.contains("Scan completed :")){                //扫描完成
        QString str_output = output.simplified();
        str_output = str_output.remove("\"");

        QStringList list = str_output.split("Cell ");
        ap_list.clear();
        foreach (QString a, list) {
            if(a.contains(" ESSID:")){
                ap_list.append(get_one_apinfo(a));
            }
        }
        emit ap_fresh_list_complete();                      //发送扫描完成信号，否则自然结束，无返回信号
    }
}

//执行连接指令，由于使用了脚本，输出并非一次性返回，而且可能存在脚本死锁，造成进程无法返回的情况
//于是使用了管道获取打印输出的方式，根据返回字符串判断运行结果
//(前述方法后来弃用，原因是使用管道会使得信号槽系统失效，具体原因不明，最后直接使用执行脚本，再定时监控的方法解决问题)
void Wifi::ap_connect(QString ssid, QString password)
{
    system("ifconfig  wlan0 up");
    QString str = QString("/root/wifi/wpa.sh %1 %2").arg(ssid).arg(password);
//    char result[10240]={0};
//    executeCMD(str.toLocal8Bit(), result);
    process2->start(str);               //扫描
    timer1->start();                     //开始监控
}

void Wifi::ap_disconnect()
{
    system("/root/wifi/close_wifi.sh");         //关闭网络
}

void Wifi::process_1s_deal()
{
    //    QByteArray output = process->readAllStandardOutput();



    QByteArray  output = process1->readAll();
    qDebug(output);
}

void Wifi::ap_create(QString name, QString key, QString gateway, QString mask)
{
    QString str = QString("/root/wifi/ap.sh %1 %2 %3 %4").arg(name).arg(key).arg(gateway).arg(mask);
    system(str.toLocal8Bit());
}

void Wifi::ap_connect_output()
{
    qDebug()<<"process2 finished!";
    QByteArray output = process2->readAll();
    qDebug(output);
    if(output.contains("Adding DNS server")){          //判断连接成功
        emit ap_connect_complete(true);
        ap_refresh_info();                          //刷新当前节点连接信息，以备显示使用
    }
    if(output.contains("key may be incorrect")){       //判断连接失败
        emit ap_connect_complete(false);
        system("/root/wifi/close_wifi.sh");         //关闭网络
    }
}

void Wifi::ap_connect_failed()
{
    //   qDebug()<<"ap_connect_failed";
    //   if(process2->state() == QProcess::Running){     //判定连接失败
    //       process2->kill();
    //       emit ap_connect_complete(false);
    //       system("/root/wifi/close_wifi.sh");         //关闭网络
    //   }
    process3->start("iproute show wlan0");
    process3->waitForFinished();
    QByteArray output = process3->readAllStandardOutput();
    QString str = output.simplified();
    QString gate = get_one_section("default via "," dev wlan0", str);         //得到当前连接的路由器地址

    qDebug()<<gate;
    qDebug()<<"222222222";
    if(gate.isEmpty()){                                                  //连接失败
        foreach (AP_INFO info, ap_list) {       //清空已有的连接信息
            if(info.is_connected){
                info.is_connected = false;
                info.ip.clear();
                info.gate.clear();
                info.mask.clear();
                info.pd_mac.clear();
            }
        }
        qDebug()<<"111111111";
        system("/root/wifi/close_wifi.sh");         //关闭网络
        emit ap_connect_complete(false);
        return ;
    }

    QString ping = "ping  -c 2 " +  gate;                     //连接成功
    qDebug()<<ping;
    system(ping.toLatin1().data());

    timer2->stop();

    emit ap_connect_complete(true);
}


//以下是私有功能函数
AP_INFO Wifi::get_one_apinfo(QString str)
{
    AP_INFO ap_info;
    ap_info.mac = get_one_section(" Address: "," ESSID:",str);
    ap_info.ssid = get_one_section(" ESSID:"," Protocol:",str);
    ap_info.proto = get_one_section(" Protocol:"," Mode:",str);
    ap_info.mode = get_one_section(" Mode:"," Frequency:",str);
//    ap_info.freq = get_one_section(" Frequency:"," Encryption key:",str);
    ap_info.freq = get_one_section(" Frequency:"," Encryption key:",str);
    if(ap_info.freq.contains("(")){         //去除括号后内容
        int ind = ap_info.freq.lastIndexOf("(");
        ap_info.freq = ap_info.freq.left(ind);
    }
    ap_info.has_key = get_one_section(" Encryption key:"," Bit Rates:",str) == QString("on") ? true : false;
    ap_info.bitrate = get_one_section(" Bit Rates:","/s",str).append("/s");
    if(ap_info.has_key){
        if(str.contains("WPA Version 1")){
            ap_info.safty.append("WPA");
        }
        if(str.contains("WPA2 Version 1")){
            ap_info.safty.append("/WPA2");
        }
        ap_info.safty.append(" PSK");
    }
    else{
        ap_info.safty = "None";
    }

    ap_info.quality = get_one_section(" Quality=","/100 ",str).toInt();
    ap_info.signal_level = get_one_section(" Signal level=","/100",str).toInt();
    ap_info.is_connected = false;

    //    qDebug()<<"ESSID:\t\t"<<ap_info.ssid;
    //    qDebug()<<"mac:\t\t"<<ap_info.mac;
    //    qDebug()<<"proto:\t\t"<<ap_info.proto;
    //    qDebug()<<"mode:\t\t"<<ap_info.mode;
    //    qDebug()<<"freq:\t\t"<<ap_info.freq;
    //    qDebug()<<"has_key:\t"<<ap_info.has_key;
    //    qDebug()<<"safty:\t\t"<<ap_info.safty;
    //    qDebug()<<"bitrate:\t"<<ap_info.bitrate;
    //    qDebug()<<"quality:\t"<<ap_info.quality;
    //    qDebug()<<"signal_level:\t"<<ap_info.signal_level;

    return ap_info;
}

QString Wifi::get_one_section(QString start, QString end, QString str)
{
    int index_f = str.indexOf(start);
    int index_l = str.indexOf(end,index_f );
    if(index_f != -1 && index_l != -1){
        index_f += start.length();
        return str.mid(index_f, index_l - index_f);
    }
    return QString();
}

void Wifi::executeCMD(const char *cmd, char *result)
{
    qDebug()<<QThread::currentThread();
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=NULL)
    {
        while(fgets(buf_ps, 1024, ptr)!=NULL)
        {
            //	       可以通过这行来获取shell命令行中的每一行的输出
//                       printf("%s", buf_ps);
            QString str = QString::fromLocal8Bit(buf_ps).simplified();
            if(str.contains("Adding DNS server")){          //判断连接成功
                emit ap_connect_complete(true);
                ap_refresh_info();                          //刷新当前节点连接信息，以备显示使用
            }
            if(str.contains("key may be incorrect")){       //判断连接失败
                emit ap_connect_complete(false);
                system("/root/wifi/close_wifi.sh");         //关闭网络
            }
            //To be 也许会有其他错误情况

            strcat(result, buf_ps);
            if(strlen(result)>1024)
                break;
        }
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }

    qDebug()<<QThread::currentThread();
}

//bool is_connected;  //是否已连接
//QString ip;         //ip地址
//QString gate;       //网关
//QString mask;       //掩码
//QString pd_mac;     //仪器mac地址

void Wifi::ap_refresh_info()
{
    qDebug()<<"\nap_refresh_info";
    process3->start("iproute show wlan0");
    process3->waitForFinished();
    QByteArray output = process3->readAllStandardOutput();
//    qDebug(output);
    QString str = output.simplified();
//    qDebug()<<str;

    QString gate = get_one_section("default via "," dev wlan0", str);         //得到当前连接的路由器地址


    if(gate.isEmpty()){     //无活动连接
        foreach (AP_INFO info, ap_list) {       //清空已有的连接信息
            if(info.is_connected){
                info.is_connected = false;
                info.ip.clear();
                info.gate.clear();
                info.mask.clear();
                info.pd_mac.clear();
            }
        }
       return;
    }

    process3->start("arp -a");
    process3->waitForFinished();
    output = process3->readAll();
    str = output.simplified();
    if(str.contains(gate)){   //已连接
        connect_ap_index = 0;
//        qDebug()<<"arp -a"<<str;
//        qDebug()<<"gate:"<<gate;
        QString mac = get_one_section(gate + ") at ", " [ether] on wlan0",str);      //查询路由器的mac地址
        mac = mac.toUpper();
        qDebug()<<"mac="<<mac;
        foreach (AP_INFO info, ap_list) {
            qDebug()<<"info.mac="<<info.mac;
            if(info.mac == mac){
                info.is_connected = true;
                info.gate = gate;
                qDebug()<<"connect_ap_index="<<connect_ap_index<<"\tgate="<<gate
                       <<"\tmac="<<mac;
                break;
            }
            connect_ap_index++;
        }
    }
    else{
        return;
    }

    process3->start("ifconfig wlan0");
    process3->waitForFinished();
    output = process3->readAll();
    qDebug(output);
    if(output.contains("wlan0")){   //已连接

        //            ap_list
    }
    else{

    }

}












