#include "cameradecode.h"
#include <QtDebug>
#include <QFile>
//#define REC_ON

CameraDecode::CameraDecode(QObject *parent) : QThread(parent)
{
    this->out_buffer_rgb=NULL;
    this->pFrame=NULL;
    this->pFrameRGB=NULL;
    this->pCodec=NULL;
    this->pCodecCtx=NULL;
    this->packet=NULL;
    this->img_convert_ctx=NULL;

    data_buf = (uint8_t *)malloc(1e6);

    init_ffmpeg_stream();

    camera_init_flag = false;

#ifdef REC_ON
    timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(10 * 1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(save_test_video_file()));
#endif
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
    pFrameRGB = av_frame_alloc();                       //申请转换帧空间

    av_log_set_level(AV_LOG_QUIET);                     //禁止输出
}


void CameraDecode::getOnePacket(QByteArray buf, int f)
{
    frame_list.append(buf);


    qDebug()<< "CameraDecode Class get one packet : length = "<<buf.length() << "\t" << f;
    qDebug()<< "current frame buf num:" << frame_list.count();

#ifdef REC_ON
    if(timer->isActive()){
        for(int i=0; i<10; i++){
            printf("%02x ", *(buf.data()+i) );
        }
        printf("\n");
        video_rec.append(buf);
    }
    else{
        timer->start();
    }
#endif
    emit read_done(f);
}

void CameraDecode::camera_init()
{
    qDebug()<<"camera_init()";
    camera_init_flag = true;
}

void CameraDecode::save_test_video_file()
{
#ifdef REC_ON
    QFile file;
    bool flag;
    //保存二进制文件（小端）
    file.setFileName("test.h264");
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian);
        out.writeBytes(video_rec.data(), video_rec.length());

        file.close();
        qDebug()<<"test video file saved!";

        video_rec.clear();
    }
    else{
        qDebug()<<"file open failed!";
    }
#endif
}
#include <QTime>
void CameraDecode::run()
{
    while (1) {
        if(camera_init_flag == true){
            camera_init_flag = false;

            qDebug()<<"open AP";
            system("/root/wifi/ap.sh    ZZDDIITT  zdit.com.cn    192.168.150.1    255.255.255.0");      //开启AP
        }
        else if(!frame_list.isEmpty()){
//            qDebug()<<"decode prepare:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
            av_init_packet(packet);                         //packet初始化
            memcpy(data_buf, frame_list.first().data(), frame_list.first().length());

            packet->data = data_buf;                      //packets数据初始化
            packet->size = frame_list.first().length();

            frame_list.removeFirst();

            decode();
        }
        else{
            msleep(10);
        }
    }
}

void CameraDecode::decode()
{
//    qDebug()<<"decode begin:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
    int re = avcodec_send_packet(pCodecCtx, packet);        //将原始数据包输入解码器
    if (re != 0 /*|| pCodecCtx->pix_fmt == -1*/)
    {
        qDebug()<<"decode failed";
        return;
    }

    if(img_convert_ctx == NULL){                            //初始化格式转换，只运行一次
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
//    qDebug()<<pCodecCtx->width<<pCodecCtx->height<<pCodecCtx->pix_fmt;

    while( avcodec_receive_frame(pCodecCtx, pFrame) == 0)   //从解码器输出解码后的一帧数据
    {
        //将解码后的帧转换成RGB格式，输出在pFrameRGB里面（实际也存储在out_buffer_rgb中）
//        qDebug()<<"1:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
        sws_scale(img_convert_ctx,
                  (uint8_t const * const *) pFrame->data,
                  pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                  pFrameRGB->linesize);
        //用QImage装载pFrameRGB中的数据
//        qDebug()<<"2:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
        QImage tmpImg((uchar *)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB888);
        finalImage = tmpImg.convertToFormat(QImage::Format_RGB888,Qt::NoAlpha);
//        qDebug()<<"3:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
        emit sigGetOneFrame(finalImage);
    }
//    qDebug()<<"decode end:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
}



