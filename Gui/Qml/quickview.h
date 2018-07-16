#ifndef QUICKVIEW_H
#define QUICKVIEW_H
#include <QQuickView>
#include <QTimer>
#include <QCursor>
#include <QImage>
#include <QPixmap>
#include <QLabel>

class QuickView : public QQuickView
{
    Q_OBJECT
public:
    QuickView(QString str = 0);

public slots:
    void show_input(QString str1, QString str2);                          //主界面交互
    void trans_input_key(quint8 key_code);      //主界面交互
    void editFinished(QString str);             //QML交互
    void qml_view_init(int y_init);             //QML应答,返回输入盘的纵坐标

signals:
    void mouseClicked(int mouseX,int mouseY);   //QML交互
    void mouseMoved(int mouseX,int mouseY);     //QML交互
    void inputClear(QString str1, QString str2);
    void input_str(QString str);
    void viewInit(int h_space);                 //给QML一个初始化参数表,h_space是view的Y坐标偏移

private slots:
//    void test_press();
    void press1();
    void press2();
    void view_init();

private:
    QTimer *timer1, *timer2, *timer3, *timer4;
    QTimer *timer_short;

    bool isWorking;

    int n;
    int _x, _y;
    int space;
    int y_min, y_max;

    void do_key_up_down(int d);
    void do_key_left_right(int d);

    void refresh();



};

#endif // QUICKVIEW_H
