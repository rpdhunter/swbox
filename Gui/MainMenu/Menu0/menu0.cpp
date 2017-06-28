#include "menu0.h"

Menu0::Menu0(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    key_val = NULL;

    this->resize(459, 219);
    this->move(10, 28);
    this->setStyleSheet("Menu0 {border-image: url(:/widgetphoto/mainmenu/tab1.png);}");

    /* menu list */
    main_title0 = new QLabel(this);
    main_title0->resize(39, 24);
    main_title0->move(2, 3);

    main_title1 = new QLabel(this);
    main_title1->resize(main_title0->width(), main_title0->height());
    main_title1->move(main_title0->x() + 45, main_title0->y());

    main_title2 = new QLabel(this);
    main_title2->resize(main_title0->width(), main_title0->height());
    main_title2->move(main_title1->x() + 45, main_title0->y());

    main_title3 = new QLabel(this);
    main_title3->resize(main_title0->width(), main_title0->height());
    main_title3->move(main_title2->x() + 45, main_title0->y());

    main_title4 = new QLabel(this);
    main_title4->resize(main_title0->width(), main_title0->height());
    main_title4->move(main_title3->x() + 45, main_title0->y());

    main_title5 = new QLabel(this);
    main_title5->resize(main_title0->width(), main_title0->height());
    main_title5->move(main_title4->x() + 45, main_title0->y());

    main_title6 = new QLabel(this);
    main_title6->resize(main_title0->width(), main_title0->height());
    main_title6->move(main_title5->x() + 45, main_title0->y());

    tevWidget = new TEVWidget(g_data,TEVWidget::Channel::Left,this);
    connect(this, &Menu0::send_key, tevWidget, &TEVWidget::trans_key);
    connect(tevWidget,SIGNAL(offset_suggest(int,int)),this,SIGNAL(offset_suggest(int,int)));
    connect(tevWidget,SIGNAL(tev_modbus_data(int,int)),this,SIGNAL(tev_modbus_data(int,int)));
    connect(tevWidget,SIGNAL(origin_pluse_points(QVector<QPoint>,int)),this,SIGNAL(origin_pluse_points(QVector<QPoint>,int)));
    connect(tevWidget,SIGNAL(startRecWave(int,int)),this,SIGNAL(startRecWave(int,int)));
}

void Menu0::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    tevWidget->working(key_val);
    fresh_table();  //改变顶层图标显示
    this->show();
}

void Menu0::showWaveData(VectorList buf, MODE mod)
{
    tevWidget->showWaveData(buf,mod);
}

void Menu0::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }
    if (key_val->grade.val0) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        key_val->grade.val1 = 0;
//        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_UP:
        if (!key_val->grade.val1) {
            key_val->grade.val1 = 1;
            key_val->grade.val2 = 0;
        }
        break;
    case KEY_DOWN:
        if (!key_val->grade.val1) {
            key_val->grade.val1 = 1;
            key_val->grade.val2 = 0;
        }
        break;
    case KEY_LEFT:

        break;

    case KEY_RIGHT:

        break;
    default:
        break;
    }
    fresh_table();  //改变顶层图标显示
    emit send_key(key_code);
}

//控制二级菜单的灰白变化
//灰色代表为被选中，白色代表被选中
//7个菜单类（不含Menu1）中均包含grade1函数，完成了二级菜单的显示控制
//作者grade1的命名方式可能包含了日后扩展grade2...的想法

//void Menu0::fresh_grade1(void)
//{
//    amplitude->hide();                                                          //hide before gui
//    pulse->hide();
//    this->show();                                                               //show current gui

//    switch (key_val->grade.val1) {
//    case 0:
//        amplitude_txt_lab->setStyleSheet("QLabel {color:gray;}");
//        pulse_txt_lab->setStyleSheet("QLabel {color:gray;}");
//        break;
//    case 1:
//        amplitude_txt_lab->setStyleSheet("QLabel {color:white;}");
//        pulse_txt_lab->setStyleSheet("QLabel {color:gray;}");
//        break;
//    case 2:
//        amplitude_txt_lab->setStyleSheet("QLabel {color:gray;}");
//        pulse_txt_lab->setStyleSheet("QLabel {color:white;}");
//        break;
//    default:
//        break;
//    }
//    emit send_title_val(*key_val);
//}

//控制顶层七个图标（对应7种测量方法）的状态变化
//图标有2种即时模式--选择模式和工作模式
//7个图标共有3×7=21种状态
//编号m?0为暗图标，出现在工作模式中，代表方法非当前使用
//编号m?1为亮图标带边，出现在选择模式中，代表方法被选中
//编号m?2为亮图标不带边，出现在工作模式中时，代表当前正使用此测量方法，出现在选择模式中时，代表此方法未被选中

//所有7个菜单类中均包含fresh_table函数，但是图标状态各自不同，协同完成了最顶层图标的显示工作

void Menu0::fresh_table(void)
{
    if (!key_val->grade.val0 && !key_val->grade.val1) {
        //选择状态，代表选中地电波模式
        if(sqlcfg->get_para()->full_featured){
            main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m11.png);}");
            main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m22.png);}");
            main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m32.png);}");
            main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m42.png);}");
            main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
            main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m62.png);}");
            main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m72.png);}");
        }
        else{
            main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m11.png);}");
            main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m20.png);}");
            main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
            main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m42.png);}");
            main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
            main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
            main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m72.png);}");

        }
    } else if (!key_val->grade.val0 && key_val->grade.val1) {
        //工作状态，代表处于地电波模式工作状态
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m12.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m20.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m70.png);}");
    }
}
