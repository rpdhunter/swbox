#include "settingmunu.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Common/common.h"

#define ITEM_HIGH   18      //行高
#define MODIFY      -2      //行高修正

SettingMunu::SettingMunu(MODE mode, QWidget *parent) : QObject(parent)
{
    this->mode = mode;    
    if(sqlcfg->get_para()->language == CN){
        wide = 150;
    }
    else{
        wide = 240;
    }

    lineEdit = new QLineEdit(parent);
    lineEdit->setText(tr(" 参 数 设 置"));
    lineEdit->resize(wide, 22);
    lineEdit->setReadOnly(true);
    lineEdit->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/bk/para_child.png);color:gray}");

    label = new QLabel(parent);
    label->resize(30,22);
    label->move(wide - 30,0);
    label->setPixmap(QPixmap(":/widgetphoto/other/down.png"));

    main_menu = new QListWidget(parent);
    main_menu->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    QStringList list;
    switch (mode) {
    case TEV1:
    case TEV2:
        list << tr("启动测试") << tr("测量值重置") << tr("图形显示") << tr("单次录波") << tr("连续录波") << tr("自动录波") << tr("通道设置...");
        break;
    case HFCT1:
    case HFCT2:
        list << tr("启动测试") << tr("测量值重置") << tr("图形显示") << tr("单次录波") << tr("连续录波") << tr("自动录波")
             << tr("通带下限") << tr("通带上限") << tr("单位切换") << tr("通道设置...");
        break;
    case UHF1:
    case UHF2:
        list << tr("启动测试") << tr("测量值重置") << tr("图形显示") << tr("单次录波") << tr("连续录波") << tr("自动录波") << tr("通道设置...");
        break;
    case AA1:
    case AA2:
        list << tr("启动测试") << tr("测量值重置") << tr("图形显示") << tr("音量") << tr("连续录波") << tr("通道设置...");
        break;
    case AE1:
    case AE2:
        list << tr("启动测试") << tr("测量值重置") << tr("图形显示") << tr("音量") << tr("连续录波") << tr("通道设置...");
        break;
    default:
        break;
    }
    main_menu->addItems(list);
    main_menu->resize(wide,list.count() * ITEM_HIGH + MODIFY);
    main_menu->move(0,22);
    main_menu->setSpacing(1);
    main_menu->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    main_menu->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    sub_menu = new QListWidget(parent);
    sub_menu->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    list.clear();
    list << tr("检测模式") << tr("增益调节") << tr("脉冲触发") << tr("黄色报警") << tr("红色报警") << tr("脉冲计数") << tr("模式识别");
    sub_menu->addItems(list);
    sub_menu->resize(150,list.count() * ITEM_HIGH + MODIFY);
    sub_menu->move(wide - 5,50);
    sub_menu->setSpacing(1);

    lineEdit->show();
    main_menu->hide();
    sub_menu->hide();
}

void SettingMunu::fresh(CHANNEL_SQL *sql, int grade)
{
    if(grade > 0 && grade <= main_menu->count() ){
        main_menu->show();
        sub_menu->hide();
        main_menu->setCurrentRow(grade - 1);
    }
    else if(grade >= SUB_MENU_NUM_BASE && grade < SUB_MENU_NUM_BASE + sub_menu->count()){
        main_menu->show();
        sub_menu->show();
        sub_menu->setCurrentRow(grade - SUB_MENU_NUM_BASE);
    }
    else{
        main_menu->hide();
        sub_menu->hide();
        return;
    }

    //主菜单
    qDebug()<<"sqlcfg->get_para()->test_mode"<<sqlcfg->get_global()->test_mode;
    if (sqlcfg->get_global()->test_mode == true) {
        main_menu->item(0)->setText(tr("结束测试"));
    }
    else {
        main_menu->item(0)->setText(tr("开始测试"));
    }

    switch (sql->chart) {
    case BASIC:
        main_menu->item(2)->setText(tr("图形显示 \t[时序图]"));
        break;
    case PRPD:
        main_menu->item(2)->setText(tr("图形显示 \t[PRPD]"));
        break;
    case PRPS:
        main_menu->item(2)->setText(tr("图形显示 \t[PRPS]"));
        break;
    case Histogram:
        main_menu->item(2)->setText(tr("图形显示 \t[柱状图]"));
        break;
    case TF:
        main_menu->item(2)->setText(tr("图形显示 \t[T-F图]"));
        break;
    case FLY:
        main_menu->item(2)->setText(tr("图形显示 \t[飞行图]"));
        break;
    case Exponent:
        main_menu->item(2)->setText(tr("图形显示  [特征指数]"));
        break;
    case Spectra:
        main_menu->item(2)->setText(tr("图形显示 \t[频谱图]"));
        break;
    case Camera:
        main_menu->item(2)->setText(tr("图形显示 \t[摄像头]"));
        break;
    default:
        break;
    }

    if(mode == AA1 || mode ==AA2 || mode == AE1 || mode == AE2 ){
        main_menu->item(3)->setText(tr("音量调节\t[×%1]").arg(QString::number(sql->vol)));
    }

    main_menu->item(4)->setText(tr("连续录波\t[%1]s").arg(QString::number(sql->rec_time)));

    if(mode == TEV1 || mode == TEV2 || mode == HFCT1 || mode == HFCT2 || mode == UHF1 || mode == UHF2){
        if(sql->auto_rec == true){
            main_menu->item(5)->setText(tr("自动录波\t[开启]"));
        }
        else{
            main_menu->item(5)->setText(tr("自动录波\t[关闭]"));
        }
    }

    if(mode == HFCT1 || mode == HFCT2){
        main_menu->item(6)->setText(tr("通带下限\t[%1]").arg(Common::filter_to_string(sql->filter_hp)));
        main_menu->item(7)->setText(tr("通带上限\t[%1]").arg(Common::filter_to_string(sql->filter_lp)));
        switch (sql->units) {
        case Units_db:
            main_menu->item(8)->setText(tr("单位切换\t[dB]"));
            break;
        case Units_pC:
            main_menu->item(8)->setText(tr("单位切换\t[pC]"));
            break;
        default:
            break;
        }
    }

    //次级菜单
    if (sql->mode == single) {
        sub_menu->item(0)->setText(tr("检测模式\t[单次]"));
    } else {
        sub_menu->item(0)->setText(tr("检测模式\t[连续]"));
    }
    sub_menu->item(1)->setText(tr("增益调节\t[×%1]").arg(QString::number(sql->gain, 'f', 1)));
    sub_menu->item(2)->setText(tr("脉冲触发\t[%1]mV").arg(QString::number(sql->fpga_threshold)));
    if(sql->units == Units_pC ){
        sub_menu->item(3)->setText(tr("黄色报警阈值\t[%1]pC").arg(QString::number(sql->low)));
        sub_menu->item(4)->setText(tr("红色报警阈值\t[%1]pC").arg(QString::number(sql->high)));
    }
    else{
        sub_menu->item(3)->setText(tr("黄色报警阈值\t[%1]dB").arg(QString::number(sql->low)));
        sub_menu->item(4)->setText(tr("红色报警阈值\t[%1]dB").arg(QString::number(sql->high)));
    }
    sub_menu->item(3)->setText(tr("黄色报警阈值\t[%1]dB").arg(QString::number(sql->low)));
    sub_menu->item(4)->setText(tr("红色报警阈值\t[%1]dB").arg(QString::number(sql->high)));
    sub_menu->item(5)->setText(tr("脉冲计数时长\t[%1]s").arg(QString::number(sql->pulse_time)));
    if(sql->mode_recognition){
        sub_menu->item(6)->setText(tr("模式识别\t[开启]"));
    }
    else{
        sub_menu->item(6)->setText(tr("模式识别\t[关闭]"));
    }
}

void SettingMunu::show()
{
    lineEdit->show();
    label->show();
}

void SettingMunu::hide()
{
    lineEdit->hide();
    label->hide();
}
