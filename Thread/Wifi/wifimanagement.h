#ifndef WIFIMANAGEMENT_H
#define WIFIMANAGEMENT_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QThread>
#include "wificommand.h"
#include "IO/Data/data.h"


class WifiManagement : public QObject
{
    Q_OBJECT
public:
    explicit WifiManagement(QObject *parent = nullptr);

    QList<AP_INFO> ap_list() const;     //返回当前AP列表
    WIFI_MODE wifi_mode() const;        //返回当前wifi工作模式
    AP_INFO current_ap() const;         //返回当前操作ap

    void ap_fresh_list();               //刷新AP列表
    void ap_connect(QString ssid, QString password);    //连接某一AP(信号ap_connect_complete(bool)用于返回结果)
    void ap_create(QString name, QString key, QString gateway = "192.168.150.1", QString mask = "255.255.255.0");       //创建自定义AP

    void ap_create_camera();            //按标准camera模式建立一个AP
    void ap_disconnect();               //断开当前连接（无返回结果）

signals:
    void s_ap_fresh_list();                             //刷新AP列表
    void ap_fresh_list_complete();                      //AP列表刷新完成,可通过ap_list()查看新的ap列表
    void s_ap_connect(QString, QString);                //AP连接
    void ap_connect_complete(bool);                     //连接AP完成,可通过ap_list()查看新的ap列表
    void s_ap_create(QString,QString,QString,QString);  //创建热点
    void ap_create_complete(bool);                      //建立热点完成

private slots:
    void deal_ap_fresh_list(QList<AP_INFO> list);       //AP刷新命令执行完成,做相应处理
    void deal_ap_connect(bool r);                       //AP连接命令执行完成,做相应处理
    void deal_ap_create(bool r);                        //AP创建命令执行完毕,做相应处理

private:
    WifiCommand *command;           //wifi控制命令
    QThread *thread;                //wifi控制命令运行的线程
    QList<AP_INFO> _ap_list;         //AP列表
    AP_INFO _current_ap;               //若当前状态是AP,_ap_info保存了AP的信息
    WIFI_MODE _wifi_mode;            //wifi工作模式
};

#endif // WIFIMANAGEMENT_H
