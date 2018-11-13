#include "wifi.h"
#include <QtDebug>

Wifi::Wifi(QObject *parent) : QObject(parent)
{
    process = new QProcess;
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(process_done(int,QProcess::ExitStatus)));

    timer = new QTimer;
    timer->setInterval(1000);
    connect(process, SIGNAL(started()), timer, SLOT(start()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), timer, SLOT(stop()));
    connect(timer, SIGNAL(timeout()), this, SLOT(process_1s_deal()));

    //    connect(process, SIGNAL(), this, SLOT());
}

void Wifi::fresh_ap_list()
{

    //    process->start("iwlist  wlan0  scan | grep -E 'Address:|ESSID:|Protocol:|Mode:|Frequency:|Encryption|Rates:|Quality='");
    system("ifconfig  wlan0 up");
    process->start("iwlist  wlan0  scan");
}

void Wifi::ap_connect(QString ssid, QString password)
{
    system("ifconfig  wlan0 up");
    QString str = QString("/root/wifi/wpa.sh %1 %2").arg(ssid).arg(password);
    qDebug()<<str;
    //    system(str.toLocal8Bit());

    //    process->start(str.toLocal8Bit());
    char result[10240]={0};
    executeCMD(str.toLocal8Bit(), result);
}

void Wifi::process_done(int,QProcess::ExitStatus status)
{
    qDebug()<<"process_done:";

    if(status == QProcess::NormalExit ){            //正常退出
        QByteArray output = process->readAllStandardOutput();

        qDebug(output);
        if(output.contains("Scan completed :")){        //扫描完成
            QString str_output = output.simplified();
            str_output = str_output.remove("\"");

            QStringList list = str_output.split("Cell ");
            ap_list.clear();
            foreach (QString a, list) {
                if(a.contains(" ESSID:")){
                    ap_list.append(get_one_apinfo(a));
                }
            }
            emit ap_list_complete();
        }
        else if(output.contains("Setting IP address")){     //连接完成
            emit ap_connect_complete(true);
        }



    }
    else{
        QByteArray output = process->readAllStandardError();
        qDebug(output);
    }

}

void Wifi::process_1s_deal()
{
    //    QByteArray output = process->readAllStandardOutput();



    QByteArray  output = process->readAll();
    qDebug(output);
}

AP_INFO Wifi::get_one_apinfo(QString str)
{
    AP_INFO ap_info;
    ap_info.mac = get_one_section(" Address: "," ESSID:",str);
    ap_info.ssid = get_one_section(" ESSID:"," Protocol:",str);
    ap_info.proto = get_one_section(" Protocol:"," Mode:",str);
    ap_info.mode = get_one_section(" Mode:"," Frequency:",str);
    ap_info.freq = get_one_section(" Frequency:"," Encryption key:",str);
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
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=NULL)
    {
        while(fgets(buf_ps, 1024, ptr)!=NULL)
        {
            //	       可以通过这行来获取shell命令行中的每一行的输出
            //           printf("%s", buf_ps);
            //           qDebug()<<"qDebug()<<"<<QString::fromLocal8Bit(buf_ps).simplified();
            QString str = QString::fromLocal8Bit(buf_ps).simplified();
            if(str.contains("Adding DNS server")){      //初步判断连接成功
                emit ap_connect_complete(true);
            }
            if(str.contains("key may be incorrect")){      //初步判断连接失败
                emit ap_connect_complete(false);
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
}










