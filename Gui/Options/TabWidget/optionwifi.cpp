#include "optionwifi.h"
#include "ui_optionwifi.h"
#include "ui_apinfo.h"
#include "Gui/Common/common.h"

OptionWifi::OptionWifi(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::OptionWifi),
    ui_ap(new Ui::ApInfomation)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    wifi_init();
    fresh();
}

OptionWifi::~OptionWifi()
{
    delete ui;
}

void OptionWifi::wifi_init()
{
    //AP列表
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->horizontalHeader()->resizeSection(0,0);        //编号
    ui->tableWidget->horizontalHeader()->resizeSection(1,200);      //SSID
    ui->tableWidget->horizontalHeader()->resizeSection(2,25);       //是否加密
    ui->tableWidget->horizontalHeader()->resizeSection(3,25);       //强度

    wifi_managment = new WifiManagement;
    connect(wifi_managment, SIGNAL(ap_fresh_list_complete()), this, SLOT(ap_fresh_list_complete()));          //刷新列表完成
    connect(wifi_managment, SIGNAL(ap_connect_complete(bool)), this, SLOT(ap_connect_complete(bool)));        //连接成功/失败处理
    connect(wifi_managment, SIGNAL(ap_create_complete(bool)), this, SLOT(ap_create_complete(bool)));          //创建成功/失败处理

    //AP信息
    ap_info_widget = new QFrame(this);
    ui_ap->setupUi(ap_info_widget);
    ap_info_widget->setGeometry(this->geometry().adjusted(3,2,-3,-2));
    ap_info_widget->setStyleSheet("QFrame {background-color:#1C1C28;}");
    ap_info_widget->hide();

    //右键菜单
    contextMenu = new QListWidget(this);
    contextMenu->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    QStringList list;
    list << tr("连接/断开连接") << tr("查看连接信息") ;
    contextMenu->addItems(list);
    contextMenu->resize(100, 40);
    contextMenu->move(250,40);
    contextMenu->setSpacing(2);
    contextMenu->hide();
    contextMenu_num = 2;        //显示菜单条目
    menu_row = 0;

    ui->groupBox_wlan->hide();
    ui->groupBox_hotpot->hide();

    wifi_password = new WifiPassword;

    inputStatus = false;
    isBuzzy = false;
    hotpot_row = 0;

    QString mac = sqlcfg->get_global()->mac;
    ui->lineEdit_wifi_hot_name->setText("window1000" + mac.mid(mac.count()-2, 2));
    ui->lineEdit_wifi_hot_password->setText("********");        //隐藏zdit.com.cn
    ui->lineEdit_wifi_hot_gate->setText("192.168." + mac.mid(mac.count()-2, 2) + ".1");
}


void OptionWifi::work(int )
{
    col = 0;
    row = 1;
    fresh();
}

void OptionWifi::do_key_ok()
{
    if(isBuzzy)
        return;

    if(ui->groupBox_wlan->isVisible()){         //wlan设置
        if(ap_info_widget->isVisible()){            //AP信息
            switch (ap_info_widget_button) {
            case 0:
                ap_info_widget->hide();
                break;
            case 1:                 //忘记密码
                qDebug()<<"ap_info forget!";
                wifi_password->del_key(ui_ap->lab_SSID->text());
                ap_fresh_list_complete();
                break;
            default:
                break;
            }
        }
        else if(contextMenu->isVisible()){          //右键菜单
            contextMenu->hide();
            switch (menu_row) {
            case 0:                                     //连接某一个wifi
                qDebug()<<"connect wifi!";
                wifi_connect();
                sqlcfg->get_global()->wifi_working = true;
                break;
            case 1:                                     //查看信息
                qDebug()<<"wifi infomation!";
                ap_info_widget->show();
                ap_info_widget_button = 0;
                break;
            default:
                break;
            }
            menu_row = 0;
        }
        else if(col == 0){                          //AP列表
            emit show_indicator(true);
            isBuzzy = true;
            wifi_managment->ap_fresh_list();            //刷新列表
            sqlcfg->get_global()->wifi_working = true;
        }
        else{
            qDebug()<<"wifi infomation!";
            ap_info_widget->show();
            ap_info_widget_button = 0;
        }
    }
    else if(ui->groupBox_hotpot->isVisible()){  //热点设置
        switch (hotpot_row) {
        case 0:         //开启热点
            qDebug()<<"open hotpot!";
            emit show_indicator(true);
            isBuzzy = true;
            if(ui->lineEdit_wifi_hot_password->text() == "********"){           //默认密码
                wifi_managment->ap_create(ui->lineEdit_wifi_hot_name->text(), "zdit.com.cn",
                                          ui->lineEdit_wifi_hot_gate->text(), ui->lineEdit_wifi_hot_mask->text());
            }
            else{                   //自定密码
                wifi_managment->ap_create(ui->lineEdit_wifi_hot_name->text(), ui->lineEdit_wifi_hot_password->text(),
                                          ui->lineEdit_wifi_hot_gate->text(), ui->lineEdit_wifi_hot_mask->text());
            }
            break;
        case 1:         //修改名称
            emit show_input(ui->lineEdit_wifi_hot_name->text(),tr("请输入wifi名称"));
            inputStatus = true;
            break;
        case 2:         //修改密码
            if(ui->lineEdit_wifi_hot_password->text() == "********"){
                emit show_input("",tr("请输入wifi密码"));
            }
            else{
                emit show_input(ui->lineEdit_wifi_hot_password->text(),tr("请输入wifi密码"));
            }
            inputStatus = true;
            break;
        case 3:         //修改网关
            emit show_input(ui->lineEdit_wifi_hot_gate->text(),tr("请输入网关"));
            inputStatus = true;
            break;
        case 4:         //修改子网掩码
            emit show_input(ui->lineEdit_wifi_hot_mask->text(),tr("请输入子网掩码"));
            inputStatus = true;
            break;
        default:
            break;
        }
        sqlcfg->get_global()->wifi_working = true;

    }
    else{                                       //切换按钮操作
        switch (row) {
        case 1:
            ui->groupBox_wlan->show();
            break;
        case 2:
            ui->groupBox_hotpot->show();
            break;
        default:
            break;
        }
    }
    fresh();
}

void OptionWifi::do_key_cancel()
{
    if(isBuzzy)
        return;

    if(ui->groupBox_wlan->isVisible()){         //wlan设置        
        if(ap_info_widget->isVisible()){            //AP信息
            ap_info_widget->hide();
            ap_info_widget_button = 0;
        }
        else if(contextMenu->isVisible()){          //右键菜单
            contextMenu->hide();
            menu_row = 0;
        }
        else if(col == 0){                          //AP列表
            ui->groupBox_wlan->hide();
        }
        else{
            col = 0;
        }
    }
    else if(ui->groupBox_hotpot->isVisible()){  //热点设置
        ui->groupBox_hotpot->hide();
        hotpot_row = 0;
    }
    else{                                       //切换按钮操作
        row = 0;
        emit quit();
    }
    fresh();
}

void OptionWifi::do_key_up_down(int d)
{
    if(isBuzzy)
        return;

    if(ui->groupBox_wlan->isVisible()){         //wlan设置
        if(ap_info_widget->isVisible()){            //AP信息
        }
        else if(contextMenu->isVisible()){          //右键菜单
            Common::change_index(menu_row, d, contextMenu_num-1, 0);
        }
        else{                                       //AP列表
            Common::change_index(col, d, ui->tableWidget->rowCount(), 0);
        }

    }
    else if(ui->groupBox_hotpot->isVisible()){  //热点设置
        Common::change_index(hotpot_row, d, 4, 0);
    }
    else{                                       //切换按钮操作
        Common::change_index(row, d, 2, 1);
    }
    fresh();
}

void OptionWifi::do_key_left_right(int d)
{
    if(isBuzzy)
        return;

    if(ui->groupBox_wlan->isVisible()){         //wlan设置
        if(ap_info_widget->isVisible()){            //AP信息
            Common::change_index(ap_info_widget_button, d, 1, 0);
        }
        else if(contextMenu->isVisible()){          //右键菜单
        }
        else if(col > 0){                           //AP列表
            contextMenu->show();
        }
    }
    else if(ui->groupBox_hotpot->isVisible()){  //热点设置

    }
    else{                                       //切换按钮操作
        Common::change_index(row, d, 2, 1);
    }
    fresh();
}

void OptionWifi::ap_fresh_list_complete()
{
    QList<AP_INFO> ap_list = wifi_managment->ap_list();
    emit show_indicator(false);
    isBuzzy = false;
    ui->tableWidget->clear();       //清空内容,释放指针
    ui->tableWidget->setRowCount(ap_list.count());

    for (int i = 0; i < ap_list.count(); ++i) {
        AP_INFO ap = ap_list.at(i);

        QTableWidgetItem *item = new QTableWidgetItem(QString::number(i));
        ui->tableWidget->setItem(i,0,item);         //序号

        item = new QTableWidgetItem(ap.ssid);
        ui->tableWidget->setItem(i,1,item);         //SSID

        if(ap.has_key){
            if(wifi_password->lookup_key(ap.ssid).isEmpty() ){
                item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/lock.png"),QString());
            }
            else{
                item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/key.png"),QString());
            }
        }
        else{
            item = new QTableWidgetItem(QIcon(),QString());
        }
        if(ap.is_connected){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/tick.png"),QString());
        }
        ui->tableWidget->setItem(i,2,item);         //秘钥(图标)


        if(ap.signal_level > 75){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi3_c.png"),QString());
        }
        else if(ap.signal_level > 50){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi2_c.png"),QString());
        }
        else if(ap.signal_level > 25){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi1_c.png"),QString());
        }
        else{
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi1_c.png"),QString());
        }
        ui->tableWidget->setItem(i,3,item);         //强度(图标)
    }
}

void OptionWifi::input_finished(QString str)
{
    if(inputStatus){            //保证不会收到非此界面的键盘输入
        inputStatus = false;
        if(str.isEmpty()){
            return;
        }
        if(ui->groupBox_wlan->isVisible()){         //wlan设置
            QString name = ui->tableWidget->item(ui->tableWidget->currentRow(),1)->text();
//            wifi_password->add_new(name, str);      //添加密码本
            emit show_indicator(true);          //开启菊花状态
            isBuzzy = true;
            wifi_managment->ap_connect(name,str);   //执行连接
        }
        else if(ui->groupBox_hotpot->isVisible()){  //热点设置
            switch (hotpot_row) {
            case 1:         //修改名称
                ui->lineEdit_wifi_hot_name->setText(str);
                break;
            case 2:         //修改密码
                ui->lineEdit_wifi_hot_password->setText(str);
                break;
            case 3:         //修改网关
                ui->lineEdit_wifi_hot_gate->setText(str);
                break;
            case 4:         //修改子网掩码
                ui->lineEdit_wifi_hot_mask->setText(str);
                break;
            default:
                break;
            }
        }
    }
}

bool OptionWifi::getInputStatus() const
{
    return inputStatus;
}

//发起wlan连接
void OptionWifi::wifi_connect()
{
    AP_INFO ap = wifi_managment->ap_list().at(col-1);
    if(ap.is_connected){            //已连接状态，断开连接
        wifi_managment->ap_disconnect();
        emit show_indicator(false);          //开启菊花状态
        isBuzzy = false;
        ap_fresh_list_complete();
        ui->label_connect_message->setText(tr("当前连接已断开"));
    }
    else{                           //未连接状态，执行连接操作
        QString name = ap.ssid;
        if(ap.has_key){     //有密码
            QString password = wifi_password->lookup_key(name);     //通过密码本查找密码
            if(password.isEmpty()){                                 //没找到,开启虚拟键盘
                emit show_input("",tr("请输入wifi密码"));
                inputStatus = true;
            }
            else{
                emit show_indicator(true);          //开启菊花状态
                isBuzzy = true;
                wifi_managment->ap_connect(name, password);
            }
        }
        else{               //无密码
            emit show_indicator(true);          //开启菊花状态
            isBuzzy = true;
            wifi_managment->ap_connect(name, "");
        }
        ui->label_connect_message->setText(tr("正在尝试连接%1").arg(wifi_managment->current_ap().ssid));
    }

}

//返回wlan连接结果
void OptionWifi::ap_connect_complete(bool f)
{
    emit show_indicator(false);          //关闭菊花状态
    isBuzzy = false;
    ap_fresh_list_complete();               //刷新列表
    if(f){
        ui->tableWidget->setCurrentCell(0,0);       //选中连接了的AP
        col = 1;
        ui->label_connect_message->setText(wifi_managment->current_ap().ssid + tr("连接成功!"));
        wifi_password->add_new(wifi_managment->current_ap().ssid, wifi_managment->current_ap().password);      //添加密码本
    }
    else{
        ui->label_connect_message->setText(tr("%1连接失败，请确认密码是否正确").arg(wifi_managment->current_ap().ssid));
    }
    fresh();
}

void OptionWifi::ap_create_complete(bool f)
{
    emit show_indicator(false);          //关闭菊花状态
    isBuzzy = false;
    if(f){
        ui->label_connect_message->setText(tr("创建热点%1成功!").arg(wifi_managment->current_ap().ssid));
    }
    else{
        ui->label_connect_message->setText(tr("创建热点%1失败!").arg(wifi_managment->current_ap().ssid));
    }
    fresh();
}

void OptionWifi::fresh()
{
    if(ui->groupBox_wlan->isVisible()){         //wlan设置
        //AP列表
        switch (col) {
        case 0:
            ui->pbt_refresh_aplist->setStyleSheet(pbt_style_select);
            break;
        default:
            ui->pbt_refresh_aplist->setStyleSheet(pbt_style_unselect);
            break;
        }
        ui->tableWidget->setCurrentCell(col - 1, 0);

        //AP信息
        if(ap_info_widget_button == 0){         //关闭
            ui_ap->pbt_forget->setStyleSheet(pbt_style_unselect);
            ui_ap->pbt_close->setStyleSheet(pbt_style_select);
        }
        else{                                   //忘记网络
            ui_ap->pbt_forget->setStyleSheet(pbt_style_select);
            ui_ap->pbt_close->setStyleSheet(pbt_style_unselect);
        }

        if(col > 0 && col <= wifi_managment->ap_list().count()){
            AP_INFO ap = wifi_managment->ap_list().at(col - 1);
            ui_ap->lab_SSID->setText(ap.ssid);              //网络名(ESSID)
            ui_ap->lab_mac->setText(ap.mac);                //mac地址(Address)
            ui_ap->lab_freq->setText(ap.freq);              //频率(Frequency)
            ui_ap->lab_bitrate->setText(ap.bitrate);        //比特率(Bit Rates)
            ui_ap->lab_signallevel->setText(QString::number(ap.signal_level));   //信号强度(Signal level)
            ui_ap->lab_safty->setText(ap.safty);            //加密协议(IE: WPA Version 1 / IE: IEEE 802.11i/WPA2 Version 1)

            ui_ap->lab_IP->setText(ap.ip);                  //ip地址
            ui_ap->lab_gate->setText(ap.gate);              //网关
            ui_ap->lab_mask->setText(ap.mask);              //掩码
            ui_ap->lab_pd_mac->setText(ap.pd_mac);          //仪器mac地址
            if(!wifi_password->lookup_key(ap.ssid).isEmpty()){                                 //是否保存密码
                ui_ap->lab_saveconnect->setText(tr("已保存"));
                ui_ap->lab_saveconnect->setStyleSheet("QLabel {color:green;}");
            }
            else{
                ui_ap->lab_saveconnect->setText(tr("未保存"));
                ui_ap->lab_saveconnect->setStyleSheet("QLabel {color:white;}");
            }
            if(ap.is_connected){                                 //是否处于连接状态
                ui_ap->lab_connect_status->setText(tr("已连接"));
                ui_ap->lab_connect_status->setStyleSheet("QLabel {color:green;}");
            }
            else{
                ui_ap->lab_connect_status->setText(tr("未连接"));
                ui_ap->lab_connect_status->setStyleSheet("QLabel {color:white;}");
            }

            //右键菜单
            contextMenu->setCurrentRow(menu_row);
            if(ap.is_connected)
                contextMenu->item(0)->setText(tr("断开连接"));
            else{
                contextMenu->item(0)->setText(tr("连接"));
            }
        }

    }
    else if(ui->groupBox_hotpot->isVisible()){  //热点设置
        ui->lineEdit_wifi_hot_name->deselect();
        ui->lineEdit_wifi_hot_password->deselect();
        ui->lineEdit_wifi_hot_gate->deselect();
        ui->lineEdit_wifi_hot_mask->deselect();
        ui->pushButton_hot->setStyleSheet("");
        switch (hotpot_row) {
        case 0:         //开启热点
            ui->pushButton_hot->setStyleSheet(pbt_style_select);
            break;
        case 1:         //热点名称
            ui->lineEdit_wifi_hot_name->selectAll();
            break;
        case 2:         //热点密码
            ui->lineEdit_wifi_hot_password->selectAll();
            break;
        case 3:         //热点网关
            ui->lineEdit_wifi_hot_gate->selectAll();
            break;
        case 4:         //热点掩码
            ui->lineEdit_wifi_hot_mask->selectAll();
            break;
        default:
            break;
        }
    }
    else{                                       //切换按钮操作
        ui->pbt_wlan->setStyleSheet(pbt_style_unselect);
        ui->pbt_hotpot->setStyleSheet(pbt_style_unselect);
        switch (row) {
        case 1:
            ui->pbt_wlan->setStyleSheet(pbt_style_select);
            break;
        case 2:
            ui->pbt_hotpot->setStyleSheet(pbt_style_select);
            break;
        default:
            break;
        }
    }
    emit show_wifi_icon(wifi_managment->wifi_mode());
}
