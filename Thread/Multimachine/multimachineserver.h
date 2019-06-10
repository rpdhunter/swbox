#ifndef MULTIMACHINESERVER_H
#define MULTIMACHINESERVER_H

#include <QTcpServer>

class MultiMachineServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MultiMachineServer(QObject *parent = nullptr);

signals:

public slots:

protected:
    void incomingConnection(qintptr socketDescriptor) override;

};

#endif // MULTIMACHINESERVER_H
