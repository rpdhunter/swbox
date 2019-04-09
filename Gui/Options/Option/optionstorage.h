#ifndef OPTIONSTORAGE_H
#define OPTIONSTORAGE_H

#include <QWidget>
#include <QtCharts/QChartGlobal>

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

class OptionStorage : public QWidget
{
    Q_OBJECT

public:
    explicit OptionStorage(QWidget *parent = 0);
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
    int col;
    void fresh();

    QChart *chart;
    QPieSeries *m_series;
    void spaceMangagementIni();     //初始化内存管理
    void reloadChart();             //重新读取chart数据
};

#endif // OPTIONSTORAGE_H
