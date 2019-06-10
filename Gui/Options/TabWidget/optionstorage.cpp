#include "optionstorage.h"
#include "ui_optionstorage.h"
#include "Gui/Common/common.h"
#include "IO/File/spacecontrol.h"
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

OptionStorage::OptionStorage(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::OptionStorage)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    spaceMangagementIni();
    fresh();
}

OptionStorage::~OptionStorage()
{
    delete ui;
}

void OptionStorage::work(int d)
{
    Common::change_index(col, d, 2, 0);
    fresh();
}

void OptionStorage::do_key_ok()
{
    switch (col) {
    case 1:
        qDebug()<<"refresh!";
        reloadChart(false);
        reloadChart(true);
        break;
    case 2:
        qDebug()<<"del all!";
        Common::del_dir(DIR_DATA);
        spaceMangagementIni();
        break;
    default:
        break;
    }
    fresh();
}

void OptionStorage::do_key_cancel()
{
    col = 0;
    fresh();
//    emit quit();
}

void OptionStorage::do_key_up_down(int d)
{
    Common::change_index(col, d, 2, 0);
    fresh();
}

void OptionStorage::do_key_left_right(int d)
{
    Common::change_index(col, d, 2, 0);
    fresh();
}

void OptionStorage::fresh()
{
    switch (col) {
    case 0:         //退出
        ui->pbt_del_all->setStyleSheet(pbt_style_unselect);
        ui->pbt_fresh->setStyleSheet(pbt_style_unselect);
        emit quit();
        break;
    case 1:         //刷新
        ui->pbt_del_all->setStyleSheet(pbt_style_unselect);
        ui->pbt_fresh->setStyleSheet(pbt_style_select);
        break;
    case 2:         //全部清除
        ui->pbt_del_all->setStyleSheet(pbt_style_select);
        ui->pbt_fresh->setStyleSheet(pbt_style_unselect);
        break;
    default:
        break;
    }
}

void OptionStorage::spaceMangagementIni()
{
    chart = new QChart;
//    chart->setTitle(tr("总空间3.2G"));
    chart->setAnimationOptions(QChart::AllAnimations);
    m_series = new QPieSeries();
    slice_unused = new QPieSlice(tr("未使用"), 100);
    slice_used = new QPieSlice(tr("已使用"), 0);
    *m_series << slice_unused << slice_used;
    slice_unused->setColor("#4EFF67");
    slice_unused->setBorderWidth(0);
    slice_used->setColor(Qt::red);
    slice_used->setBorderWidth(0);
    m_series->setLabelsVisible(false);
    chart->addSeries(m_series);
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0,0,0,0));
    ui->chartView->setChart(chart);

    m_series->setPieSize(1);
    m_series->setHoleSize(0.8);

    label_title = new QLabel("80%", ui->chartView);
    label_title->move(40,60);
    label_title->setMinimumWidth(100);
    label_title->setAlignment(Qt::AlignCenter);
    label_title->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:40px;color:green; font-weight: bold;}");

    label_subtitle = new QLabel(tr("可用空间3.2G"), ui->chartView);
    label_subtitle->move(40,100);
    label_subtitle->setMinimumWidth(100);
    label_subtitle->setAlignment(Qt::AlignCenter);
    label_subtitle->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:14px;color:green;}");

    reloadChart(true);
}

void OptionStorage::reloadChart(bool f)
{
    if(f){
        chart->setAnimationOptions(QChart::AllAnimations);
        float m_DataLog, m_PRPDLog, m_Asset, m_WaveForm, m_Others;
        m_DataLog = SpaceControl::dirFileSize(DIR_DATALOG);
        m_PRPDLog = SpaceControl::dirFileSize(DIR_PRPDLOG);
        m_Asset = SpaceControl::dirFileSize(DIR_ASSET);
        m_WaveForm = SpaceControl::dirFileSize(DIR_WAVE);
        m_Others = qAbs(SpaceControl::dirFileSize(DIR_USB) - m_WaveForm - m_Asset - m_PRPDLog - m_DataLog);

        qint64 total, used, available;
        QString persent;
        SpaceControl::disk_info(total, used, available, persent);

        ui->label_data->setText(SpaceControl::size_to_string(m_DataLog));
        ui->label_PRPD->setText(SpaceControl::size_to_string(m_PRPDLog));
        ui->label_Asset->setText(SpaceControl::size_to_string(m_Asset));
        ui->label_Waveform->setText(SpaceControl::size_to_string(m_WaveForm));
        ui->label_Others->setText(SpaceControl::size_to_string(m_Others));

        int a = 100.0 * available / total;
        int u = 100.0 * used / total;

    //    qDebug()<<"available:"<<available<<"\tused:"<<used << "\ttotal:"<< total;
        slice_used->setValue(u);
        slice_unused->setValue(a);

        label_title->setText(QString::number(a) + "%");
        label_subtitle->setText(tr("可用空间") + SpaceControl::size_to_string(available * 1000));
    //            qDebug()<<"persent"<<a;
    //    chart->setTitle(tr("总空间%1").arg(SpaceControl::size_to_string((available + used) * 1000)));
    }
    else{
        chart->setAnimationOptions(QChart::NoAnimation);
        slice_used->setValue(0);
        slice_unused->setValue(100);
    }
}
