#include "multimachineclient.h"

MultiMachineClient::MultiMachineClient(QObject *parent) : QObject(parent)
{
    startTimer(5000);

    tcpSocket = new QTcpSocket;
    tcpSocket->connectToHost("192.168.20.1", 6555);
}

void MultiMachineClient::write(QByteArray array)
{
    if(tcpSocket->state() == QAbstractSocket::ConnectedState){
//        qDebug()<<"MultiMachineClient::write"<<array;
        tcpSocket->write(array);
    }
}

//用于重连
void MultiMachineClient::timerEvent(QTimerEvent *)
{
    qDebug()<<"MultiMachineClient::timerEvent"<<tcpSocket->state();
    if(tcpSocket->state() == QAbstractSocket::UnconnectedState){
        tcpSocket->connectToHost("192.168.20.1", 6555);
    }
}
