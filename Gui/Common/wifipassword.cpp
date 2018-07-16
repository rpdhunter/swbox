#include "wifipassword.h"
//#include <QMapIterator>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

WifiPassword::WifiPassword(QObject *parent) : QObject(parent)
{
    read();
}

QString WifiPassword::lookup_key(QString name)
{
    return map.value(name);
}

void WifiPassword::add_new(QString name, QString password)
{
    if(!map.contains(name)){
        map.insert(name, password);
        save();
    }
}

void WifiPassword::del_key(QString name)
{
    if(!name.isEmpty()){
        map.remove(name);
        save();
    }
}

void WifiPassword::read()
{
    QFile file("/root/wifi/keys.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList list = line.split(",");
        if(list.count() == 2){
            if(!map.contains(list.first())){
                map.insert(list.first(),list.last());
            }
        }
        else{
            qDebug()<<"read wifi password error!";
        }
    }
}

void WifiPassword::save()
{
    QFile file("/root/wifi/keys.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    QMapIterator<QString, QString> i(map);
    while (i.hasNext()) {
        i.next();
        out << i.key() << "," << i.value() <<"\n";
    }

}
