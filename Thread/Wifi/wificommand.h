#ifndef WIFICOMMAND_H
#define WIFICOMMAND_H

#include <QObject>
#include <QProcess>
#include <QTimer>

struct AP_INFO{
    QString ssid;       //网络名(ESSID)
    QString mac;        //mac地址(Address)
    QString proto;      //协议(Protocol)
    QString mode;       //模式(Mode)
    QString freq;       //频率(Frequency)
    QString bitrate;    //比特率(Bit Rates)
    int quality;        //品质(Quality)
    int signal_level;   //信号强度(Signal level)
    bool has_key;       //是否加密(Encryption key)
    QString safty;      //加密协议(IE: WPA Version 1 / IE: IEEE 802.11i/WPA2 Version 1)

    bool is_connected;  //是否已连接
    QString ip;         //ip地址
    QString gate;       //网关
    QString mask;       //掩码
    QString pd_mac;     //仪器mac地址

    QString password;   //密码
};

class WifiCommand : public QObject
{
    Q_OBJECT
public:
    explicit WifiCommand(QObject *parent = nullptr);

signals:
    void ap_fresh_list_complete(QList<AP_INFO>);    //发送AP列表
    void ap_connect_complete(bool);                 //连接AP完成,返回结果
    void ap_create_complete(bool);                  //创建AP完成，返回结果

public slots:
    void ap_disconnect();               //断开当前连接（无返回结果）
    void ap_fresh_info(AP_INFO &ap);     //刷新当前连接的信息

    void ap_fresh_list();           //刷新AP列表
    void ap_connect(QString ssid, QString password);    //连接某一AP(信号ap_connect_complete(bool)用于返回结果)
    void ap_create(QString name, QString key, QString gateway = "192.168.150.1", QString mask = "255.255.255.0");       //创建自定义AP


private:    
    void ap_connect_check();            //检查连接是否成功
    void ap_create_check(QString gateway);             //检查创建是否成功
    AP_INFO get_one_apinfo(QString str);                                //获得一个AP的具体信息
    QString get_one_section(QString start, QString end, QString str);   //找到start和end之间的字符串


};

#endif // WIFICOMMAND_H
