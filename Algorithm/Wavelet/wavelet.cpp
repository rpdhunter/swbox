﻿#include "wavelet.h"
#include <vectormath.h>
#include <statistics.h>
#include <dwt.h>
#include <QtDebug>

using namespace std;
using namespace splab;

typedef float   Type;

Wavelet::Wavelet()
{
}

QVector<int> Wavelet::set_filter(QVector<int> sig, uint level, QString name)
{
    if(name != "db4"){
        qDebug()<<"the wavelet type is not db4!";
        return sig;
    }

    /******************************* [ signal ] ******************************/
    Vector<Type> s;

    s.resize(sig.count());
    for(int i=0; i<sig.count(); i++)
    {
        s[i] = sig.at(i);
    }

    /******************************** [ DWT ] ********************************/
    DWT<Type> discreteWT("db4");
    Vector<Type> coefs = discreteWT.dwt( s, level );

    Vector<Type> details;
    for (uint i = 1; i <= level; ++i) {
        details = discreteWT.getDetial(coefs,i);
        for (int j = 0; j < details.size(); ++j) {
            details[j] = 0;
        }
        discreteWT.setDetial (details,coefs,i);
    }


    /******************************** [ IDWT ] *******************************/
    level = 0 ;
    Vector<Type> x = discreteWT.idwt( coefs, level );
//    qDebug() << "The relative error is : norm(s-x) / norm(s) = "
//         << norm(s-x)/norm(s) << endl;

    QVector<int> result;
    for(int i=0; i<sig.count(); i++)
    {
        result.append(x[i]);
    }

    return result;
}









