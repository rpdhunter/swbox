#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <QListWidget>
#include "IO/SqlCfg/sqlcfg.h"

class ContextMenu : public QListWidget
{
    Q_OBJECT
public:
    explicit ContextMenu(CHANNEL_SQL *channel_sql, QWidget *parent = nullptr);

    void toggle_state();        //切换次级菜单状态
    void fresh(int index = 0);       //刷新菜单状态,并选中index编号的菜单

signals:

public slots:

private:
    CHANNEL_SQL *channel_sql;
};

#endif // CONTEXTMENU_H
