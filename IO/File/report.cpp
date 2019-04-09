#include "report.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QDir>



Report::Report(QObject *parent) : QObject(parent)
{
    instrument_model = "PDTEV";   //仪器型号
    manufacturer = "ZIT";       //生产厂家

    //检测背景
    temperature = 20;         //温度(℃)
    humidity = 40;            //湿度(％)
    inspector = "";          //检测人员
    department = "";         //检测单位


    logtools = new LogTools;
}

void Report::create_report(QString path)
{
    read_log_data(path);        //从文件读取数据
    data_2_record();            //将数据转换成报告格
    create_XML(path);           //生成xml文件
}

void Report::create_XML(QString path)
{
    if(record.isEmpty() ){
        qDebug()<<"record is empty, return";
        return;
    }
    QDomDocument doc;

    //表头
    QString strHeader( "version=\"1.0\" encoding=\"UTF-8\"" );
    doc.appendChild( doc.createProcessingInstruction("xml", strHeader) );

    //根节点
    QDomElement rootNode = creat_dom_child_0(doc, "report", "generated time", QDateTime::currentDateTime().toString());

    //一般内容
    QDomElement generalNode = creat_dom_child_1(doc, "general", "", "", rootNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("仪器型号"), instrument_model, generalNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("生产厂家"), manufacturer, generalNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("温度(℃)"), QString::number(temperature), generalNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("湿度(％)"), QString::number(humidity), generalNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("检测人员"), inspector, generalNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("检测单位"), department, generalNode);

    //资产信息
    QDomElement assetNode = creat_dom_child_1(doc, "asset", "substation", record.first().asset.substation, rootNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("片区"), record.first().asset.area, assetNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("变电站"), record.first().asset.substation, assetNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("设备"), record.first().asset.equipment, assetNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("电压等级"), record.first().asset.voltage, assetNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("间隔名称"), record.first().asset.interval, assetNode);
    creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("测量部位"), record.first().asset.position, assetNode);

    //测量内容
    foreach (ReportRecord r, record) {
        QDomElement modeNode = creat_dom_child_1(doc, "detection", "mode", Common::mode_to_string(r.mode), rootNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("测量模式"), Common::mode_to_string(r.mode), modeNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("开始测量时间"), r.begin, modeNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("结束测量时间"), r.end, modeNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("最大测量值"), QString::number(r.max_val), modeNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("最大每秒脉冲数"), QString::number(r.max_pulse_num), modeNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("平均测量值"), QString::number(r.average_val), modeNode);
        creat_dom_child_2(doc, "attrib", "param", QString::fromUtf8("平均每秒脉冲数"), QString::number(r.average_pulse_num), modeNode);
    }

    QString fileName = path + '/' + "report.xml";
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)){
        qDebug()<<"create XML file failed!";
        return;
    }

    QTextStream stream(&file);
    doc.save(stream, 4);
    file.close();

    qDebug()<<"create XML file succeed!";
}

QDomElement Report::creat_dom_child_0(QDomDocument d, QString tagName, QString name, QString value)
{
    QDomElement n = d.createElement(tagName);           //根节点
    n.setAttribute(name, value);
    d.appendChild(n);
    return n;
}

QDomElement Report::creat_dom_child_1(QDomDocument d, QString tagName, QString name, QString value, QDomElement node)
{
    QDomElement n = d.createElement(tagName);           //一级节点
    n.setAttribute(name, value);
    node.appendChild(n);

    return n;
}

QDomElement Report::creat_dom_child_2(QDomDocument d, QString tagName, QString name, QString value, QString text, QDomElement node)
{
    QDomElement n = d.createElement(tagName);           //二级节点
    n.setAttribute(name, value);
    node.appendChild(n);

    QDomText textNode = d.createTextNode(text);
    n.appendChild(textNode);

    return n;
}



void Report::read_log_data(QString path)
{
    QDir dir;
    dir.setPath(path);

    qDebug()<<"create_report:"<<dir.path();

    QStringList list_report = dir.entryList(QDir::Files,QDir::Name);

//    qDebug()<<list_report;

    //从文件读取各通道的普通测量数据,以Report_Data数组形式保存
    report_data.clear();

    foreach (QString normal_log, list_report) {
        if(normal_log.isEmpty() || !normal_log.contains("NORMAL"))
            continue;
        QVector<LOG_DATA> log_data;
        QString file_path = dir.path() + '/' + normal_log;
//        qDebug()<<"read file:"<<file_path;
        logtools->read_normal_log(file_path, log_data);
        if(!log_data.isEmpty()){
            ReportData d;
            d.log_data = log_data;
            d.mode = mode_from_string(normal_log);
            report_data.append(d);
        }

        qDebug()<< QString("get %1 LOG_DATA from %2").arg(log_data.count()).arg(normal_log);
    }
}

void Report::data_2_record()
{
    record.clear();
    foreach (ReportData d, report_data) {
        record.append(d.data_2_report() );
    }
}

MODE Report::mode_from_string(QString str)
{
    if(str.contains("TEV1")){
        return TEV1;
    }
    if(str.contains("TEV2")){
        return TEV2;
    }
    if(str.contains("HFCT1")){
        return HFCT1;
    }
    if(str.contains("HFCT2")){
        return HFCT2;
    }
    if(str.contains("UHF1")){
        return UHF1;
    }
    if(str.contains("UHF2")){
        return UHF2;
    }
    if(str.contains("AA1")){
        return AA1;
    }
    if(str.contains("AA2")){
        return AA2;
    }
    if(str.contains("AE1")){
        return AE1;
    }
    if(str.contains("AE2")){
        return AE2;
    }
    return Disable;
}
