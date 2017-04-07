#ifndef FILETOOLS_H
#define FILETOOLS_H

#include <QObject>

class FileTools : public QObject
{
    Q_OBJECT
public:
    explicit FileTools(QObject *parent = 0);

signals:

public slots:
    void saveWaveToFile(qint32 *wvData, int len, int mode);  //保存录波文件

private:
    int mode_fpga;

    qint32 *recWaveData; //录波数据

    int buf_size;

    QString getFilePath();     //返回保存波形文件的目录

    void saveCfgFile(QString str);     //生成配置文件

};

#endif // FILETOOLS_H
