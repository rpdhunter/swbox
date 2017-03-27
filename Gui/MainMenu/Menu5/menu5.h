#ifndef MENU5_H
#define MENU5_H

#include <QFrame>
#include <QLabel>
#include "IO/Key/key.h"

class Menu5 : public QFrame
{
    Q_OBJECT
public:
    explicit Menu5(QWidget *parent = 0);

    void working(CURRENT_KEY_VALUE *val);

signals:
    void send_title_val(CURRENT_KEY_VALUE val);
    void send_key(quint8);

public slots:
    void trans_key(quint8);

private:
    CURRENT_KEY_VALUE *key_val;

    QLabel *main_title0, *main_title1, *main_title2, *main_title3,
        *main_title4, *main_title5, *main_title6, *bk_lab;

    QLabel *load_data_lab, *env_recd_lab, *back_recd_lab, *test_pos_lab;
    QLabel *load_data_txt_lab, *env_recd_txt_lab, *back_recd_txt_lab, *test_pos_txt_lab;

    void fresh_grade1(void);
    void fresh_table(void);
};

#endif // MENU5_H
