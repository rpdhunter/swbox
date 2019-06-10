#include "wifimanagement.h"
#include <QtDebug>

WifiManagement::WifiManagement(QObject *parent) : QObject(parent)
{
//    this->_wifi_mode = wifi_ap_camera;      //默认启动为标准AP模式
    this->_wifi_mode = wifi_notwork;

    qRegisterMetaType<QList<AP_INFO>>("QList<AP_INFO>");

    command = new WifiCommand;
    thread = new QThread;
    command->moveToThread(thread);
    thread->start();

    //刷新列表请求及回应
    connect(this, SIGNAL(s_ap_fresh_list()), command, SLOT(ap_fresh_list()));
    connect(command, SIGNAL(ap_fresh_list_complete(QList<AP_INFO>)), this, SLOT(deal_ap_fresh_list(QList<AP_INFO>)));

    //连接AP请求及回应
    connect(this, SIGNAL(s_ap_connect(QString,QString)), command, SLOT(ap_connect(QString,QString)));
    connect(command, SIGNAL(ap_connect_complete(bool)), this, SLOT(deal_ap_connect(bool)));

    //创建热点请求及回应
    connect(this, SIGNAL(s_ap_create(QString,QString,QString,QString)), command, SLOT(ap_create(QString,QString,QString,QString)));
    connect(command, SIGNAL(ap_create_complete(bool)), this, SLOT(deal_ap_create(bool)));

}

/****************************************************
 * 1.调用系统命令刷新AP列表,耗时约3秒
 * 2.如果原来就处于连接状态，则刷新列表前，应保存已连接信息
 * *************************************************/
void WifiManagement::ap_fresh_list()
{
    if(this->_wifi_mode == wifi_sta_custom && !_ap_list.isEmpty()){
        _current_ap = _ap_list.first();
    }
    emit s_ap_fresh_list();
}

/****************************************************
 * 1.接收命令获取的列表
 * 2.对比自有列表,如果原来有活动连接,则将活动连接对应的AP信息删除,
 *   原有连接更新信息后置顶;
 *   如果找不到,则当前连接模式不变,原连接置顶
 * 3.发送刷新完成信号
 * *************************************************/
void WifiManagement::deal_ap_fresh_list(QList<AP_INFO> list)
{
    if(this->_wifi_mode == wifi_sta_custom){
        for (int i = 0; i < list.count(); ++i) {
            if(this->_current_ap.mac == list.at(i).mac){      //mac地址相同,则认为是同一AP
                AP_INFO info= list.takeAt(i);
                _current_ap.quality = info.quality;             //更新当前连接的强度和质量
                _current_ap.signal_level = info.signal_level;
                break;
            }
        }
        list.prepend(_current_ap);              //如果刷新后,原连接丢失,则不改变连接状态(这一点待实验)
    }
    _ap_list = list;
    emit ap_fresh_list_complete();
}

/****************************************************
 * 1.调用系统命令连接指定ssid,耗时约11秒
 * 2.连接之前保存待连接ap
 * *************************************************/
void WifiManagement::ap_connect(QString ssid, QString password)
{
    foreach (AP_INFO ap, _ap_list) {
        if(ap.ssid == ssid){
            _current_ap = ap;
            _current_ap.password = password;
            break;
        }
    }
    qDebug()<<"connect ap:"<<ssid<<"\tpassword:"<<password;
    emit s_ap_connect(ssid, password);
}

/****************************************************
 * 1.连接命令已完成
 * 2.进一步向command查询连接信息,并更新本地列表
 *   如果连接成功，将成功的AP移到列表顶端
 * 3.处理先前的成功连接（如果有的话）
 * 4.发送连接完成信号
 * *************************************************/
void WifiManagement::deal_ap_connect(bool r)
{

    if(r){
        if(_ap_list.isEmpty()){                     //这种模式应用于直接连接，没有提前刷新列表
            AP_INFO ap;
            command->ap_fresh_info(ap);
            _ap_list.append(ap);
            emit ap_connect_complete(true);
            return;
        }
        else{
            if(_ap_list.first().is_connected){          //如果之前有活动连接，先要重置活动连接状态
                _ap_list[0].is_connected = false;
                _ap_list[0].ip.clear();
                _ap_list[0].gate.clear();
                _ap_list[0].mask.clear();
                _ap_list[0].pd_mac.clear();
            }
            for (int i = 0; i < _ap_list.count(); ++i) {
                if(_ap_list.at(i).ssid == _current_ap.ssid){    //判断出尝试连接的AP
                    command->ap_fresh_info(_ap_list[i]);     //刷新尝试连接的ap信息
                    if(_ap_list[i].is_connected){            //确认已经连接成功了
                        AP_INFO ap = _ap_list.takeAt(i);
                        _ap_list.prepend(ap);
                        this->_wifi_mode = wifi_sta_custom;        //改变状态
                        emit ap_connect_complete(true);                        
                        return;
                    }
                }
            }
        }
    }
    this->_wifi_mode = wifi_notwork;            //连接失败
    emit ap_connect_complete(false);
}

/****************************************************
 * 调用系统命令建立一个自定义AP,耗时约6秒
 * *************************************************/
void WifiManagement::ap_create(QString name, QString key, QString gateway, QString mask)
{
    emit s_ap_create(name, key, gateway, mask);
    this->_wifi_mode = wifi_ap_custom;
    this->_current_ap.ssid = name;
    this->_current_ap.password = key;
    this->_current_ap.gate = gateway;
    this->_current_ap.mask = mask;
}

/****************************************************
 * 调用系统命令建立供摄像头和同步器使用的AP,耗时约6秒
 * *************************************************/
void WifiManagement::ap_create_camera()
{
    command->ap_create("window100000", "zdit.com.cn", "192.168.150.1", "255.255.255.0");
    this->_wifi_mode = wifi_ap_camera;
    this->_current_ap.ssid = "window100000";
    this->_current_ap.password = "zdit.com.cn";
    this->_current_ap.gate = "192.168.150.1";
    this->_current_ap.mask = "255.255.255.0";
}

/****************************************************
 * 1.AP建立命令已完成
 * 2.进一步向command查询AP信息,并更新本地AP信息
 * 3.发送AP建立完成信号
 * *************************************************/
void WifiManagement::deal_ap_create(bool r)
{
    if(r){
        this->_wifi_mode = wifi_ap_custom;
    }
    else{
        this->_wifi_mode = wifi_notwork;
    }
    emit ap_create_complete(r);
}

/****************************************************
 * 调用系统命令断开连接,立刻完成
 * *************************************************/
void WifiManagement::ap_disconnect()
{
    command->ap_disconnect();
    this->_wifi_mode = wifi_notwork;
    command->ap_fresh_info(_ap_list[0]);     //刷新当前ap信息
}

/****************************************************
 * 返回当前wifi模式
 * *************************************************/
WIFI_MODE WifiManagement::wifi_mode() const
{
    return _wifi_mode;
}

/****************************************************
 * 返回当前ap列表
 * *************************************************/
QList<AP_INFO> WifiManagement::ap_list() const
{
    return _ap_list;
}

/****************************************************
 * 返回当前焦点ap
 * *************************************************/
AP_INFO WifiManagement::current_ap() const
{
    return _current_ap;
}
