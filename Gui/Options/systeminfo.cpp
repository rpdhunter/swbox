#include "systeminfo.h"
#include "ui_systeminfo.h"
#include <QFile>
#include <QTime>
#include <QDir>

SystemInfo::SystemInfo(QWidget *parent) : QFrame(parent),ui(new Ui::SystemInfoUi)
{
    ui->setupUi(this);
    this->setStyleSheet("SystemInfo {background-color:lightGray;}");
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    key_val = NULL;

#ifdef OHV
    ui->label_logo_text->setText("copyright © 2014-2017 Ohv Diagnostic GmbH, Ltd\nAll rights reserved.");
    ui->label_logo->setPixmap(QPixmap(":/widgetphoto/bk/ohv_low.png"));
#elif AMG
    ui->label_logo_text->setText("copyright © 2016-2017 Australian Microgrid Technology Pty Lte\nAll rights reserved.");
    ui->label_logo->setPixmap(QPixmap(":/widgetphoto/bk/amg_low.png"));
#elif ZDIT
    ui->label_logo_text->setText("copyright © 2016-2017 ZIT(Nanjing) Technology Co., Ltd\nAll rights reserved.");
    ui->label_logo->setPixmap(QPixmap(":/widgetphoto/bk/zdit_low.png"));
#else
    ui->label_logo_text->setText("");
#endif

    QDir dir;
    //创建文件夹
    if(!dir.exists("/root/OEM/")){
        dir.mkdir("/root/OEM/");
    }

    QFile file("/root/OEM/version");
    if(file.open(QIODevice::ReadWrite | QIODevice::Text) ){
        QByteArray line;
        if(!file.atEnd()) {     //读设备型号
            line = file.readLine();                        
        }
        else{
#ifdef OHV
            line = "PDTEV";
#elif AMG
            line = "PDcable";
#elif ZDIT
            line = "PD3000";
#else
            line = "PD3000";
#endif
            file.write(line);
        }
        line.replace("\n","");
        ui->label_name->setText(line);

        if(!file.atEnd()) {     //读版权信息
            line = file.readLine();            
        }
        else{
#ifdef OHV
            line = "\ncopyright © 2014-2017 Ohv Diagnostic GmbH, Ltd";
#elif AMG
            line = "\ncopyright © 2016-2017 Australian Microgrid Technology Pty Lte";
#elif ZDIT
            line = "\ncopyright © 2016-2017 ZIT(Nanjing) Technology Co., Ltd";
#else
            line = "\n";
#endif
            file.write(line);
        }
        line.replace("\n","");
        line.replace("?","©");
        ui->label_logo_text->setText(line + "\nAll rights reserved.");

        if(!file.atEnd()) {     //读设备识别码
            line = file.readLine();

        }
        else{                   //生成设备识别码
            QByteArray str_mac("5a4954");       //设备前三位是ZIT的askii码
            for (int i = 0; i < 3; ++i) {       //后三位随机生成
                qsrand(QTime::currentTime().msec() + i);
                str_mac.append(QByteArray::number(15 + qrand()%240, 16) );
            }
            line = "\n" + str_mac;
            file.write(line);
//            QByteArray macAddress = QByteArray::fromHex(str_mac);
//            ui->label_mac->setText(macAddress.toHex('-').toUpper());
//            file.write("\n" + str_mac);
        }
        line.replace("\n","");
        ui->label_mac->setText(line.fromHex(line).toHex('-').toUpper());
    }
    file.close();
    QFile file1("/root/OEM/logo_low.png");
    if(file1.exists()){
        ui->label_logo->setPixmap(QPixmap("/root/OEM/logo_low.png"));
    }

    this->hide();
}

void SystemInfo::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void SystemInfo::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 4) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        key_val->grade.val2 = 0;
        fresh_parent();
        this->hide();
        break;
    case KEY_CANCEL:
        key_val->grade.val2 = 0;
        fresh_parent();
        this->hide();
        break;
    case KEY_UP:
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    default:
        break;
    }
}

