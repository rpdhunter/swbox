#ifndef FFT_H
#define FFT_H

#include "IO/Com/rdb/data_types.h"
#include <NE10.h>
#include <NE10_dsp.h>
#include <NE10_init.h>
#include <NE10_types.h>
#include <QVector>

class FFT
{
public:
    FFT();

//    QVector<qint32> fft4096(int ibuf[]);
    QVector<qint32> fft2048(int ibuf[]);
    QVector<qint32> fft2048(QVector<qint32> list);
    QVector<qint32> fft64(QVector<qint32> list);
    QVector<qint32> fft32(QVector<qint32> list);

private:

#if 0
    //input and output
    static ne10_fft_r2c_cfg_float32_t fft_cfg_32;
    /* 17个数据结构，r0i0,r1i1```r16i16 */
    static ne10_fft_cpx_float32_t * fft_out_32;
//    #ifdef USING_FFT64
    static ne10_fft_r2c_cfg_float32_t fft_cfg_64;
    /* 33个数据结构，r0i0,r1i1```r32i32 */
    static ne10_fft_cpx_float32_t * fft_out_64;
//    #endif

#endif

    ne10_fft_r2c_cfg_float32_t rfft_cfg_32;
    ne10_fft_cpx_float32_t * rfft_out_32;

    ne10_fft_r2c_cfg_int32_t fft_cfg_32;
    ne10_fft_cpx_int32_t * fft_out_32;
    ne10_fft_r2c_cfg_int32_t fft_cfg_64;
    ne10_fft_cpx_int32_t * fft_out_64;
    ne10_fft_r2c_cfg_int32_t fft_cfg_2048;
    ne10_fft_cpx_int32_t * fft_out_2048;


    int init_fft();

    void rfft32_prot_calc (
        float32 ibuf [], 		/* 采样数组 */
        float32 mbuf [],		/* 谐波幅值，包括0-16次谐波 */
        float32 * base_real,	/* 基波实部 */
        float32 * base_imag		/* 基波虚部 */
        );

    void rfft64_harm_calc (
        float32 ibuf [], 		/* 采样数组 */
        float32 mbuf []			/* 谐波幅值，包括0-32次谐波 */
        );

    void rfft2048_harm_calc (int ibuf[], 		/* 采样数组 */
        int mbuf[]			/* 谐波幅值，包括0-1024次谐波 */
        );
};

#endif // FFT_H
