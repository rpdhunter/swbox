#include "rfctwidget.h"
#include "ui_rfctwidget.h"
#include <QLineEdit>
#include <qwt_plot.h>
#include <QTimer>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include "../Menu0/barchart.h"

#define SETTING_NUM 5           //设置菜单条目数


RFCTWidget::RFCTWidget(QWidget *parent, G_PARA *g_data) :
    QFrame(parent),
    ui(new Ui::RFCTWidget)
{
    ui->setupUi(this);

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("RFCTWidget {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    key_val = NULL;
    data = g_data;
    sql_para = sqlcfg->get_para();

    QLineEdit *lineEdit = new QLineEdit;
    ui->comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray; }");
    ui->comboBox->setLineEdit(lineEdit);
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    ui->comboBox->lineEdit()->setReadOnly(true);
    ui->comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    ui->comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");



    //每隔1秒，刷新
    timer1 = new QTimer(this);
    timer1->setInterval(1000);
    timer1->start();
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_plot()));

    PRPS_inti();

    //1ms读取一次数据
    timer2 = new QTimer(this);
    timer2->setInterval(1);
    timer2->start();
    connect(timer2, SIGNAL(timeout()), this, SLOT(doRfctData()));

}

RFCTWidget::~RFCTWidget()
{
    delete ui;
}

void RFCTWidget::PRPS_inti()
{
    plot_PRPS = new QwtPlot(ui->widget);
    plot_PRPS->resize(200, 140);
    plot_PRPS->setStyleSheet("background:transparent;color:gray;");

    plot_PRPS->setAxisScale(QwtPlot::xBottom, 0, 13);
    plot_PRPS->setAxisScale(QwtPlot::yLeft, 0, 60, 20);

    plot_PRPS->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPS->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    //plot->setAxisMaxMajor(QwtPlot::yLeft, 2);

    plot_PRPS->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot_PRPS->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    plot_PRPS->plotLayout()->setAlignCanvasToScales(true);

    int high=40, low=20;
    d_PRPS = new BarChart(plot_PRPS, &db, &high, &low);
    connect(timer1, SIGNAL(timeout()), d_PRPS, SLOT(fresh()) );
}

//根据读取到的序列，计算电缆局放值
double RFCTWidget::compute_pC(QVector<double> list)
{
    if(list.length() < 2 )
        return 0;

    QVector<double> pC;
    int first=0,last=0;
    for (int i = 0; i < list.length() - 1; ++i) {
        if(list.at(i) * list.at(i+1) <= 0){
            last = i+1;
            if(last-first > 1){ //至少有3个点
                pC.append( compute_one_pC(list.mid(first,last-first+1)) ); //计算一个脉冲的pC值
                first = i;
            }
        }
    }

//    qDebug()<<"divide:"<<pC;

    if(pC.isEmpty()){   //说明序列没有子脉冲，则把整个序列看做一个脉冲
        return compute_one_pC(list);
    }

    double max = 0;
    foreach (double l, pC) {
        max = MAX(qAbs(l),max);
    }
    return max;



}

double RFCTWidget::compute_one_pC(QVector<double> list)
{
    if(list.length() < 2 )
        return 0;

    double t1=0, t2=0, s=0;
    int first = 0, last = list.length()-1;

    if(list.at(0) * list.at(1) <= 0){
        t1 = triangle(list.at(1), list.at(0));
        first = 1;
    }

    if(list.at(last-1) * list.at(last) <= 0){
        t2 = triangle(list.at(last-1), list.at(last));
        last = list.length() - 2;
    }

    s = simpson(list.mid(first,last - first + 1));

//    qDebug()<<"t1="<<t1<<"\ts="<<s<<"\tt2="<<t2<<"\t"<<list << "simpson :"<<list.mid(first,last - first + 1);

    return t1 + s + t2 ;
}

//list是已标准化的脉冲序列（如何标准化，以后探讨）
//list长度至少为2
//函数实现用复化辛普生公式求积分
double RFCTWidget::simpson(QVector<double> list)
{
    if(list.length() < 2 )
        return 0;

    if(list.length() == 2)
        return (list.at(0) + list.at(1)) / 2 ;      //梯形公式

    int n = (list.length()-1) / 2 ;     //n至少为1
    double S = list.at(0) - list.at(2*n);
    for (int i = 1; i <= n; ++i) {
        S += 4*list.at(2*i-1) + 2*list.at(2*i);
    }
    if(list.length() % 2 == 1){     //如果长度为奇数，正好使用复化辛普生公式，否则补上一个梯形公式做结尾
        return S / 3;
    }
    else{
        return (list.at(2*n) + list.last() )/2 + S/3 ;
    }
}

//返回序列首（尾）遇到符号变化时，三角形的面积
//d1为靠近序列内部的数据点，d2为序列首（尾）部数据点
double RFCTWidget::triangle(double d1, double d2)
{
    return d1*d1 / (d1 - d2) / 2;
}


void RFCTWidget::fresh_plot()
{
//    QVector<double> list;
//    list << 1 << 3 << -10 << 3 << 0;
//    qDebug()<< "\ndata is: " << list;
//    qDebug()<<"pC is : "<<compute_pC(list);


    if(pCList.isEmpty()){
        qDebug()<<"pC is : 0 ";
        db = 0;
    }
    else{
        double max = 0;
        foreach (double l, pCList) {
            max = MAX(qAbs(l),max);
        }
        qDebug()<<"pC is : "<< max << pCList;
        db = max * 0.001;

        pCList.clear();
    }

    ui->label_val->setText(QString("%1").arg(db));
    plot_PRPS->replot();
}


void RFCTWidget::doRfctData()
{
    if(group != data->recv_para.groupNum){      //判断数据有效性
        group = data->recv_para.groupNum;
        if(data->recv_para.rfctData[0] == 0){
            QVector<double> list;
            for (int i = 2; i < 254; ++i) {
                if(data->recv_para.rfctData[i] == 0x55aa){
                    break;
                }
                else{
                    list.append(((double)data->recv_para.rfctData[i] - 0x8000));
//                    list.append(data->recv_para.recData[i]);
                }
            }

            double max = 0;
            foreach (double l, list) {
                max = MAX(qAbs(l),max);
            }
            qDebug()<<"one pluse data number: "<< list.length() << "\t first = "<<list.first()<<"\tlast = "<<list.last()
                      <<"\tmax = "<<max;


            pCList.append(compute_pC(list) );
        }


    }
}

void RFCTWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void RFCTWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 5) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_UP:
        if (key_val->grade.val1) {
            if (key_val->grade.val2 < 2) {
                key_val->grade.val2 = SETTING_NUM;
            } else {
                key_val->grade.val2--;
            }
        }
        break;
    case KEY_DOWN:
        if (key_val->grade.val1) {
            if (key_val->grade.val2 >= SETTING_NUM) {
                key_val->grade.val2 = 1;
            } else {
                key_val->grade.val2++;
            }
        }
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    default:
        break;
    }
    emit fresh_parent();
    fresh_setting();
}


void RFCTWidget::fresh_setting()
{
    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == 5) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));

}


