#ifndef WAVELET_H
#define WAVELET_H
#include <QVector>



class Wavelet
{
public:
    Wavelet();
    static QVector<int> set_filter(QVector<int> sig, uint level, QString name = "db4");      //sig是信号,level是离散小波变换层数,name是使用小波种类

private:


};

#endif // WAVELET_H
