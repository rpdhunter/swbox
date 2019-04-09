#ifndef GPSINFO_H
#define GPSINFO_H

#include <QObject>
#include <QDateTime>

class GPSInfo : public QObject
{
    Q_OBJECT
public:
    explicit GPSInfo(float temp, float humi, QList<QByteArray> list, QObject *parent = nullptr);


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
};

#endif // GPSINFO_H
