#include "menu6.h"

Menu6::Menu6(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    key_val = NULL;

    this->resize(459, 219);
    this->move(10, 28);
    this->setStyleSheet("Menu6 {border-image: url(:/widgetphoto/mainmenu/tab7.png);}");

    /* menu list */
    //菜单栏的重绘，当然是"设置"这一栏是高亮的，表示选中状态
    main_title0 = new QLabel(this);
    main_title0->resize(39, 24);
    main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m10.png);}");
    main_title0->move(2, 3);

    main_title1 = new QLabel(this);
    main_title1->resize(main_title0->width(), main_title0->height());
    main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m20.png);}");
    main_title1->move(main_title0->x() + 45, main_title0->y());

    main_title2 = new QLabel(this);
    main_title2->resize(main_title0->width(), main_title0->height());
    main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
    main_title2->move(main_title1->x() + 45, main_title0->y());

    main_title3 = new QLabel(this);
    main_title3->resize(main_title0->width(), main_title0->height());
    main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
    main_title3->move(main_title2->x() + 45, main_title0->y());

    main_title4 = new QLabel(this);
    main_title4->resize(main_title0->width(), main_title0->height());
    main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
    main_title4->move(main_title3->x() + 45, main_title0->y());

    main_title5 = new QLabel(this);
    main_title5->resize(main_title0->width(), main_title0->height());
    main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
    main_title5->move(main_title4->x() + 45, main_title0->y());

    main_title6 = new QLabel(this);
    main_title6->resize(main_title0->width(), main_title0->height());
    main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m71.png);}");
    main_title6->move(main_title5->x() + 45, main_title0->y());

    /* grade II menu image */
    //二级菜单的重绘
    options_lab = new QLabel(this);
    options_lab->resize(174, 42);
    options_lab->move(150, 50);
    options_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    options_txt_lab = new QLabel(options_lab);
    options_txt_lab->resize(153, 21);
    options_txt_lab->move(11, 7);
    options_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    options_txt_lab->setStyleSheet("QLabel {color:gray;}");
    options_txt_lab->setAlignment(Qt::AlignCenter);
    options_txt_lab->setText(tr("参数设置"));

    /* grade II menu image */
    debug_lab = new QLabel(this);
    debug_lab->resize(options_lab->width(), options_lab->height());
    debug_lab->move(options_lab->x(), options_lab->y() + 27);
    debug_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    debug_txt_lab = new QLabel(debug_lab);
    debug_txt_lab->resize(153, 21);
    debug_txt_lab->move(11, 7);
    debug_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    debug_txt_lab->setStyleSheet("QLabel {color:gray;}");
    debug_txt_lab->setAlignment(Qt::AlignCenter);
    debug_txt_lab->setText(tr("调试模式"));

    /* grade II menu image */
    systeminfo_lab = new QLabel(this);
    systeminfo_lab->resize(options_lab->width(), options_lab->height());
    systeminfo_lab->move(options_lab->x(), debug_lab->y() + 27);
    systeminfo_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    systeminfo_txt_lab = new QLabel(systeminfo_lab);
    systeminfo_txt_lab->resize(153, 21);
    systeminfo_txt_lab->move(11, 7);
    systeminfo_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    systeminfo_txt_lab->setStyleSheet("QLabel {color:gray;}");
    systeminfo_txt_lab->setAlignment(Qt::AlignCenter);
    systeminfo_txt_lab->setText(tr("系统信息"));

    /* grade II menu image */
    default_lab = new QLabel(this);
    default_lab->resize(options_lab->width(), options_lab->height());
    default_lab->move(options_lab->x(), systeminfo_lab->y() + 27);
    default_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    default_txt_lab = new QLabel(default_lab);
    default_txt_lab->resize(153, 21);
    default_txt_lab->move(11, 7);
    default_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    default_txt_lab->setStyleSheet("QLabel {color:gray;}");
    default_txt_lab->setAlignment(Qt::AlignCenter);
    default_txt_lab->setText(tr("恢复出厂"));



    /* 5 setting mode */
    options = new Options(this,g_data);
    debugset = new DebugSet(this,g_data);
    systeminfo = new SystemInfo(this);
    factoryreset = new FactoryReset(this);



    /* create signal and slots */
    connect(this, &Menu6::send_key, options, &Options::trans_key);
    connect(this, &Menu6::send_key, debugset, &DebugSet::trans_key);
    connect(this, &Menu6::send_key, systeminfo, &SystemInfo::trans_key);
    connect(this, &Menu6::send_key, factoryreset, &FactoryReset::trans_key);

    connect(options, &Options::fresh_parent, this, &Menu6::fresh_grade1);
    connect(debugset, &DebugSet::fresh_parent, this, &Menu6::fresh_grade1);
    connect(systeminfo, &SystemInfo::fresh_parent, this, &Menu6::fresh_grade1);
    connect(factoryreset, &FactoryReset::fresh_parent, this, &Menu6::fresh_grade1);

    //录波信号
    connect(debugset,SIGNAL(startRecWv(int,int)),this,SIGNAL(startRecWv(int,int)));

    //频率变化
    connect(options,SIGNAL(fregChanged(int)),this,SIGNAL(fregChanged(int)) );
    connect(options,SIGNAL(closeTimeChanged(int)),this,SIGNAL(closeTimeChanged(int)) );
    connect(options,SIGNAL(maxResetTimeChanged(int)),this,SIGNAL(maxResetTimeChanged(int)));
    connect(factoryreset,SIGNAL(sysReset()),this,SIGNAL(sysReset()));
}

void Menu6::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    if (!key_val->grade.val1) {
        options->hide();
        systeminfo->hide();
        debugset->hide();
        factoryreset->hide();
    }
    fresh_table();
    this->show();
}

void Menu6::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }
    if (key_val->grade.val0 != 6) {                                             //not current menu
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if (!key_val->grade.val1) {
            key_val->grade.val1 = 1;
            fresh_grade1();
        } else if (key_val->grade.val2) {                                       //disable key if in children menu now
            break;
        } else if (key_val->grade.val1 == 1) {
            key_val->grade.val2 = 1;
            options->working(key_val);
            debugset->hide();
            systeminfo->hide();
            factoryreset->hide();
        } else if (key_val->grade.val1 == 2) {
            key_val->grade.val2 = 1;
            options->hide();
            debugset->working(key_val);
            systeminfo->hide();
            factoryreset->hide();
        } else if (key_val->grade.val1 == 3) {
            key_val->grade.val2 = 1;
            options->hide();
            debugset->hide();
            systeminfo->working(key_val);
            factoryreset->hide();
        } else if (key_val->grade.val1 == 4) {
            key_val->grade.val2 = 1;
            options->hide();
            debugset->hide();
            systeminfo->hide();
            factoryreset->working(key_val);
        }
        break;
    case KEY_CANCEL:
        if (!key_val->grade.val2) {
            key_val->grade.val1 = 0;
            fresh_grade1();
        }
        break;
    case KEY_UP:
        if (!key_val->grade.val2) {
            if (key_val->grade.val1 > 1) {
                key_val->grade.val1--;
            } else {
                key_val->grade.val1 = 4;
            }
            fresh_grade1();
        }
        break;
    case KEY_DOWN:
        if (!key_val->grade.val2) {
            if (key_val->grade.val1 < 4) {
                key_val->grade.val1++;
            } else {
                key_val->grade.val1 = 1;
            }
            fresh_grade1();
        }
        break;
    case KEY_LEFT:

        break;

    case KEY_RIGHT:

        break;
    default:
        break;
    }
    fresh_table();
    emit send_key(key_code);
}

void Menu6::showWaveData(VectorList buf,MODE mod)
{
    debugset->showWaveData(buf,mod);
}

void Menu6::set_offset_suggest(int a, int b)
{
    debugset->set_offset_suggest(a,b);
}


//二级菜单选中状态
void Menu6::fresh_grade1(void)
{
//    printf("\nkey_val->grade.val1 is : %d",key_val->grade.val1);
//    printf("\tkey_val->grade.val2 is : %d",key_val->grade.val2);
//    printf("\tkey_val->grade.val3 is : %d",key_val->grade.val3);



    options->hide();
    systeminfo->hide();
    debugset->hide();
    factoryreset->hide();

    this->show();                                                               //show current gui

    switch (key_val->grade.val1) {
    case 0:
        options_txt_lab->setStyleSheet("QLabel {color:gray;}");
        debug_txt_lab->setStyleSheet("QLabel {color:gray;}");
        systeminfo_txt_lab->setStyleSheet("QLabel {color:gray;}");
        default_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 1:
        options_txt_lab->setStyleSheet("QLabel {color:white;}");
        debug_txt_lab->setStyleSheet("QLabel {color:gray;}");
        systeminfo_txt_lab->setStyleSheet("QLabel {color:gray;}");
        default_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 2:
        options_txt_lab->setStyleSheet("QLabel {color:gray;}");
        debug_txt_lab->setStyleSheet("QLabel {color:white;}");
        systeminfo_txt_lab->setStyleSheet("QLabel {color:gray;}");
        default_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 3:
        options_txt_lab->setStyleSheet("QLabel {color:gray;}");
        debug_txt_lab->setStyleSheet("QLabel {color:gray;}");
        systeminfo_txt_lab->setStyleSheet("QLabel {color:white;}");
        default_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 4:
        options_txt_lab->setStyleSheet("QLabel {color:gray;}");
        debug_txt_lab->setStyleSheet("QLabel {color:gray;}");
        systeminfo_txt_lab->setStyleSheet("QLabel {color:gray;}");
        default_txt_lab->setStyleSheet("QLabel {color:white;}");
        break;
    default:
        break;
    }
    emit send_title_val(*key_val);
}

//主菜单选中状态
void Menu6::fresh_table(void)
{
    if (key_val->grade.val0 == 6 && !key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m12.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m22.png);}");
//        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m32.png);}");
//        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m42.png);}");
//        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m52.png);}");
//        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m62.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m71.png);}");
    } else if (key_val->grade.val0 == 6 && key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m10.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m20.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m72.png);}");
    }
}
