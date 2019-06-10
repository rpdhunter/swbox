#ifndef GPSINFO_H
#define GPSINFO_H

#include <QObject>
#include <QDateTime>

class GPSInfo : public QObject
{
    Q_OBJECT
public:
    explicit GPSInfo(QObject *parent = nullptr);

    void init(float temp, float humi, QList<QByteArray> list);

    QString gps_str();


    float temperature;      //温度
    float humidity;         //湿度

    bool enable;            //有效性
    QDateTime datetime;     //时间日期
    QString latitude;       //纬度,"N"北,"S"南
    double latitude_val;    //纬度数值
    QString longitude;      //经度,"W"西,"E"东
    double longitude_val;   //经度数值
    float speed;            //航速(0-999.9节)
    float direction;        //航向(0-359.9°)

    int timezone();         //计算得到时区

signals:

public slots:

private:
    QList<float> temp_list;             //记录收到的温度
    void add_temp(float wendu);         //温度补偿
};

#endif // GPSINFO_H
