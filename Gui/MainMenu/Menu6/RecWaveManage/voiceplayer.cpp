#include "voiceplayer.h"
#include "ui_voiceplayer.h"
#include <QtDebug>

VoicePlayer::VoicePlayer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VoicePlayer)
{
    ui->setupUi(this);

    qDebug()<<"dasdasd1111";
}

VoicePlayer::~VoicePlayer()
{
    delete ui;
}
