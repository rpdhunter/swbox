#include "tabwidget.h"
//#include "IO/Data/data.h"

TabWidget::TabWidget(SQL_PARA *sql, QWidget *parent) : QWidget(parent)
{
    sql_para = sql;
    row = 0;
    col = 0;

    pbt_style_unselect = "QPushButton{border-style:none; border:1px solid #FFFFFF;"
                       "background-color: transparent;"
                       "color: #FFFFFF;"
                       "padding: 5px;"
                       "min-height: 15px;"
                       "border-radius: 10px;"
                       "}";
    pbt_style_select = "QPushButton{border-style:none; border:0px solid #FFFFFF;"
                         "background-color: #0EC3FD;"
                         "color:#FFFFFF;"
                         "padding:5px;"
                         "min-height:15px;"
                         "border-radius: 10px;"
                         "}";
}
