#ifndef TESTTOOLS_H
#define TESTTOOLS_H

#include <QObject>
#include "IO/Data/data.h"

//测试工具类
class TestTools : public QObject
{
    Q_OBJECT
public:
    explicit TestTools(QObject *parent = nullptr);

    static void save_test_file(QVector<int> list, QString path = DIR_TEST);
    static void save_test_file(QVector<QPoint> list, QString path = DIR_TEST);

signals:

public slots:

};

#endif // TESTTOOLS_H
