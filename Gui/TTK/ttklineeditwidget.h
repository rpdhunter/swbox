#ifndef TTKLINEEDITWIDGET_H
#define TTKLINEEDITWIDGET_H

#include <QLineEdit>
#include "ttkglobal.h"
#include "ttkglobaldefine.h"

class QPropertyAnimation;

class TTKLineEditWidget : public QLineEdit
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKLineEditWidget)
public:
    explicit TTKLineEditWidget(QWidget *parent = nullptr);
    virtual ~TTKLineEditWidget();

    void select(bool f);

    void setColor(const QColor &color);

    virtual QSize sizeHint() const override;

private Q_SLOTS:
    void valueChanged(const QVariant &value);
    void animationFinished();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;

    bool m_isAnimating;
    float m_currentValue;
    QColor m_color;
    QPropertyAnimation *m_animation;

};

#endif // TTKLINEEDITWIDGET_H
