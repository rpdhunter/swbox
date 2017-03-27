#ifndef MENU2_H
#define MENU2_H

#include <QFrame>
#include <QLabel>
#include "PhaseSpectra/phasespectra.h"
#include "PulseSpectra/pulsespectra.h"
#include "WaveSpectra/wavespectra.h"
#include "IO/Key/key.h"

class Menu2 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu2(QWidget *parent = 0);

    void working(CURRENT_KEY_VALUE *val);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;
    PhaseSpectra *phasespectra;
    PulseSpectra *pulsespectra;
    WaveSpectra *wavespectra;

    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;
    QLabel *phase_lab, *pulse_lab, *wave_lab;
    QLabel *phase_txt_lab, *pulse_txt_lab, *wave_txt_lab;

    void fresh_grade1(void);
    void fresh_table(void);
};
#endif // MENU2_H
