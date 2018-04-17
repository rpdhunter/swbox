#include "buzzer.h"

Buzzer::Buzzer(G_PARA *data)
{
    this->data = data;
    red_flag = false;
}

void Buzzer::song1()
{
//    beep(L_1,M_4);
//    beep(L_2,M_4);
//    beep(L_3,M_4);
//    beep(L_4,M_4);
//    beep(L_5,M_4);
//    beep(L_6,M_4);
//    beep(L_7,M_4);

//    beep(N_1,M_4);
//    beep(N_2,M_4);
//    beep(N_3,M_4);
//    beep(N_4,M_4);
//    beep(N_5,M_4);
//    beep(N_6,M_4);
//    beep(N_7,M_4);

    beep(H_1,M_4);
    beep(H_2,M_4);
    beep(H_3,M_4);
    beep(H_4,M_4);
    beep(H_5,M_4);
    beep(H_6,M_4);
    beep(H_7,M_4);
}

//《铃儿响叮当》
void Buzzer::song2()
{
    beep(L_5,M_2);
    beep(N_3,M_2);
    beep(N_2,M_2);
    beep(N_1,M_2);
    beep(L_5,M_6);

    beep(L_5,M_1);
    beep(L_5,M_1);
    beep(L_5,M_2);
    beep(N_3,M_2);
    beep(N_2,M_2);
    beep(N_1,M_2);
    beep(L_6,M_6);

    beep(L_6,M_2);
    beep(L_6,M_2);
    beep(N_4,M_2);
    beep(N_3,M_2);
    beep(N_2,M_2);
    beep(L_7,M_6);

    beep(L_5,M_2);
    beep(N_5,M_2);
    beep(N_5,M_2);
    beep(N_4,M_2);
    beep(N_2,M_2);
    beep(N_3,M_4);

    beep(N_3,M_2);
    beep(N_3,M_2);
    beep(N_3,M_4);

    beep(N_3,M_2);
    beep(N_3,M_2);
    beep(N_3,M_4);

    beep(N_3,M_2);
    beep(N_5,M_2);
    beep(N_1,M_3);
    beep(N_2,M_1);
    beep(N_3,M_8);

    beep(N_4,M_2);
    beep(N_4,M_2);
    beep(N_4,M_3);
    beep(N_4,M_1);
    beep(N_4,M_2);
    beep(N_3,M_2);
    beep(N_3,M_2);
    beep(N_3,M_1);
    beep(N_3,M_1);

    beep(N_3,M_2);
    beep(N_2,M_2);
    beep(N_2,M_2);
    beep(N_1,M_2);
    beep(N_2,M_2);
    beep(N_5,M_6);


}

void Buzzer::yellow()
{
    red_flag = false;
    this->start();
}

void Buzzer::red()
{
    red_flag = true;
    this->start();
}

void Buzzer::run()
{
//    song1();
//    song2();
    if(red_flag){
        beep(H_3,M_4);
        beep(H_3,M_4);
    }
    beep(H_3,M_4);

}

void Buzzer::beep(Buzzer::NOTE note, Buzzer::METER meter)
{
    data->set_send_para (sp_buzzer,1);

    switch (note) {
    case L_1:
        data->set_send_para (sp_buzzer_freq, 100000000/220.0/2/4);
        break;
    case L_2:
        data->set_send_para (sp_buzzer_freq, 100000000/246.94/2/4);
        break;
    case L_3:
        data->set_send_para (sp_buzzer_freq, 100000000/277.19/2/4);
        break;
    case L_4:
        data->set_send_para (sp_buzzer_freq, 100000000/293.67/2/4);
        break;
    case L_5:
        data->set_send_para (sp_buzzer_freq, 100000000/329.63/2/4);
        break;
    case L_6:
        data->set_send_para (sp_buzzer_freq, 100000000/370.0/2/4);
        break;
    case L_7:
        data->set_send_para (sp_buzzer_freq, 100000000/415.31/2/4);
        break;
    case N_1:
        data->set_send_para (sp_buzzer_freq, 100000000/440.0/2/4);
        break;
    case N_2:
        data->set_send_para (sp_buzzer_freq, 100000000/493.88/2/4);
        break;
    case N_3:
        data->set_send_para (sp_buzzer_freq, 100000000/554.37/2/4);
        break;
    case N_4:
        data->set_send_para (sp_buzzer_freq, 100000000/587.33/2/4);
        break;
    case N_5:
        data->set_send_para (sp_buzzer_freq, 100000000/659.33/2/4);
        break;
    case N_6:
        data->set_send_para (sp_buzzer_freq, 100000000/739.99/2/4);
        break;
    case N_7:
        data->set_send_para (sp_buzzer_freq, 100000000/830.61/2/4);
        break;
    case H_1:
        data->set_send_para (sp_buzzer_freq, 100000000/880.0/2/4);
        break;
    case H_2:
        data->set_send_para (sp_buzzer_freq, 100000000/987.76/2/4);
        break;
    case H_3:
        data->set_send_para (sp_buzzer_freq, 100000000/1108.73/2/4);
        break;
    case H_4:
        data->set_send_para (sp_buzzer_freq, 100000000/1174.66/2/4);
        break;
    case H_5:
        data->set_send_para (sp_buzzer_freq, 100000000/1318.52/2/4);
        break;
    case H_6:
        data->set_send_para (sp_buzzer_freq, 100000000/1479.98/2/4);
        break;
    case H_7:
        data->set_send_para (sp_buzzer_freq, 100000000/1661.22/2/4);
        break;
    default:
        data->set_send_para (sp_buzzer,0);
        break;
    }

    msleep(meter*150);

    data->set_send_para (sp_buzzer,0);
    msleep(50);

}
