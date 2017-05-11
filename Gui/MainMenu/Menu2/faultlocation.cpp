#include "faultlocation.h"
#include "ui_faultlocation.h"
#include <QLineEdit>
#include <qwt_dial_needle.h>
#include <QTimer>

FaultLocation::FaultLocation(G_PARA *data, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FaultLocation)
{
    ui->setupUi(this);

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("FaultLocation {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    key_val = NULL;


    this->data = data;

    QLineEdit *lineEdit = new QLineEdit;
    ui->comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray; }");
    ui->comboBox->setLineEdit(lineEdit);
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    ui->comboBox->lineEdit()->setReadOnly(true);
    ui->comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    ui->comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");

    QwtDialSimpleNeedle *needle = new QwtDialSimpleNeedle( QwtDialSimpleNeedle::Arrow,false, Qt::red );
    needle->setWidth(8);
    ui->Compass->setNeedle( needle );

    QwtCompassScaleDraw *scaleDraw = new QwtCompassScaleDraw();
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Ticks, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Labels, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 3 );

    ui->Compass->setScaleDraw( scaleDraw );

    ui->Compass->setValue(330);

    from = 360;
    to = 360;
    speed = 50;

    timer = new QTimer;
    timer->setInterval(2000);
    connect(timer,SIGNAL(timeout()),this,SLOT(setCompassValue()));
    timer->start();

    timer1 = new QTimer;
    timer1->setInterval(2000/4/speed);
    connect(timer1,SIGNAL(timeout()),this,SLOT(setMiniCompassValue()));
    timer1->start();



}

FaultLocation::~FaultLocation()
{
    delete ui;
}

void FaultLocation::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    this->show();
}

void FaultLocation::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if(key_val->grade.val0 != 2){
        return;
    }

    switch (key_code) {
    case KEY_OK:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_UP:
        if (key_val->grade.val2 < 2) {
            key_val->grade.val2 = 4;
        } else {
            key_val->grade.val2--;
        }
        break;
    case KEY_DOWN:
        if (key_val->grade.val2 > 3) {
            key_val->grade.val2 = 1;
        } else {
            key_val->grade.val2++;
        }
        break;
    case KEY_LEFT:
        switch (key_val->grade.val2) {
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
        break;
    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    fresh_setting();
}

void FaultLocation::setCompassValue()
{

    int a = qrand()%180 - 90;
//    qDebug()<<"a="<<a;

    to = 360+a;

    process = 1;
}

void FaultLocation::setMiniCompassValue()
{
    if(process){
        ui->Compass->setValue( ((int)(from + process * (to - from) * 1.0 / speed )) % 360 );
        process ++ ;        //process范围是1-11
    }

    if(process == speed +1 ){
        from = to;
        process = 0;
    }
}

void FaultLocation::fresh_setting()
{
//    qDebug()<<"key_val->grade.val0 is :"<<key_val->grade.val0;
//    qDebug()<<"key_val->grade.val1 is :"<<key_val->grade.val1;
//    qDebug()<<"key_val->grade.val2 is :"<<key_val->grade.val2;

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == 2) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}





