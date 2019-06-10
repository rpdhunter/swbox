#include "multimachineserver.h"
#include "multimachinethread.h"

MultiMachineServer::MultiMachineServer(QObject *parent) : QTcpServer(parent)
{
    if(this->listen(QHostAddress::Any, 6555)) {
        qDebug()<<this->errorString(); //错误信息
    }
    qDebug()<<"server enabled!";
}

void MultiMachineServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<<"get one new Connection";
    MultiMachineThread *thread = new MultiMachineThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}
