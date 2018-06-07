#ifndef REPORT_H
#define REPORT_H

#include <QObject>
#include <QDomElement>
#include "IO/Data/data.h"
#include "logtools.h"
#include "Gui/Common/common.h"

struct ReportAsset
{
    QString area;           //片区
    QString substation;     //变电站
    QString equipment;      //设备
    QString voltage;        //电压等级
    QString interval;       //间隔名称
    QString position;       //测量部位
};

struct ReportRecord
{
    ReportAsset asset;      //资产信息
    MODE mode;              //当前测量模式
    QString begin;        //开始测量时间
    QString end;          //结束测量时间
    double max_val;         //最大测量值
    int max_pulse_num;      //每秒脉冲数
    double average_val;         //平均测量值
    int average_pulse_num;      //平均每秒脉冲数
    QString prpd_file_namae;//PRPD文件名
    QString rec_file_name;  //录波文件名
};

class Report : public QObject
{
    Q_OBJECT
public:
    explicit Report(QObject *parent = nullptr);

    void create_report(QString path);

signals:

public slots:

private:
    struct ReportData
    {
        QVector<LOG_DATA> log_data;
        MODE mode;
        ReportRecord data_2_report()
        {
            ReportRecord r;
            r.mode = mode;              //当前测量模式
            r.begin = log_data.first().datetime;        //开始测量时间
            r.end = log_data.last().datetime;          //结束测量时间
            r.max_val = get_max_val();         //最大测量值
            r.max_pulse_num = get_max_pulse();     //最大每秒脉冲数
            r.average_val = get_average_val();         //最大测量值
            r.average_pulse_num = get_average_pulse();     //最大每秒脉冲数

            return r;
        }

        double get_max_val(){
            double max = 0;
            foreach (LOG_DATA d, log_data) {
                if(d.val > max){
                    max = d.val;
                }
            }
            return max;
        }

        double get_average_val(){
            double sum = 0;
            foreach (LOG_DATA d, log_data) {
                sum += d.val ;
            }
            return sum / log_data.count();
        }

        int get_max_pulse(){
            int max = 0;
            foreach (LOG_DATA d, log_data) {
                if(d.pulse > max){
                    max = d.val;
                }
            }
            return max;
        }

        double get_average_pulse(){
            double sum = 0;
            foreach (LOG_DATA d, log_data) {
                sum += d.pulse ;
            }
            return sum / log_data.count();
        }
    };

    //仪器信息
    QString instrument_model;   //仪器型号
    QString manufacturer;       //生产厂家

    //检测背景
    double temperature;         //温度(℃)
    double humidity;            //湿度(％)
    QString inspector;          //检测人员
    QString department;         //检测单位    

    QList<ReportData> report_data;     //原始测量数据,从文件读取(对于一次测量,report_data包含了全部的普测数据)
    QList<ReportRecord> record;         //记录数据,从Report_Data生成,可直接写入报告

    LogTools *logtools;

    void read_log_data(QString path);
    void data_2_record();
    void create_XML(QString path);      //生成XML报告

    //创建根节点
    QDomElement creat_dom_child_0(QDomDocument d, QString tagName, QString name, QString value);
    //创建一级节点
    QDomElement creat_dom_child_1(QDomDocument d, QString tagName, QString name, QString value, QDomElement node);
    //创建二级节点
    QDomElement creat_dom_child_2(QDomDocument d, QString tagName, QString name, QString value, QString text, QDomElement node);

    MODE mode_from_string(QString str);

};

#endif // REPORT_H
