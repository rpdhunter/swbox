#ifndef PRPSSCENE_H
#define PRPSSCENE_H

#include <QGraphicsScene>
#include <QPalette>
#include "IO/Data/data.h"

class PRPSScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit PRPSScene(QObject *parent = 0);

signals:

public slots:
//    void addPRPD(VectorList list_x, VectorList list_y);     //添加一组PRPD数据
    void addPRPD(QVector<QPointF> list);     //添加一组PRPD数据

private:
    void axisInit();
    void setText(QString str, QPointF P);
    void transData(QPointF &P);
    QPalette pal;
    QPen pen;
};

#endif // PRPSSCENE_H
