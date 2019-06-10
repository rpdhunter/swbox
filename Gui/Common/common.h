﻿#ifndef COMMON_H
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
#include <QRadioButton>
#include "IO/Com/rdb/rdb.h"
#include "Algorithm/compute.h"

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

enum PRPD_MODE{
    PRPD_single,
    PRPD_double,
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
    static void set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value, PRPD_MODE mode = PRPD_double);                      //完成PRPD的样式设置
    static void set_fly_style(QwtPlot *plot, QwtPlotSpectroCurve *d_fly, int max_value);                      //完成飞行图的样式设置
    static void set_TF_style(QwtPlot *plot, QwtPlotSpectroCurve *d_TF, int max_value);                      //完成TF的样式设置
    static void set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram, int xBottom_min, int xBottom_max,
                                    int yLeft_min, int yLeft_max, QString title);                      //完成histogram的样式设置
    static void set_Spectra_style(QwtPlot *plot, QwtPlotHistogram *d_histogram, int xBottom_min, int xBottom_max,
                                    int yLeft_min, int yLeft_max, QString title);                      //完成频谱图的样式设置

    static void setTab(QLabel *label);              //设置tabwidget的标签格式

    static double physical_value(int code_value, MODE mode);        //根据码值返回物理值
    static double physical_value(int code_value, double gain, MODE mode);        //根据码值返回物理值
    static int code_value(double physical_value, MODE mode);        //根据物理值返回码值
    static double physical_threshold(MODE mode);                    //返回各模式下的物理阈值
    static qint32 offset_zero_code(MODE mode);                      //返回各模式下的零偏置（码值）
    static QString mode_to_string(MODE val);
    static MODE string_to_mode(QString str);
    static void write_fpga_offset_debug(G_PARA *data);                    //根据当前通道设置fpga参数，debug界面中设置的fpga参数
    static int time_to_phase(quint32 x);             //时标到相位转换(相位没有转化到0-360°)
    static QVector<int> smooth(QVector<int> datalist, int n);             //平滑滤波
    static QVector<int> smooth_2(QVector<int> datalist, int n);             //2阶平滑滤波

    #define KALMAN_Q 0.02       //Q:过程噪声，Q增大，动态响应变快，收敛稳定性变坏
    #define KALMAN_R 12.0000     //R:测量噪声，R增大，动态响应变慢，收敛稳定性变好
    /* 卡尔曼滤波处理 */
    static float kalman_filter_core (float ResrcData, float ProcessNiose_Q, float MeasureNoise_R, float &x_last, float &p_last);
    static QVector<int> kalman_filter (QVector<int> wave);

    static double tev_freq_compensation(int pulse_num);      //tev频率补偿
    static QString secton_three(int n);     //三位分节法显示数字
    static void create_hard_link(QString str_src, QString file_name);        //在资产空间创建硬连接，str_src是源文件完整路径，file_name是文件名
    static QLabel *set_mainwindow_lab(QLabel *lab, int n, QTabWidget *widget);

    static QString str_to_cn(QString str);      //字符串使用utf8重新编码
    static bool mk_dir(QString path);
    static bool del_dir(QString path);
    static bool rename_dir(QString old_path, QString new_path);                  //重命名文件夹

    static void rdb_set_yc_value(uint yc_no,double val,uint qc = 0);
    static void rdb_set_yc_prpd(int first_point, QVector<QPoint> list);     //保存PRPD
    //    static void rdb_set_yc_value(uint yc_no,int val,uint qc = 0);
    static qint32 merge_int32(qint16 a, qint16 b);      //合并2个16位成为一个32位
    static double rdb_get_yc_value(uint yc_no);     //查得yc值
    static int rdb_get_yc_qc(uint yc_no);         //查得品质值(0或1)
    static void rdb_set_yx_value(uint yc_no, uint val);
    static int rdb_check_test_start();
    static void rdb_set_dz_value(uint dz_no, char val);        //修改rdb设定值
    static void rdb_dz_init();          //初始化rdb定值表

    static void select_root(QTreeView *v, QAbstractItemModel *model);
    static void select_up(QTreeView *v, QAbstractItemModel *model);
    static void select_down(QTreeView *v, QAbstractItemModel *model);
    static void expand_collapse(QTreeView *v);

    static void check_base_dir();       //检测基本文件夹
    static void messagebox_show_and_init(QMessageBox *box);
    static void messagebox_switch(QMessageBox *box);
    static QString filter_to_string(int f);
    static double filter_to_number(int f);
    static void adjust_filter_list(QList<int> &list, double cut_off_low, double cut_off_high);      //修正滤波器的可选范围
    static QVector<int> set_filter(QVector<int> wave, MODE mode);            //加装滤波器

    static int time_interval(struct timeval start_time, struct timeval stop_time);      //返回两时间间隔(us)
    static void time_addusec(struct timeval &time, int usec);                //添加微秒数,可为负值
//    static quint64 dirFileSize(const QString &path);                //返回路径文件夹大小
//    static QString

    static void change_rbt_status(bool flag, QRadioButton *b0, QRadioButton *b1);       //改变一对QRadioButton的选择状态样式
    static void check_restart_file();           //检查restart标志文件

    static CHANNEL_SQL *channel_sql(MODE mode);     //返回通道的sql指针
    static int mode_to_channel(MODE mode);          //通过模式返回主界面通道
//    static double channel_factor(MODE mode);        //通道放大因数
    static QString sensor_freq_to_string(int s);
    static void save_date_time(QDateTime dt);       //保存时间日期
};

#endif // COMMON_H
