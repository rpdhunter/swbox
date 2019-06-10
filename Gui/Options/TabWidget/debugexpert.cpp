#include "debugexpert.h"
#include "ui_debugexpert.h"
#include "Gui/Common/common.h"
#include "Thread/Sync/synccompute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/mii.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <netinet/in.h>
#define 	reteck(ret)     \
    if(ret < 0){    \
    printf("%m! \"%s\" : line: %d\n", __func__, __LINE__);   \
    }

DebugExpert::DebugExpert(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::DebugExpert)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    QButtonGroup *group1 = new QButtonGroup(this);
    group1->addButton( ui->eth_off );
    group1->addButton( ui->eth_on );

    QButtonGroup *group2 = new QButtonGroup(this);
    group2->addButton( ui->rbt_auto );
    group2->addButton( ui->rbt_uncharge );
    group2->addButton( ui->rbt_charge );
}

DebugExpert::~DebugExpert()
{
    delete ui;
}

void DebugExpert::work()
{
    row = 1;
    fresh();
    ui->label_sync->setText("本机mac地址:" + sqlcfg->get_global()->mac);
}

void DebugExpert::do_key_ok()
{
    switch (row) {
    case 1:         //多机互联

        break;
    case 2:         //固化通道信息

        break;
    case 3:         //温度补偿

        break;
    case 4:         //同步器匹配
        ui->label_sync->setText("正在连接同步器，连接时间:0s");
        wifi_managment = new WifiManagement;
        connect(wifi_managment, SIGNAL(ap_connect_complete(bool)), this, SLOT(ap_connect_complete(bool)));
        wifi_managment->ap_connect("WIFI-PDTEV", "012345678");   //执行连接
        timer_wifi = new QTimer;
        connect(timer_wifi, SIGNAL(timeout()), this, SLOT(fresh_wifi_time()));
        timer_wifi->start(1000);
        sec = 0;
        return;
    case 5:         //管理以太网口
        ethernet_enable(sql_para->ethernet);
        break;
    default:
        break;
    }
    emit save();
    fresh();
}

void DebugExpert::do_key_cancel()
{
    row = 0;
    fresh();
    emit quit();
}

void DebugExpert::do_key_up_down(int d)
{
    Common::change_index(row, d, 5, 1);
    fresh();
}

void DebugExpert::do_key_left_right(int d)
{
    QList<int> list;
    switch (row) {
    case 1:     //互联模式
        list << multimachine_server << multimachine_client1 << multimachine_client2 << multimachine_close;
        Common::change_index(sql_para->multimachine_mode, d, list);           //改变互联模式,需要重启
        break;
    case 3:     //温度补偿
        list.clear();
        list << temp_auto << temp_uncharge << temp_charge;
        Common::change_index(sql_para->temp_compensation, d, list);
        break;
    case 5:     //以太网口
        sql_para->ethernet = !sql_para->ethernet;
        break;
    default:
        break;
    }
    fresh();
}

void DebugExpert::fresh()
{
    switch (sql_para->multimachine_mode) {
    case multimachine_server:
        ui->rbt_multi_server->setChecked(true);
        break;
    case multimachine_client1:
        ui->rbt_multi_client1->setChecked(true);
        break;
    case multimachine_client2:
        ui->rbt_multi_client2->setChecked(true);
        break;
    case multimachine_close:
        ui->rbt_multi_close->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para->temp_compensation) {
    case temp_auto:
        ui->rbt_auto->setChecked(true);
        break;
    case temp_uncharge:
        ui->rbt_uncharge->setChecked(true);
        break;
    case temp_charge:
        ui->rbt_charge->setChecked(true);
        break;
    default:
        break;
    }

    if(sql_para->ethernet) {
        ui->eth_on->setChecked(true);
    }
    else{
        ui->eth_off->setChecked(true);
    }

    ui->lab_multi->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_freeze_data->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_temp->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_sync->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_eth->setStyleSheet("QLabel{color:#FFFFFF;}");
    switch (row) {
    case 1:         //多机互联
        ui->lab_multi->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 2:         //固化通道信息
        ui->lab_freeze_data->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 3:         //温度补偿
        ui->lab_temp->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 4:         //同步器匹配
        ui->lab_sync->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 5:         //以太网口
        ui->lab_eth->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    default:
        break;
    }
}

void DebugExpert::ap_connect_complete(bool f)
{
    if(f){          //连接成功
        ui->label_sync->setText("连接同步器成功，正在建立socket连接");
        tcpSocket = new QTcpSocket;
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(socket_connected()));
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(get_reply()));
        tcpSocket->connectToHost("192.168.5.1", 6992);
    }
    else{           //连接失败
        ui->label_sync->setText("连接同步器失败，请检查设备后重新连接！");
    }
    timer_wifi->stop();
}

void DebugExpert::socket_connected()
{
    ui->label_sync->setText("socket连接已建立，正在写入同步器！");

    uchar send_buf[8];
    send_buf[0] = 0xFF;         //设备地址
    send_buf[1] = 0x06;         //功能码
    send_buf[2] = 0x00;         //寄存器地址(高位)
    send_buf[3] = 0x63;         //寄存器地址(低位)
    send_buf[4] = 0x00;         //值(高位)
    send_buf[5] = sqlcfg->get_global()->mac_code;         //值(低位)

    //装配CRC校验码
    unsigned short crc_calc = SyncCompute::modbus_crc (send_buf, 6);     //crc的前后顺序不一样
    send_buf [6] = crc_calc / 0x100;
    send_buf [7] = crc_calc % 0x100;

    for (int i = 0; i < 8; ++i) {
        printf("%02x ", send_buf[i]);
    }
    printf("\n");


    tcpSocket->write((char *)send_buf, 8);
    ui->label_sync->setText("写入同步器成功，请重启仪器！");
}

void DebugExpert::get_reply()
{
    QByteArray array = tcpSocket->readAll();
    qDebug()<<"get one reply"<<array;
}

void DebugExpert::fresh_wifi_time()
{
    sec++;
    ui->label_sync->setText(tr("正在连接同步器，连接时间:%1s").arg(sec));
}

void DebugExpert::ethernet_enable(bool f)
{
    struct mii_ioctl_data *mii = NULL;
    struct ifreq ifr;
    int ret;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    int sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    reteck(sockfd);

    //get phy address in smi bus
    ret = ioctl(sockfd, SIOCGMIIPHY, &ifr);
    reteck(ret);

    mii = (struct mii_ioctl_data*)&ifr.ifr_data;

    mii->reg_num    = 0;
    ret = ioctl(sockfd, SIOCGMIIREG, &ifr);
    reteck(ret);
    printf("read phy addr: 0x%x  reg: 0x%x   value : 0x%x\n\n", mii->phy_id, mii->reg_num, mii->val_out);

    mii->reg_num    = 0;

    if(f){      //开启网口
        mii->val_in  =  mii->val_out  & 0xf7ff;
    }
    else{       //关闭网口
        mii->val_in  =  mii->val_out  | (1<<11);
    }

    ret = ioctl(sockfd, SIOCSMIIREG, &ifr);
    reteck(ret);
    printf("write phy addr: 0x%x  reg: 0x%x  value : 0x%x\n\n", mii->phy_id, mii->reg_num, mii->val_in);

//lab:
//    close(sockfd);
}





