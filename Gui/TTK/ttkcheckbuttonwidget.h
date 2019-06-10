#ifndef TTKCHECKBUTTONWIDGET_H
#define TTKCHECKBUTTONWIDGET_H

#include "ttkcheckable.h"
#include "ttkglobal.h"

class TTKCheckButtonWidget : public TTKCheckable
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKCheckButtonWidget)
public:
    explicit TTKCheckButtonWidget(QWidget *parent = nullptr);

};

#endif // TTKCHECKBUTTONWIDGET_H
