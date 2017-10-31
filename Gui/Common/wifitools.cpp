#include "wifitools.h"
#include "IO/Other/wifi.h"
#include <QtDebug>

WifiTools::WifiTools(int serial_fd, WifiConfig *wifi_config, WorkMode m)
{
    this->serial_fd = serial_fd;
    this->wifi_config = wifi_config;
    mode = m;
    //    this->setAutoDelete(false);
}

WifiTools::WifiTools(int serial_fd, WifiConfig *wifi_config, WifiTools::WorkMode m, QString name, QString password)
{
    this->serial_fd = serial_fd;
    this->wifi_config = wifi_config;
    mode = m;
    this->name = name;
    this->password = password;
}

WifiTools::~WifiTools()
{
//    delete wifi_config;
    qDebug()<<"~WifiTools()";
}

void WifiTools::run()
{
    if(wifi_config == NULL){
        wifi_config = new WifiConfig(serial_fd);
        qDebug()<<"wifi_config="<<wifi_config;
    }
    if(mode == Init){
        QStringList list = wifi_config->wifi_get_list();
        qDebug()<<"wifi_config="<<wifi_config;
        emit wifi_list(list,wifi_config);
    }
    else if(mode == Connect){
        wifi_config->wifi_connect(name,password);
    }
    else if(mode == Info){
        QString ip, netmask, gateway, name, mac;
        int indicator;
        wifi_config->wifi_infomation(ip,netmask, gateway, name, mac,indicator);
    }
    else if(mode == Hotpot){
        wifi_config->wifi_hotpot(name,password);
        emit wifi_hotpot_finished();
    }

}



















