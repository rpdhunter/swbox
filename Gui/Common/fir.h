#ifndef FIR_H
#define FIR_H

#include <QObject>
#include <QVector>
#include "IO/SqlCfg/sqlcfg.h"

class Fir : public QObject
{
    Q_OBJECT
public:
    explicit Fir(QObject *parent = nullptr);

    QVector<int> set_filter(QVector<int> wave, FILTER l);

signals:

public slots:

private:
//    typedef float (* p_lfp) (float, float, int);
//    p_lfp dlfp_w = DigFil_hp_500k;
    QVector<float> set_filter_origin(QVector<float> wave, FILTER l);
    QVector<int> set_filter_origin(QVector<int> wave, FILTER l);


    float DigFil_hp_500k(float invar, float initval, int setic);
    float DigFil_hp_1M(float invar, float initval, int setic);
    float DigFil_hp_1M5(float invar, float initval, int setic);
    float DigFil_hp_1M8(float invar, float initval, int setic);
    float DigFil_hp_2M(float invar, float initval, int setic);
    float DigFil_hp_2M5(float invar, float initval, int setic);
    float DigFil_hp_3M(float invar, float initval, int setic);
    float DigFil_hp_5M(float invar, float initval, int setic);
    float DigFil_hp_8M(float invar, float initval, int setic);
    float DigFil_hp_10M(float invar, float initval, int setic);
    float DigFil_hp_12M(float invar, float initval, int setic);
    float DigFil_hp_15M(float invar, float initval, int setic);
    float DigFil_hp_18M(float invar, float initval, int setic);
    float DigFil_hp_20M(float invar, float initval, int setic);
    float DigFil_hp_22M(float invar, float initval, int setic);
    float DigFil_hp_25M(float invar, float initval, int setic);
    float DigFil_hp_28M(float invar, float initval, int setic);
    float DigFil_hp_30M(float invar, float initval, int setic);
    float DigFil_hp_32M(float invar, float initval, int setic);
    float DigFil_hp_35M(float invar, float initval, int setic);

    float DigFil_lp_2M(float invar, float initval, int setic);
    float DigFil_lp_5M(float invar, float initval, int setic);
    float DigFil_lp_8M(float invar, float initval, int setic);
    float DigFil_lp_10M(float invar, float initval, int setic);
    float DigFil_lp_12M(float invar, float initval, int setic);
    float DigFil_lp_15M(float invar, float initval, int setic);
    float DigFil_lp_18M(float invar, float initval, int setic);
    float DigFil_lp_20M(float invar, float initval, int setic);
    float DigFil_lp_22M(float invar, float initval, int setic);
    float DigFil_lp_25M(float invar, float initval, int setic);
    float DigFil_lp_28M(float invar, float initval, int setic);
    float DigFil_lp_30M(float invar, float initval, int setic);
    float DigFil_lp_32M(float invar, float initval, int setic);
    float DigFil_lp_35M(float invar, float initval, int setic);
    float DigFil_lp_38M(float invar, float initval, int setic);
    float DigFil_lp_40M(float invar, float initval, int setic);

};

#endif // FIR_H
