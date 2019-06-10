#include "gpsinfo.h"
#include <QtDebug>
#include "Gui/Common/common.h"

#define TEMP_INIT -100      //温度初始值

GPSInfo::GPSInfo(QObject *parent) : QObject(parent)
{
    temperature = TEMP_INIT;
}

void GPSInfo::init(float temp, float humi, QList<QByteArray> list)
{
//    temperature = temp;
    add_temp(temp);
    humidity = humi + 20;
    if(humidity > 100){
        humidity = 100;
    }
    if(list.count() < 3){
        return;
    }
    if(list.at(2) == "A"){
        enable = true;
    }
    else{
        enable = false;
        return;
    }

    if(list.count() < 10){
        return;
    }

    latitude_val = list.at(3).toFloat()/100.0;
    latitude = list.at(4);
    longitude_val = list.at(5).toFloat()/100.0;
    longitude = list.at(6);

    speed = list.at(7).toFloat();
    direction = list.at(8).toFloat();

    datetime = QDateTime::fromString(list.at(9) + list.at(1),"ddMMyyhhmmss.zzz").addYears(100).addSecs(60*60*timezone());

//    qDebug()<<"GPS DateTime:"<<datetime;
    if(datetime.isValid()){
        Common::save_date_time(datetime);           //保存时间
    }
}

QString GPSInfo::gps_str()
{
    QString str;
    if(enable){
        if(latitude == "N"){
            str.append(tr("N%1°").arg(QString::number(latitude_val, 'f', 4)));
        }
        else{
            str.append(tr("S%1°").arg(QString::number(latitude_val, 'f', 4)));
        }

        str.append(" ");

        if(longitude == "E"){
            str.append(tr("E%1°").arg(QString::number(longitude_val, 'f', 4)));
        }
        else{
            str.append(tr("W%1°").arg(QString::number(longitude_val, 'f', 4)));
        }

        str.append(" ");
    }

    if(temperature != TEMP_INIT){
        str.append(tr("%1°C ").arg(QString::number(temperature, 'f', 1)));
    }
    str.append(tr("%1%").arg(QString::number(humidity, 'f', 1)));

    return str;
}

int GPSInfo::timezone()
{
#if 0
    int t_z = qRound(longitude_val/15);         //根据纬度计算出时区
    if(longitude == "W"){
        t_z = - t_z;
    }
    return t_z;
#else
    return sqlcfg->get_para()->time_zone;       //使用设置的时区
#endif
}

void GPSInfo::add_temp(float wendu)
{
    temp_list.append(wendu);
    if(temp_list.count() > 4){
        float del = temp_list.last() - temp_list.first();      //温度变化值
        QVector<float> temp_table;                          //定义温度表,通过查表确定温度修正值
        temp_table << 17 << 16 << 15 << 14 << 13            //0.1, 0.3, 0.5, 0.7, 0.9
                   << 12.5 << 12 << 11.5 << 11.25 << 11     //1.1, 1.3, 1.5, 1.7, 1.9
                   << 10.75 << 10.5;                        //2.1, 2.3
        int n = qRound( (del - 0.1) / 0.2 );        //四舍五入取整
        float modify;
        if(n < 0){
            modify = 18;
        }
        else if(n < temp_table.count()){
            modify = temp_table.at(n);
        }
        else{
            modify = 10;
        }

        switch (sqlcfg->get_para()->temp_compensation) {
        case temp_auto:     //自动
            if(sqlcfg->get_para()->sync_mode == sync_vac220_110){
                modify += 12;
            }
            else{
                modify -= 6;
            }
            break;
        case temp_uncharge: //未充电
            modify -= 6;
            break;
        case temp_charge:   //充电
            modify += 12;
            break;
        default:
            break;
        }


        wendu -= modify;                  //温度加入动态修正
        qDebug()<<temp_list;
        qDebug()<<"del:"<<del<<"\tTemp modify:"<<modify<<"\tTemp:"<<wendu;


        temp_list.removeFirst();

        temperature = wendu;
    }
}
