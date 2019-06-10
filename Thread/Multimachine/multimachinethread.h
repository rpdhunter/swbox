#ifndef MULTIMACHINETHREAD_H
#define MULTIMACHINETHREAD_H

#include <QThread>
#include <QTcpSocket>

class MultiMachineThread : public QThread
{
    Q_OBJECT
public:
    explicit MultiMachineThread(int socketDescriptor, QObject *parent = nullptr);

    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);

private slots:
    void deal_data();

private:
    QTcpSocket *tcpSocket;
    int socketDescriptor;
};

#endif // MULTIMACHINETHREAD_H
