#include "fft.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FFT_POINT_NUM_32		(32)
#define FFT_OUT_BUF_NUM_32		((FFT_POINT_NUM_32 / 2) + 1)
#define FFT_POINT_NUM_64		(64)
#define FFT_OUT_BUF_NUM_64		((FFT_POINT_NUM_64 / 2) + 1)
#define FFT_POINT_NUM_2048		(2048)
#define FFT_OUT_BUF_NUM_2048	((FFT_POINT_NUM_2048 / 2) + 1)

#define PROT_DC_MAG_FACTOR			(1.0f / 2048)
//#define PROT_AC_MAG_FACTOR			(1.0f / ((2048 >> 1) * 1.414f /* 调整为有效值 */))
#define PROT_AC_MAG_FACTOR			(1.0f / (2048 >> 1))


FFT::FFT()
{
    init_fft();
}

QVector<qint32> FFT::fft2048(int ibuf[])
{
    int i;
    ne10_int32_t real, imag;
    int temp;
    QVector<qint32> r;

    ne10_fft_r2c_1d_int32_neon (fft_out_2048, ibuf, fft_cfg_2048, 0);

    for (i = 0; i < FFT_OUT_BUF_NUM_2048; i++) {
        real = fft_out_2048 [i].r;
        imag = fft_out_2048 [i].i;
        temp = sqrt ( real * real + imag * imag );

        if( i == 0 ){
            r.append( temp * PROT_DC_MAG_FACTOR );
        }
        else {
            r.append( temp * PROT_AC_MAG_FACTOR);
        }
    }

    return r;
}

QVector<qint32> FFT::fft64(int ibuf[])
{
    int i;
    ne10_int32_t real, imag;
    int temp;
    QVector<qint32> r;

    ne10_fft_r2c_1d_int32_neon (fft_out_64, ibuf, fft_cfg_64, 0);

    for (i = 0; i < FFT_OUT_BUF_NUM_64; i++) {
        real = fft_out_64 [i].r;
        imag = fft_out_64 [i].i;
        temp = sqrt ( real * real + imag * imag );

        if( i == 0 ){
            r.append( temp * PROT_DC_MAG_FACTOR );
        }
        else {
            r.append( temp * PROT_AC_MAG_FACTOR);
        }
    }

    return r;
}

int FFT::init_fft()
{
#if 0
    fft_cfg_32 = ne10_fft_alloc_r2c_float32 (FFT_POINT_NUM_32);
    if (fft_cfg_32 == NULL) {
        printf ("======ERROR, FFT alloc fails\n");
        return -1;
    }
    fft_out_32 = (ne10_fft_cpx_float32_t *) NE10_MALLOC (FFT_OUT_BUF_NUM_32 * sizeof (ne10_fft_cpx_float32_t));
    if (fft_out_32 == NULL) {
        printf ("======ERROR, FFT alloc fails\n");
        return -1;
    }
#endif

    fft_cfg_64 = ne10_fft_alloc_r2c_int32 (FFT_POINT_NUM_64);
    if (fft_cfg_64 == NULL) {
        printf ("======ERROR, FFT alloc fails\n");
        return -1;
    }
    fft_out_64 = (ne10_fft_cpx_int32_t *) NE10_MALLOC (FFT_OUT_BUF_NUM_64 * sizeof (ne10_fft_cpx_int32_t));
    if (fft_out_64 == NULL) {
        printf ("======ERROR, FFT alloc fails\n");
        return -1;
    }


    fft_cfg_2048 = ne10_fft_alloc_r2c_int32 (FFT_POINT_NUM_2048);
    if (fft_cfg_2048 == NULL) {
        printf ("======ERROR, FFT alloc fails\n");
        return -1;
    }
    fft_out_2048 = (ne10_fft_cpx_int32_t *) NE10_MALLOC (FFT_OUT_BUF_NUM_2048 * sizeof (ne10_fft_cpx_int32_t));
    if (fft_out_2048 == NULL) {
        printf ("======ERROR, FFT alloc fails\n");
        return -1;
    }

    return 0;
}

void FFT::rfft32_prot_calc(float32 ibuf[], float32 mbuf[], float32 *base_real, float32 *base_imag)
{
    int i;
    ne10_float32_t real, imag;
    double temp;

    ne10_fft_r2c_1d_float32_neon (fft_out_32, ibuf, fft_cfg_32);

    for (i = 0; i < FFT_OUT_BUF_NUM_32; i++) {
        real = fft_out_32 [i].r;
        imag = fft_out_32 [i].i;
        temp = real * real + imag * imag;
        mbuf [i] = sqrt (temp);
    }
    * base_real = fft_out_32 [1].r;
    * base_imag = fft_out_32 [1].i;
}

#if 0
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
#endif

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
