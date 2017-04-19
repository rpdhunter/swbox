#include "amplitude1.h"
#include "ui_amplitude1.h"

Amplitude1::Amplitude1(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Amplitude1)
{
    ui->setupUi(this);
}

Amplitude1::~Amplitude1()
{
    delete ui;
}
