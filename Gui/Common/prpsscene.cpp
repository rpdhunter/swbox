#include "prpsscene.h"
#include <QGraphicsTextItem>
#include <QtDebug>

PRPSScene::PRPSScene(QObject *parent) : QGraphicsScene(parent)
{
    pen.setColor(Qt::red);


//    pal.setColor(QPalette::Text,Qt::red);
//    setPalette(pal);

    axisInit();

}

void PRPSScene::addPRPD(QVector<QPointF> list)
{
    foreach (QPointF P, list) {
        if(P.x()>360 || P.x()<0 || qAbs(P.y()) > 60){
            qDebug()<<"PRPS input data error!";
            return;
        }

    }
}

void PRPSScene::axisInit()
{
//    QPoint d_P(-70,60);     //定义投影
//    QPoint P0(0,0), P1(100,0), P2(0,-70);

    QPoint d_P(-70,35);     //定义投影
    QPoint P0(0,-20), P1(100,20), P2(0,-70);

    QPolygon polygon;
    polygon << P0 << P1 << P1+d_P << P0+d_P << P2+d_P << P2;
    addPolygon(polygon,pen);

    for (int i = 0; i <= 5; ++i) {
        addLine(QLine(P0+(P1-P0)*i/5, P0+(P1-P0)*i/5 +d_P),pen);
        setText(QString("%1").arg(i*10), P0+(P1-P0)*i/5 + d_P + QPoint(-18,-5));
    }
    addLine(QLine((P0+P2)/2, (P0+P2)/2+d_P),pen);
    addLine(QLine(P0+d_P/2, P1+d_P/2),pen);
    setText(QString("360"), P1);
    setText(QString("180"), P1+d_P/2);
    setText(QString("  0"), P1+d_P);
    addLine(QLine(P0+d_P/2, P2+d_P/2),pen);

}

void PRPSScene::setText(QString str, QPointF P)
{
    QGraphicsTextItem *text = this->addText(str);
    text->setDefaultTextColor(Qt::gray);
    text->setPos(P);
}

void PRPSScene::transData(QPointF &P)
{

}
