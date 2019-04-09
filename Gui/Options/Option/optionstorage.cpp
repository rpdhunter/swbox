#include "optionstorage.h"
#include "ui_optionstorage.h"
#include "Gui/Common/common.h"
#include "IO/File/spacecontrol.h"
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

OptionStorage::OptionStorage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionStorage)
{
    ui->setupUi(this);

    col = 0;
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
        qDebug()<<"smart del!";
        spaceMangagementIni();
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
    Common::change_index(col, d, 2, 1);
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
        ui->pbt_del_smart->setStyleSheet("");
        ui->pbt_del_all->setStyleSheet("");
        emit quit();
        break;
    case 1:         //智能清理
        ui->pbt_del_smart->setStyleSheet("QPushButton {background-color:gray;}");
        ui->pbt_del_all->setStyleSheet("");
        break;
    case 2:         //全部清除
        ui->pbt_del_smart->setStyleSheet("r");
        ui->pbt_del_all->setStyleSheet("QPushButton {background-color:gray;}");
        break;
    default:
        break;
    }
}

void OptionStorage::spaceMangagementIni()
{
    chart = new QChart;
    chart->setTitle(tr("总空间3.2G"));
    chart->setAnimationOptions(QChart::AllAnimations);
    m_series = new QPieSeries();
    *m_series << new QPieSlice(tr("已使用"), 0);
    *m_series << new QPieSlice(tr("未使用"), 100);
    m_series->slices().at(0)->setColor(Qt::blue);
    m_series->slices().at(1)->setColor(Qt::darkGray);
    m_series->setLabelsVisible();
    chart->addSeries(m_series);
    chart->legend()->hide();
    chart->setBackgroundBrush(Qt::lightGray);
    chart->setMargins(QMargins(0,0,0,0));
    ui->chartView->setChart(chart);

    m_series->setPieSize(0.5);

    reloadChart();
}

void OptionStorage::reloadChart()
{
    float m_DataLog, m_PRPDLog, m_Asset, m_WaveForm, m_Others;
    m_DataLog = SpaceControl::dirFileSize(DIR_DATALOG);
    m_PRPDLog = SpaceControl::dirFileSize(DIR_PRPDLOG);
    m_Asset = SpaceControl::dirFileSize(DIR_ASSET);
    m_WaveForm = SpaceControl::dirFileSize(DIR_WAVE);
    m_Others = qAbs(SpaceControl::dirFileSize(DIR_DATA) - m_WaveForm - m_Asset - m_PRPDLog - m_DataLog);

    qint64 total, used, available;
    QString persent;
    SpaceControl::disk_info(total, used, available, persent);

    ui->label_data->setText(SpaceControl::size_to_string(m_DataLog));
    ui->label_PRPD->setText(SpaceControl::size_to_string(m_PRPDLog));
    ui->label_Asset->setText(SpaceControl::size_to_string(m_Asset));
    ui->label_Waveform->setText(SpaceControl::size_to_string(m_WaveForm));
    ui->label_Others->setText(SpaceControl::size_to_string(m_Others));

    ui->label_Available->setText(SpaceControl::size_to_string(available * 1000));

    m_series->slices().at(0)->setValue(used);
    m_series->slices().at(1)->setValue(available);
    chart->setTitle(tr("总空间%1").arg(SpaceControl::size_to_string((available + used) * 1000)));
}
