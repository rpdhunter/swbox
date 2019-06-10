#include "prpsscene.h"
#include <QGraphicsTextItem>
#include <QtDebug>
#include <QtMath>
#include "IO/SqlCfg/sqlcfg.h"


LineItem::LineItem(QColor color, QLineF line, QGraphicsItem *parent) : QGraphicsLineItem(line, parent)
{
    QPen pen(color);
    setPen(pen);
    myColor = color;
    myLine = line;
    myNum = 0;
}

PRPSScene::PRPSScene(MODE mode, int data_max, QObject *parent) : QGraphicsScene(parent)
{
    this->mode = mode;

    pen_gray.setColor(Qt::gray);
    pen_red.setColor(Qt::red);
    pen_yellow.setColor(Qt::yellow);
    pen_green.setColor(Qt::green);

    this->data_max = data_max;

    axisInit();
    data_group_num = 10 * 5;     //场景容量
}

//绘制坐标轴
void PRPSScene::axisInit()
{
    P_shadow = QPointF(-70,35);      //定义投影
    P0 = QPointF(20,-20);        //定义原点
    Px_max = P0 + QPointF(100,40);
    Py_max = P0 + QPointF(0,-48);

    QPolygonF polygon;
    polygon << P0 << Px_max << Px_max+P_shadow << P0+P_shadow << Py_max+P_shadow << Py_max;
    addPolygon(polygon,pen_gray);

    for (int i = 0; i <= 5; ++i) {
        addLine(QLineF(P0+(Px_max-P0)*i/5, P0+(Px_max-P0)*i/5 +P_shadow),pen_gray);
        setText(QString("%1").arg(i*10), P0+(Px_max-P0)*i/5 + P_shadow + QPoint(-18,-5));
    }
    addLine(QLineF((P0+Py_max)/2, (P0+Py_max)/2+P_shadow),pen_gray);
    addLine(QLineF(P0+P_shadow/2, Px_max+P_shadow/2),pen_gray);
    setText(QString("360"), Px_max);
    setText(QString("180"), Px_max+P_shadow/2);
    setText(QString("  0"), Px_max+P_shadow);
    addLine(QLineF(P0+P_shadow/2, Py_max+P_shadow/2),pen_gray);

    QPointF P_adjust(-25,-10);
//    if(mode == TEV1 || mode == TEV2 || mode == UHF1 || mode == UHF2){
//        data_max = 60;
//    }
//    else if(mode == HFCT1 || mode == HFCT2){
//        data_max = hfct_max;
//    }

    if(data_max > 1000){
        setText(QString("%1k").arg(data_max/1000), Py_max+P_shadow + P_adjust);
        setText(QString("%1k").arg(data_max/2000), (Py_max + P0)/2 + P_shadow + P_adjust);
    }
    else{
        setText(QString("%1").arg(data_max), Py_max+P_shadow + P_adjust);
        setText(QString("%1").arg(data_max/2), (Py_max + P0)/2 + P_shadow + P_adjust);
    }

//    QPointF P_adjust(-20,-10);
//    if(mode == HFCT1 || mode == HFCT2){
//        P_adjust = P_adjust + QPointF(-10,0);
//    }
//    setText(QString("%1").arg(data_max), Py_max+P_shadow + P_adjust);
//    setText(QString("%1").arg(data_max/2), (Py_max + P0)/2 + P_shadow + P_adjust);
}

/*****************************************
 * 输入数据要求:
 * 1.同周期(20ms)的一组脉冲数据
 * 2.每个脉冲数据用点QPointF的模式保存
 * 3.x保存时间分量,范围0-360
 * 4.y保存幅值分量,范围可变,TEV模式下,范围是-60-60
 * **************************************/
void PRPSScene::addPRPD(QVector<QPoint> list)
{
    //移动
    QList<QGraphicsItem *> itemList = items();

//    qDebug()<<"PRPS itemList:"<<itemList.count();
    qreal dx = -(Px_max-P0).x() / data_group_num;
    qreal dy = -(Px_max-P0).y() / data_group_num;
//    qDebug()<<dx<<"\t"<<dy;

    foreach (QGraphicsItem *item, itemList) {
        if(item->type() == LineItem::Type){
            item->moveBy( dx, dy);
            LineItem *_item = qgraphicsitem_cast<LineItem *>(item);
            _item->addNum();
            if(_item->num() == data_group_num + 1){     //超出坐标便移除
                removeItem(_item);
            }
        }
    }

    //新建
    foreach (QPoint P, list) {

        P.setX(P.x() % 360 );
        P.setY(qAbs(P.y()) );

        if(qAbs(P.y()) > data_max){
//            qDebug()<<"PRPS input data error! P.y="<<qAbs(P.y());
            P.setY(data_max);
        }

        QPointF P_t = transData(P);

        QColor c;
        if( P.y() < low ){
            c = Qt::green;
        }
        else if( P.y() < high ){
            c = Qt::yellow;
        }
        else{
            c = Qt::red;
        }
        LineItem *item = new LineItem(c, QLineF(P_t + Px_max-P0, P_t + Px_max-P0 + (Py_max-P0) * ( 1.0* P.y()/data_max ) ) );
        addItem(item);
    }
}

void PRPSScene::setText(QString str, QPointF P)
{
    QGraphicsTextItem *text = this->addText(str);
    text->setDefaultTextColor(Qt::gray);
    text->setPos(P);
}

//将逻辑脉冲点转换成物理坐标点
QPointF PRPSScene::transData(QPoint P)
{
    return ( P0 + P_shadow * (360.0 - P.x()) / 360.0 );
}




