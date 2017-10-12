#include "wifi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <QtDebug>

#define PRINT_MSG		0


WifiConfig::WifiConfig()
{
    if (init_wifi_dev (&wifi_at_cmd, UART_BAUNDRATE) != 0) {
        printf ("failed to init modbus device\n");
    }
}

WifiConfig::~WifiConfig()
{
    qDebug()<<"~WifiConfig()";
}

QStringList WifiConfig::wifi_get_list()
{
    if (wifi_enter_cmd_mode (&wifi_at_cmd) == 0) {
        printf ("enter cmd mode\n");
    }
    else {
        printf ("error: enter cmd mode\n");
    }

    //    ap_item_t ap_list [WSCAN_NUM];
    ap_num = wifi_get_ap_list (ap_list);

    //    if (ap_num > 0) {
    //        //printf (" AP Number: %d\n", ap_num);
    //        for (i = 0; i < ap_num; i++) {
    //            //printf (" AP %d: %s, %s/%s\n", i, ap_list [i].ssid, ap_list [i].auth, ap_list [i].encry);
    //        }
    //    }
    //    else {
    //        //printf ("NO AP\n");
    //    }

    QStringList list;
    for (int i = 0; i < ap_num; i++) {
        list.append(QString::fromLatin1(ap_list [i].ssid) );
    }

    wifi_exit_cmd_mode (&wifi_at_cmd);
    printf ("exit cmd mode\n");

    return list;
}

int WifiConfig::wifi_connect(QString name, QString password)
{
    QByteArray ba = password.toLatin1();
    char *mm = ba.data();
    for (int i = 0; i < ap_num; i++) {
        if( QString::fromLatin1(ap_list [i].ssid) == name){
            qDebug()<<"name"<<name << "password" << password;
            wifi_config_sta_tcp_client (ap_list [i].ssid, ap_list [i].auth, ap_list [i].encry, mm, AP_IP_DST, AP_PORT_DST);
            wifi_reboot ();
            break;
        }
    }
    return 0;
}

int WifiConfig::wifi_hotpot(QString name, QString password)
{
    wifi_enter_cmd_mode (&wifi_at_cmd);
    QByteArray n = name.toLatin1(), p = password.toLatin1();
    wifi_config_ap_tcp_server (n.data(), AP_AUTH, AP_ENCRY, p.data(), AP_IP, AP_PORT);
    wifi_reboot ();
    return 0;
}

void WifiConfig::wifi_infomation(QString &str_ip_addr, QString &str_netmask, QString &str_gateway,
                                 QString &str_name, QString &str_mac, int &indicator)
{
    if (wifi_enter_cmd_mode (&wifi_at_cmd) == 0) {
        printf ("enter cmd mode\n");
    }
    else {
        printf ("error: enter cmd mode\n");
    }

    unsigned int ip_addr, netmask, gateway;
    //    int indicator;
    string_32_t ap_name;
    char mac_addr [6];

    wifi_get_sta_net_status (&ip_addr, &netmask, &gateway);
    printf ("sta ip %d.%d.%d.%d, mask %d.%d.%d.%d, gateway %d.%d.%d.%d\n",
            (ip_addr >> 24) & 0xff, (ip_addr >> 16) & 0xff, (ip_addr >> 8) & 0xff, ip_addr & 0xff,
            (netmask >> 24) & 0xff, (netmask >> 16) & 0xff, (netmask >> 8) & 0xff, netmask & 0xff,
            (gateway >> 24) & 0xff, (gateway >> 16) & 0xff, (gateway >> 8) & 0xff, gateway & 0xff);
    str_ip_addr = QString("%1.%2.%3.%4").arg((ip_addr >> 24) & 0xff).arg((ip_addr >> 16) & 0xff).arg((ip_addr >> 8) & 0xff).arg(ip_addr & 0xff);
    str_netmask = QString("%1.%2.%3.%4").arg((netmask >> 24) & 0xff).arg((netmask >> 16) & 0xff).arg((netmask >> 8) & 0xff).arg(netmask & 0xff);
    str_gateway = QString("%1.%2.%3.%4").arg((gateway >> 24) & 0xff).arg((gateway >> 16) & 0xff).arg((gateway >> 8) & 0xff).arg(gateway & 0xff);

    wifi_get_sta_link_status (ap_name, mac_addr, &indicator);
    printf ("ap ssid %s, mac_addr %02x:%02x:%02x:%02x:%02x:%02x, indicator %d\n",
            ap_name,
            mac_addr [0], mac_addr [1], mac_addr [2], mac_addr [3], mac_addr [4], mac_addr [5],
            indicator);
    str_name = QString::fromLatin1(ap_name);
    str_mac = QString("%1:%2:%3:%4:%5:%6").arg((int)mac_addr[0],2,16).arg((int)mac_addr[1],2,16).arg((int)mac_addr[2],2,16)
            .arg((int)mac_addr[3],2,16).arg((int)mac_addr[4],2,16).arg((int)mac_addr[5],2,16);



    wifi_exit_cmd_mode (&wifi_at_cmd);
    printf ("exit cmd mode\n");
}

void WifiConfig::wifi_set_enable(bool enable)
{
    wifi_start(enable);
}

int WifiConfig::wifi_ap_num()
{
    return this->ap_num;
}

void WifiConfig::wifi_server()
{
    if (wifi_enter_cmd_mode (&wifi_at_cmd) == 0) {
        printf ("enter cmd mode\n");
    }
    else {
        printf ("error: enter cmd mode\n");
    }

    wifi_config_ap_tcp_server (AP_SSID, AP_AUTH, AP_ENCRY, AP_KEY, AP_IP, AP_PORT);
    wifi_reboot ();
}

void WifiConfig::wifi_client()
{
    if (wifi_enter_cmd_mode (&wifi_at_cmd) == 0) {
        printf ("enter cmd mode\n");
    }
    else {
        printf ("error: enter cmd mode\n");
    }

    //    ap_item_t ap_list [WSCAN_NUM];
    //    int i, ap_num = wifi_get_ap_list (ap_list);
    //    if (ap_num > 0) {
    //        printf (" AP Number: %d\n", ap_num);
    //        for (i = 0; i < ap_num; i++) {
    //            printf (" AP %d: %s, %s/%s\n", i, ap_list [i].ssid, ap_list [i].auth, ap_list [i].encry);
    //        }
    //    }
    //    else {
    //        printf ("NO AP\n");
    //    }

    for (int i = 0; i < ap_num; i++) {
        if (strcmp (ap_list [i].ssid, "zdit") == 0) {
            //            wifi_config_sta_tcp_server (ap_list [i].ssid, ap_list [i].auth, ap_list [i].encry, "zdit.com.cn", AP_PORT);
            //            wifi_config_sta_tcp_client (ap_list [i].ssid, ap_list [i].auth, ap_list [i].encry, 手动, AP_IP_DST, AP_PORT_DST);
            wifi_config_sta_tcp_client (ap_list [i].ssid, ap_list [i].auth, ap_list [i].encry, "zdit.com.cn", AP_IP_DST, AP_PORT_DST);
            wifi_reboot ();
        }
    }

    qDebug()<<"33";
}

void WifiConfig::wifi_sync()
{
    if (wifi_enter_cmd_mode (&wifi_at_cmd) == 0) {
        printf ("enter cmd mode\n");
    }
    else {
        printf ("error: enter cmd mode\n");
    }

    wifi_config_sta_tcp_client (AP_SSID_DST, AP_AUTH_DST, AP_ENCRY_DST, AP_KEY_DST,
                                AP_IP_DST, AP_PORT_DST);
    wifi_reboot ();
}

void WifiConfig::wifi_info()
{
    if (wifi_enter_cmd_mode (&wifi_at_cmd) == 0) {
        printf ("enter cmd mode\n");
    }
    else {
        printf ("error: enter cmd mode\n");
    }

    unsigned int ip, net_mask, gateway;
    int ind;
    string_32_t ap_name;
    char mac_addr [6];

    wifi_get_sta_net_status (&ip, &net_mask, &gateway);
    printf ("sta ip %d.%d.%d.%d, mask %d.%d.%d.%d, gateway %d.%d.%d.%d\n",
            (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff,
            (net_mask >> 24) & 0xff, (net_mask >> 16) & 0xff, (net_mask >> 8) & 0xff, net_mask & 0xff,
            (gateway >> 24) & 0xff, (gateway >> 16) & 0xff, (gateway >> 8) & 0xff, gateway & 0xff);
    wifi_get_sta_link_status (ap_name, mac_addr, &ind);
    printf ("ap ssid %s, mac_addr %02x:%02x:%02x:%02x:%02x:%02x, indicator %d\n",
            ap_name,
            mac_addr [0], mac_addr [1], mac_addr [2], mac_addr [3], mac_addr [4], mac_addr [5],
            ind);

    wifi_exit_cmd_mode (&wifi_at_cmd);
    printf ("exit cmd mode\n");

}

int WifiConfig::wifi_reboot()
{
    wifi_do_cmd (&wifi_at_cmd, WCMD_Z, 0, NULL);

    return 0;
}

int WifiConfig::wifi_get_ap_list(WifiConfig::ap_item_t ap_lst[])
{
    int i, num;

    if (ap_lst == NULL) {
        return -1;
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WSCAN, 0, NULL);

    num = wifi_at_cmd.wscan.item_num;
    for (i = 0; i < num; i++) {
        strcpy (ap_lst [i].ssid, wifi_at_cmd.wscan.witem [i].ssid);
        strcpy (ap_lst [i].auth, wifi_at_cmd.wscan.witem [i].auth);
        strcpy (ap_lst [i].encry, wifi_at_cmd.wscan.witem [i].encry);
        ap_lst [i].indicator = wifi_at_cmd.wscan.witem [i].indicator;
    }

    return num;
}

int WifiConfig::wifi_start(int start)
{
    wifi_stat_t stat;

    if (start) {
        stat.status = 1;
    }
    else {
        stat.status = 0;
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WIFI, 1, &stat);

    return 0;
}

int WifiConfig::wifi_get_status(int *stat)
{
    wifi_do_cmd (&wifi_at_cmd, WCMD_WIFI, 0, NULL);

    if (stat != NULL) {
        * stat = wifi_at_cmd.wifi.status;
    }

    return 0;
}

int WifiConfig::wifi_get_sta_net_status(unsigned int *ip, unsigned int *net_mask, unsigned int *gateway)
{
    unsigned int temp;

    wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 0, NULL);

    temp = wifi_at_cmd.wann.address;
    if (ip != NULL) {
        //        sprintf (ip, "%d.%d.%d.%d", (temp >> 24) & 0xff, (temp >> 16) & 0xff, (temp >> 8) & 0xff, temp & 0xff);
        * ip = temp;
    }

    temp = wifi_at_cmd.wann.mask;
    if (net_mask != NULL) {
        //        sprintf (net_mask, "%d.%d.%d.%d", (temp >> 24) & 0xff, (temp >> 16) & 0xff, (temp >> 8) & 0xff, temp & 0xff);
        * net_mask = temp;
    }

    temp = wifi_at_cmd.wann.gateway;
    if (gateway != NULL) {
        //        sprintf (gateway, "%d.%d.%d.%d", (temp >> 24) & 0xff, (temp >> 16) & 0xff, (temp >> 8) & 0xff, temp & 0xff);
        * gateway = temp;
    }

    return 0;
}

int WifiConfig::wifi_get_sta_link_status(WifiConfig::string_32_t ssid, char ap_mac[], int *indicator)
{
    wifi_do_cmd (&wifi_at_cmd, WCMD_WSLK, 0, NULL);
    if (ssid != NULL) {
        strcpy (ssid, wifi_at_cmd.wslk.ssid);
    }
    if (ap_mac != NULL) {
        memcpy (ap_mac, wifi_at_cmd.wslk.mac, sizeof (wifi_at_cmd.wslk.mac));
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WSLQ, 0, NULL);
    if (indicator != NULL) {
        * indicator = wifi_at_cmd.wslq.indicator;
    }

    return 0;
}

int WifiConfig::wifi_config_ap_tcp_server(WifiConfig::string_32_t ssid, char *auth, char *encry, char *key, char *ip, unsigned short port)
{
    wmode_t wmode;
    lann_t lann;
    wap_t wap;
    wakey_t wakey;
    netp_t netp;
    unsigned int ipaddr;

    qDebug()<<"11";

    if (check_ap_auth_encry (auth, encry) != 0) {
        printf ("auth: %s, encry: %s is invalid\n", auth, encry);
        return -1;
    }
    ipaddr = ntohl (inet_addr (ip));

    qDebug()<<"22";

    wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 0, NULL);
    if (strcmp (wifi_at_cmd.wmode.mode, WMODE_AP) != 0) {
        strcpy (wmode.mode, WMODE_AP);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 1, &wmode);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 0, NULL);
        printf (" set wmode: %s\n", wifi_at_cmd.wmode.mode);
    }

    qDebug()<<"33";

    wifi_do_cmd (&wifi_at_cmd, WCMD_WAP, 0, NULL);
    if (strcmp (wifi_at_cmd.wap.ssid, ssid) != 0) {
        memcpy (&wap, &wifi_at_cmd.wap, sizeof (wap));
        printf ("1SSID: %s, PASSWORD: %s \n", ssid, key);

        strcpy (wap.ssid, ssid);

        printf ("2SSID: %s, PASSWORD: %s \n", wap.ssid, key);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WAP, 1, &wap);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WAP, 0, NULL);
        printf (" set wifi_mode: %s, ssid: %s, channel: %s\n", wifi_at_cmd.wap.wifi_mode, wifi_at_cmd.wap.ssid, wifi_at_cmd.wap.channel);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WAKEY, 0, NULL);
    if (strcmp (wifi_at_cmd.wakey.auth, auth) != 0 ||
            strcmp (wifi_at_cmd.wakey.encry, encry) != 0 ||
            strcmp (wifi_at_cmd.wakey.key, key) != 0) {

        strcpy (wakey.auth, auth);
        strcpy (wakey.encry, encry);
        strcpy (wakey.key, key);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WAKEY, 1, &wakey);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WAKEY, 0, NULL);
        printf (" set auth: %s, encry: %s, key: %s\n", wifi_at_cmd.wakey.auth, wifi_at_cmd.wakey.encry, wifi_at_cmd.wakey.key);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_LANN, 0, NULL);
    if (wifi_at_cmd.lann.ipaddress != ipaddr) {
        lann.ipaddress = ipaddr;
        lann.mask = 0xffffff00;
        wifi_do_cmd (&wifi_at_cmd, WCMD_LANN, 1, &lann);
        wifi_do_cmd (&wifi_at_cmd, WCMD_LANN, 0, NULL);
        printf (" set ipaddress: 0x%08x, mask: 0x%08x\n", wifi_at_cmd.lann.ipaddress, wifi_at_cmd.lann.mask);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 0, NULL);
    if (strcmp (wifi_at_cmd.netp.proto, PROTO_TCP) != 0 ||
            strcmp (wifi_at_cmd.netp.cs, CS_SERVER) != 0 ||
            wifi_at_cmd.netp.port != port) {
        strcpy (netp.proto, PROTO_TCP);
        strcpy (netp.cs, CS_SERVER);
        netp.port = port;
        netp.ip = ntohl (inet_addr (AP_IP));
        wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 1, &netp);
        wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 0, NULL);
        printf (" set tcp server port: %d\n", wifi_at_cmd.netp.port);
    }

    return 0;
}

int WifiConfig::wifi_config_sta_tcp_server(WifiConfig::string_32_t ssid_dst, char *auth, char *encry, char *key, unsigned short port)
{
    wmode_t wmode;
    wsssid_t wsssid;
    wskey_t wskey;
    wann_t wann;
    netp_t netp;

    if (check_sta_auth_encry (auth, encry) != 0) {
        printf ("auth: %s, encry: %s is invalid\n", auth, encry);
        return -1;
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 0, NULL);
    if (strcmp (wifi_at_cmd.wmode.mode, WMODE_STA) != 0) {
        strcpy (wmode.mode, WMODE_STA);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 1, &wmode);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 0, NULL);
        printf (" set wmode: %s\n", wifi_at_cmd.wmode.mode);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WSSSID, 0, NULL);
    if (strcmp (wifi_at_cmd.wsssid.ssid, ssid_dst) != 0) {
        strcpy (wsssid.ssid, ssid_dst);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSSSID, 1, &wsssid);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSSSID, 0, NULL);
        printf (" set wsssid: %s\n", wifi_at_cmd.wsssid.ssid);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WSKEY, 0, NULL);
    if (strcmp (wifi_at_cmd.wskey.auth, auth) != 0||
            strcmp (wifi_at_cmd.wskey.encry, encry) != 0 ||
            strcmp (wifi_at_cmd.wskey.key, key) != 0) {
        strcpy (wskey.auth, auth);
        strcpy (wskey.encry, encry);
        strcpy (wskey.key, key);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSKEY, 1, &wskey);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSKEY, 0, NULL);
        printf (" set sta auth: %s, encry: %s, key: %s\n",
                wifi_at_cmd.wskey.auth,
                wifi_at_cmd.wskey.encry,
                wifi_at_cmd.wskey.key);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 0, NULL);
    if (strcmp (wifi_at_cmd.wann.mode, MODE_DHCP) != 0) {
        memcpy (&wann, &wifi_at_cmd.wann, sizeof (wann));
        strcpy (wann.mode, MODE_DHCP);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 1, &wann);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 0, NULL);
        printf (" set sta wann mode: %s\n", wifi_at_cmd.wann.mode);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 0, NULL);
    if (strcmp (wifi_at_cmd.netp.proto, PROTO_TCP) != 0 ||
            strcmp (wifi_at_cmd.netp.cs, CS_SERVER) != 0 ||
            wifi_at_cmd.netp.port != port) {
        strcpy (netp.proto, PROTO_TCP);
        strcpy (netp.cs, CS_SERVER);
        netp.port = port;
        netp.ip = ntohl (inet_addr (AP_IP));
        wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 1, &netp);
        wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 0, NULL);
        printf (" set tcp server port: %d\n", wifi_at_cmd.netp.port);
    }

    return 0;
}

int WifiConfig::wifi_config_sta_tcp_client(WifiConfig::string_32_t ssid_dst, char *auth, char *encry, char *key, char *ip_dst, unsigned short port_dst)
{
    wmode_t wmode;
    wsssid_t wsssid;
    wskey_t wskey;
    wann_t wann;
    netp_t netp;
    unsigned int ipaddr;

    if (check_sta_auth_encry (auth, encry) != 0) {
        printf ("auth: %s, encry: %s is invalid\n", auth, encry);
        return -1;
    }
    ipaddr = ntohl (inet_addr (ip_dst));

    wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 0, NULL);
    if (strcmp (wifi_at_cmd.wmode.mode, WMODE_STA) != 0) {
        strcpy (wmode.mode, WMODE_STA);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 1, &wmode);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WMODE, 0, NULL);
        printf (" set wmode: %s\n", wifi_at_cmd.wmode.mode);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WSSSID, 0, NULL);
    if (strcmp (wifi_at_cmd.wsssid.ssid, ssid_dst) != 0) {
        strcpy (wsssid.ssid, ssid_dst);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSSSID, 1, &wsssid);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSSSID, 0, NULL);
        printf (" set wsssid: %s\n", wifi_at_cmd.wsssid.ssid);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WSKEY, 0, NULL);
    if (strcmp (wifi_at_cmd.wskey.auth, auth) != 0||
            strcmp (wifi_at_cmd.wskey.encry, encry) != 0 ||
            strcmp (wifi_at_cmd.wskey.key, key) != 0) {
        strcpy (wskey.auth, auth);
        strcpy (wskey.encry, encry);
        strcpy (wskey.key, key);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSKEY, 1, &wskey);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WSKEY, 0, NULL);
        printf (" set sta auth: %s, encry: %s, key: %s\n",
                wifi_at_cmd.wskey.auth,
                wifi_at_cmd.wskey.encry,
                wifi_at_cmd.wskey.key);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 0, NULL);
    if (strcmp (wifi_at_cmd.wann.mode, MODE_DHCP) != 0) {
        memcpy (&wann, &wifi_at_cmd.wann, sizeof (wann));
        strcpy (wann.mode, MODE_DHCP);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 1, &wann);
        wifi_do_cmd (&wifi_at_cmd, WCMD_WANN, 0, NULL);
        printf (" set sta wann mode: %s\n", wifi_at_cmd.wann.mode);
    }

    wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 0, NULL);
    if (strcmp (wifi_at_cmd.netp.proto, PROTO_TCP) != 0 ||
            strcmp (wifi_at_cmd.netp.cs, CS_CLIENT) != 0 ||
            wifi_at_cmd.netp.port != port_dst ||
            wifi_at_cmd.netp.ip != ipaddr) {
        strcpy (netp.proto, PROTO_TCP);
        strcpy (netp.cs, CS_CLIENT);
        netp.port = port_dst;
        netp.ip = ipaddr;
        wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 1, &netp);
        wifi_do_cmd (&wifi_at_cmd, WCMD_NETP, 0, NULL);
        printf (" set tcp client ip_dst: 0x%08x, port_dst: %d\n", wifi_at_cmd.netp.ip, wifi_at_cmd.netp.port);
    }

    return 0;
}

int WifiConfig::init_wifi_dev(WifiConfig::wifi_at_cmd_t *wa, int baundrate)
{
    /* open uart */
    wa->com_fd = uart_open (UART_PORT, baundrate, 0, 8, 1, 'N'); //打开串口，返回文件描述符
    if (wa->com_fd < 0) {
        printf ("failed to open port %s\n", UART_PORT);
        return -1;
    }

    return 0;
}

int WifiConfig::close_wifi_dev(WifiConfig::wifi_at_cmd_t *wa)
{
    uart_close (wa->com_fd);

    return 0;
}

int WifiConfig::wifi_recv_msg(WifiConfig::wifi_at_cmd_t *wa)
{
    int len;

    len = 0;
    wa->recv_len = 0;
    do {
        usleep (300000);
        len = uart_recv (wa->com_fd, (unsigned char *)(wa->recv_buf + wa->recv_len), sizeof (wa->recv_buf) - wa->recv_len);
        wa->recv_len += len;
    } while (len != 0);
    wa->recv_buf [wa->recv_len] = 0;
#if PRINT_MSG
    printf ("--recv msg:\n%s\n--recv over\n", wa->recv_buf);
#endif

    return wa->recv_len;
}

int WifiConfig::wifi_send_msg(WifiConfig::wifi_at_cmd_t *wa)
{
    int sent_len;

    sent_len = uart_send (wa->com_fd, (unsigned char *)wa->send_buf, wa->send_len);
#if PRINT_MSG
    printf ("--send msg:\n%s\n--send over\n", wa->send_buf);
#endif

    return sent_len;
}

int WifiConfig::build_cmd_req(WifiConfig::wifi_at_cmd_t *wa, int cmd, int b_set, void *set_data)
{
    char temp_buf [256];
    wmode_t * pwmode;
    netp_t * pnetp;
    wsssid_t * pwsssid;
    lann_t * plann;
    wskey_t * pwskey;
    wann_t * pwann;
    wap_t * pwap;
    wakey_t * pwakey;

    switch (cmd) {
    case WCMD_BEGIN_PLUS:
        strcpy (wa->send_buf, "+++");
        wa->send_len = 3;
        return wa->send_len;
    case WCMD_BEGIN_A:
        strcpy (wa->send_buf, "a");
        wa->send_len = 1;
        return wa->send_len;
    }

    strcpy (wa->send_buf, "AT+");

    switch (cmd) {
    case WCMD_E:
        strcat (wa->send_buf, "E");
        break;
    case WCMD_WMODE:
        if (b_set) {
            pwmode = (wmode_t *)set_data;

            strcat (wa->send_buf, "WMODE=");
            strcat (wa->send_buf, pwmode->mode);
        }
        else {
            strcat (wa->send_buf, "WMODE");
        }
        break;
    case WCMD_ENTM:
        strcat (wa->send_buf, "ENTM");
        break;
    case WCMD_TMODE:
        break;
    case WCMD_MID:
        break;
    case WCMD_RELD:
        break;
    case WCMD_Z:
        strcat (wa->send_buf, "Z");
        break;
    case WCMD_H:
        break;
    case WCMD_CFGTF:
        break;
    case WCMD_NETP:
        if (b_set) {
            pnetp = (netp_t *)set_data;

            strcat (wa->send_buf, "NETP=");
            sprintf (temp_buf, "%s,%s,%d,%d.%d.%d.%d",
                     pnetp->proto, pnetp->cs, pnetp->port,
                     (pnetp->ip >> 24) & 0xff,
                     (pnetp->ip >> 16) & 0xff,
                     (pnetp->ip >> 8) & 0xff,
                     pnetp->ip & 0xff);
            strcat (wa->send_buf, temp_buf);
        }
        else {
            strcat (wa->send_buf, "NETP");
        }
        break;
    case WCMD_WSSSID:
        if (b_set) {
            pwsssid = (wsssid_t *)set_data;

            strcat (wa->send_buf, "WSSSID=");
            strcat (wa->send_buf, pwsssid->ssid);
        }
        else {
            strcat (wa->send_buf, "WSSSID");
        }
        break;
    case WCMD_WSKEY:
        if (b_set) {
            pwskey = (wskey_t *)set_data;

            strcat (wa->send_buf, "WSKEY=");
            sprintf (temp_buf, "%s,%s,%s",
                     pwskey->auth, pwskey->encry, pwskey->key);
            strcat (wa->send_buf, temp_buf);
        }
        else {
            strcat (wa->send_buf, "WSKEY");
        }
        break;
    case WCMD_WANN:
        if (b_set) {
            pwann = (wann_t *)set_data;

            strcat (wa->send_buf, "WANN=");
            sprintf (temp_buf, "%s,%d.%d.%d.%d,%d.%d.%d.%d,%d.%d.%d.%d",
                     pwann->mode,
                     (pwann->address >> 24) & 0xff,
                     (pwann->address >> 16) & 0xff,
                     (pwann->address >> 8) & 0xff,
                     pwann->address & 0xff,
                     (pwann->mask>> 24) & 0xff,
                     (pwann->mask>> 16) & 0xff,
                     (pwann->mask>> 8) & 0xff,
                     pwann->mask & 0xff,
                     (pwann->gateway >> 24) & 0xff,
                     (pwann->gateway >> 16) & 0xff,
                     (pwann->gateway >> 8) & 0xff,
                     pwann->gateway & 0xff);
        }
        else {
            strcat (wa->send_buf, "WANN");
        }
        break;
    case WCMD_WSCAN:
        strcat (wa->send_buf, "WSCAN");
        break;
    case WCMD_WSLK:
        strcat (wa->send_buf, "WSLK");
        break;
    case WCMD_WSLQ:
        strcat (wa->send_buf, "WSLQ");
        break;
    case WCMD_LANN:
        if (b_set) {
            plann = (lann_t *)set_data;

            sprintf (temp_buf, "%d.%d.%d.%d,%d.%d.%d.%d",
                     (plann->ipaddress >> 24) & 0xff,
                     (plann->ipaddress >> 16) & 0xff,
                     (plann->ipaddress >> 8) & 0xff,
                     plann->ipaddress & 0xff,
                     (plann->mask >> 24) & 0xff,
                     (plann->mask >> 16) & 0xff,
                     (plann->mask >> 8) & 0xff,
                     plann->mask & 0xff);

            strcat (wa->send_buf, "LANN=");
            strcat (wa->send_buf, temp_buf);
        }
        else {
            strcat (wa->send_buf, "LANN");
        }
        break;
    case WCMD_WAP:
        if (b_set) {
            pwap = (wap_t *)set_data;

            sprintf (temp_buf, "%s,%s,%s", pwap->wifi_mode, pwap->ssid, pwap->channel);

            strcat (wa->send_buf, "WAP=");
            strcat (wa->send_buf, temp_buf);
        }
        else {
            strcat (wa->send_buf, "WAP");
        }
        break;
    case WCMD_WAKEY:
        if (b_set) {
            pwakey = (wakey_t *)set_data;

            sprintf (temp_buf, "%s,%s,%s", pwakey->auth, pwakey->encry, pwakey->key);

            strcat (wa->send_buf, "WAKEY=");
            strcat (wa->send_buf, temp_buf);
        }
        else {
            strcat (wa->send_buf, "WAKEY");
        }
        break;
    default:
        wa->send_buf [0] = 0;
        wa->send_len = 0;
        return -1;
    }

    strcat (wa->send_buf, "\r\n");

    wa->send_len = strlen (wa->send_buf);

    return wa->send_len;
}

int WifiConfig::check_cmd_resp(WifiConfig::wifi_at_cmd_t *wa)
{
    char seps [] = "\r\n"/*" \r\n"*/, * tok;
    int i;

    tok = strtok (wa->recv_buf, seps);
    while (tok != NULL) {
        if (strncasecmp (tok, "+ok", 3) == 0) {
            if (tok [3] == '=') {
                i = 4;
                while ((i < BUF_SIZE) && ((tok [i] == '\0') || (tok [i] == '\r') || (tok [i] == '\n'))) { i++;}
                wa->resp_paras = tok + i;
            }
            else {
                wa->resp_paras = NULL;
            }
            return 1;
        }
        else if (strncasecmp (tok, "+ERR", 4) == 0) {
            sscanf (tok, "+ERR=%d", &wa->err_code);
            return 0;
        }
        tok = strtok (NULL, seps);
    }

    return -1;
}

int WifiConfig::check_ap_auth_encry(char *auth, char *encry)
{
    if (strcmp (encry, ENCRY_NONE) == 0) {
        if (strcmp (auth, AUTH_OPEN) == 0) {
            return 0;
        }
    }
    else if (strcmp (encry, ENCRY_AES) == 0) {
        if (strcmp (auth, AUTH_WPA2PSK) == 0) {
            return 0;
        }
    }

    return -1;
}

int WifiConfig::check_sta_auth_encry(char *auth, char *encry)
{
    if (strcmp (encry, ENCRY_NONE) == 0) {
        if (strcmp (auth, AUTH_OPEN) == 0) {
            return 0;
        }
    }
    else if (strcmp (encry, ENCRY_WEP_H) == 0) {
        if (strcmp (auth, AUTH_OPEN) == 0 ||
                strcmp (auth, AUTH_SHARED) == 0) {
            return 0;
        }
    }
    else if (strcmp (encry, ENCRY_WEP_A) == 0) {
        if (strcmp (auth, AUTH_OPEN) == 0 ||
                strcmp (auth, AUTH_SHARED) == 0) {
            return 0;
        }
    }
    else if (strcmp (encry, ENCRY_TKIP) == 0) {
        if (strcmp (auth, AUTH_WPAPSK) == 0 ||
                strcmp (auth, AUTH_WPA2PSK) == 0) {
            return 0;
        }
    }
    else if (strcmp (encry, ENCRY_AES) == 0) {
        if (strcmp (auth, AUTH_WPAPSK) == 0 ||
                strcmp (auth, AUTH_WPA2PSK) == 0) {
            return 0;
        }
    }

    return -1;
}

int WifiConfig::wifi_enter_cmd_mode(WifiConfig::wifi_at_cmd_t *wa)
{
    int ret = -1;

    build_cmd_req (wa, WCMD_BEGIN_PLUS, 0, NULL);
    wifi_send_msg (wa);
    wifi_recv_msg (wa);
    if ((wa->recv_len == 0) || (strncasecmp (wa->recv_buf, "a", 1) != 0)) {
        printf ("error: enter cmd mode step1\n");
        return -1;
    }

    build_cmd_req (wa, WCMD_BEGIN_A, 0, NULL);
    wifi_send_msg (wa);
    wifi_recv_msg (wa);
    if ((wa->recv_len == 0) || ((ret = check_cmd_resp (wa)) != 1)) {
        printf ("error: enter cmd mode step2, ret %d, err_code %d", ret, wa->err_code);
        return -1;
    }

    build_cmd_req (wa, WCMD_E, 0, NULL);
    wifi_send_msg (wa);
    wifi_recv_msg (wa);
    if ((wa->recv_len == 0) || (check_cmd_resp (wa) != 1)) {
        printf ("error: enter cmd mode step3, ret %d, err_code %d", ret, wa->err_code);
        return -1;
    }

    return 0;
}

int WifiConfig::wifi_exit_cmd_mode(WifiConfig::wifi_at_cmd_t *wa)
{
    int ret = -1;

    build_cmd_req (wa, WCMD_E, 0, NULL);
    wifi_send_msg (wa);

    wifi_recv_msg (wa);
    if ((wa->recv_len == 0) || ((ret = check_cmd_resp (wa)) != 1)) {
        printf ("error: exit cmd mode step3, ret %d, err_code %d", ret, wa->err_code);
    }

    build_cmd_req (wa, WCMD_ENTM, 0, NULL);
    wifi_send_msg (wa);

    wifi_recv_msg (wa);
    if ((wa->recv_len == 0) || ((ret = check_cmd_resp (wa)) != 1)) {
        printf ("error: exit cmd mode step3, ret %d, err_code %d", ret, wa->err_code);
    }

    return 0;
}

int WifiConfig::wifi_do_cmd(WifiConfig::wifi_at_cmd_t *wa, int cmd, int b_set, void *set_data)
{
    int ret = -1;
    unsigned int temp1 [6];
    char temp_str_large [BUF_SIZE], temp_str [32];
    char * tok, seps [] = ",\r\n";
    int index, i, j, len;

    build_cmd_req (wa, cmd, b_set, set_data);
    wifi_send_msg (wa);

    if (b_set ||
            cmd == WCMD_WSCAN) {
        sleep (1);
    }

    wifi_recv_msg (wa);
    if ((wa->recv_len == 0) || ((ret = check_cmd_resp (&wifi_at_cmd)) != 1)) {
        printf ("cmd %d ret %d, err_code %d\n", cmd, ret, wifi_at_cmd.err_code);
        return -1;
    }
    else {
#if PRINT_MSG
        printf ("cmd %d resp paras: %s\n", cmd, wa->resp_paras);
#endif
    }

    switch (cmd) {
    case WCMD_WMODE:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wmode.mode, tok);
            }
            else {
                wa->wmode.mode [0] = 0;
                break;
            }
        }
        break;
    case WCMD_NETP:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->netp.proto, tok);
            }
            else {
                wa->netp.proto [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->netp.cs, tok);
            }
            else {
                wa->netp.cs [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                wa->netp.port = atoi (tok);
            }
            else {
                wa->netp.port = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                wa->netp.ip = ntohl (inet_addr (tok));
            }
            else {
                wa->netp.ip = 0;
                break;
            }
        }
        break;
    case WCMD_WSSSID:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wsssid.ssid, tok);
            }
            else {
                wa->wsssid.ssid [0] = 0;
            }
        }
        break;
    case WCMD_WSKEY:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wskey.auth, tok);
            }
            else {
                wa->wskey.auth [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->wskey.encry, tok);
            }
            else {
                wa->wskey.encry [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->wskey.key, tok);
            }
            else {
                wa->wskey.key [0] = 0;
                break;
            }
        }
        break;
    case WCMD_WANN:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wann.mode, tok);
            }
            else {
                wa->wann.mode [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                wa->wann.address = ntohl (inet_addr (tok));
            }
            else {
                wa->wann.address = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                wa->wann.mask = ntohl (inet_addr (tok));
            }
            else {
                wa->wann.mask = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                wa->wann.gateway = ntohl (inet_addr (tok));
            }
            else {
                wa->wann.gateway = 0;
                break;
            }
        }
        break;
    case WCMD_WSCAN:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            while (tok != NULL) {
                if (strcmp (tok, "Indicator") == 0) {
                    break;
                }
                tok = strtok (NULL, seps);
            }

            index = wa->wscan.item_num = 0;
            tok = strtok (NULL, seps);
            while (tok != NULL) {
                wa->wscan.witem [index].ch = atoi (tok);

                tok = strtok (NULL, seps);
                if (tok != NULL) {
                    strcpy (wa->wscan.witem [index].ssid, tok);
                    //printf ("ap %s\n", wa->wscan.witem [index].ssid);
                }
                else {
                    wa->wscan.witem [index].ssid [0] = 0;
                    break;
                }

                tok = strtok (NULL, seps);
                if (tok != NULL) {
                    sscanf (tok, "%02x:%02x:%02x:%02x:%02x:%02x",
                            &temp1 [0], &temp1 [1], &temp1 [2], &temp1 [3], &temp1 [4], &temp1 [5]);
                    wa->wscan.witem [index].bssid [0] = temp1 [0] & 0xff;
                    wa->wscan.witem [index].bssid [1] = temp1 [1] & 0xff;
                    wa->wscan.witem [index].bssid [2] = temp1 [2] & 0xff;
                    wa->wscan.witem [index].bssid [3] = temp1 [3] & 0xff;
                    wa->wscan.witem [index].bssid [4] = temp1 [4] & 0xff;
                    wa->wscan.witem [index].bssid [5] = temp1 [5] & 0xff;
                }
                else {
                    memset (wa->wscan.witem [index].bssid, 0, 6);
                    break;
                }

                tok = strtok (NULL, seps);
                if (tok != NULL) {
                    strcpy (temp_str, tok);
                    //printf ("security: %s\n", temp_str);
                    len = strlen (temp_str);
                    for (i = 0; i < len; i++) {
                        if (temp_str [i] == '/') {
                            temp_str [i] = 0;
                            break;
                        }
                    }
                    if (i >= len) {
                        wa->wscan.witem [index].auth [0] = 0;
                        wa->wscan.witem [index].encry [0] = 0;
                    }
                    else {
                        if (strstr (temp_str, AUTH_OPEN) != 0) {
                            strcpy (wa->wscan.witem [index].auth, AUTH_OPEN);
                        }
                        else if (strstr (temp_str, AUTH_SHARED) != 0) {
                            strcpy (wa->wscan.witem [index].auth, AUTH_SHARED);
                        }
                        else if (strstr (temp_str, AUTH_WPA2PSK) != 0) {	/* AUTH_WPA2PSK优先于AUTH_WPAPSK */
                            strcpy (wa->wscan.witem [index].auth, AUTH_WPA2PSK);
                        }
                        else if (strstr (temp_str, AUTH_WPAPSK) != 0) {
                            strcpy (wa->wscan.witem [index].auth, AUTH_WPAPSK);
                        }
                        else {
                            strcpy (wa->wscan.witem [index].auth, AUTH_OPEN);
                        }
                        strcpy (wa->wscan.witem [index].encry, &temp_str [i + 1]);
                        //printf ("auth %s\nencry %s\n", wa->wscan.witem [index].auth, wa->wscan.witem [index].encry);
                    }
                }
                else {
                    wa->wscan.witem [index].auth [0] = 0;
                    wa->wscan.witem [index].encry [0] = 0;
                    break;
                }

                tok = strtok (NULL, seps);
                if (tok != NULL) {
                    wa->wscan.witem [index].indicator = atoi (tok);
                }
                else {
                    wa->wscan.witem [index].indicator = 0;
                    break;
                }

                index++;
                wa->wscan.item_num = index;
                if (index >= WSCAN_NUM) {
                    break;
                }

                tok = strtok (NULL, seps);
            }
        }
        break;
    case WCMD_WSLK:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            len = strlen (wa->resp_paras);
            for (i = 0, j = 0; i < len; i++) {
                if (temp_str_large [i] == '(') {
                    temp_str_large [i] = 0;
                    j = i + 1;
                }
                else if (temp_str_large [i] == ')') {
                    temp_str_large [i] = 0;
                    break;
                }
            }
            if (j != 0) {
                strcpy (wa->wslk.ssid, temp_str_large);
                sscanf (&temp_str_large [j], "%02x:%02x:%02x:%02x:%02x:%02x",
                        &temp1 [0], &temp1 [1], &temp1 [2], &temp1 [3], &temp1 [4], &temp1 [5]);
                wa->wslk.mac [0] = temp1 [0] & 0xff;
                wa->wslk.mac [1] = temp1 [1] & 0xff;
                wa->wslk.mac [2] = temp1 [2] & 0xff;
                wa->wslk.mac [3] = temp1 [3] & 0xff;
                wa->wslk.mac [4] = temp1 [4] & 0xff;
                wa->wslk.mac [5] = temp1 [5] & 0xff;
            }
            else {
                wa->wslk.ssid [0] = 0;
                memset (wa->wslk.mac, 0, sizeof (wa->wslk.mac));
            }
        }
        break;
    case WCMD_WSLQ:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wslq.ind_desc, tok);
            }
            else {
                wa->wslq.ind_desc [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                /*
                    for (i = 0; i < strlen (tok); i++) {
                        if (tok [i] == '%') {
                            tok [i] = 0;
                            break;
                        }
                    }
                    */
                wa->wslq.indicator = atoi (tok);
            }
            else {
                wa->wslq.indicator = 0;
                break;
            }
        }
        break;
    case WCMD_LANN:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                wa->lann.ipaddress = ntohl (inet_addr (tok));
            }
            else {
                wa->lann.ipaddress = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                wa->lann.mask = ntohl (inet_addr (tok));
            }
            else {
                wa->lann.mask = 0;
                break;
            }
        }
        break;
    case WCMD_WAP:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wap.wifi_mode, tok);
            }
            else {
                wa->wap.wifi_mode [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->wap.ssid, tok);
            }
            else {
                wa->wap.ssid [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->wap.channel, tok);
            }
            else {
                wa->wap.channel [0] = 0;
                break;
            }
        }
        break;
    case WCMD_WAKEY:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                strcpy (wa->wakey.auth, tok);
            }
            else {
                wa->wakey.auth [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->wakey.encry, tok);
            }
            else {
                wa->wakey.encry [0] = 0;
                break;
            }

            tok = strtok (NULL, seps);
            if (tok != NULL) {
                strcpy (wa->wakey.key, tok);
            }
            else {
                wa->wakey.key [0] = 0;
                break;
            }
        }
        break;
    case WCMD_WIFI:
        if (!b_set) {
            strncpy (temp_str_large, wa->resp_paras, sizeof (temp_str_large) - 1);

            tok = strtok (temp_str_large, seps);
            if (tok != NULL) {
                if (strcasecmp (tok, WIFI_STAT_START) == 0) {
                    wa->wifi.status = 1;
                }
                else {
                    wa->wifi.status = 0;
                }
            }
            else {
                wa->wifi.status = 0;
            }
        }
        break;
    default:
        break;
    }

    return 0;
}

