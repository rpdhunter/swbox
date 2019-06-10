#include "multimachinethread.h"
#include <QDataStream>
#include <QPoint>
#include "Gui/Common/common.h"

MultiMachineThread::MultiMachineThread(int socketDescriptor, QObject *parent) : QThread(parent), socketDescriptor(socketDescriptor)
{
}

void MultiMachineThread::run()
{
    tcpSocket = new QTcpSocket;
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(deal_data()));
    if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
        emit error(tcpSocket->error());
        return;
    }

    while (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->waitForReadyRead();
    }

    tcpSocket->disconnectFromHost();
    tcpSocket->waitForDisconnected();
}

void MultiMachineThread::deal_data()
{
    QByteArray array = tcpSocket->readAll();
    qDebug()<<"MultiMachineServer get data! size =" << array.count();
    QDataStream in(&array,QIODevice::ReadWrite);
    in.setVersion(QDataStream::Qt_5_9);
    int val, pulse;
    int ch;
    QPoint P;
    in >> ch;       //通道号
    in >> val;      //主测量值
    in >> pulse;    //脉冲数

    qDebug()<< "ch=" << ch;
    qDebug()<< "val=" << val;
    qDebug()<< "pulse=" << pulse;
    QVector<QPoint> pulse_1000ms;
    while(!in.atEnd()){
        in >> P;
        pulse_1000ms.append(P);
        qDebug()<<P;
    }

    qDebug()<< pulse_1000ms;

    switch (ch) {
    case 3:
        Common::rdb_set_yc_value(HFCT3_amplitude_yc,val,0);
        Common::rdb_set_yc_value(HFCT3_num_yc,pulse,0);
        Common::rdb_set_yc_prpd(HFCT3_RPRD1_yc, pulse_1000ms);
        break;
    case 4:
        Common::rdb_set_yc_value(HFCT4_amplitude_yc,val,0);
        Common::rdb_set_yc_value(HFCT4_num_yc,pulse,0);
        Common::rdb_set_yc_prpd(HFCT4_RPRD1_yc, pulse_1000ms);
        break;
    case 5:
        Common::rdb_set_yc_value(HFCT5_amplitude_yc,val,0);
        Common::rdb_set_yc_value(HFCT5_num_yc,pulse,0);
        Common::rdb_set_yc_prpd(HFCT5_RPRD1_yc, pulse_1000ms);
        break;
    case 6:
        Common::rdb_set_yc_value(HFCT6_amplitude_yc,val,0);
        Common::rdb_set_yc_value(HFCT6_num_yc,pulse,0);
        Common::rdb_set_yc_prpd(HFCT6_RPRD1_yc, pulse_1000ms);
        break;
    default:
        break;
    }


}
