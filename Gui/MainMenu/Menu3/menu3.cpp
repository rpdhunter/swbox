#include "menu3.h"

Menu3::Menu3(QWidget *parent, G_PARA *g_data) : QFrame(parent)
{
    key_val = NULL;

    this->resize(459, 219);
    this->move(10, 28);
    this->setStyleSheet("Menu3 {border-image: url(:/widgetphoto/mainmenu/tab4.png);}");

    /* menu list */
    main_title0 = new QLabel(this);
    main_title0->resize(39, 24);
    main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m10.png);}");
    main_title0->move(2, 3);

    main_title1 = new QLabel(this);
    main_title1->resize(main_title0->width(), main_title0->height());
    main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m21.png);}");
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
    main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m70.png);}");
    main_title6->move(main_title5->x() + 45, main_title0->y());

    /* content */

    aaultrasonic = new AAUltrasonic2(this, g_data);

    connect(this, SIGNAL(send_key(quint8)), aaultrasonic, SLOT(trans_key(quint8)));

}

void Menu3::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    aaultrasonic->working(key_val);
    fresh_table();
    this->show();
}

void Menu3::sysReset()
{
//    aaultrasonic->sysReset();
}

void Menu3::maxReset()
{
//    aaultrasonic->maxReset();
}

void Menu3::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 3) {
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
            //emit send_title_val(*key_val);
        }
        break;
    case KEY_DOWN:
        if (!key_val->grade.val1) {
            key_val->grade.val1 = 1;
            key_val->grade.val2 = 0;
            //emit send_title_val(*key_val);
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

void Menu3::fresh_table(void)
{
    if (key_val->grade.val0 == 3 && !key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m12.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m22.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m32.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m41.png);}");
//        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m52.png);}");
//        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m62.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m72.png);}");
    } else if (key_val->grade.val0 == 3 && key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m10.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m22.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m70.png);}");
    }
}
