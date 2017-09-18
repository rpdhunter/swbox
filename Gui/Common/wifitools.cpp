#include "wifitools.h"
#include "IO/Other/wifi.h"
#include <QtDebug>

WifiTools::WifiTools(WifiConfig *wifi_config)
{
    this->wifi_config = wifi_config;
}

WifiTools::~WifiTools()
{
    delete wifi_config;
    qDebug()<<"~WifiTools()";
}

void WifiTools::run()
{
    wifi_config = new WifiConfig();
//    wifi_config->wifi_client();
    QStringList list = wifi_config->wifi_get_list();
    emit wifi_list(list);
}
