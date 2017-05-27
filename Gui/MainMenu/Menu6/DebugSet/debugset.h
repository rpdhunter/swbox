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
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QProxyStyle>
#include <QPainter>
#include "recwaveform.h"


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
            s.rheight() = 40;
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
                    str = tr("高级");
                }
                else if(tab->text == "3"){
                    str = tr("录波数据查看");
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

    void set_offset_suggest1(int a,int b);
    void set_offset_suggest2(int a,int b);
    void set_AA_offset_suggest(int a);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
    void showWaveData(VectorList wv, MODE n);

signals:
    void fresh_parent(void);
    void startRecWv(int,int);      //开始录播信号,前一个参数是通道数，后一个参数是录播时常（仅对超声录波有用）
    void fregChanged(int);      //频率设置变化
    void send_key(quint8);
    void update_statusBar(QString);


private slots:
    void on_comboBox_currentIndexChanged(int index);

private:
    void fresh();       //刷新界面

    void iniUi();

    void resetPassword();

    void saveSql();

    void readSql(); //从SQL读取数据至UI


    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    SQL_PARA *sql_para;


    QWidget *widget;        //密码窗口
    QLabel *lab1, *lab2;
    QLineEdit *passwordEdit;
    bool pass;  //查看是否有权进入调试模式
    QString password;   //密码
    QString password_set;   //密码设定值

    RecWaveForm *recWaveForm;

    QTimer *timer;      //备用

    Ui::DebugUi *ui;

};

#endif // DEBUGSET_H
