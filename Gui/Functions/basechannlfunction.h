#ifndef BASECHANNLFUNCTION_H
#define BASECHANNLFUNCTION_H

#include <QObject>

class BaseChannlFunction : public QObject
{
    Q_OBJECT
public:
    explicit BaseChannlFunction(QObject *parent = nullptr);

signals:

public slots:
};

#endif // BASECHANNLFUNCTION_H