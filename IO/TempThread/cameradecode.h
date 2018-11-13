#ifndef CAMERADECODE_H
#define CAMERADECODE_H

#include <QThread>
#include <QImage>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/time.h"
#include "libavutil/pixfmt.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

class CameraDecode : public QThread
{
    Q_OBJECT
public:
    explicit CameraDecode(QObject *parent = nullptr);
    ~CameraDecode();

public slots:
    void getOnePacket(QByteArray buf, int f);  //接收到原始数据帧
    void camera_init();

signals:
    void sigGetOneFrame(QImage);        //转码后的一帧
    void read_done(int);

protected:
    void run();

private:
    unsigned char* out_buffer_rgb;

    AVCodecContext  *pCodecCtx;         //解码上下文（1500行）
    AVCodec         *pCodec;
    AVFrame         *pFrame,*pFrameRGB;
    AVPacket        *packet;
    QImage           finalImage;
    struct SwsContext *img_convert_ctx;

    void decode();

    int get_packet;
    void init_ffmpeg_stream();

    uint8_t *camera_buf0, *camera_buf1, *camera_buf2;       //接收UDPSocket数据的缓冲区
    int flag0, flag1, flag2;                                //缓冲区对应的标志位

    bool camera_init_flag;

};

#endif // CAMERADECODE_H
