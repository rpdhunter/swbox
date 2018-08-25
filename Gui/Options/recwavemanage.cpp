#include "recwavemanage.h"
#include "ui_voiceplayer.h"
#include <QtDebug>
#include <QMessageBox>
#include <QAbstractButton>
#include "IO/Other/filetools.h"
#include <QThreadPool>

RecWaveManage::RecWaveManage(QWidget *parent) : BaseWidget(NULL, parent), ui(new Ui::Form)
{
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    tableWidget = new QTableWidget(this);
    tableWidget->resize(CHANNEL_X, CHANNEL_Y);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setVisible(false);
    tableWidget->verticalHeader()->setDefaultSectionSize(16);
    tableWidget->horizontalHeader()->setDefaultSectionSize(350);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setGridStyle(Qt::NoPen);
    tableWidget->setStyleSheet("alternate-background-color: lightGray;");

    recWaveForm = new RecWaveForm(6,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(show_indicator(bool)),this,SIGNAL(show_indicator(bool)) );

    contextMenu = new QListWidget(this);        //右键菜单
    contextMenu->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    QStringList list;
    list << tr("查看波形") << tr("收藏/取消收藏") << tr("删除当前波形") << tr("删除全部波形") << tr("播放声音");
    contextMenu->addItems(list);
    contextMenu->resize(100, 90);
    contextMenu->move(300,10);
    contextMenu->setSpacing(2);
    contextMenu->hide();

    contextMenu_num = 3;        //显示菜单条目

    dir = QDir(DIR_WAVE);
    dir_favorite = QDir(DIR_FAVORITE);
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

    this->hide();
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
    ui->label_name->setText(tableWidget->currentItem()->text());
    ui->label_current->setText(QString::number(p/100.0, 'f', 1) + "s" );
    ui->label_All->setText(QString::number(all/100.0, 'f', 1) + "s" );
}

void RecWaveManage::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    reload_tablewidget();
//    listWidget->setCurrentRow(0);
    this->show();
}

void RecWaveManage::reload_tablewidget()
{
    //录波数据列表初始化
    QDir dir = QDir(DIR_WAVE);
    QDir dir_favorite = QDir(DIR_FAVORITE);
//    int r = tableWidget->currentRow();
    tableWidget->clear();
    qDebug()<<dir;

    QStringList filters;
    filters << "*.DAT" ;
    dir.setNameFilters(filters);
    dir_favorite.setNameFilters(filters);

    QStringList list = dir.entryList(QDir::Files);
    QStringList list_favorite = dir_favorite.entryList(QDir::Files);

    list.replaceInStrings(".DAT", "");
    list_favorite.replaceInStrings(".DAT", "");

    QFileInfoList list_file_info = dir.entryInfoList(QDir::Files);
    QFileInfoList list_favorite_file_info = dir_favorite.entryInfoList(QDir::Files);

    tableWidget->setRowCount(list_file_info.length() + list_favorite_file_info.length());
    tableWidget->setColumnCount(2);

    for (int i = 0; i < list_favorite_file_info.length(); ++i) {
        tableWidget->setItem(i, 0, new QTableWidgetItem(QString("%1☆").arg(list_favorite.at(i))) );
        tableWidget->setItem(i, 1, new QTableWidgetItem(QString("%1 KB").arg(list_favorite_file_info.at(i).size() / 1000) ) );
    }

    for (int i = 0; i < list_file_info.length(); ++i) {
        tableWidget->setItem(i + list_favorite_file_info.length(), 0, new QTableWidgetItem(list.at(i)) );
        tableWidget->setItem(i + list_favorite_file_info.length(), 1,
                             new QTableWidgetItem(QString("%1 KB").arg(list_file_info.at(i).size() / 1000) ) );
    }

    refresh();

//    tableWidget->setCurrentCell(r,0);       //保存选择状态
}

void RecWaveManage::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val1 != 3) {
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

    BaseWidget::trans_key(key_code);
    refresh();
}


void RecWaveManage::do_favorite()
{
    QString filename = tableWidget->currentItem()->text();
    QString filepath, newpath;
    if(filename.contains(QString("☆")) ){
        filepath = DIR_FAVORITE"/" + filename.remove(QString("☆") );
        newpath = DIR_WAVE"/" + filename.remove(QString("☆") );
    }
    else {
        filepath = DIR_WAVE"/" + filename;
        newpath = DIR_FAVORITE"/" + filename;
    }

    QFile file;
    file.setFileName(filepath + ".DAT");
    file.rename(newpath + ".DAT");

    file.setFileName(filepath + ".CFG");
    file.rename(newpath + ".CFG");

    file.setFileName(filepath + ".mp3");
    file.rename(newpath + ".mp3");

    system( "sync");
    reload_tablewidget();

}

//删除全部文件
void RecWaveManage::deleteAll()
{
    qDebug()<<"delete All";
    QStringList list = dir.entryList(QDir::Files);
//    QStringList list = dir.entryList();
    qDebug()<<"to be delete number:"<<list.length();
//    qDebug()<<"to be delete number:"<<dir.entryInfoList().length();

    foreach (QString l, list) {
        dir.remove(l);
    }

    list = dir_sd.entryList(QDir::Files);
    foreach (QString l, list) {
        dir_sd.remove(l);
    }
    key_val->grade.val3 = 0;

    system( "sync");
}

void RecWaveManage::deleteCurrent()
{
    dir.remove(tableWidget->currentItem()->text() + ".DAT");
    dir.remove(tableWidget->currentItem()->text() + ".CFG");
    dir.remove(tableWidget->currentItem()->text() + ".mp3");

    dir_sd.remove(tableWidget->currentItem()->text() + ".DAT");
    dir_sd.remove(tableWidget->currentItem()->text() + ".CFG");
    dir_sd.remove(tableWidget->currentItem()->text() + ".mp3");

    if(tableWidget->currentItem()->text().contains(QString("☆"))){
        dir_favorite.remove(tableWidget->currentItem()->text().remove(QString("☆")) + ".DAT");
        dir_favorite.remove(tableWidget->currentItem()->text().remove(QString("☆")) + ".CFG");
        dir_favorite.remove(tableWidget->currentItem()->text().remove(QString("☆")) + ".mp3");
    }

    if(key_val->grade.val3 > 0){
        key_val->grade.val3--;
    }

    system( "sync");
    reload_tablewidget();
}

void RecWaveManage::readVoiceData()
{
    emit show_indicator(true);
    FileTools *filetools = new FileTools(tableWidget->currentItem()->text(),FileTools::Read);      //开一个线程，为了不影响数据接口性能
    QThreadPool::globalInstance()->start(filetools);
    connect(filetools,SIGNAL(readFinished(VectorList,MODE)),this,SLOT(start_play(VectorList,MODE)) );
}

void RecWaveManage::do_key_ok()
{
    if(reset_flag == 0){
        if(tableWidget->currentRow()>=0){
            reset_flag = 0;
            contextMenu->hide();
            switch (key_val->grade.val4) {
            case 0:             //查看波形
            case 1:             //查看波形
                key_val->grade.val5 = 1;
                key_val->grade.val4 = 0;
                recWaveForm->working(key_val,tableWidget->currentItem()->text());
                break;
            case 2:
                key_val->grade.val4 = 0;
                do_favorite();
                break;
            case 3:             //删除当前波形
                key_val->grade.val4 = 0;
                deleteCurrent();
                break;
            case 4:             //删除全部波形
                key_val->grade.val4 = 0;
                reset_flag = 1;
                break;
            case 5:             //播放声音
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
        reload_tablewidget();
    }
}

void RecWaveManage::do_key_cancel()
{
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
        this->hide();
        emit fresh_parent();
    }
    if(player->isVisible()){
        player->setVisible(false);
    }
}

void RecWaveManage::do_key_up_down(int d)
{
    if(reset_flag == 0){
        if(key_val->grade.val4 > 0){
            Common::change_index(key_val->grade.val4,d,contextMenu_num,1);
        }
        else{
            Common::change_index(key_val->grade.val3,d,tableWidget->rowCount(),1);
        }
    }
}

void RecWaveManage::do_key_left_right(int d)
{
    if(reset_flag == 0 ){
        if(key_val->grade.val3){
            if(key_val->grade.val4 == 0){
                key_val->grade.val4 ++;
                contextMenu->show();
            }
        }
    }
    else{
        Common::change_index(reset_flag,d,2,1);
    }
}

void RecWaveManage::start_play(VectorList list,MODE mode)
{
    //包络线需要做数据扩展，匹配播放频率
    if(mode == AA1_ENVELOPE || mode == AA2_ENVELOPE || mode == AE1_ENVELOPE || mode == AE2_ENVELOPE ){
        VectorList l;
        l.resize(list.count()*10);
        for (int i = 0; i < list.count(); ++i) {
            for (int j = 0; j < 10; ++j) {
                l[i*10+j] = list.at(i);
            }
        }
        emit play_voice(l);
    }
    else{
        emit play_voice(list);
    }
    emit show_indicator(false);    
}

void RecWaveManage::refresh()
{
    tableWidget->setCurrentCell(key_val->grade.val3 - 1, 0);

    contextMenu->setCurrentRow(key_val->grade.val4 - 1);
    if(key_val->grade.val4 == 0){
        contextMenu->setVisible(false);
    }
    if(tableWidget->currentRow()>=0){            //根据条目改变菜单
        if(tableWidget->currentItem()->text().contains(("AA")) || tableWidget->currentItem()->text().contains(("AE"))){
            contextMenu->item(4)->setHidden(false);
            contextMenu_num = 5;
            contextMenu->resize(100 + 10, 19*contextMenu_num);
        }
        else{
            contextMenu->item(4)->setHidden(true);
            contextMenu_num = 4;
            contextMenu->resize(100 + 10, 19*contextMenu_num);
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

