#ifndef RECWAVEMANAGE_H
#define RECWAVEMANAGE_H

#include <QFrame>
#include <IO/Key/key.h>
#include <QListWidget>
#include <QTableWidget>
#include "../Common/recwaveform.h"
#include <QDir>
//#include "voiceplayer.h"

class QMessageBox;
namespace Ui {
class Form;
}

class RecWaveManage : public QFrame
{
    Q_OBJECT
public:
    explicit RecWaveManage(QWidget *parent = 0);
    ~RecWaveManage();

    void working(CURRENT_KEY_VALUE *val);

public slots:    
    void trans_key(quint8 key_code);
    void playVoiceProgress(int p, int all, bool f);      //控制播放器进度

signals:
    void fresh_parent();
    void send_key(quint8);

    void play_voice(VectorList);        //发送播放声音的指令
    void stop_play_voice();             //终止播放

    void show_indicator(bool);      //显示菊花

private slots:
    void start_play(VectorList list,MODE);

private:
    CURRENT_KEY_VALUE *key_val;

    QTableWidget *tableWidget;
    RecWaveForm *recWaveForm;

    QListWidget *contextMenu;

    QFrame *player;

    void reload_tablewidget();
    void refresh();
    void do_favorite();

    void deleteAll();
    void deleteCurrent();

    void readVoiceData();

    int contextMenu_num;
    int reset_flag;     //0为隐藏，1为取消，2为确认

    QDir dir, dir_favorite, dir_sd;
    QMessageBox *box;

    Ui::Form *ui;
};

#endif // RECWAVEMANAGE_H
