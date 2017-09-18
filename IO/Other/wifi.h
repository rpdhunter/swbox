#ifndef WIFI_H
#define WIFI_H

#include <QStringList>
#include <QString>
#include "../Modbus/uart_oper.h"

#define UART_PORT				"/dev/ttyPS0"
//#define UART_BAUNDRATE			460800
#define UART_BAUNDRATE			115200
#define BUF_SIZE				2048

#define WSCAN_NUM				10

#define WMODE_AP				"AP"
#define WMODE_STA				"STA"
#define WMODE_APSTA				"APSTA"

/* 做AP的参数 */
#define AP_SSID					"PDTEV-WIFI"
#define AP_AUTH					AUTH_WPA2PSK
#define AP_ENCRY				ENCRY_AES
#define AP_KEY					"pdtev12345678"
#define AP_IP					"192.168.19.254"
#define AP_PORT					8999

/* 做STA的参数(连接WIFI同步器) */
#define AP_SSID_DST				"PD-SYNC"
#define AP_AUTH_DST				AUTH_WPA2PSK
#define AP_ENCRY_DST			ENCRY_AES
#define AP_KEY_DST				"pd-sync-key"
#define AP_IP_DST				"192.168.21.254"
#define AP_PORT_DST				9000

#define PROTO_TCP				"TCP"
#define PROTO_UDP				"UDP"

#define CS_SERVER				"Server"
#define CS_CLIENT				"Client"

#define AUTH_OPEN				"OPEN"
#define AUTH_SHARED				"SHARED"
#define AUTH_WPAPSK				"WPAPSK"
#define AUTH_WPA2PSK			"WPA2PSK"

#define ENCRY_NONE				"NONE"
#define ENCRY_WEP_H				"WEP-H"
#define ENCRY_WEP_A				"WEP-A"
#define ENCRY_TKIP				"TKIP"
#define ENCRY_AES				"AES"

#define MODE_STATIC				"static"
#define MODE_DHCP				"DHCP"


class WifiConfig
{
public:
    WifiConfig();

    void wifi_server();     //热点
    void wifi_client();     //接入热点
    void wifi_sync();       //连接同步
    void wifi_info();       //查看信息

    QStringList wifi_get_list();


private:
    enum wifi_cmd_e {
        WCMD_E = 1,					/* 开启关闭回显 */
        WCMD_WMODE,
        WCMD_ENTM,					/* 进入透传模式 */
        WCMD_TMODE,
        WCMD_MID,
        WCMD_RELD,
        WCMD_Z,						/* 重启模块 */
        WCMD_H,
        WCMD_CFGTF,
        WCMD_UART,
        WCMD_UARTF,
        WCMD_UARTFT,
        WCMD_UARTFL,
        WCMD_UARTTE,
        WCMD_PING,
        WCMD_SEND,
        WCMD_RECV,
        WCMD_NETP,					/* 设置/查询网络协议参数 */
        WCMD_MAXSK,
        WCMD_TCPLK,
        WCMD_TCPTO,
        WCMD_TCPDIS,
        WCMD_SOCKB,
        WCMD_TCPDISB,
        WCMD_TCPTOB,
        WCMD_TCPLKB,
        WCMD_SNDB,
        WCMD_RCVB,
        WCMD_WSSSID,
        WCMD_WSKEY,
        WCMD_WANN,
        WCMD_WSMAC,
        WCMD_WSLK,
        WCMD_WSLQ,
        WCMD_WSCAN,
        WCMD_WSDNS,
        WCMD_LANN,
        WCMD_WAP,
        WCMD_WAKEY,
        WCMD_WAMAC,
        WCMD_WADHCP,
        WCMD_WALK,
        WCMD_WALKIND,
        WCMD_NTPRF = 46,
        WCMD_NTPEN,
        WCMD_NTPTM,
        WCMD_WRMID = 58,
        WCMD_ASWD,
        WCMD_MDCH,
        WCMD_TXPWR,
        WCMD_WPS,
        WCMD_WPSBTNEN,
        WCMD_SMTLK,
        WCMD_LPTIO,
        WCMD_USERVER,
        WCMD_CUSTOMER,
        WCMD_RPTMAC,
        WCMD_WRRPTMAC,
        WCMD_WIFI,
        WCMD_WIFICHK,
        WCMD_CHKTIME,
        WCMD_RSEN,

        WCMD_BEGIN_PLUS = 100,
        WCMD_BEGIN_A
    };

    typedef char string_32_t [32];

    typedef struct wmode_s {
        char	mode [8];
    } wmode_t;

    typedef struct netp_s {
        char	proto [8];
        char	cs [12];
        unsigned int port;
        unsigned int ip;
    } netp_t;

    typedef struct wsssid_s {
        string_32_t	ssid;
    } wsssid_t;

    typedef struct wskey_s {
        char	auth [32];
        char	encry [8];
        char	key [64];
    } wskey_t;

    typedef struct wann_s {
        char	mode [8];
        unsigned int address;
        unsigned int mask;
        unsigned int gateway;
    } wann_t;

    typedef struct wscan_item_s {
        int		ch;
        string_32_t	ssid;
        char	bssid [8];
        char	auth [32];
        char	encry [8];
        int		indicator;
    } wscan_item_t;

    typedef struct wscan_s {
        int item_num;
        wscan_item_t witem [WSCAN_NUM];
    } wscan_t;

    typedef struct wslk_s {
        char	ssid [32];
        char	mac [6];
    } wslk_t;

    typedef struct wslq_s {
        char	ind_desc [16];
        int		indicator;
    } wslq_t;

    typedef struct lann_s {
        unsigned int ipaddress;
        unsigned int mask;
    } lann_t;

    typedef struct wap_s {
        char	wifi_mode [8];
        string_32_t	ssid;
        char	channel [8];
    } wap_t;

    typedef wskey_t wakey_t;

    typedef struct wifi_at_cmd_s {
        int		com_fd;
        char	send_buf [BUF_SIZE];
        int		send_len;
        char	recv_buf [BUF_SIZE];
        int		recv_len;
        int		err_code;
        char *	resp_paras;

        wmode_t	wmode;
        netp_t	netp;
        wsssid_t wsssid;
        wskey_t	wskey;
        wann_t 	wann;
        wscan_t wscan;
        wslk_t	wslk;
        wslq_t	wslq;
        lann_t	lann;
        wap_t	wap;
        wakey_t	wakey;
    } wifi_at_cmd_t;

    typedef struct ap_item_s {
        string_32_t		ssid;
        char			auth [32];
        char			encry [8];
        int             indicator;
    } ap_item_t;

    /* WIFI模块重启 */
    int wifi_reboot ();

    /* 获取AP列表 */
    int wifi_get_ap_list (ap_item_t ap_lst []);

    /* 查询STA状态下的IP地址 */
    int wifi_get_sta_net_status (unsigned int * ip, unsigned int * net_mask, unsigned int * gateway);

    /* 查询wifi无线连接状态 */
    int wifi_get_sta_link_status (string_32_t ssid, char ap_mac [], int * indicator);

    /* 建立AP热点，启动服务器，可进行IEC101,MODBUS通信 */
    int wifi_config_ap_tcp_server (string_32_t ssid, char * auth, char * encry, char * key, char * ip, unsigned short port);

    /* 连接到AP热点，启动服务器，可进行IEC101,MODBUS通信 */
    int wifi_config_sta_tcp_server (string_32_t ssid_dst, char * auth, char * encry, char * key, unsigned short port);

    /* 连接到AP热点，并连接指定的TCP服务器，可进行外同步(连接WIFI同步器)，IEC101,MODBUS通信 */
    int wifi_config_sta_tcp_client (string_32_t ssid_dst, char * auth, char * encry, char * key, char * ip_dst, unsigned short port_dst);

    wifi_at_cmd_t wifi_at_cmd;

    int init_wifi_dev (wifi_at_cmd_t * wa, int baundrate);
    int close_wifi_dev (wifi_at_cmd_t * wa);
    int wifi_recv_msg (wifi_at_cmd_t * wa);
    int wifi_send_msg (wifi_at_cmd_t * wa);
    int build_cmd_req (wifi_at_cmd_t * wa, int cmd, int b_set, void * set_data);
    int check_cmd_resp (wifi_at_cmd_t * wa);
    static int check_ap_auth_encry (char * auth, char * encry);
    static int check_sta_auth_encry (char * auth, char * encry);
    int wifi_enter_cmd_mode (wifi_at_cmd_t * wa);
    int wifi_exit_cmd_mode (wifi_at_cmd_t * wa);
    int wifi_do_cmd (wifi_at_cmd_t * wa, int cmd, int b_set, void * set_data);


};

#endif // WIFI_H











