#ifndef MAINMENU_H
#define MAINMENU_H

#include <QFrame>
#include <QLabel>
#include <QTimer>
#include <QFont>
#include <QFontDatabase>
#include "StatusBar/statusbar.h"
#include "Menu0/menu0.h"
#include "Menu1/menu1.h"
#include "Menu2/menu2.h"
#include "Menu3/menu3.h"
#include "Menu4/menu4.h"
#include "Menu5/menu5.h"
#include "Menu6/menu6.h"
#include "IO/Key/key.h"
#include "IO/Data/data.h"

enum MENU_LIST {
    MENU0 = 0,
    MENU1,
    MENU2,
    MENU3,
    MENU4,
    MENU5,
    MENU6,
};

class MainMenu : public QFrame
{
#define MAINMENU_NUM        8                                                   //main menu number
    Q_OBJECT
public:
    explicit MainMenu(QWidget *parent = NULL, G_PARA *g_data = NULL);

signals:
    void send_key(quint8);

    void startRecWv(int,int);      //开始录播信号,参数范围0-5,0为地电波,1为AA超声...
    void closeTimeChanged(int);
    void tev_modbus_data(int,int);
    void aa_modbus_data(int);
    void play_voice(VectorList);        //发送播放声音的指令
    void stop_play_voice();             //终止播放

public slots:
    void fresh_menu(void);
    void fresh_title(CURRENT_KEY_VALUE);
    void trans_key(quint8);
    void showWaveData(VectorList buf, MODE mod);
    void fresg_freq(int fre);   //刷新频率

    void showReminTime(int s, QString str); //状态栏显示剩余时间
    void sysReset();        //把系统中各显示部位设成默认值

    void playVoiceProgress(int p, int all, bool f);      //控制播放器进度

private:
    StatusBar *statusbar;
    Menu0 *menu0;
    Menu1 *menu1;
    Menu2 *menu2;
    Menu3 *menu3;
    Menu4 *menu4;
    Menu5 *menu5;
    Menu6 *menu6;
    G_PARA *data;

    QLabel *freqLab, *menu_title_name;
    QLabel *sys_info;

    int grade0_val;
    CURRENT_KEY_VALUE key_val;

//    QTimer *max_reset_timer;   //最大值清零计时器

};

#endif // MAINMENU_H
