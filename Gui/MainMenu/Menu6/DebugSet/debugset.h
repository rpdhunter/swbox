#ifndef DEBUGSET_H
#define DEBUGSET_H

#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTimer>
#include <QVector>
#include "IO/Key/key.h"
#include "IO/Data/fifodata.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QProxyStyle>
#include <QPainter>


class QwtPlot;
class QwtPlotCurve;

namespace Ui {
class DebugUi;
}

class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
        const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
            s.transpose();
            s.rwidth() = 120; // 设置每个tabBar中item的大小
            s.rheight() = 44;
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel) {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
                QRect allRect = tab->rect;

                if (tab->state & QStyle::State_Selected) {
                    painter->save();
                    painter->setPen(0x89cfff);
//                    painter->setBrush(QBrush(0x89cfff));
                    painter->setBrush(QBrush(Qt::darkCyan));
                    painter->drawRect(allRect.adjusted(6, 6, -6, -6));
                    painter->restore();
                }
                QTextOption option;
                option.setAlignment(Qt::AlignCenter);
                if (tab->state & QStyle::State_Selected) {
                    painter->setPen(0xf8fcff);
                }
                else {
                    painter->setPen(0x5d5d5d);
                }


                //这里很奇怪，只能显示2个字符，所以迫不得已做了这个转换
                QString str;
                if(tab->text == "0"){
                    str = tr("TEV");
                }
                else if(tab->text == "1"){
                    str = tr("AA超声");
                }
                else if(tab->text == "2"){
                    str = tr("录波");
                }
                painter->drawText(allRect, str, option);
//                qDebug()<<"tab text is:"<<tab->text;
                return;
            }
        }

        if (element == CE_TabBarTab) {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
};


class DebugSet : public QFrame
{
    Q_OBJECT
public:
    DebugSet(QWidget *parent = NULL, G_PARA *g_data = NULL);

    void set_offset_suggest(int a,int b);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
    void showWaveData(quint32 *wv, int, int n);
    void plotPrapare();     //准备绘图,完成倒计时功能

signals:
    void fresh_parent(void);
    void startRecWv(int);      //开始录播信号
    void fregChanged(int);      //频率设置变化

private:
    void plotShow();    //绘图

    void fresh();       //刷新界面

    void iniUi();

    void resetPassword();
    CURRENT_KEY_VALUE *key_val;

    SQL_PARA *sql_para;

    QWidget *widget;        //密码窗口
    QLabel *lab1, *lab2;
    QLineEdit *passwordEdit;
    bool pass;  //查看是否有权进入调试模式
    QString password;   //密码
    QString password_set;   //密码设定值

    G_PARA *data;

    int time_c; //内置的计时器,数值为0时,开始绘图

    QwtPlot *plot;
    QVector<QPointF> wave;
    QwtPlotCurve *curve;

    Ui::DebugUi *ui;

};

#endif // DEBUGSET_H
