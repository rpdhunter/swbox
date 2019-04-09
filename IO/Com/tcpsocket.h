#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QSerialPort>
#include <QThread>

class TcpSocket : public QThread
{
    Q_OBJECT
public:
    TcpSocket(QObject *parent = NULL);

    ~TcpSocket();

signals:
    void connect_ok(int m);

protected:
    void run(void);
};


#endif // TCPSOCKET_H

