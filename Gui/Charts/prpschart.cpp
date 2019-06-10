#include "prpschart.h"
#include <QGraphicsView>

PRPSChart::PRPSChart(QObject *parent) : BaseChart(parent)
{
    chart = NULL;
}

void PRPSChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    scene = new PRPSScene(mode,max_value);
    chart = new QGraphicsView(parent);
    chart->move(10,35);
    chart->resize(220, 160);
    chart->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chart->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    chart->setStyleSheet("background:transparent;color:gray;");
    chart->setScene(scene);
    reset_colormap();

    timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(set_data()));

    hasData = false;

    startTimer(100);        //100ms检测一次数据
}

void PRPSChart::add_data(QVector<QPoint> point_list)
{
    scene->addPRPD(point_list.mid(0,20));
    timer->start(200);
    hasData = true;
}

void PRPSChart::save_data()
{

}

void PRPSChart::reset_colormap()
{
    switch (mode) {
    case TEV1:
        scene->set_color_map(sqlcfg->get_para()->tev1_sql.high, sqlcfg->get_para()->tev1_sql.low );
        break;
    case TEV2:
        scene->set_color_map(sqlcfg->get_para()->tev2_sql.high, sqlcfg->get_para()->tev2_sql.low );
        break;
    case HFCT1:
        scene->set_color_map(sqlcfg->get_para()->hfct1_sql.high, sqlcfg->get_para()->hfct1_sql.low );
        break;
    case HFCT2:
        scene->set_color_map(sqlcfg->get_para()->hfct2_sql.high, sqlcfg->get_para()->hfct2_sql.low );
        break;
    case UHF1:
        scene->set_color_map(sqlcfg->get_para()->uhf1_sql.high, sqlcfg->get_para()->uhf1_sql.low );
        break;
    case UHF2:
        scene->set_color_map(sqlcfg->get_para()->uhf2_sql.high, sqlcfg->get_para()->uhf2_sql.low );
        break;
    case AA1:
        scene->set_color_map(sqlcfg->get_para()->aa1_sql.high, sqlcfg->get_para()->aa1_sql.low );
        break;
    case AA2:
        scene->set_color_map(sqlcfg->get_para()->aa2_sql.high, sqlcfg->get_para()->aa2_sql.low );
        break;
    case AE1:
        scene->set_color_map(sqlcfg->get_para()->ae1_sql.high, sqlcfg->get_para()->ae1_sql.low );
        break;
    case AE2:
        scene->set_color_map(sqlcfg->get_para()->ae2_sql.high, sqlcfg->get_para()->ae2_sql.low );
        break;
    default:
        break;
    }
}

void PRPSChart::reset_colormap(int high, int low)
{
    scene->set_color_map(high, low );
}

void PRPSChart::hide()
{
    if(chart != NULL)
        chart->hide();
}

void PRPSChart::show()
{
    if(chart != NULL)
        chart->show();
}

void PRPSChart::set_data()
{
    hasData = false;
}

void PRPSChart::timerEvent(QTimerEvent *)
{
    if(!hasData){         //最近没有数据输入，则输入一个空数据
        QVector<QPoint> point_list;
        scene->addPRPD(point_list);
    }
}
