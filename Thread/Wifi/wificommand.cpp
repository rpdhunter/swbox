#include "wificommand.h"
#include <QtDebug>

WifiCommand::WifiCommand(QObject *parent) : QObject(parent)
{
}

/*****************************************************************************
 * 刷新AP列表
 * 不需要检测是否刷新成功
 * **************************************************************************/
void WifiCommand::ap_fresh_list()
{
    system("ifconfig  wlan0 up");                         //打开wifi设备
    QProcess process;
    process.start("iwlist  wlan0  scan");                 //扫描
    process.waitForFinished(10 * 1000);                   //阻塞线程,直到运行结束或者超过10秒
    QByteArray output = process.readAllStandardOutput();
    if(output.contains("Scan completed :")){                //扫描完成
        QString str_output = output.simplified();
        str_output = str_output.remove("\"");

        QStringList list = str_output.split("Cell ");
        QList<AP_INFO> ap_list;
        foreach (QString a, list) {
            if(a.contains(" ESSID:")){
                ap_list.append(get_one_apinfo(a));
            }
        }
        emit ap_fresh_list_complete(ap_list);                      //发送扫描完成信号，否则自然结束，无返回信号
    }
}

/*****************************************************************************
 * 执行连接指令
 * 曾将尝试使用1.QProcess同步执行 2.管道 3.系统system调用，再定时监控的 4.QProcess同步执行
 * 1进程可能永远不能执行完毕 2会造成信号槽失效 3会浪费时间，效率不高，程序也复杂
 * 最终选用第4种方法，比较完美地解决了问题
 * **************************************************************************/
void WifiCommand::ap_connect(QString ssid, QString password)
{
    system("ifconfig  wlan0 up");               //打开wifi设备
    QString str = QString("/root/wifi/wpa.sh %1 %2 ").arg(ssid).arg(password);
    QProcess process;
    process.start(str);
    process.waitForFinished(15000);
//    QByteArray array = process.readAllStandardOutput();
//    qDebug(array);
    ap_connect_check();
}

/***************************************************
 * 验证AP是否连接成功
 * ************************************************/
void WifiCommand::ap_connect_check()
{
    AP_INFO ap;
    ap_fresh_info(ap);      //检测当前ap是否连接

    if(ap.is_connected){
        qDebug()<<"ap_connect_check: true";
        emit ap_connect_complete(true);
    }
    else{
        qDebug()<<"ap_connect_check: false";
        emit ap_connect_complete(false);
    }
}

/*****************************************************************************
 * 建立一个AP
 * **************************************************************************/
void WifiCommand::ap_create(QString name, QString key, QString gateway, QString mask)
{
    system("ifconfig  wlan0 up");               //打开wifi设备
    QString str = QString("/root/wifi/ap.sh %1 %2 %3 %4").arg(name).arg(key).arg(gateway).arg(mask);
    QProcess process;
    process.start(str);
    process.waitForFinished(15000);
    QByteArray array = process.readAllStandardOutput();
    qDebug(array);
    system(str.toLocal8Bit());
    ap_create_check(gateway);
}

/***************************************************
 * 验证AP是否创建成功
 * 依据是ifconfig查询到的IP是否等于创建时给定的IP
 * ************************************************/
void WifiCommand::ap_create_check(QString gateway)
{
    QProcess process;
    process.start("ifconfig wlan0");
    process.waitForFinished();
    QByteArray output = process.readAll();

    QString str = output.simplified();
    qDebug()<<str;
    QString ip = get_one_section(" inet addr:"," Bcast:", str);
    if(ip == gateway){
        emit ap_create_complete(true);
    }
    else{
        emit ap_create_complete(false);
    }
}

/***************************************************
 * 关闭网络
 * ************************************************/
void WifiCommand::ap_disconnect()
{
    system("/root/wifi/close_wifi.sh");
}

/***************************************************
 * 刷新当前ap信息(输入值必须为已经确定连接的ap)
 * ************************************************/
void WifiCommand::ap_fresh_info(AP_INFO &ap)
{
    qDebug()<<"\nap_fresh_info";
    QProcess process;
    process.start("iproute show wlan0");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str = output.simplified();
    QString gate = get_one_section("default via "," dev wlan0", str);         //得到当前连接的路由器地址
    if(gate.isEmpty()){     //无活动连接
        ap.is_connected = false;
        ap.ip.clear();
        ap.gate.clear();
        ap.mask.clear();
        ap.pd_mac.clear();
       return;
    }
    else{
        process.start("ifconfig wlan0");
        process.waitForFinished();
        output = process.readAll();

        str = output.simplified();
        qDebug()<<str;
        QString mac = get_one_section("HWaddr "," inet addr:", str);
        QString ip = get_one_section(" inet addr:"," Bcast:", str);
        QString mask = get_one_section(" Mask:"," inet6 addr:", str);
        ap.is_connected = true;
        ap.ip = ip;
        ap.gate = gate;
        ap.mask = mask;
        ap.pd_mac = mac;
    }
}

/***************************************************
 * 从一段字符串中提炼出一个AP的信息
 * ************************************************/
AP_INFO WifiCommand::get_one_apinfo(QString str)
{
    AP_INFO ap_info;
    ap_info.mac = get_one_section(" Address: "," ESSID:",str);
    ap_info.ssid = get_one_section(" ESSID:"," Protocol:",str);
    ap_info.proto = get_one_section(" Protocol:"," Mode:",str);
    ap_info.mode = get_one_section(" Mode:"," Frequency:",str);
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
            if(ap_info.safty.isEmpty()){
                ap_info.safty.append("WPA2");
            }
            else{
                ap_info.safty.append("/WPA2");
            }
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

/***************************************************
 * 提炼出给定首尾字符串之间的字段,并返回
 * ************************************************/
QString WifiCommand::get_one_section(QString start, QString end, QString str)
{
    int index_f = str.indexOf(start);
    int index_l = str.indexOf(end,index_f );
    if(index_f != -1 && index_l != -1){
        index_f += start.length();
        return str.mid(index_f, index_l - index_f);
    }
    return QString();
}



