#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QComboBox>
#include <qwt_plot.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_histogram.h>
#include <QLabel>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"

#define FREEZE_TIME     5000            //秒界面锁定时间

#if 0
/* 路径等定义 */
#define BASE_DIR			"."
#define LOGO_DIR			BASE_DIR"/logo"
#define LOGO_NAME			"logo.png"
#define LOGO_SML_NAME		"logo_small.png"

/* zdit定义 */
#define ZDIT_NAME			"ZIT(Nanjing) Technology Co., Ltd"
#define ZDIT_NAME_SHORT		"ZDIT"
#define ZDIT_LOGO_PATH		LOGO_DIR"/zdit/"LOGO_NAME
#define ZDIT_LOGO_SML_PATH	LOGO_DIR"/zdit/"LOGO_SML_NAME

#define ZDIT_PD_MOD_ALL		"iPD3000"
#define ZDIT_PD_MODEL		ZDIT_PD_MOD_ALL

/* ohv定义 */
#define OHV_NAME			"Ohv Diagnostic GmbH ,Ltd"
#define OHV_NAME_SHORT		"OHV"
#define OHV_LOGO_PATH		LOGO_DIR"/ohv/"LOGO_NAME
#define OHV_LOGO_SML_PATH	LOGO_DIR"/ohv/"LOGO_SML_NAME

#define OHV_PD_MOD_TEV		"PDTEV"
#define OHV_PD_MOD_CAB		"PDCABLE"
#define OHV_PD_MODEL		OHV_PD_MOD_TEV

/* amg定义 */
#define AMG_NAME			"Australian Microgrid Technology Pty Lte"
#define AMG_NAME_SHORT		"AMG"
#define AMG_LOGO_PATH		LOGO_DIR"/amg/"LOGO_NAME
#define AMG_LOGO_SML_PATH	LOGO_DIR"/amg/"LOGO_SML_NAME

#define AMG_PD_MOD_TEV		"PDTev"
#define AMG_PD_MOD_CAB		"PDCable"
#define AMG_PD_MODEL		AMG_PD_MOD_CAB

/* 版本定义 */
#define SW_VERSION			"2.1"
#define HW_VERSION			"1.2"

typedef struct oem_info_e {
    char *	oem_name;			/* 全名，如Ohv Diagnostic GmbH ,Ltd */
    char *	oem_name_short;		/* 简称，如OHV */
    char *	logo_path;			/* logo图片所在路径 */
    char *	logo_sml_path;		/* 小logo路径 */
    char *	model;				/* 型号 */
    char * 	sw_ver;				/* 软件版本 */
    char *	hw_ver;				/* 硬件版本 */
} oem_info_t;

enum oem_index_e {
    oem_none = 0,
    oem_zdit,
    oem_ohv,
    oem_amg,

    oem_num
};

oem_info_t oem_list [oem_num] = {
    /* oem_zdit */
    {
        ZDIT_NAME, ZDIT_NAME_SHORT, ZDIT_LOGO_PATH, ZDIT_LOGO_SML_PATH,
        ZDIT_PD_MODEL, /* 请注意修改为实际型号 */
        SW_VERSION, HW_VERSION},

    /* oem_ohv */
    {
        OHV_NAME, OHV_NAME_SHORT, OHV_LOGO_PATH, OHV_LOGO_SML_PATH,
        OHV_PD_MODEL, /* 请注意修改为实际型号 */
        SW_VERSION, HW_VERSION},

    /* oem_amg */
    {
        AMG_NAME, AMG_NAME_SHORT, AMG_LOGO_PATH, AMG_LOGO_SML_PATH,
        AMG_PD_MODEL, /* 请注意修改为实际型号 */
        SW_VERSION, HW_VERSION},
};
#endif

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
    static void change_value(int &value, int value_a, int value_b); //完成value的档位变化
    static void change_value(bool &value, bool value_a, bool value_b); //完成value的档位变化

    static void set_comboBox_style(QComboBox *comboBox);                  //完成QComboBox的样式设置
    static void set_barchart_style(QwtPlot *plot, int v_max);                  //完成barchart的样式设置
    static void set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value);                      //完成PRPD的样式设置
    static void set_TF_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value);                      //完成TF的样式设置
    static void set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram);                      //完成histogram的样式设置
    static void setTab(QLabel *label);              //设置tabwidget的标签格式

    static double physical_value(int code_value, MODE mode);        //根据码值返回物理值
    static int code_value(double physical_value, MODE mode);        //根据物理值返回码值
    static double physical_threshold(MODE mode);                    //返回各模式下的物理阈值
    static QString MODE_toString(MODE val);

};

#endif // COMMON_H
