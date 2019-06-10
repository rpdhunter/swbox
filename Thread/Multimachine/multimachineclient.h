#ifndef MULTIMACHINECLIENT_H
#define MULTIMACHINECLIENT_H

#include <QObject>
#include <QTcpSocket>

class MultiMachineClient : public QObject
{
    Q_OBJECT
public:
    explicit MultiMachineClient(QObject *parent = nullptr);

    void write(QByteArray array);       //发送报文

signals:

public slots:

protected:
    void timerEvent(QTimerEvent *);

private:
    QTcpSocket *tcpSocket;
};

#endif // MULTIMACHINECLIENT_H
