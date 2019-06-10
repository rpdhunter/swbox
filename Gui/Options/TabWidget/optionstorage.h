#ifndef OPTIONSTORAGE_H
#define OPTIONSTORAGE_H

#include "tabwidget.h"
#include <QtCharts/QChartGlobal>
#include <QLabel>

namespace Ui {
class OptionStorage;
}

QT_CHARTS_BEGIN_NAMESPACE
class QChart;
class QChartView;
class QPieSeries;
class QPieSlice;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class OptionStorage : public TabWidget
{
    Q_OBJECT

public:
    explicit OptionStorage(SQL_PARA *sql, QWidget *parent = 0);
    ~OptionStorage();

    void work(int d);
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);

signals:
    void quit();        //完全退出的信号
    void save();        //保存设置

private:
    Ui::OptionStorage *ui;
    void fresh();

    QChart *chart;
    QPieSeries *m_series;
    QPieSlice *slice_unused, *slice_used;
    QLabel *label_title;
    QLabel *label_subtitle;
    void spaceMangagementIni();     //初始化内存管理
    void reloadChart(bool f);             //重新读取chart数据
};

#endif // OPTIONSTORAGE_H
