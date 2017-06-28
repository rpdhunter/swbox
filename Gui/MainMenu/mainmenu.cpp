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
//    freqLab->move(this->width()-freqLab->width()+14, 248);
    freqLab->move(310, 245);
    freqLab->setStyleSheet("color:white;");
    freqLab->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
//    menu_title->setText(tr("当前状态:"));
    freqLab->setText(tr("%1Hz").arg(sqlcfg->get_para()->freq_val));
    freqLab->setAlignment(Qt::AlignVCenter);

    menu_title_name = new QLabel(this);
    menu_title_name->resize(250, 24);
//    menu_title_name->move(70, 0);
    menu_title_name->move(10, 245);
    menu_title_name->setStyleSheet("color:white;");
    menu_title_name->setAlignment(Qt::AlignVCenter);


    sys_info = new QLabel(this);
    sys_info->setPixmap(QPixmap(":/widgetphoto/ohv_gary.png"));
    sys_info->resize(120,20);
    sys_info->move(350,245);
    sys_info->setScaledContents(true);

    /* status bar */
    statusbar = new StatusBar(this);
    statusbar->show();

    /* menu view */
    menu0 = new Menu0(this, g_data);
    menu0->hide();

    menu1 = new Menu1(this, g_data);
    menu1->hide();

    menu2 = new Menu2(this, g_data);
    menu2->hide();

    menu3 = new Menu3(this, g_data);
    menu3->hide();

    menu4 = new Menu4(this);
    menu4->hide();

    menu5 = new Menu5(this, g_data);
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

    //录波
    connect(menu6,SIGNAL(startRecWv(int,int)),this,SIGNAL(startRecWv(int,int)));
    connect(menu0,SIGNAL(startRecWave(int,int)),this,SIGNAL(startRecWv(int,int)));
    connect(menu1,SIGNAL(startRecWave(int,int)),this,SIGNAL(startRecWv(int,int)));
    connect(menu3,SIGNAL(startRecWave(int,int)),this,SIGNAL(startRecWv(int,int)));

    //杂项状态即时显示
    connect(menu6,SIGNAL(fregChanged(int)),this,SLOT(fresg_freq(int)));
    connect(menu6,SIGNAL(closeTimeChanged(int)),this,SIGNAL(closeTimeChanged(int)));
    connect(menu6,SIGNAL(sysReset()),this,SLOT(sysReset()));
    connect(menu6,SIGNAL(update_statusBar(QString)),menu_title_name,SLOT(setText(QString)));

    //偏置
    connect(menu0,SIGNAL(offset_suggest(int,int)),menu6,SLOT(set_offset_suggest1(int,int)));
    connect(menu1,SIGNAL(offset_suggest(int,int)),menu6,SLOT(set_offset_suggest2(int,int)));
    connect(menu3,SIGNAL(offset_suggest(int)),menu6,SLOT(set_AA_offset_suggest(int)));

    //modbus相关
    connect(menu0,SIGNAL(tev_modbus_data(int,int)),this,SIGNAL(tev_modbus_data(int,int)));
    connect(menu3,SIGNAL(aa_modbus_data(int)),this,SIGNAL(aa_modbus_data(int)));

    connect(menu0,SIGNAL(tev_modbus_data(int,int)),menu2,SLOT(showLeftData(int,int)));
    connect(menu1,SIGNAL(tev_modbus_data(int,int)),menu2,SLOT(showRightData(int,int)));

    //故障定位相关
    connect(menu0,SIGNAL(origin_pluse_points(QVector<QPoint>,int)),menu2,SLOT(get_origin_points(QVector<QPoint>,int)));
    connect(menu1,SIGNAL(origin_pluse_points(QVector<QPoint>,int)),menu2,SLOT(get_origin_points(QVector<QPoint>,int)));

    //播放声音
    connect(menu6,SIGNAL(play_voice(VectorList)),this,SIGNAL(play_voice(VectorList)));
    connect(menu6,SIGNAL(stop_play_voice()),this,SIGNAL(stop_play_voice()));
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
        menu_title_name->setText(tr("地电波检测(通道1)"));
        break;
    case MENU1:
        menu_title_name->setText(tr("地电波检测(通道2)"));
        break;
    case MENU2:
        menu_title_name->setText(tr("地电波检测(双通道定位)"));
        break;
    case MENU3:
        menu_title_name->setText(tr("AA超声波检测"));
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
        menu_title_name->setText(tr("电缆局放检测"));
        break;
    case MENU6:
        if (!val.grade.val1) {
            menu_title_name->setText(tr("系统设置"));
        } else if (val.grade.val1 == 1){
            menu_title_name->setText(tr("系统设置-参数设置"));
        } else if (val.grade.val1 == 2) {
            menu_title_name->setText(tr("系统设置-调试模式"));
        } else if (val.grade.val1 == 3){
            menu_title_name->setText(tr("系统设置-录波管理"));
        } else if (val.grade.val1 == 4) {
            menu_title_name->setText(tr("系统设置-系统信息"));
        } else if (val.grade.val1 == 5) {
            menu_title_name->setText(tr("系统设置-恢复出厂"));
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
            if(key_val.grade.val0 == 5){
                emit switch_rfct_mode(0);
            }

            if (key_val.grade.val0 == GRADE0_MENU_MIN_NUM) {
                key_val.grade.val0 = GRADE0_MENU_MAX_NUM;
            } else {
                if(sqlcfg->get_para()->full_featured){
                    if(key_val.grade.val0 == 5){
                        key_val.grade.val0 = 3;
                    }
                    else{
                        key_val.grade.val0--;
                    }
                }
                else{
                    key_val.grade.val0 -= 3;
                }
            }

            if(key_val.grade.val0 == 5){
                emit switch_rfct_mode(1);
            }
            break;

        case KEY_RIGHT:

            if(key_val.grade.val0 == 5){
                emit switch_rfct_mode(0);
            }

            if (key_val.grade.val0 == GRADE0_MENU_MAX_NUM) {
                key_val.grade.val0 = GRADE0_MENU_MIN_NUM;
            } else {
                if(sqlcfg->get_para()->full_featured){
                    if(key_val.grade.val0 == 3){
                        key_val.grade.val0 = 5;
                    }
                    else{
                        key_val.grade.val0++;
                    }
                }
                else{
                    key_val.grade.val0 += 3;
                }
            }

            if(key_val.grade.val0 == 5){
                emit switch_rfct_mode(1);
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

void MainMenu::showWaveData(VectorList buf,MODE mod)
{
    if(menu6->isVisible()){
        menu6->showWaveData(buf,mod);
        qDebug()<<"menu6 show wave data!";
    }
    if(menu0->isVisible() && mod == TEV1){
        menu0->showWaveData(buf,mod);
        qDebug()<<"menu0 show wave data!";
    }
    if(menu1->isVisible() && mod == TEV2){
        menu1->showWaveData(buf,mod);
        qDebug()<<"menu1 show wave data!";
    }
    if(menu3->isVisible() && mod == AA_Ultrasonic){
        menu3->showWaveData(buf,mod);
        qDebug()<<"menu3 show wave data!";
    }

    if(menu2->isVisible() && mod == TEV_Double){
        menu2->showWaveData(buf,mod);
        qDebug()<<"menu3 show wave data!";
    }

//    if(menu5->isVisible() && (mod == TEV1 || mod == TEV2 ) ){
//        menu5->showWaveData(buf,mod);
//        qDebug()<<"menu3 show wave data!";
//    }

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
	data->set_send_para (sp_freq, (sqlcfg->get_para()->freq_val == 50) ? 0 : 1);
    freqLab->setText(tr("%1Hz").arg(sqlcfg->get_para()->freq_val));
	data->set_send_para (sp_backlight, sqlcfg->get_para()->backlight);
}

void MainMenu::playVoiceProgress(int p, int all,bool f)
{
    menu6->playVoiceProgress(p,all,f);
}





