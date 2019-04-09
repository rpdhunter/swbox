#include "contextmenu.h"

ContextMenu::ContextMenu(CHANNEL_SQL *sql, QWidget *parent) : QListWidget(parent)
{
    this->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    QStringList list;
    list << "1" << "2" << "3" << "4" << "5" << "6" << "7";
    this->addItems(list);

    this->channel_sql = sql;

//    fresh();
    if(sqlcfg->get_para()->language == CN){
        this->resize(140,135);
    }
    else{
        this->resize(240,120);
    }
    this->move(150,50);
    this->setSpacing(2);
    this->raise();
    this->hide();
}

void ContextMenu::toggle_state()
{
    if(this->isHidden()){
        this->show();
        fresh();
    }
    else{
        this->hide();
    }
}

void ContextMenu::fresh(int index)
{
    if (channel_sql->mode == single) {
        this->item(0)->setText(tr("检测模式\t[单次]"));
    } else {
        this->item(0)->setText(tr("检测模式\t[连续]"));
    }
    this->item(1)->setText(tr("增益调节\t[×%1]").arg(QString::number(channel_sql->gain, 'f', 1)));
    this->item(2)->setText(tr("脉冲触发\t[%1]mV").arg(QString::number(channel_sql->fpga_threshold)));
    this->item(3)->setText(tr("黄色报警阈值\t[%1]dB").arg(QString::number(channel_sql->low)));
    this->item(4)->setText(tr("红色报警阈值\t[%1]dB").arg(QString::number(channel_sql->high)));
    this->item(5)->setText(tr("脉冲计数时长\t[%1]s").arg(QString::number(channel_sql->pulse_time)));
    if(channel_sql->mode_recognition){
        this->item(6)->setText(tr("模式识别\t[on]"));

    }
    else{
        this->item(6)->setText(tr("模式识别\t[off]"));

    }

    if(index < this->count())
        this->setCurrentRow(index);
}
