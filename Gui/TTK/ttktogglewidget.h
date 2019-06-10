#ifndef TTKTOGGLEWIDGET_H
#define TTKTOGGLEWIDGET_H

#include <QAbstractButton>
#include "ttkglobal.h"
#include "ttkglobaldefine.h"

class QState;
class QStateMachine;
class TTKToggleWidget;

class TTKToggleThumb : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal shift WRITE setShift READ shift)
    Q_PROPERTY(QColor thumbColor WRITE setThumbColor READ thumbColor)
public:
    explicit TTKToggleThumb(TTKToggleWidget *parent = 0);

    void setShift(qreal shift);

    inline qreal shift() const { return m_shift; }
    inline qreal offset() const { return m_offset; }

    void setThumbColor(const QColor &color);
    inline QColor thumbColor() const { return m_thumbColor; }

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    void updateOffset();

    TTKToggleWidget *m_toggle;
    QColor m_thumbColor;
    qreal m_shift, m_offset;

};

class TTKToggleTrack : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor trackColor WRITE setTrackColor READ trackColor)
public:
    explicit TTKToggleTrack(TTKToggleWidget *parent = 0);

    void setTrackColor(const QColor &color);
    inline QColor trackColor() const { return m_trackColor; }

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    TTKToggleWidget *m_toggle;
    QColor m_trackColor;

};

class TTKToggleWidget : public QAbstractButton
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKToggleWidget)
public:
    explicit TTKToggleWidget(QWidget *parent = nullptr);
    ~TTKToggleWidget();

    void setDisabledColor(const QColor &color);
    QColor disabledColor() const;

    void setActiveColor(const QColor &color);
    QColor activeColor() const;

    void setInactiveColor(const QColor &color);
    QColor inactiveColor() const;

    void setTrackColor(const QColor &color);
    QColor trackColor() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;

    void setupProperties();

    TTKToggleTrack *m_track;
    TTKToggleThumb *m_thumb;
    QStateMachine *m_stateMachine;
    QState *m_offState, *m_onState;
    Qt::Orientation m_orientation;
    QColor m_disabledColor, m_activeColor, m_inactiveColor, m_trackColor;

};

#endif // TTKTOGGLEWIDGET_H
