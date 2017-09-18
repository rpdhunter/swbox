#ifndef WIFITOOLS_H
#define WIFITOOLS_H

#include <QObject>
#include <QRunnable>
#include "IO/Other/wifi.h"

class WifiTools : public QObject, public QRunnable
{
    Q_OBJECT
public:
    WifiTools(WifiConfig *wifi_config);
    ~WifiTools();

    void run();

signals:
    void wifi_list(QStringList);

private:
    WifiConfig *wifi_config;

};

#endif // WIFITOOLS_H
