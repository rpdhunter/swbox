﻿#include "logtools.h"
#include <QFile>
#include <QDir>
#include <QtDebug>
#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Common/common.h"
#include "spacecontrol.h"


LogTools::LogTools(MODE mode, QObject *parent) : QObject(parent)
{
    this->mode = mode;

    log_timer = 0;
    path_normal = DIR_DATALOG"/" + Common::mode_to_string(mode) + "_NORMAL" + ".log";
    path_asset = DIR_ASSET_NORMAL"/" + Common::mode_to_string(mode) + "_NORMAL" + ".log";

    //读取文件,资产文件不需要在初始化时读取
    read_normal_log(path_normal,data_normal);
}

//完成2个功能：
//1.保存一般测量数据至公共空间
//2.保存一般测量数据至资产空间，资产空间的数据是公共空间数据的子集
void LogTools::dealLog(double val, int pulse, double degree, int is_current, QString result)
{
    log_timer ++;   //计数器累加

    if(log_timer % LOG_TIME_INTERVAL == 0 || qAbs((val-data_normal.last().val)/data_normal.last().val) > 0.2){
        //录入当前节点
        LOG_DATA d;
        d.datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz");
        d.val = val;
        d.pulse = pulse;
        d.degree = degree;
        d.is_current = is_current;
        d.result = result;
        data_normal.append(d);
        data_asset.append(d);

        //判断条目数
        while (data_normal.length() > MAX_LOG_NUM) {
            data_normal.removeFirst();
        }
        while (data_asset.length() > MAX_LOG_NUM) {
            data_asset.removeFirst();
        }

//        //保存文件
////        write_normal_log(path_normal,data_normal);
//        write_normal_log(path_asset,data_asset);
    }

    //60s保存文件
    if(log_timer % SAVE_TIME_INTERVAL == 0){
        write_normal_log(path_normal,data_normal);
        write_normal_log(path_asset,data_asset);
    }

    if(log_timer == SAVE_TIME_INTERVAL * LOG_TIME_INTERVAL){
        log_timer = 0;      //计数器清零
    }
}

//完成PRPD文件的保存
//一般测量保存至标准目录
//资产测量保存至资产目录（以硬链接的形式）
void LogTools::dealRPRDLog(QVector<QwtPoint3D> points)
{
    if(points.isEmpty()){
        return;
    }
    save_PRPD_zdit(points);
    if(mode == HFCT1 || mode == HFCT2){
        save_PRPD_State_Grid(points);           //高频模式保存国网格式
    }
}

void LogTools::save_PRPD_zdit(QVector<QwtPoint3D> points)
{
    QFile file;
    bool flag;

    QString file_name = QString(Common::mode_to_string(mode) + "_PRPD_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") + ".log");
    file.setFileName(DIR_PRPDLOG"/" + file_name);

    QString str_src = file.fileName();
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QTextStream out(&file);

        //保存纯数据格式
        foreach (QwtPoint3D p, points) {
            out << p.x() << "\t"
                << p.y() << "\t"
                << p.z() << "\n";
        }

        qDebug()<<"ZDIT PRPD file saved! \tnum="<< points.length() << "\tmode = "<<Common::mode_to_string(mode);

        file.close();
        Common::create_hard_link(str_src, file_name);           //建立硬连接
    }

    SpaceControl::file_bySize(DIR_PRPDLOG, 200e6);         //大于200M,开删
}

void LogTools::save_PRPD_State_Grid(QVector<QwtPoint3D> points)
{
    QFile file;
    bool flag;

    QString format_str;
    if(mode == HFCT1 || mode == HFCT2){
        format_str = ".HF";
    }
    else if(mode == TEV1 || mode == TEV2){
        format_str = ".TEV";
    }
    else if(mode == UHF1 || mode == UHF2){
        format_str = ".UHF";
    }
    else if(mode == AA1 || mode == AA2){
        format_str = ".AA";
    }
    else if(mode == AE1 || mode == AE2){
        format_str = ".AE";
    }
    QString file_name = QDateTime::currentDateTime().toString("yyyyMMdd_HHmm") + format_str;
    file.setFileName(DIR_PRPDLOG"/" + file_name);

    QString str_src = file.fileName();
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QDataStream out(&file);
//        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);            //设置浮点数流格式为单精度（float占4字节）

        out << (float)1.0;          //版本号（4字节）
        out << (quint8)0x00;        //谱图类型 0x00——PRPD，0x01——PRPS（1字节）
        out << (int)360;            //工频相位窗数（4字节）
        out << (qint32)121;         //幅值范围（4字节）
        out << (quint8)0x01;        //幅值单位 0x01——db，0x02——mv，0x03——pC（4字节）
        out << (float)-60;          //幅值上限（4字节）
        out << (float)60;           //幅值下限（4字节）
        out << (qint32)0;           //工频周期数（4字节）

        char t[] = {100,0,0,0,0,0,0,0,0}; //放电概率，8字节对应8种放电类型（8字节）
        out.writeRawData(t,8);
        int map[360][121];
        for (int i = 0; i < 360; ++i) {
            for (int j = 0; j < 121; ++j) {
                map[i][j] = 0;
            }
        }
        int x,y;
        foreach (QwtPoint3D p, points) {
            x = p.x();
            if(p.y()>6000){
                y = 6000;
            }
            else{
                y = p.y();
            }
            y = 20*log10(y/6.0 );      //取DB
            map[x][y] += 1;
        }

        for (int i = 0; i < 360; ++i) {     //写入
            for (int j = 0; j < 121; ++j) {
                out << map[i][j];
            }
        }
        qDebug()<<"State_Grid PRPD file saved! \tnum="<< points.length() << "\tmode = "<<Common::mode_to_string(mode);

        file.close();
        Common::create_hard_link(str_src, file_name);           //建立硬连接
    }
}

void LogTools::change_current_asset_dir()
{
    path_asset = QString::fromLocal8Bit(sqlcfg->get_global()->current_dir) + "/" + Common::mode_to_string(mode) + "_NORMAL" + ".log";
//    printf("current dir:%s\n", path_asset.toLocal8Bit().data());
    read_normal_log(path_normal,data_normal);
}

//读入历史数据，保存在链表中
void LogTools::read_normal_log(QString path, QVector<LOG_DATA> &log_data)
{
    QFile file;
    file.setFileName(path);
    bool flag = file.open(QIODevice::ReadOnly);

    log_data.clear();
    if(flag){
        QTextStream in(&file);
        LOG_DATA d;
        for (int i = 0; i < MAX_LOG_NUM; ++i) {
            if (!in.atEnd()) {
                in >> d.datetime >> d.val >> d.pulse >> d.degree >> d.is_current;
                log_data.append(d);
            }
            else{
                break;
            }
        }

        if(!log_data.isEmpty())
            log_data.removeLast();  //移除最后一行

        file.close();
    }
    else{
        qDebug()<<"found no former log file" << path;

    }
}

void LogTools::save_log()
{
    write_normal_log(path_normal,data_normal);
    write_normal_log(path_asset,data_asset);

    qDebug()<<"save_log"<<mode;

}

void LogTools::write_normal_log(QString path, QVector<LOG_DATA> log_data)
{
    QFile file;
    file.setFileName(path);
    bool flag = file.open(QIODevice::WriteOnly);
//    if(mode == TEV1)
//        printf("writeFile:%s\n", path.toLocal8Bit().data());

    if(flag){
        QTextStream out(&file);

        foreach (LOG_DATA d, log_data) {
            out << d.datetime << "\t"
                << d.val << "\t"
                << d.pulse << "\t"
                << d.degree << "\t"
                << d.is_current << "\t"
                << d.result << "\n";
        }

        file.close();
    }
    else{
        qDebug()<<"log file saved failed! ";
    }    
    SpaceControl::file_bySize(path, 200e6);         //大于200M,开删
}









