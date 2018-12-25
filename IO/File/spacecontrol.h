#ifndef SPACECONTROL_H
#define SPACECONTROL_H

#include <QObject>

class SpaceControl : public QObject
{
    Q_OBJECT
public:
    explicit SpaceControl(QObject *parent = nullptr);

//    static void total_control();                            //总体控制,开机检测,内存使用超过3G唤出
    static void file_bySize(QString path, quint64 size);          //按大小管理文件夹,整体大于给定size,最早的文件被删除,直到整体大小小于size
    static void dir_byNum(QString path, int num);               //按文件(文件夹)数量管理文件夹,文件数量大于num,最早的文件被删除,直到文件数量小于num
    static void removeOldFile_byNum(QString path, int num);       //删除num个老旧文件,越老越优先
    static void removeOldFile_smart();              //智能删除的老文件

    static quint64 dirFileSize(const QString &path);        //查看文件夹大小
    static QString size_to_string(double size);                //将文件大小转为适合的字符串输出,size使用byte作单位

    static void disk_info(qint64 &total, qint64 &used, qint64 &available, QString &persent);
signals:

public slots:
};

#endif // SPACECONTROL_H
