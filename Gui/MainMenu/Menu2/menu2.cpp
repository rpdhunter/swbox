#include "menu2.h"

Menu2::Menu2(QWidget *parent) : QFrame(parent)
{
    key_val = NULL;

    this->resize(459, 219);
    this->move(10, 28);
    this->setStyleSheet("Menu2 {border-image: url(:/widgetphoto/mainmenu/tab3.png);}");

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
    main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m31.png);}");
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

    /* grade II menu image */
    phase_lab = new QLabel(this);
    phase_lab->resize(174, 42);
    phase_lab->move(150, 50);
    phase_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    phase_txt_lab = new QLabel(phase_lab);
    phase_txt_lab->resize(153, 21);
    phase_txt_lab->move(11, 7);
    phase_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    phase_txt_lab->setStyleSheet("QLabel {color:gray;}");
    phase_txt_lab->setAlignment(Qt::AlignCenter);
    phase_txt_lab->setText(tr("相位谱图检测模式"));

    /* grade II menu image */
    pulse_lab = new QLabel(this);
    pulse_lab->resize(phase_lab->width(), phase_lab->height());
    pulse_lab->move(phase_lab->x(), phase_lab->y() + 27);
    pulse_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    pulse_txt_lab = new QLabel(pulse_lab);
    pulse_txt_lab->resize(153, 21);
    pulse_txt_lab->move(11, 7);
    pulse_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    pulse_txt_lab->setStyleSheet("QLabel {color:gray;}");
    pulse_txt_lab->setAlignment(Qt::AlignCenter);
    pulse_txt_lab->setText(tr("脉冲谱图检测模式"));

    /* grade II menu image */
    wave_lab = new QLabel(this);
    wave_lab->resize(phase_lab->width(), phase_lab->height());
    wave_lab->move(pulse_lab->x(), pulse_lab->y() + 27);
    wave_lab->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/grade2_bk.png);}");

    /* grade II menu text */
    wave_txt_lab = new QLabel(wave_lab);
    wave_txt_lab->resize(153, 21);
    wave_txt_lab->move(11, 7);
    wave_txt_lab->setAttribute(Qt::WA_TranslucentBackground, true);
    wave_txt_lab->setStyleSheet("QLabel {color:gray;}");
    wave_txt_lab->setAlignment(Qt::AlignCenter);
    wave_txt_lab->setText(tr("波形谱图检测模式"));

    /* three work mode: phase, pulse and wave */
    phasespectra = new PhaseSpectra(this);
    pulsespectra = new PulseSpectra(this);
    wavespectra = new WaveSpectra(this);

    /* create signal and slots */
    connect(this, &Menu2::send_key, phasespectra, &PhaseSpectra::trans_key);
    connect(this, &Menu2::send_key, pulsespectra, &PulseSpectra::trans_key);
    connect(this, &Menu2::send_key, wavespectra, &WaveSpectra::trans_key);

    connect(phasespectra, &PhaseSpectra::fresh_parent, this, &Menu2::fresh_grade1);
    connect(pulsespectra, &PulseSpectra::fresh_parent, this, &Menu2::fresh_grade1);
    connect(wavespectra, &WaveSpectra::fresh_parent, this, &Menu2::fresh_grade1);
}

void Menu2::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    if (!key_val->grade.val1) {
        phasespectra->hide();
        pulsespectra->hide();
        wavespectra->hide();
    }
    fresh_table();
    this->show();
}

void Menu2::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }
    if (key_val->grade.val0 != 2) {                                             //not current menu
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if (!key_val->grade.val1) {
            key_val->grade.val1 = 1;
            fresh_grade1();
        } else if (key_val->grade.val1 == 1) {
            key_val->grade.val2 = 1;
            phasespectra->working(key_val);
            pulsespectra->hide();
            wavespectra->hide();
        } else if (key_val->grade.val1 == 2) {
            key_val->grade.val2 = 1;
            phasespectra->hide();
            pulsespectra->working(key_val);
            wavespectra->hide();
        } else if (key_val->grade.val1 == 3) {
            key_val->grade.val2 = 1;
            phasespectra->hide();
            pulsespectra->hide();
            wavespectra->working(key_val);
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
                key_val->grade.val1 = 3;
            }
            fresh_grade1();
        }
        break;
    case KEY_DOWN:
        if (!key_val->grade.val2) {
            if (key_val->grade.val1 < 3) {
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

void Menu2::fresh_grade1(void)
{
    phasespectra->hide();
    pulsespectra->hide();
    wavespectra->hide();
    this->show();                                                               //show current gui

    switch (key_val->grade.val1) {
    case 0:
        phase_txt_lab->setStyleSheet("QLabel {color:gray;}");
        pulse_txt_lab->setStyleSheet("QLabel {color:gray;}");
        wave_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 1:
        phase_txt_lab->setStyleSheet("QLabel {color:white;}");
        pulse_txt_lab->setStyleSheet("QLabel {color:gray;}");
        wave_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 2:
        phase_txt_lab->setStyleSheet("QLabel {color:gray;}");
        pulse_txt_lab->setStyleSheet("QLabel {color:white;}");
        wave_txt_lab->setStyleSheet("QLabel {color:gray;}");
        break;
    case 3:
        phase_txt_lab->setStyleSheet("QLabel {color:gray;}");
        pulse_txt_lab->setStyleSheet("QLabel {color:gray;}");
        wave_txt_lab->setStyleSheet("QLabel {color:white;}");
        break;
    default:
        break;
    }
    emit send_title_val(*key_val);
}

void Menu2::fresh_table(void)
{
    if (key_val->grade.val0 == 2 && !key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m12.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m22.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m31.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m42.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m52.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m62.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m72.png);}");
    } else if (key_val->grade.val0 == 2 && key_val->grade.val1) {
        main_title0->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m10.png);}");
        main_title1->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m20.png);}");
        main_title2->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m32.png);}");
        main_title3->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m40.png);}");
        main_title4->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m50.png);}");
        main_title5->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m60.png);}");
        main_title6->setStyleSheet("QLabel {border-image: url(:/widgetphoto/mainmenu/m70.png);}");
    }
}
