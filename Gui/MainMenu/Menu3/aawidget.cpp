#include "aawidget.h"
#include "ui_aawidget.h"
#include <QLineEdit>
#include <QTimer>
#include "../Menu0/barchart.h"
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>


#define SETTING_NUM 7           //设置菜单条目数


AAWidget::AAWidget(QWidget *parent, G_PARA *g_data) :
    QFrame(parent),
    ui(new Ui::AAWidget)
{
    ui->setupUi(this);

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("AAWidget {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    key_val = NULL;

    temp_db = 0;
    db = 0;

    sql_para = sqlcfg->get_para();

    //开机设置一下音量
    this->data = g_data;
	data->set_send_para (sp_aa_vol, sql_para->aaultra_sql.vol);

    QLineEdit *lineEdit = new QLineEdit;
    ui->comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray; }");
    ui->comboBox->setLineEdit(lineEdit);
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    ui->comboBox->lineEdit()->setReadOnly(true);
    ui->comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    ui->comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");

    //图形设置
    ui->qwtPlot->setStyleSheet("background:transparent;color:gray;");
    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, 0, 13);
    ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, 60, 20);

    ui->qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    ui->qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    ui->qwtPlot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    ui->qwtPlot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    ui->qwtPlot->plotLayout()->setAlignCanvasToScales(true);


    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    timer1 = new QTimer(this);
    timer1->setInterval(100);
    if (sql_para->aaultra_sql.mode == continuous) {
        timer1->start();
    }
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_2()));   //每0.1秒刷新一次数据状态，明显的变化需要快速显示

    timer2 = new QTimer(this);
    timer2->setInterval(1000);
    if (sql_para->aaultra_sql.mode == continuous) {
        timer2->start();
    }
    connect(timer2, SIGNAL(timeout()), this, SLOT(fresh_1()));   //每1秒刷新一次数据状态


    chart = new BarChart(ui->qwtPlot, &db, &sql_para->aaultra_sql.high, &sql_para->aaultra_sql.low);
    connect(timer2, &QTimer::timeout, chart, &BarChart::fresh);

    recWaveForm = new RecWaveForm(this);
    recWaveForm->hide();
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));

}

AAWidget::~AAWidget()
{
    delete ui;
}

void AAWidget::showWaveData(VectorList buf, MODE mod)
{
    key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
    key_val->grade.val5 = 1;
    recWaveForm->working(key_val,buf,mod);
}

void AAWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void AAWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 3) {
        return;
    }

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val2 == 7){
            this->maxReset();
        }

        if(key_val->grade.val2 == 6){       //发送录波信号
            emit startRecWave(2, sql_para->aaultra_sql.time);
        }

        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        if (sql_para->aaultra_sql.vol != sqlcfg->get_para()->aaultra_sql.vol){   //判断音量调节生效
			data->set_send_para (sp_aa_vol, sql_para->aaultra_sql.vol);
            qDebug()<<"vol changed!";
        }

        sqlcfg->sql_save(sql_para);
        timer1->start();                                                         //and timer no stop
        timer2->start();
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
        switch (key_val->grade.val2) {
        case 1:
            if (sql_para->aaultra_sql.mode == single) {
                sql_para->aaultra_sql.mode = continuous;
            } else {
                sql_para->aaultra_sql.mode = single;
            }
            break;
        case 2:
//            if (sql_para->aaultra_sql.gain < 1) {
//                sql_para->aaultra_sql.gain = 1;
//            } else if (sql_para->aaultra_sql.gain > 10) {
//                sql_para->aaultra_sql.gain = 10;
//            } else {
//                sql_para->aaultra_sql.gain -= 0.1;
//            }
            if (sql_para->aaultra_sql.gain > 0.15) {
                sql_para->aaultra_sql.gain -= 0.1;
            }
            break;
        case 3:
            if (sql_para->aaultra_sql.vol > VOL_MIN) {
                sql_para->aaultra_sql.vol--;
				data->set_send_para (sp_aa_vol, sql_para->aaultra_sql.vol);
                qDebug()<<"vol changed!";
            }
            break;
        case 4:
            if(sql_para->aaultra_sql.low>1){
                sql_para->aaultra_sql.low --;
            }
            break;
        case 5:
            if(sql_para->aaultra_sql.high > sql_para->aaultra_sql.low){
                sql_para->aaultra_sql.high --;
            }
            break;
        case 6:
            if(sql_para->aaultra_sql.time > TIME_MIN){
                sql_para->aaultra_sql.time --;
            }
            break;
        default:
            break;
        }
        break;
    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            if (sql_para->aaultra_sql.mode == single) {
                sql_para->aaultra_sql.mode = continuous;
            } else {
                sql_para->aaultra_sql.mode = single;
            }
            break;
        case 2:
//            if (sql_para->aaultra_sql.gain <1) {
//                sql_para->aaultra_sql.gain = 1;
//            } else if (sql_para->aaultra_sql.gain > 10) {
//                sql_para->aaultra_sql.gain = 10;
//            } else {
//                sql_para->aaultra_sql.gain += 0.1;
//            }
            if (sql_para->aaultra_sql.gain < 99.95) {
                sql_para->aaultra_sql.gain += 0.1;
            }
            break;
        case 3:
            if (sql_para->aaultra_sql.vol < VOL_MAX) {
                sql_para->aaultra_sql.vol++;
            }
			data->set_send_para (sp_aa_vol, sql_para->aaultra_sql.vol);
            qDebug()<<"vol changed!";
            break;
        case 4:
            if(sql_para->aaultra_sql.low < sql_para->aaultra_sql.high){
                sql_para->aaultra_sql.low ++;
            }
            break;
        case 5:
            if(sql_para->aaultra_sql.high < 60){
                sql_para->aaultra_sql.high ++;
            }
            break;            
        case 6:
            if(sql_para->aaultra_sql.time < TIME_MAX){
                sql_para->aaultra_sql.time ++;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    fresh_setting();
}

void AAWidget::calc_aa_value (double * aa_val, double * aa_db, int * offset)
{
	int d;

    d = (int)data->recv_para.ldata1_max - (int)data->recv_para.ldata1_min ;      //最大值-最小值=幅值
//    qDebug()<<"max="<<data->recv_para.ldata1_max << " min = "<<data->recv_para.ldata1_min;
//	* offset = d / 100;
    * offset = ( d - 1 / sql_para->aaultra_sql.gain / AA_FACTOR ) / 100;

    * aa_val = (d - sql_para->aaultra_sql.aa_offset * 100) * sql_para->aaultra_sql.gain * AA_FACTOR;
	* aa_db = 20 * log10 (* aa_val);
    if(* aa_db < 0){
        * aa_db = 0;
    }
}

void AAWidget::fresh(bool f)
{
    int offset;
    double val,val_db;

    if (sql_para->aaultra_sql.mode == single) {
        timer1->stop();
        timer2->stop();
    }

	calc_aa_value (&val, &val_db, &offset);
    emit offset_suggest (offset);

    if(db < int(val_db)){
        db = int(val_db);      //每秒的最大值
    }

    //记录并显示最大值
    if (max_db <val_db ) {
        max_db = val_db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
    }

    if(f){  //直接显示（1s一次）
        ui->label_val->setText(QString::number(val_db, 'f', 1));
        temp_db = val_db;
        //彩色显示
        if ( val_db > sql_para->aaultra_sql.high) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
        } else if (val_db >= sql_para->aaultra_sql.low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }
        emit aa_modbus_data(db);
//        db = val;
    }
    else{   //条件显示
        if(qAbs(val_db-temp_db ) > sqlcfg->get_para()->aaultra_sql.aa_step){
            ui->label_val->setText(QString::number(val_db, 'f', 1));
            if ( val_db > sql_para->aaultra_sql.high) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            } else if (val_db >= sql_para->aaultra_sql.low) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            } else {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
            }
        }
    }

    double max_val_range;
    if (val < 1) {
        max_val_range = 1;
        ui->label_range->setText("μV");                                                //range fresh
    } else if (val < 100) {
        max_val_range = 100;
        ui->label_range->setText("100μV");                                              //range fresh
    } else if (val < 1000) {
        max_val_range = 1000;
        ui->label_range->setText("mV");                                                 //range fresh
    } else {
        max_val_range = 10000;
        ui->label_range->setText("10mV");                                               //range fresh
    }
    ui->progressBar->setValue(val * 100 / max_val_range);

//    ui->label_range->setText(QString("%1").arg(data->recv_para.ldata1_max * 4 * sql_para->aaultra_sql.gain * AA_FACTOR));
}

void AAWidget::fresh_1()
{
    fresh(true);
    ui->qwtPlot->replot();
}

void AAWidget::fresh_2()
{
    fresh(false);
}

void AAWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
    qDebug()<<"AAUltrasonic max reset!";
}

void AAWidget::fresh_setting()
{
    if (sql_para->aaultra_sql.mode == single) {
        ui->comboBox->setItemText(0,tr("检测模式    [单次]"));
    } else {
        ui->comboBox->setItemText(0,tr("检测模式    [连续]"));
    }

    ui->comboBox->setItemText(1,tr("增益调节    [×%1]").arg(QString::number(sql_para->aaultra_sql.gain, 'f', 1)) );
    ui->comboBox->setItemText(2,tr("音量调节    [×%1]").arg(QString::number(sql_para->aaultra_sql.vol)));
    ui->comboBox->setItemText(3,tr("黄色报警阈值[%1]dB").arg(QString::number(sql_para->aaultra_sql.low)));
    ui->comboBox->setItemText(4,tr("红色报警阈值[%1]dB").arg(QString::number(sql_para->aaultra_sql.high)));
    ui->comboBox->setItemText(5,tr("波形记录     [%1]s").arg(sql_para->aaultra_sql.time));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 ==3) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}


