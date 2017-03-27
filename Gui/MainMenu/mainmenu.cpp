#include "mainmenu.h"


//控制主界面显示
//核心是根据键盘事件，改变图标状态
MainMenu::MainMenu(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
//    ch = 0;
    key_val.val = 0;
    data = g_data;

    this->setGeometry(0, 0, 480, 272);
    this->setStyleSheet("MainMenu {border-image: url(:/widgetphoto/bk.png);}");

    /* menu title */
    freqLab = new QLabel(this);
    freqLab->resize(60, 24);
//    menu_title->move(7, 0);
    freqLab->move(this->width()-freqLab->width()+14, 248);
    freqLab->setStyleSheet("color:white;");
    freqLab->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
//    menu_title->setText(tr("当前状态:"));
    freqLab->setText(tr("%1Hz").arg(sqlcfg->get_para()->freq_val));
    freqLab->setAlignment(Qt::AlignVCenter);

    menu_title_name = new QLabel(this);
    menu_title_name->resize(250, 24);
//    menu_title_name->move(70, 0);
    menu_title_name->move(7, 248);
    menu_title_name->setStyleSheet("color:white;");
    menu_title_name->setAlignment(Qt::AlignVCenter);

    /* status bar */
    statusbar = new StatusBar(this);
    statusbar->show();

    /* menu view */
    menu0 = new Menu0(this, g_data);
    menu0->hide();

    menu1 = new Menu1(this, g_data);
    menu1->hide();

    menu2 = new Menu2(this);
    menu2->hide();

    menu3 = new Menu3(this);
    menu3->hide();

    menu4 = new Menu4(this);
    menu4->hide();

    menu5 = new Menu5(this);
    menu5->hide();

    menu6 = new Menu6(this, g_data);
    menu6->hide();

    fresh_menu();
    fresh_title(key_val);

    /* send key value to every next grade menu */
    connect(this, &MainMenu::send_key, menu0, &Menu0::trans_key);
    connect(this, &MainMenu::send_key, menu1, &Menu1::trans_key);
    connect(this, &MainMenu::send_key, menu2, &Menu2::trans_key);
    connect(this, &MainMenu::send_key, menu3, &Menu3::trans_key);
    connect(this, &MainMenu::send_key, menu4, &Menu4::trans_key);
    connect(this, &MainMenu::send_key, menu5, &Menu5::trans_key);
    connect(this, &MainMenu::send_key, menu6, &Menu6::trans_key);

    /* every grade menu respon menu title(total key value) */
    connect(menu0, &Menu0::send_title_val, this, &MainMenu::fresh_title);
    connect(menu1, &Menu1::send_title_val, this, &MainMenu::fresh_title);
    connect(menu2, &Menu2::send_title_val, this, &MainMenu::fresh_title);
    connect(menu3, &Menu3::send_title_val, this, &MainMenu::fresh_title);
    connect(menu4, &Menu4::send_title_val, this, &MainMenu::fresh_title);
    connect(menu5, &Menu5::send_title_val, this, &MainMenu::fresh_title);
    connect(menu6, &Menu6::send_title_val, this, &MainMenu::fresh_title);

    connect(menu6,SIGNAL(startRecWv(int)),this,SIGNAL(startRecWv(int)));
    connect(menu6,SIGNAL(fregChanged(int)),this,SLOT(fresg_freq(int)));
    connect(menu6,SIGNAL(closeTimeChanged(int)),this,SIGNAL(closeTimeChanged(int)));
    connect(menu6,SIGNAL(maxResetTimeChanged(int)),this,SLOT(setMaxResetTime(int)));
    connect(menu6,SIGNAL(sysReset()),this,SLOT(sysReset()));


    max_reset_timer = new QTimer(this);
//    max_reset_timer->setInterval(sqlcfg->get_para()->reset_time * 1000 *60);    //单位为分钟
//    max_reset_timer->start();
    setMaxResetTime(sqlcfg->get_para()->reset_time);
    connect(max_reset_timer, SIGNAL(timeout()),this,SLOT(maxValReset()));

    connect(menu0,SIGNAL(offset_suggest(int,int)),menu6,SLOT(set_offset_suggest(int,int)));
}

//改变菜单栏选中状态
void MainMenu::fresh_menu(void)
{
    switch (key_val.grade.val0) {
    case MENU0:
        menu0->working(&key_val);
        menu1->hide();
        menu2->hide();
        menu3->hide();
        menu4->hide();
        menu5->hide();
        menu6->hide();
        break;
    case MENU1:
        menu1->working(&key_val);
        menu0->hide();
        menu2->hide();
        menu3->hide();
        menu4->hide();
        menu5->hide();
        menu6->hide();
        break;
    case MENU2:
        menu2->working(&key_val);
        menu0->hide();
        menu1->hide();
        menu3->hide();
        menu4->hide();
        menu5->hide();
        menu6->hide();
        break;
    case MENU3:
        menu3->working(&key_val);
        menu0->hide();
        menu1->hide();
        menu2->hide();
        menu4->hide();
        menu5->hide();
        menu6->hide();
        break;
    case MENU4:
        menu4->working(&key_val);
        menu0->hide();
        menu1->hide();
        menu2->hide();
        menu3->hide();
        menu5->hide();
        menu6->hide();
        break;
    case MENU5:
        menu5->working(&key_val);
        menu0->hide();
        menu1->hide();
        menu2->hide();
        menu3->hide();
        menu4->hide();
        menu6->hide();
        break;
    case MENU6:
        menu6->working(&key_val);
        menu0->hide();
        menu1->hide();
        menu2->hide();
        menu3->hide();
        menu4->hide();
        menu5->hide();
        break;
    default:
        break;
    }
}

//刷新标题显示
void MainMenu::fresh_title(CURRENT_KEY_VALUE val)
{
    switch (val.grade.val0) {
    case MENU0:
        menu_title_name->setText(tr("地电波检测(TEV)"));
        break;
    case MENU1:
        menu_title_name->setText(tr("AA超声波检测"));
        break;
    case MENU2:
        if (!val.grade.val1) {
            menu_title_name->setText(tr("AE超声波"));
        } else if (val.grade.val1 == 1){
            menu_title_name->setText(tr("AE超声波-相位谱图检测"));
        } else if (val.grade.val1 == 2) {
            menu_title_name->setText(tr("AE超声波-脉冲谱图检测"));
        } else if (val.grade.val1 == 3) {
            menu_title_name->setText(tr("AE超声波-波形谱图检测"));
        }
        break;
    case MENU3:
        if (!val.grade.val1) {
            menu_title_name->setText(tr("特高频检测"));
        } else if (val.grade.val1 == 1){
            menu_title_name->setText(tr("特高频检测-幅值检测模式"));
        } else if (val.grade.val1 == 2) {
            menu_title_name->setText(tr("特高频检测-周期谱图检测模式"));
        } else if (val.grade.val1 == 3) {
            menu_title_name->setText(tr("特高频检测-PRPS检测模式"));
        }
        break;
    case MENU4:
        if (!val.grade.val1) {
            menu_title_name->setText(tr("高频电流检测"));
        } else if (val.grade.val1 == 1){
            menu_title_name->setText(tr("高频电流检测-幅值检测模式"));
        } else if (val.grade.val1 == 2) {
            menu_title_name->setText(tr("高频电流检测-周期谱图检测模式"));
        }
        break;
    case MENU5:
        if (!val.grade.val1) {
            menu_title_name->setText(tr("智能巡检"));
        } else if (val.grade.val1 == 1){
            menu_title_name->setText(tr("智能巡检-载入数据"));
        } else if (val.grade.val1 == 2) {
            menu_title_name->setText(tr("智能巡检-环境记录"));
        } else if (val.grade.val1 == 3){
            menu_title_name->setText(tr("智能巡检-背景记录"));
        } else if (val.grade.val1 == 4) {
            menu_title_name->setText(tr("智能巡检-测试部位"));
        }
        break;
    case MENU6:
        if (!val.grade.val1) {
            menu_title_name->setText(tr("系统设置"));
        } else if (val.grade.val1 == 1){
            menu_title_name->setText(tr("系统设置-参数设置"));
        } else if (val.grade.val1 == 2) {
            menu_title_name->setText(tr("系统设置-调试模式"));
        } else if (val.grade.val1 == 3){
            menu_title_name->setText(tr("系统设置-系统信息"));
        } else if (val.grade.val1 == 4) {
            menu_title_name->setText(tr("系统设置-恢复出厂"));
        } else if (val.grade.val1 == 5) {
            menu_title_name->setText(tr("系统设置-"));
        }
        break;
    default:
        break;
    }
}

//将按键信息，转换成key_val变量，进行保存
void MainMenu::trans_key(quint8 key_code)
{
//    qDebug("key_val = %02x [FILE:%s LINE:%d]", key_code, __FILE__, __LINE__); //打印键盘码
    if (!key_val.grade.val1) {                                                  //current grade0 menu
        switch (key_code) {
        case KEY_LEFT:
            if (key_val.grade.val0 == GRADE0_MENU_MIN_NUM) {
                key_val.grade.val0 = GRADE0_MENU_MAX_NUM;
            } else {
                if(key_val.grade.val0 == 6){
                    key_val.grade.val0 = 1;
                }
                else{
                    key_val.grade.val0--;
                }
            }
            break;
        case KEY_RIGHT:
            if (key_val.grade.val0 == GRADE0_MENU_MAX_NUM) {
                key_val.grade.val0 = GRADE0_MENU_MIN_NUM;
            } else {
                if(key_val.grade.val0 == 1){
                    key_val.grade.val0 = 6;
                }
                else{
                    key_val.grade.val0++;
                }
            }
            break;
        case KEY_OK:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_CANCEL:
            break;
        default:
            break;
        }
        fresh_menu();
        fresh_title(key_val);
    }
    emit send_key(key_code);                                                    //not current grade key
}

void MainMenu::showWaveData(quint32 *buf, int len, int mod)
{
    menu6->showWaveData(buf,len,mod);
}

void MainMenu::fresg_freq(int fre)
{
    this->freqLab->setText(QString("%1Hz").arg(fre));
}

void MainMenu::showReminTime(int s,QString str)
{
    if(s >50){
        this->fresh_title(key_val);
    }
    else{
        menu_title_name->setText(tr("%1秒后%2").arg(s).arg(str));
    }
}

void MainMenu::sysReset()
{
    data->send_para.freq.rval = (FREQ_REG << 16) | ((sqlcfg->get_para()->freq_val == 50) ? 0 : 1);
    data->send_para.freq.flag = 1;
    freqLab->setText(tr("%1Hz").arg(sqlcfg->get_para()->freq_val));
    data->send_para.blacklight.rval = (BLACKLIGHT_REG << 16) | sqlcfg->get_para()->backlight;
    data->send_para.blacklight.flag = 1;
    menu0->sysReset();
    menu1->sysReset();
}

void MainMenu::maxValReset()
{
    menu0->maxReset();
    menu1->maxReset();
}

void MainMenu::setMaxResetTime(int m)
{
//    qDebug()<<"m = "<<sqlcfg->get_para()->reset_time;

    if(m>0){
        max_reset_timer->setInterval(m * 1000 *60);    //单位为分钟
        max_reset_timer->start();
    }
    else{
        max_reset_timer->stop();
    }
}

void MainMenu::showMaxResetTime()
{
    if(max_reset_timer->isActive()){
        showReminTime(max_reset_timer->remainingTime()/1000,tr("测量数据最大值将重置!"));
        qDebug()<<"max_reset_timer is Active! interval="<<max_reset_timer->interval();
    }


}

