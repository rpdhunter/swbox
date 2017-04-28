#ifndef MENU6_H
#define MENU6_H

#include "IO/Key/key.h"
//#include "SelfTest/selftest.h"
#include "Option/options.h"
#include "SystemInfo/systeminfo.h"
#include "DebugSet/debugset.h"
#include "FactoryReset/factoryreset.h"
#include <QFrame>
#include <QLabel>

class Menu6 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu6(QWidget *parent = 0, G_PARA *g_data = NULL);

    void working(CURRENT_KEY_VALUE *val);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);

    void startRecWv(int,int);      //开始录播信号
    void fregChanged(int);
    void closeTimeChanged(int);
    void maxResetTimeChanged(int);
    void sysReset();

public slots:
    void trans_key(quint8);
    void showWaveData(VectorList buf, MODE mod);

    void set_offset_suggest(int a,int b);

private:
    CURRENT_KEY_VALUE *key_val;

    Options *options;
    SystemInfo *systeminfo;
    DebugSet *debugset;     //new!
    FactoryReset *factoryreset;


    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6;

    QLabel *bk_lab;

    QLabel *options_lab, *debug_lab, *systeminfo_lab, *default_lab;
    QLabel *options_txt_lab, *debug_txt_lab, *systeminfo_txt_lab, *default_txt_lab;

    void fresh_grade1(void);
    void fresh_table(void);
};

#endif // MENU6_H
