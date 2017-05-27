#include "factoryreset.h"
#include <QVBoxLayout>
#include <QPushButton>

FactoryReset::FactoryReset(QWidget *parent) : QFrame(parent)
{
    key_val = NULL;

    this->resize(274,100);
    this->move(100,65);
    this->setStyleSheet("FactoryReset {background-color:lightGray;}");

    btnBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                  | QDialogButtonBox::Cancel,this);
    label = new QLabel(this);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->setText(tr("将系统恢复出厂设置吗?"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(btnBox);
    this->setLayout(layout);

    buttonIni(false);       //默认状态为不重置
}


void FactoryReset::buttonIni(bool f)
{
    if(f){
        btnBox->button(QDialogButtonBox::Ok)->setStyleSheet("QPushButton {background-color:gray;}");
        btnBox->button(QDialogButtonBox::Cancel)->setStyleSheet("QPushButton {background-color:lightGray;}");
        reset = 1;
    }
    else{
        btnBox->button(QDialogButtonBox::Cancel)->setStyleSheet("QPushButton {background-color:gray;}");
        btnBox->button(QDialogButtonBox::Ok)->setStyleSheet("QPushButton {background-color:lightGray;}");
        reset = 0;
    }
}

void FactoryReset::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void FactoryReset::trans_key(quint8 key_code)
{


    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 5) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val2 == 1 && key_val->grade.val3 == 1){
            if(reset == 1){
                if(reset == 1){
                    sqlcfg->sql_save(sqlcfg->default_config());
                    qDebug()<<tr("Factory Reset!");
                    emit sysReset();
                }
            }
            key_val->grade.val2 = 0;
            key_val->grade.val3 = 0;
            emit fresh_parent();
        }
        else{
            key_val->grade.val3 = 1;
            buttonIni(false);
        }

        break;
    case KEY_CANCEL:
        key_val->grade.val2 = 0;
        key_val->grade.val3 = 0;
        emit fresh_parent();
        break;
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        buttonIni(true);
        break;
    case KEY_RIGHT:
        buttonIni(false);
        break;
    default:
        break;
    }
}


