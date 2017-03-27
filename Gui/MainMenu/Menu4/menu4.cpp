#include "menu4.h"

Menu4::Menu4(QWidget *parent) : QFrame(parent)
{
    key_val = NULL;

    this->resize(459, 219);
    this->move(10, 28);
    this->setStyleSheet("Menu4 {border-image: url(:/widgetphoto/mainmenu/tab5.png);}");

    /* menu list */
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
    main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m51.png);}");
    main_title4->move(main_title3->x() + 45, main_title0->y());

    main_title5 = new QLabel(this);
    main_title5->resize(main_title0->width(), main_title0->height());
    main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
    main_title5->move(main_title4->x() + 45, main_title0->y());

    main_title6 = new QLabel(this);
    main_title6->resize(main_title0->width(), main_title0->height());
    main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m70.png);}");
    main_title6->move(main_title5->x() + 45, main_title0->y());

    /* grade II menu image */
    amplitude_lab = new QLabel(this);
    amplitude_lab->resize(174, 42);
    amplitude_lab->move(150, 50);
    amplitude_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    amplitude_txt_lab = new QLabel(amplitude_lab);
    amplitude_txt_lab->resize(153, 21);
    amplitude_txt_lab->move(11, 7);
    amplitude_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    amplitude_txt_lab->setStyleSheet("QLabel {color:gray;}");
    amplitude_txt_lab->setAlignment(Qt::AlignCenter);
    amplitude_txt_lab->setText(tr("幅值检测模式"));

    /* grade II menu image */
    atlas_lab = new QLabel(this);
    atlas_lab->resize(amplitude_lab->width(), amplitude_lab->height());
    atlas_lab->move(amplitude_lab->x(), amplitude_lab->y() + 27);
    atlas_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    atlas_txt_lab = new QLabel(atlas_lab);
    atlas_txt_lab->resize(153, 21);
    atlas_txt_lab->move(11, 7);
    atlas_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    atlas_txt_lab->setStyleSheet("QLabel {color:gray;}");
    atlas_txt_lab->setAlignment(Qt::AlignCenter);
    atlas_txt_lab->setText(tr("周期谱图检测模式"));

    /* two work mode: phase, pulse and wave */
    hfcamplitude = new HFCAmplitude(this);
    hfcatlas = new HFCAtlas(this);

    /* create signal and slots */
    connect(this, &Menu4::send_key, hfcamplitude, &HFCAmplitude::trans_key);
    connect(this, &Menu4::send_key, hfcatlas, &HFCAtlas::trans_key);

    connect(hfcamplitude, &HFCAmplitude::fresh_parent, this, &Menu4::fresh_grade1);
    connect(hfcatlas, &HFCAtlas::fresh_parent, this, &Menu4::fresh_grade1);
}

void Menu4::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    if (!key_val->grade.val1) {
        hfcamplitude->hide();
        hfcatlas->hide();
    }
    fresh_table();
    this->show();
}

void Menu4::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }
    if (key_val->grade.val0 != 4) {                                             //not current menu
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if (!key_val->grade.val1) {
            key_val->grade.val1 = 1;
            fresh_grade1();
        } else if (key_val->grade.val1 == 1) {
            key_val->grade.val2 = 1;
            hfcamplitude->working(key_val);
            hfcatlas->hide();
        } else if (key_val->grade.val1 == 2) {
            key_val->grade.val2 = 1;
            hfcamplitude->hide();
            hfcatlas->working(key_val);
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
                key_val->grade.val1 = 2;
            }
            fresh_grade1();
        }
        break;
    case KEY_DOWN:
        if (!key_val->grade.val2) {
            if (key_val->grade.val1 < 2) {
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

void Menu4::fresh_grade1(void)
{
    hfcamplitude->hide();
    hfcatlas->hide();
    this->show();                                                               //show current gui

    switch (key_val->grade.val1) {
    case 0:
        amplitude_txt_lab->setStyleSheet("QLabel {color:gray;}");
        atlas_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 1:
        amplitude_txt_lab->setStyleSheet("QLabel {color:white;}");
        atlas_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 2:
        amplitude_txt_lab->setStyleSheet("QLabel {color:gray;}");
        atlas_txt_lab->setStyleSheet("QLabel {color:white;}");
        break;
    default:
        break;
    }
    emit send_title_val(*key_val);
}

void Menu4::fresh_table(void)
{
    if (key_val->grade.val0 == 4 && !key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m12.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m22.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m32.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m42.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m51.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m62.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m72.png);}");
    } else if (key_val->grade.val0 == 4 && key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m10.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m20.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m30.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m52.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m70.png);}");
    }
}
