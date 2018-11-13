#include "cameradecode.h"
#include <QtDebug>

CameraDecode::CameraDecode(QObject *parent) : QThread(parent)
{
    this->out_buffer_rgb=NULL;
    this->pFrame=NULL;
    this->pFrameRGB=NULL;
    this->pCodec=NULL;
    this->pCodecCtx=NULL;
    this->packet=NULL;
    this->img_convert_ctx=NULL;

    camera_buf0 = (uint8_t *)malloc(1e6);
    camera_buf1 = (uint8_t *)malloc(1e6);
    camera_buf2 = (uint8_t *)malloc(1e6);

    flag0 = 0;
    flag1 = 0;
    flag2 = 0;
    get_packet = 0;

    init_ffmpeg_stream();

    camera_init_flag = false;
}

CameraDecode::~CameraDecode()
{
    av_packet_unref(packet);
    av_free(out_buffer_rgb);
    av_free(pFrame);
    av_free(pFrameRGB);
    sws_freeContext(img_convert_ctx);
    avcodec_close(pCodecCtx);
}

void CameraDecode::init_ffmpeg_stream()
{
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);    //找寻H264格式解码器
    if (!pCodec){
        printf("avcodec_find_encoder failed");
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);         //根据解码器，生成解码器上下文
    if (!pCodecCtx){
        printf("avcodec_alloc_context3 failed");
        return;
    }

    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;         //设置解码类型为视频
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    avcodec_open2(pCodecCtx, pCodec, NULL);             //打开解码器

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));  //申请包空间
    pFrame = av_frame_alloc();                          //申请帧空间
    pFrameRGB = av_frame_alloc();
}


void CameraDecode::getOnePacket(QByteArray buf, int f)
{
    qDebug()<< "CameraDecode Class get one packet : length = "<<buf.length() << "\t" << f;
    av_init_packet(packet);                         //packet初始化
    if(flag0 == 0){
        qDebug()<<"get_packet0";
        memcpy(camera_buf0, buf.data(), buf.length());
        packet->data = camera_buf0;                      //packets数据初始化
        packet->size = buf.length();
        flag0 = 1;
    }
    else if(flag1 == 0){
        qDebug()<<"get_packet1";
        memcpy(camera_buf1, buf.data(), buf.length());
        packet->data = camera_buf1;                      //packets数据初始化
        packet->size = buf.length();
        flag1 = 1;
    }
    else if(flag2 == 0){
        qDebug()<<"get_packet2";
        memcpy(camera_buf2, buf.data(), buf.length());
        packet->data = camera_buf2;                      //packets数据初始化
        packet->size = buf.length();
        flag2 = 1;
    }
    emit read_done(f);

//    qDebug()<<"get_packet:"<<get_packet;
//    if(get_packet == 2){
//        qDebug()<<"decode imcomplete";
//    }
//    else{
//        get_packet = 1;
    //    }
}

void CameraDecode::camera_init()
{
    qDebug()<<"camera_init()";
    camera_init_flag = true;
}

void CameraDecode::run()
{
    while (1) {
        if(camera_init_flag == true){
            camera_init_flag = false;

            qDebug()<<"open AP";
//            system("./camera");
            system("/root/wifi/ap.sh    ZZDDIITT  zdit.com.cn    192.168.150.1    255.255.255.0");      //开启AP
        }
        else if(flag0 == 1){
            flag0 = 2;
            decode();           //解码一帧
            qDebug()<<"flag0"<<flag0;
            flag0 = 0;
        }
        else if(flag1 == 1){
            flag1 = 2;
            decode();           //解码一帧
            flag1 = 0;
        }
        else if(flag2 == 1){
            flag2 = 2;
            decode();           //解码一帧
            flag2 = 0;
        }
        else{
            msleep(1);
        }
    }
}

void CameraDecode::decode()
{
    int re = avcodec_send_packet(pCodecCtx, packet);        //将原始数据包输入解码器

//    if(packet->size > 10)
//        qDebug()<< *packet->data << *(packet->data+1) << *(packet->data+2) << *(packet->data+3) << *(packet->data+4)
//                << *(packet->data+5) << *(packet->data+6) << *(packet->data+7) << *(packet->data+8) << *(packet->data+9)
//                << *(packet->data+10) << *(packet->data+11) << *(packet->data+12) << *(packet->data+13) << *(packet->data+14)
//                << *(packet->data+15) << *(packet->data+16) << *(packet->data+17) << *(packet->data+18) << *(packet->data+19);


    if (re != 0 /*|| pCodecCtx->pix_fmt == -1*/)
    {

        qDebug()<<"decode failed";
        return;
    }



    if(img_convert_ctx == NULL){
        qDebug()<<"11";
        if(pCodecCtx->pix_fmt == -1){
            pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        }
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                         pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                         AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

        size_t numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,pCodecCtx->height, 1);
        out_buffer_rgb = (uchar*) av_malloc(numBytes * sizeof(uint8_t));
        av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer_rgb, AV_PIX_FMT_RGB24,
                             pCodecCtx->width, pCodecCtx->height, 1);

    }
    qDebug()<<pCodecCtx->width<<pCodecCtx->height<<pCodecCtx->pix_fmt;

    while( avcodec_receive_frame(pCodecCtx, pFrame) == 0)   //从解码器输出解码后的一帧数据
    {
        qDebug()<<"one frame";
        //将解码后的帧转换成RGB格式，输出在pFrameRGB里面（实际也存储在out_buffer_rgb中）
        sws_scale(img_convert_ctx,
                  (uint8_t const * const *) pFrame->data,
                  pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                  pFrameRGB->linesize);
        //用QImage装载pFrameRGB中的数据
        QImage tmpImg((uchar *)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB888);
        finalImage = tmpImg.convertToFormat(QImage::Format_RGB888,Qt::NoAlpha);
        emit sigGetOneFrame(finalImage);
    }
}



