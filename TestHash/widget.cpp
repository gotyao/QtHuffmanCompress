#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QString Filename;
    Filename = QFileDialog::getOpenFileName(this,
                                            "open a file",
                                            "C:",
                                            "txt(*.txt) ;; all (*.*)");
    if(Filename.isEmpty())
        return ;

    QFile file(Filename);
    if (file.open(QFile::ReadOnly)){
        string str;
        QByteArray array;

        do{
            array = file.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            QString temp = toUtf8(array);
            str = temp.toStdString();

            hash.push_back(Word(MyHash(str), str));
        }while(!(file.atEnd()));
    }
    else return ;
    max = 1;

    file.close();
    insertionSort(hash);

    ui->label->installEventFilter(this);
    draw(ui->label);
}

Widget::~Widget(){
    delete ui;
}

bool Widget::eventFilter(QObject *obj, QEvent *event){
    if(obj == ui->label){
        draw(ui->label);
        return true;
    }
    else return QWidget::eventFilter(obj, event);
}

void Widget::insertionSort(vector<Word> &a){
    for(int i = 1; i < a.size(); i++){
        int j = i;
        while(a[j-1].hashvalue > a[j].hashvalue){
            swap(a[j-1], a[j]);
            if(j != 1) j--;
        }
        if(a[j-1].hashvalue == a[j].hashvalue && a[j-1].str != a[j].str){
            a[j-1].cnt++;
            if(a[j-1].cnt > max) max = a[j-1].cnt;
            a.erase(a.begin() + j);
            i--;
        }
    }
}

void Widget::draw(QLabel *label){
    QPainter painter(ui->label);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    painter.drawLine(0,500,1200,500);

    ui->label_3->setText(QString::number(hash.size()));

    double hPercnt = 500.0 / max;
    if(max == 1) hPercnt = 200.0;
    double width = 1200.0 / hash.size();

    for(auto it = hash.begin(); it != hash.end(); it++){
        if(it->cnt == 1){
            painter.setPen(Qt::green);
            painter.setBrush(Qt::green);
        }
        else{
            painter.setPen(Qt::red);
            painter.setBrush(Qt::red);
        }

        painter.drawRect(
            QRectF( (it-hash.begin())*width, 500.0 - hPercnt * it->cnt, width, hPercnt * it->cnt)
        );
    }
}

//测试使用的函数
//BKDHash
unsigned MyHash(string str){
    unsigned seed = 131; //13, 131, 1313...
    unsigned hash = 0;

    for (auto c : str) {
        hash = hash * seed + c;
    }

    return (hash & 0x7fffffff);
}

//Times33Hash
//unsigned MyHash(string str){
//    unsigned hash = 5381;

//    for (auto c : str) {
//        hash = (hash << 7) + hash + c;
//    }

//    return (hash & 0x7fffffff); // %(2^32)
//}

//APHash
//unsigned MyHash(string str){
//    unsigned hash = 0;
//    int i = 0;

//    for(auto c : str){
//        if(!(i & 1)){
//            hash ^= ((hash << 7) ^ c ^ (hash >> 3));
//        }
//        else{
//            hash ^= (~((hash << 11) ^ c ^ (hash >> 5)));
//        }
//    }

//    return (hash & 0x7fffffff);
//}
