#ifndef HFCTWIDGET_H
#define HFCTWIDGET_H

#include "channelwidget.h"
#include "Algorithm/Bp/bpjudge.h"
#include "Thread/Multimachine/multimachineclient.h"
#include "Thread/Multimachine/multimachineserver.h"

namespace Ui {
class HFCTWidget;
}

class HFCTWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~HFCTWidget();

private slots:
    void fresh_100ms();
    void fresh_1000ms();    

private:
    Ui::HFCTWidget *ui;
    BpJudge *bpjudge;

    void do_key_ok();
//    void do_key_cancel();
//    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void data_reset();

    void judge_multimachine();      //判断多机互联的主从模式
    MultiMachineServer *server;
    MultiMachineClient *client;
    void do_multimachine();         //发送数据给主机(客户端模式)

};

#endif // HFCTWIDGET_H
