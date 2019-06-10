#ifndef WIFIPASSWORD_H
#define WIFIPASSWORD_H

#include <QObject>
#include <QMap>

class WifiPassword : public QObject
{
    Q_OBJECT
public:
    explicit WifiPassword(QObject *parent = nullptr);

    QString lookup_key(QString name);               //根据用户名寻找密码
    void add_new(QString name, QString password);   //增加新密码至密码本
    void del_key(QString name);                     //根据用户名删除密码

signals:

public slots:

private:
    QMap<QString, QString> map;

    void read();                 //从文件读取用户名密码
    void save();                 //保存用户名密码至文件

};

#endif // WIFIPASSWORD_H
