#include "ttkradiobuttonwidget.h"

#include <QStateMachine>
#include <QPropertyAnimation>

TTKRadioButtonWidget::TTKRadioButtonWidget(QWidget *parent)
    : TTKCheckable(parent)
{
    setAutoExclusive(true);

    setCheckedIcon(QIcon(":/res/radio_button_checked"));
    setUncheckedIcon(QIcon(":/res/radio_button_unchecked"));

    m_uncheckedState->assignProperty(m_checkedIcon, "iconSize", 0);
    m_uncheckedState->assignProperty(m_uncheckedIcon, "iconSize", ICON_SIZE);

    m_disabledUncheckedState->assignProperty(m_checkedIcon, "iconSize", 0);
    m_disabledUncheckedState->assignProperty(m_uncheckedIcon, "iconSize", ICON_SIZE);

    m_checkedState->assignProperty(m_uncheckedIcon, "iconSize", 0);
    m_checkedState->assignProperty(m_checkedIcon, "iconSize", ICON_SIZE);

    m_disabledCheckedState->assignProperty(m_uncheckedIcon, "iconSize", 0);
    m_disabledCheckedState->assignProperty(m_checkedIcon, "iconSize", ICON_SIZE);

    m_uncheckedState->assignProperty(m_checkedIcon, "opacity", 0);
    m_uncheckedState->assignProperty(m_uncheckedIcon, "opacity", 1);

    m_checkedState->assignProperty(m_uncheckedIcon, "opacity", 0);
    m_checkedState->assignProperty(m_checkedIcon, "opacity", 1);

    m_checkedIcon->setIconSize(0);

    m_checkedState->assignProperty(m_checkedIcon, "color", checkedColor());
    m_checkedState->assignProperty(m_uncheckedIcon, "color", uncheckedColor());
    m_uncheckedState->assignProperty(m_uncheckedIcon, "color", uncheckedColor());

    QPropertyAnimation *animation;

    animation = new QPropertyAnimation(m_checkedIcon, "iconSize", this);
    animation->setDuration(250);
    m_stateMachine->addDefaultAnimation(animation);

    animation = new QPropertyAnimation(m_uncheckedIcon, "iconSize", this);
    animation->setDuration(250);
    m_stateMachine->addDefaultAnimation(animation);

    animation = new QPropertyAnimation(m_uncheckedIcon, "opacity", this);
    animation->setDuration(250);
    m_stateMachine->addDefaultAnimation(animation);

    animation = new QPropertyAnimation(m_checkedIcon, "opacity", this);
    animation->setDuration(250);
    m_stateMachine->addDefaultAnimation(animation);

    connect(this, SIGNAL(toggled(bool)), this, SLOT(setRadioTextColor(bool)));
}

void TTKRadioButtonWidget::setRadioTextColor(bool checked)
{
    if(checked){
//        setTextColor("#0EC3FD");
        setTextColor("#4EFF67");
    }
    else{
        setTextColor("#FFFFFF");
    }

}

void TTKRadioButtonWidget::setupProperties()
{
    TTKCheckable::setupProperties();

    m_checkedState->assignProperty(m_checkedIcon, "color", checkedColor());
    m_checkedState->assignProperty(m_uncheckedIcon, "color", uncheckedColor());
    m_uncheckedState->assignProperty(m_uncheckedIcon, "color", uncheckedColor());
}
