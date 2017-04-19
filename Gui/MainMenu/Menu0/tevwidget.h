#ifndef AMPLITUDE1_H
#define AMPLITUDE1_H

#include <QWidget>

namespace Ui {
class Amplitude1;
}

class Amplitude1 : public QWidget
{
    Q_OBJECT

public:
    explicit Amplitude1(QWidget *parent = 0);
    ~Amplitude1();

private:
    Ui::Amplitude1 *ui;
};

#endif // AMPLITUDE1_H
