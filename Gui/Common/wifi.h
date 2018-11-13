#ifndef WIFI_H
#define WIFI_H

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
};

class Wifi : public QObject
{
    Q_OBJECT
public:
    explicit Wifi(QObject *parent = nullptr);

    void fresh_ap_list();       //刷新AP列表
    QList<AP_INFO> ap_list;     //AP列表

    void ap_connect(QString ssid, QString password);        //连接AP


signals:
    void ap_list_complete();    //刷新AP列表结束
    void ap_connect_complete(bool);     //连接AP完成

public slots:
    void process_done(int, QProcess::ExitStatus status);
    void process_1s_deal();         //每秒钟处理输出

private:
    QProcess *process;
    AP_INFO get_one_apinfo(QString str);            //获得一个AP的具体信息
    QString get_one_section(QString start, QString end, QString str);    //找到start和end之间的字符串

    QTimer *timer;
    void executeCMD(const char *cmd, char *result);
};

#endif // WIFI_H
