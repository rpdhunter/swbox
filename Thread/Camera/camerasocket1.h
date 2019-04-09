#ifndef CAMERASOCKET1_H
#define CAMERASOCKET1_H

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QSerialPort>
#include <QThread>
#include <QTimer>
#include "cameradata.h"


class CameraSocket1 : public QObject
{
    Q_OBJECT
public:
    CameraSocket1(QObject *parent = NULL);
    ~CameraSocket1();
    void connect_camera();

private slots:
    void  keep_alive();

signals:
    void sendOnePacket(QByteArray packet);

private:

    int connect_socket();
    void  Login_Req(int fd, char *user, char *password);

    void  link_one(char *src);
    void  link_two(char *src1, char *src2);
    void  link_two_1(char *src1, char *src2);
    void  link_three(char *src1,char *src2);
    void  link_four(char *src);
    void  link_four_1(char *src);
    void  get_control_flow(char *buf, int id, int seq_num, char tol_pck, char cur_pck, unsigned short mes_id, int data_len);
    int   socket_write (int fd, char * buf, int len);
    void  KeepAlive(int fd, int id);
    void  Monitor_Claim(int fd, int id, char *act, unsigned short mes_id);


    CameraData *data;
    QTimer *timer_alive;
};

#endif // CAMERASOCKET1_H
