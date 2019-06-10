#ifndef TTKMARQUEELABEL_H
#define TTKMARQUEELABEL_H

#include <QWidget>
#include "ttkglobal.h"
#include "ttkglobaldefine.h"

class TTKMarqueeLabel : public QWidget
{
    Q_OBJECT
    TTK_DECLARE_MODULE(TTKMarqueeLabel)
public:
    explicit TTKMarqueeLabel(QWidget *parent = nullptr);

    void setText(const QString &newText);
    inline QString text() const { return m_myText.trimmed(); }

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    QString m_myText;
    int m_offset, m_myTimerId;

};

#endif // TTKMARQUEELABEL_H
