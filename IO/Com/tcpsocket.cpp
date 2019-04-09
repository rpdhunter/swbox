#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#include <QSerialPortInfo>
#include <QSerialPort>
#include "tcpsocket.h"
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

TcpSocket::TcpSocket(QObject *parent) : QThread(parent)
{

}

TcpSocket::~TcpSocket()
{

}

void TcpSocket::run()
{
    int fd;
    int socket_fd = -1;
    int ret;
    int reuse = 1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        qDebug()<<"socket is error!";
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr=inet_addr("192.168.150.1");
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(6991);
    ret = bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret != 0){
        qDebug()<<"bind is error!";
    }

    ret = listen(fd,5);
    if(ret != 0){
        qDebug()<<"listen is error!";
    }

    struct sockaddr_in client_addr;
    socklen_t  cliaddr_len = sizeof(client_addr);
    qDebug()<<"running accept";
    while(1)
    {
        socket_fd = accept(fd, (struct sockaddr*)&client_addr, &cliaddr_len);
        if(socket_fd < 0){
            qDebug()<<"connect is error";
        }
        emit connect_ok(socket_fd);
    }
}
