#ifndef TTKRADIOBUTTONWIDGET_H
#define TTKRADIOBUTTONWIDGET_H

#include "ttkcheckable.h"
#include "ttkglobal.h"

class TTKRadioButtonWidget : public TTKCheckable
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKRadioButtonWidget)
public:
    explicit TTKRadioButtonWidget(QWidget *parent = nullptr);

public slots:
    void setRadioTextColor(bool checked);

protected:
    virtual void setupProperties() override;

};

#endif // TTKRADIOBUTTONWIDGET_H
