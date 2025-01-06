#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QLabel>
#include <QPaintDevice>
#include <QPainter>
#include <QPen>
#include <QFileDialog>
#include <QStringDecoder>
#include <bits/stdc++.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

using namespace std;

struct Word{
    unsigned hashvalue;
    string str;
    unsigned cnt;
    Word(unsigned v, string s, unsigned c = 1){
        hashvalue = v, str = s, cnt = c;
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    int max;
    vector<Word> hash;
    void draw(QLabel *label);//绘图
    bool eventFilter(QObject *obj, QEvent *event);//辅助绘图
    void insertionSort(vector<Word>& a);

private:
    Ui::Widget *ui;
};

unsigned MyHash(string str);

#endif // WIDGET_H
