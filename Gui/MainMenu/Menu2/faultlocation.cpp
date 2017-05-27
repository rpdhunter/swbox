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

//    timer = new QTimer;
//    timer->setInterval(2000);
//    connect(timer,SIGNAL(timeout()),this,SLOT(setCompassValue()));
//    timer->start();

    timer1 = new QTimer;
    timer1->setInterval(2000/4/speed);
    connect(timer1,SIGNAL(timeout()),this,SLOT(setMiniCompassValue()));
    timer1->start();



}

FaultLocation::~FaultLocation()
{
    delete ui;
}

void FaultLocation::get_origin_points(QVector<QPoint> p, int group)
{

    if(group <= 3 && group >=0){
        groupNum_left = group;
        points_left = p;
//        qDebug()<<"FaultLocation get data! Left  " << groupNum_left <<"\t"<<  points_left.length();
    }
    else if(group >=4 && group <=7){
        groupNum_right = group;
        points_right = p;
//        qDebug()<<"FaultLocation get data! Right  "<< groupNum_right <<"\t"<<  points_right.length();;
    }
    compare();

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

    int a = qrand()%180 - 90; //范围是-90~90
//    qDebug()<<"a="<<a;

    to = 360+a;              //范围是270~450

    process = 1;
}

void FaultLocation::setCompassValue(int c)
{
    int a = c; //范围是-90~90

    to = 360+a;              //范围是270~450

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

void FaultLocation::compare()
{
    if(groupNum_left + 4 == groupNum_right && points_left.length()>0 && points_right.length()>0){
        qDebug()<<"get valid data!\t"<<points_left.length()<<"\t"<<points_right.length();
        int c1 = 0, c2 =0, n = 0;
        foreach (QPoint p1, points_left) {
            foreach (QPoint p2, points_right) {
                if(qAbs(p1.x()-p2.x())<3 && p1.y()*p2.y()>0){
                    qDebug()<<"found close point pair!   "<<p1<<p2 << "\t"<<p1.x()-p2.x();
                    c1 += qAbs(p1.y() );
                    c2 += qAbs(p2.y() );
                    n++;        //计数器
                }
            }
        }

        if(n>0){

            if( (points_left.length()-points_right.length() )* (c1 - c2) > 0 ){
                qDebug()<< "c1=" << c1 * 1.0 / n <<"\tc2="<< c2 * 1.0 / n << "\tc1-c2="<< (c1 - c2)*1.0 / n;
                setCompassValue(-10*(c1 - c2)/ n);
            }
        }
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





