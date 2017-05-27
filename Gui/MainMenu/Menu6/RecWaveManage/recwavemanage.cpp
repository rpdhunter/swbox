#include "recwavemanage.h"
#include <QtDebug>
#include <QMessageBox>
#include <QAbstractButton>
#include "ui_voiceplayer.h"

RecWaveManage::RecWaveManage(QWidget *parent) : QFrame(parent), ui(new Ui::Form)
{
    key_val = NULL;

    this->resize(455, 185);
    this->move(2, 31);

    listWidget = new QListWidget(this);
    listWidget->resize(455, 185);

    recWaveForm = new RecWaveForm(this);
    recWaveForm->hide();
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));

    contextMenu = new QListWidget(this);        //右键菜单
    contextMenu->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");
    QStringList list;
    list << tr("查看波形") << tr("删除当前波形") << tr("删除全部波形") << tr("播放声音");
    contextMenu->addItems(list);
    contextMenu->resize(100, 75);
    contextMenu->move(300,10);
    contextMenu->setSpacing(2);
    contextMenu->hide();

    contextMenu_num = 3;        //显示菜单条目

    dir = QDir("/root/WaveForm/");

    dir_sd = QDir("/mmc/sdcard/WaveForm/");

    box = new QMessageBox(QMessageBox::Warning,tr("删除全部文件"),tr("将要删除本机保存的全部波形文件.\n确定要删除吗?"),
                          QMessageBox::Ok | QMessageBox::Cancel,this);

    box->hide();
    box->move(135,100);

    reset_flag = 0;

    player = new QFrame(this);
    ui->setupUi(player);
    player->resize(454,100);
    player->move(2,88);
    player->setStyleSheet("QFrame {background-color:lightGray}");
    player->hide();

}

RecWaveManage::~RecWaveManage()
{
    delete ui;
}

void RecWaveManage::playVoiceProgress(int p, int all, bool f)
{
    ui->horizontalSlider->setRange(0,all);
    ui->horizontalSlider->setValue(p);
    player->setVisible(f);
    ui->label_name->setText(listWidget->currentItem()->text());
    ui->label_current->setText(QString::number(p/100.0, 'f', 1) + "s" );
    ui->label_All->setText(QString::number(all/100.0, 'f', 1) + "s" );
}



void RecWaveManage::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    listWdigetIni();
//    listWidget->setCurrentRow(0);
    this->show();
}

void RecWaveManage::listWdigetIni()
{
    //录波数据列表初始化
//    QDir dir = QDir("/root/WaveForm/");
    int r = listWidget->currentRow();
    listWidget->clear();

    QStringList filters;
    filters << "*.DAT" ;
    dir.setNameFilters(filters);
    QStringList list = dir.entryList(QDir::Files);
    list.replaceInStrings(".DAT", "");
    listWidget->addItems(list);

    listWidget->setCurrentRow(r);       //保存选择状态
}

void RecWaveManage::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 3) {
        return;
    }

    if(player->isVisible()){
        emit stop_play_voice();
        return;
    }

    if (key_val->grade.val5 != 0) {
        emit send_key(key_code);
        return;
    }


    switch (key_code) {
    case KEY_OK:
        if(reset_flag == 0){
            if(listWidget->currentRow()>=0){
                reset_flag = 0;
                contextMenu->hide();
                switch (key_val->grade.val4) {
                case 0:             //查看波形
                case 1:             //查看波形
                    key_val->grade.val5 = 1;
                    key_val->grade.val4 = 0;
                    recWaveForm->working(key_val,listWidget->currentItem()->text());
                    break;
                case 2:             //删除当前波形
                    key_val->grade.val4 = 0;
                    deleteCurrent();
                    break;
                case 3:             //删除全部波形
                    key_val->grade.val4 = 0;
                    reset_flag = 1;
                    break;
                case 4:             //播放声音
                    key_val->grade.val4 = 0;
                    readVoiceData();
                    qDebug()<<"play voice";
                    break;
                default:
                    break;
                }
            }
        }
        else if(reset_flag == 1){
            reset_flag = 0;
        }
        else if(reset_flag == 2){
            reset_flag = 0;
            deleteAll();        //确定删除
            listWdigetIni();
        }

        break;
    case KEY_CANCEL:        
        if(reset_flag > 0){
            reset_flag = 0;
        }
        else if(key_val->grade.val4 >0){     //小退
            key_val->grade.val4 = 0;
        }
        else{                           //大退
            key_val->grade.val2 = 0;
            key_val->grade.val3 = 0;
            key_val->grade.val5 = 0;
            emit fresh_parent();
        }

        break;
    case KEY_UP:
        if(reset_flag == 0){
            if(key_val->grade.val4 > 0){
                if(key_val->grade.val4 > 1){
                    key_val->grade.val4 --;
                }
                else{
                    key_val->grade.val4 = contextMenu_num;
                }

            }
            else if(key_val->grade.val3 > 1){
                key_val->grade.val3 --;
            }
        }
        break;
    case KEY_DOWN:
        if(reset_flag == 0){
            if(key_val->grade.val4 > 0){
                if(key_val->grade.val4 < contextMenu_num){
                    key_val->grade.val4 ++;
                }
                else{
                    key_val->grade.val4 = 1;
                }

            }
            else if(key_val->grade.val3 < listWidget->count()){
                key_val->grade.val3 ++;
            }
        }
        break;
    case KEY_LEFT:
        if(reset_flag == 1){
            reset_flag = 2;
        }
        else if(reset_flag == 2){
            reset_flag = 1;
        }
        else{
            contextMenu->hide();
        }
        break;
    case KEY_RIGHT:
        if(reset_flag == 1){
            reset_flag = 2;
        }
        else if(reset_flag == 2){
            reset_flag = 1;
        }
        else{
            if(key_val->grade.val3){
                if(key_val->grade.val4 == 0){
                    key_val->grade.val4 ++;
                    contextMenu->show();
                }
//                qDebug()<<contextMenu->visualItemRect(contextMenu->currentItem());
            }
        }

        break;
    default:
        break;
    }

    refresh();
}

void RecWaveManage::refresh()
{
    listWidget->setCurrentRow(key_val->grade.val3 - 1);
    contextMenu->setCurrentRow(key_val->grade.val4 - 1);
    if(key_val->grade.val4 == 0){
        contextMenu->setVisible(false);
    }
    if(listWidget->currentRow()>=0){            //根据条目改变菜单
        if(listWidget->currentItem()->text().contains(("AAUltrasonic"))){
            contextMenu->item(3)->setHidden(false);
            contextMenu_num = 4;
            contextMenu->resize(100, 75);
        }
        else{
            contextMenu->item(3)->setHidden(true);
            contextMenu_num = 3;
            contextMenu->resize(100, 57);
        }
    }

    if(reset_flag){
        box->show();
        if(reset_flag == 1){
            box->button(QMessageBox::Ok)->setStyleSheet("");
            box->button(QMessageBox::Cancel)->setStyleSheet("QPushButton {background-color:gray;}");
            box->setDefaultButton(QMessageBox::Cancel);
        }
        else if(reset_flag == 2){
            box->button(QMessageBox::Ok)->setStyleSheet("QPushButton {background-color:gray;}");
            box->button(QMessageBox::Cancel)->setStyleSheet("");
            box->setDefaultButton(QMessageBox::Ok);
        }

    }
    else{
        box->hide();
    }



}

//删除全部文件
void RecWaveManage::deleteAll()
{
    qDebug()<<"delete All";
    QStringList list = dir.entryList(QDir::Files);
    foreach (QString l, list) {
        dir.remove(l);
    }

    list = dir_sd.entryList(QDir::Files);
    foreach (QString l, list) {
        dir_sd.remove(l);
    }

    system( "sync");
}

void RecWaveManage::deleteCurrent()
{
    dir.remove(listWidget->currentItem()->text() + ".DAT");
    dir.remove(listWidget->currentItem()->text() + ".CFG");
    dir.remove(listWidget->currentItem()->text() + ".mp3");

    dir_sd.remove(listWidget->currentItem()->text() + ".DAT");
    dir_sd.remove(listWidget->currentItem()->text() + ".CFG");
    dir_sd.remove(listWidget->currentItem()->text() + ".mp3");

    system( "sync");
    listWdigetIni();
}

void RecWaveManage::readVoiceData()
{
    QFile file;
    file.setFileName("/root/WaveForm/"+listWidget->currentItem()->text()+".DAT");

    if (!file.open(QIODevice::ReadOnly)){
        qDebug()<<"file open failed!";
        return;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 t1,t2;
    qint16 v;
    VectorList list;
    while (!in.atEnd()) {
        in >> t1 >> t2 >> v;
        list.append((qint32)(v));
    }

    emit play_voice(list);

    file.close();

}


