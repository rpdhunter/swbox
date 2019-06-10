#ifndef TTKCHECKABLE_H
#define TTKCHECKABLE_H

#include <QAbstractButton>
#include "ttkglobaldefine.h"

class QState;
class QStateMachine;
class QSignalTransition;

class TTKCheckable;

#define ICON_SIZE 20

class TTKCheckableIcon : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit TTKCheckableIcon(const QIcon &icon, TTKCheckable *parent = 0);

    virtual QSize sizeHint() const override;

    void setIcon(const QIcon &icon);
    inline QIcon icon() const { return m_icon; }

    void setColor(const QColor &color);
    inline QColor color() const { return m_color; }

    void setIconSize(qreal size);
    inline qreal iconSize() const { return m_iconSize; }

    void setOpacity(qreal opacity);
    inline qreal opacity() const { return m_opacity; }

protected:
    virtual void paintEvent(QPaintEvent *event) override;

    TTKCheckable *m_checkable;
    QColor m_color;
    QIcon m_icon;
    qreal m_iconSize, m_opacity;

};

class TTKCheckable : public QAbstractButton
{
    Q_OBJECT
public:
    enum LabelPosition
    {
        LabelPositionLeft,
        LabelPositionRight,
    };

    explicit TTKCheckable(QWidget *parent = nullptr);
    virtual ~TTKCheckable();

    void setLabelPosition(LabelPosition placement);
    LabelPosition labelPosition() const;

    void setCheckedColor(const QColor &color);
    QColor checkedColor() const;

    void setUncheckedColor(const QColor &color);
    QColor uncheckedColor() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setDisabledColor(const QColor &color);
    QColor disabledColor() const;

    void setCheckedIcon(const QIcon &icon);
    QIcon checkedIcon() const;

    void setUncheckedIcon(const QIcon &icon);
    QIcon uncheckedIcon() const;

    virtual QSize sizeHint() const override;

protected:
    virtual bool event(QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    virtual void setupProperties();

    TTKCheckableIcon *m_checkedIcon, *m_uncheckedIcon;
    QStateMachine *m_stateMachine;
    QState *m_uncheckedState, *m_checkedState;
    QState *m_disabledUncheckedState, *m_disabledCheckedState;
    QSignalTransition *m_uncheckedTransition, *m_checkedTransition;
    TTKCheckable::LabelPosition  m_labelPosition;
    QColor m_checkedColor, m_uncheckedColor, m_textColor, m_disabledColor;

};

#endif // TTKCHECKABLE_H
