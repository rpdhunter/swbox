#include "fft.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FFT_POINT_NUM_32		(32)
#define FFT_OUT_BUF_NUM_32		((FFT_POINT_NUM_32 / 2) + 1)
#define FFT_POINT_NUM_64		(64)
#define FFT_OUT_BUF_NUM_64		((FFT_POINT_NUM_64 / 2) + 1)
#define FFT_POINT_NUM_128		(128)
#define FFT_OUT_BUF_NUM_128		((FFT_POINT_NUM_128 / 2) + 1)
#define FFT_POINT_NUM_256		(256)
#define FFT_OUT_BUF_NUM_256		((FFT_POINT_NUM_256 / 2) + 1)
#define FFT_POINT_NUM_1024		(1024)
#define FFT_OUT_BUF_NUM_1024	((FFT_POINT_NUM_1024 / 2) + 1)
#define FFT_POINT_NUM_2048		(2048)
#define FFT_OUT_BUF_NUM_2048	((FFT_POINT_NUM_2048 / 2) + 1)

#define PROT_DC_MAG_FACTOR_32			(1.0f / 32)
#define PROT_AC_MAG_FACTOR_32			(1.0f / (32 >> 1))
#define PROT_DC_MAG_FACTOR_64			(1.0f / 64)
#define PROT_AC_MAG_FACTOR_64			(1.0f / (64 >> 1))
#define PROT_DC_MAG_FACTOR_128			(1.0f / 128)
#define PROT_AC_MAG_FACTOR_128			(1.0f / (128 >> 1))
#define PROT_DC_MAG_FACTOR_256			(1.0f / 256)
#define PROT_AC_MAG_FACTOR_256			(1.0f / (256 >> 1))
#define PROT_DC_MAG_FACTOR_1024			(1.0f / 1024)
#define PROT_AC_MAG_FACTOR_1024			(1.0f / (1024 >> 1))
#define PROT_DC_MAG_FACTOR_2048			(1.0f / 2048)
#define PROT_AC_MAG_FACTOR_2048			(1.0f / (2048 >> 1))

#include <QtDebug>

FFT::FFT()
{
    init_fft();
}

int FFT::init_fft()
{
    //2048点浮点数数fft初始化
    fft_cfg_2048 = ne10_fft_alloc_r2c_float32 (FFT_POINT_NUM_2048);
    if (fft_cfg_2048 == NULL) {
        printf ("======ERROR, FFT2048 alloc fails\n");
        return -1;
    }
    fft_out_2048 = (ne10_fft_cpx_float32_t *) NE10_MALLOC (FFT_OUT_BUF_NUM_2048 * sizeof (ne10_fft_cpx_float32_t));
    if (fft_out_2048 == NULL) {
        printf ("======ERROR, FFT2048 alloc fails\n");
        return -1;
    }

    //1024点浮点数数fft初始化
    fft_cfg_1024 = ne10_fft_alloc_r2c_float32 (FFT_POINT_NUM_1024);
    if (fft_cfg_1024 == NULL) {
        printf ("======ERROR, FFT1024 alloc fails\n");
        return -1;
    }
    fft_out_1024 = (ne10_fft_cpx_float32_t *) NE10_MALLOC (FFT_OUT_BUF_NUM_1024 * sizeof (ne10_fft_cpx_float32_t));
    if (fft_out_1024 == NULL) {
        printf ("======ERROR, FFT1024 alloc fails\n");
        return -1;
    }

    //128点浮点数数fft初始化
    fft_cfg_128 = ne10_fft_alloc_r2c_float32 (FFT_POINT_NUM_128);
    if (fft_cfg_128 == NULL) {
        printf ("======ERROR, FFT128 alloc fails\n");
        return -1;
    }
    fft_out_128 = (ne10_fft_cpx_float32_t *) NE10_MALLOC (FFT_OUT_BUF_NUM_128 * sizeof (ne10_fft_cpx_float32_t));
    if (fft_out_128 == NULL) {
        printf ("======ERROR, FFT128 alloc fails\n");
        return -1;
    }

    return 0;
}

QVector<qint32> FFT::fft2048(QVector<qint32> list)
{
//    qDebug()<<"fft2048";
    if(list.count() != 2048){
        qDebug()<<"fft2048 error! list.count = "<<list.count();
    }
    float32 ibuf[2048];
    int i;
    for (i = 0; i < 2048; ++i) {
        ibuf[i] = list.at(i);
    }

    ne10_float32_t real, imag;
    float32 temp;
    QVector<qint32> r;
    ne10_fft_r2c_1d_float32_neon (fft_out_2048, ibuf, fft_cfg_2048);

    for (i = 0; i < FFT_OUT_BUF_NUM_2048; i++) {
        real = fft_out_2048 [i].r;
        imag = fft_out_2048 [i].i;
        temp = sqrt ( real * real + imag * imag );

        if( i == 0 ){
            r.append( temp * PROT_DC_MAG_FACTOR_2048 );
        }
        else {
            r.append( temp * PROT_AC_MAG_FACTOR_2048);
        }
    }

    return r;
}

QVector<float> FFT::fft1024(QVector<float> list)
{
//    qDebug()<<"fft1024";
    if(list.count() != 1024){
        qDebug()<<"fft1024 error! list.count = "<<list.count();
    }

    float32 ibuf[1024];
    int i;
    for (i = 0; i < 1024; ++i) {
        ibuf[i] = list.at(i);
    }

    ne10_float32_t real, imag;
    float32 temp;
    QVector<float> r;
    ne10_fft_r2c_1d_float32_neon (fft_out_1024, ibuf, fft_cfg_1024);

    for (i = 0; i < FFT_OUT_BUF_NUM_1024; i++) {
        real = fft_out_1024 [i].r;
        imag = fft_out_1024 [i].i;
        temp = sqrt ( real * real + imag * imag );

        if( i == 0 ){
            r.append( temp * PROT_DC_MAG_FACTOR_1024 );
        }
        else {
            r.append( temp * PROT_AC_MAG_FACTOR_1024);
        }
    }

    return r;
}

QVector<qint32> FFT::fft128(QVector<qint32> list)
{
//    qDebug()<<"fft128";
    if(list.count() != 128){
        qDebug()<<"fft128 error! list.count = "<<list.count();
    }
    float32 ibuf[128];
    int i;
    for (i = 0; i < 128; ++i) {
        ibuf[i] = list.at(i);
    }

    ne10_float32_t real, imag;
    float32 temp;
    QVector<qint32> r;
    ne10_fft_r2c_1d_float32_neon (fft_out_128, ibuf, fft_cfg_128);

    for (i = 0; i < FFT_OUT_BUF_NUM_128; i++) {
        real = fft_out_128 [i].r;
        imag = fft_out_128 [i].i;
        temp = sqrt ( real * real + imag * imag );

        if( i == 0 ){
            r.append( temp * PROT_DC_MAG_FACTOR_128 );
        }
        else {
            r.append( temp * PROT_AC_MAG_FACTOR_128);
        }
    }

    return r;
}

#if 0
void FFT::rfft32_prot_calc(float32 ibuf[], float32 mbuf[], float32 *base_real, float32 *base_imag)
{
    int i;
    ne10_float32_t real, imag;
    double temp;

    ne10_fft_r2c_1d_float32_neon (rfft_out_32, ibuf, rfft_cfg_32);

    for (i = 0; i < FFT_OUT_BUF_NUM_32; i++) {
        real = rfft_out_32 [i].r;
        imag = rfft_out_32 [i].i;
        temp = real * real + imag * imag;
        mbuf [i] = sqrt (temp);
    }
    * base_real = rfft_out_32 [1].r;
    * base_imag = rfft_out_32 [1].i;
}


void FFT::rfft64_harm_calc(float32 ibuf[], float32 mbuf[])
{
    int i;
    ne10_float32_t real, imag;
    double temp;

    ne10_fft_r2c_1d_int32_neon (fft_out_64, ibuf, fft_cfg_64);

    for (i = 0; i < FFT_OUT_BUF_NUM_64; i++) {
        real = fft_out_64 [i].r;
        imag = fft_out_64 [i].i;
        temp = real * real + imag * imag;
        mbuf [i] = sqrt (temp);
    }
}

void FFT::rfft2048_harm_calc(int ibuf[], int mbuf[])
{
    int i;
    ne10_int32_t real, imag;
    int temp;

    ne10_fft_r2c_1d_int32_neon (fft_out_2048, ibuf, fft_cfg_2048, 0);

    for (i = 0; i < FFT_OUT_BUF_NUM_2048; i++) {
        real = fft_out_2048 [i].r;
        imag = fft_out_2048 [i].i;
        temp = real * real + imag * imag;
        mbuf [i] = sqrt (temp);
    }
}

#endif
