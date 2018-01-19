#ifndef BUZZER_H
#define BUZZER_H

#include <QThread>
#include "../Data/data.h"

class Buzzer : public QThread
{
    Q_OBJECT
public:
    explicit Buzzer(G_PARA *data);
    void song1();
    void song2();
    void yellow();
    void red();

signals:

public slots:

protected:
    void run(void);

private:
    enum NOTE{      //定义音符
        L_1,
        L_2,
        L_3,
        L_4,
        L_5,
        L_6,
        L_7,
        N_1,
        N_2,
        N_3,
        N_4,
        N_5,
        N_6,
        N_7,
        H_1,
        H_2,
        H_3,
        H_4,
        H_5,
        H_6,
        H_7,
        ZERO
    };
    enum METER{     //定义节拍
        M_1 = 1,
        M_2,
        M_3,
        M_4,
        M_5,
        M_6,
        M_7,
        M_8
    };
    G_PARA *data;
    void beep(NOTE note, METER meter);

    bool red_flag;

};

#endif // BUZZER_H
