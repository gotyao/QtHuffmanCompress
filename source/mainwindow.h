#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QStringDecoder>
#include "Huffman.h"
#include "adaptivehuffman.h"
#include "Mymap.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //主窗口的构造函数
    MainWindow(QWidget *parent = nullptr);
    //主窗口的析构函数
    ~MainWindow();
    //动态Huffman编码树
    AdaptiveHuffman adaptiveTree;
    //使用自定义map类统计字符频率，用字符作为key, 频率作为val
    Mymap frequency;
    //存储字符及频率
    vector<Data> character;
    //压缩文件末尾无效二进制位数
    unsigned empty;

private slots:
    //实现解压文件功能
    void on_Decompressed_clicked();
    void on_addFrequencyFile_clicked();
    void on_addCodeFile_clicked();
    void on_addCompressedFile_clicked();

    //实现适应性Huffman编码
    void on_textFileListClear_clicked();
    void on_reConstruct_toggled();
    void on_textFileListAppend_clicked();

    //实现文本相同检测
    void on_addCmpFile_1_clicked();
    void on_addCmpFile_2_clicked();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
