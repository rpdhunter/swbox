#ifndef WIFITOOLS_H
#define WIFITOOLS_H

#include <QObject>
#include <QRunnable>
#include "IO/Other/wifi.h"

class WifiTools : public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum WorkMode{
        Init,
        Connect,
        Hotpot,
        Info
    };

    WifiTools(WifiConfig *wifi_config, WorkMode m);
    WifiTools(WifiConfig *wifi_config, WorkMode m, QString name, QString password);
    ~WifiTools();

    void run();

signals:
    void wifi_list(QStringList, WifiConfig*);
    void wifi_hotpot_finished();        //热点已成功

private:
    WifiConfig *wifi_config;
    WorkMode mode;
    QString name, password;

};

#endif // WIFITOOLS_H
