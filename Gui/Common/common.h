#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QComboBox>
#include <qwt_plot.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_histogram.h>
#include <QLabel>
#include <QDir>
#include <QListWidget>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QTreeView>
#include <QAbstractItemModel>
#include <QMessageBox>

#define FREEZE_TIME     5000            //秒界面锁定时间

class MyKey{
public:
    MyKey(int X=0,int Y=0){
        x = X;
        y = Y;
    }
    bool operator < (const MyKey another) const
    {
        if(this->x < another.x ){
            return true;
        }
        else if(this->x > another.x){
            return false;
        }
        else if(this->y < another.y){       //this->x == another.x
            return true;
        }
        else {                              //this->x == another.x
            return false;
        }
    }

    bool operator > (const MyKey another) const
    {
        if(this->x > another.x ){
            return true;
        }
        else if(this->x < another.x){
            return false;
        }
        else if(this->y > another.y){       //this->x == another.x
            return true;
        }
        else {                              //this->x == another.x
            return false;
        }
    }

    bool operator == (const MyKey another) const
    {
        if(this->x == another.x && this->y == another.y){
            return true;
        }
        else {
            return false;
        }
    }

    int x;
    int y;
};


class Common : public QObject
{
public:
    explicit Common(QObject *parent = 0);

    static void change_index(unsigned char &index, int d_index,     //完成index的循环增减功能
                             int max_index, int min_index);
    static void change_index(double &index, double d_index,     //完成index的循环增减功能
                             double max_index, double min_index);
    static void change_index(int &index, int d_index,     //完成index的循环增减功能
                             int max_index, int min_index);
    static void change_index(int &index, int d_index,     //完成index的循环增减功能
                             QList<int> list);
    static void change_value(int &value, int value_a, int value_b); //完成value的档位变化
    static void change_value(bool &value, bool value_a, bool value_b); //完成value的档位变化

    static void set_comboBox_style(QComboBox *comboBox);                  //完成QComboBox的样式设置
    static void set_contextMenu_style(QListWidget *w, QStringList list, QPoint pos);
    static void set_barchart_style(QwtPlot *plot, int v_max);                  //完成barchart的样式设置
    static void set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value);                      //完成PRPD的样式设置
    static void set_fly_style(QwtPlot *plot, QwtPlotSpectroCurve *d_fly, int max_value);                      //完成飞行图的样式设置
    static void set_TF_style(QwtPlot *plot, QwtPlotSpectroCurve *d_TF, int max_value);                      //完成TF的样式设置
    static void set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram, int xBottom_min, int xBottom_max,
                                    int yLeft_min, int yLeft_max, QString title);                      //完成histogram的样式设置
    static void set_Spectra_style(QwtPlot *plot, QwtPlotHistogram *d_histogram, int xBottom_min, int xBottom_max,
                                    int yLeft_min, int yLeft_max, QString title);                      //完成频谱图的样式设置

    static void setTab(QLabel *label);              //设置tabwidget的标签格式

    static double physical_value(int code_value, MODE mode);        //根据码值返回物理值
    static int code_value(double physical_value, MODE mode);        //根据物理值返回码值
    static double physical_threshold(MODE mode);                    //返回各模式下的物理阈值
    static qint32 offset_zero_code(MODE mode);                      //返回各模式下的零偏置（码值）
    static QString MODE_toString(MODE val);
    static void write_fpga_offset_debug(G_PARA *data);                    //根据当前通道设置fpga参数，debug界面中设置的fpga参数
    static void calc_aa_value (G_PARA *data, MODE mode, L_CHANNEL_SQL *x_sql, double * aa_val, double * aa_db, int * offset);
    static QVector<QPoint> calc_pulse_list(QVector<int> datalist, QVector<int> timelist, int threshold);          //根据给出的序列和阈值计算脉冲序列
    static QVector<QPoint> calc_pulse_list(QVector<int> datalist, int threshold);          //根据给出的序列和阈值计算脉冲序列
    static int time_to_phase(int x);             //时标到相位转换
    static QVector<int> smooth(QVector<int> datalist, int n);             //平滑滤波
    static QVector<int> smooth_2(QVector<int> datalist, int n);             //2阶平滑滤波

    #define KALMAN_Q 0.02       //Q:过程噪声，Q增大，动态响应变快，收敛稳定性变坏
    #define KALMAN_R 12.0000     //R:测量噪声，R增大，动态响应变慢，收敛稳定性变好
    /* 卡尔曼滤波处理 */
    static float kalman_filter_core (float ResrcData, float ProcessNiose_Q, float MeasureNoise_R, float &x_last, float &p_last);
    static QVector<int> kalman_filter (QVector<int> wave);
    static double avrage(QVector<double> list);
    static int avrage(QVector<int> list);
    static double tev_freq_compensation(int pulse_num);      //tev频率补偿
    static QString secton_three(int n);     //三位分节法显示数字
    static void create_hard_link(QString str_src, QString file_name);        //在资产空间创建硬连接，str_src是源文件完整路径，file_name是文件名
    static QLabel *set_mainwindow_lab(QLabel *lab, int n, QTabWidget *widget);

    static QString str_to_cn(QString str);      //字符串使用utf8重新编码
    static bool mk_dir(QString path);
    static bool del_dir(QString path);
    static bool rename_dir(QString old_path, QString new_path);                  //重命名文件夹
    static int max_at(QVector<double> list);            //找到最大值的位置，返回序号
    static int max_at(QVector<int> list);            //找到最大值的位置，返回序号
    static void rdb_set_value(uint yc_no,double val,uint qc = 0);

    static void select_root(QTreeView *v, QAbstractItemModel *model);
    static void select_up(QTreeView *v, QAbstractItemModel *model);
    static void select_down(QTreeView *v, QAbstractItemModel *model);
    static void expand_collapse(QTreeView *v);

    static void check_base_dir();       //检测基本文件夹
    static void messagebox_show_and_init(QMessageBox *box);
    static void messagebox_switch(QMessageBox *box);

};

#endif // COMMON_H
