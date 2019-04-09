#include "gpsinfo.h"

GPSInfo::GPSInfo(float temp, float humi, QList<QByteArray> list, QObject *parent) : QObject(parent)
{
    temperature = temp;
    humidity = humi;
    if(list.at(2) == "A"){
        enable = true;
    }
    else{
        enable = false;
    }

    latitude_val = list.at(3).toFloat()/100.0;
    latitude = list.at(4);
    longitude_val = list.at(5).toFloat()/100.0;
    longitude = list.at(6);

    speed = list.at(7).toFloat();
    direction = list.at(8).toFloat();

    datetime = QDateTime::fromString(list.at(9) + list.at(1),"ddMMyyhhmmss.zzz").addYears(100).addSecs(60*60*timezone());

            //0"\x00\x00\x00\x00$GNRMC", 1"021538.000", 2"A", 3"3157.2718", 4"N", 5"11846.4420", 6"E", 7"0.72", 8"0.00", 9"080419", "", "", "A*78\r\n\x14\xC2\xBF"
}

int GPSInfo::timezone()
{
    int t_z = qRound(longitude_val/15);
    if(longitude == "W"){
        t_z = - t_z;
    }
    return t_z;
}
