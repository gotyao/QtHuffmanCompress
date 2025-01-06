#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //统计文件字符数
    unsigned FileSize{0};
    empty = 0;
    //使用Qt自带的消息框操作获取文件路径
    ui->setupUi(this);
    this->setWindowTitle("Huffman Coding");
    ui->textFileList->append("当前流内无文件");
    ui->reConstruct->setChecked(true);
    QString Filename = QFileDialog::getOpenFileName(this,
                                            "请选择需要压缩的文件",
                                            "C:",
                                            "txt(*.txt) ;; all (*.*)");
    if(Filename.isEmpty()){
        ui->statusbar->showMessage("未选择待压缩文件");
        return ;
    }

    QFile textfile(Filename);//Qt自带的文件类型
    if(!textfile.open(QIODevice::ReadOnly)){
        ui->statusbar->showMessage("打开待压缩文件失败");
        return ;
    }
    long long Sizeoftextfile{textfile.size()};
    QByteArray array;//Qt自带的二进制流类型，按照二进制存储数据，因此可以适应多种编码方式。在输入时几乎不影响，但是输出时可以方便地输出utf-8编码的字符串

    //开始计时
    clock_t startload, endcompressed;
    startload = clock();

    //读取文件，获知字符频数
    do{
        array = textfile.readLine();//按段落读取文件的二进制数据
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);//从二进制流转为QString的编码方式，这里为utf-8
        QString str = toUtf8(array);//按照utf-8的编码方式对二进制流进行解码
        string temp = str.toStdString();//从Qt的QString类型转换为更加熟悉的std::string类型，方便后续操作
        //上述按段落读取文件的操作可以使用fgets实现，这里采取离Qt建议的实现方式，一方面更加贴近Qt的架构，一方面也能跟后续输出解压缩文件的操作格式统一

        for(int i = 0; i < temp.size(); i++){
            unsigned judge = (1 << 7);
            int size{0};//该utf-8字符的长度
            while(temp[i] & judge){//通过前缀1的数量判断字符长度
                size++;
                judge >>= 1;
            }

            unsigned a{0};
            if(size){//长度不为1，不能用单个char来保存字符
                for(int j = 0; j < size; j++){
                    a <<= 8;
                    a += (unsigned char)temp[i++];
                }
                i--;
                FileSize += size;
            }
            else{//长度为1，直接转为char
                a = temp[i];
                FileSize++;
            }
            //增加字符频数
            frequency[a]++;

            if(frequency[a] == 1) {//是新字符，将其加入到数组内
                character.push_back(a);
            }
        }
    }while(!(textfile.atEnd()));
    textfile.close();

    for(int i = 0; i < character.size(); i++){
        //更新每个字符的频率
        character[i].f = frequency[character[i].c];
    }

    //使用数组构造静态Huffman树
    HuffmanTree tree(character);
    //将每个字符及其频率输出到HuffmanCode中
    map<unsigned, string> HuffmanCode;
    tree.CreateCode(HuffmanCode);

    //实现压缩文件输出
    while(Filename.back() != '.') Filename.erase(Filename.end()-1);
    Filename.erase(Filename.end()-1);
    QFile CompressedFile(Filename + "_compressed.bin");//确定压缩文件输出路径和文件名
    if(!CompressedFile.open(QIODevice::WriteOnly)){
        ui->statusbar->showMessage("新建压缩文件失败");
        return ;
    }
    QDataStream CompressedOut(&CompressedFile);//类似std::fstream的类
    textfile.open(QIODevice::ReadOnly);

    long long SizeofCompressedFile{0};
    unsigned a{0}, l{0};
    //再次读取文件，获知原文件字符顺序
    do{
        array = textfile.readLine();
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
        QString str = toUtf8(array);
        string temp = str.toStdString();
        //以上操作与首次读取文件相同，故不再做说明

        for(int i = 0; i < temp.size(); i++){
            unsigned judge = (1 << 7);
            int size{0};
            while(temp[i] & judge){
                size++;
                judge >>= 1;
            }

            unsigned ch{0};
            if(size){
                for(int j = 0; j < size; j++){
                    ch <<= 8;
                    ch += (unsigned char)temp[i++];
                }
                i--;
            }
            else{
                ch = temp[i];
            }
            //读入字符方式与首次读入相同

            string code = HuffmanCode[ch];//获知字符的Huffman编码
            for(auto e : code){
                //将字符串转为二进制存储在a内
                a <<= 1;
                if(e == '1') a++;
                l++;

                //当a存储满的时候，输出a并且将a置0以便下一次输出
                if(l == 32){
                    CompressedOut << a;
                    SizeofCompressedFile += 4;
                    a = l = 0;
                }
            }
        }
    }while(!(textfile.atEnd()));

    if(l != 32){//还有有效信息未输出，进行多一次输出
        while(l != 32){//确保a存储的信息和上一位之间不存在多余的0
            a <<= 1;
            empty++;
            l++;
        }
        CompressedOut << a;
        SizeofCompressedFile += 4;
    }

    CompressedFile.close();
    textfile.close();

    //输出字符频率
    QFile FrequencyFile(Filename + "_frequency.txt");
    FrequencyFile.open(QIODevice::WriteOnly);
    QTextStream FrequencyOut(&FrequencyFile);

    //首先输出压缩末尾无效二进制位数，方便解压时判断结束位置
    FrequencyOut << QString::number(empty) << '\n';
    //遍历character数组，依次输出字符与字符频率
    for(int i = 0; i < character.size(); i++){
        unsigned utfch = character[i].c;
        array.clear();
        //由unsigned转为utf-8字符
        while(utfch){
            char c = utfch;
            utfch >>= 8;
            array.insert(0, c);
        }
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);//从二进制流转为QString的编码方式，这里为utf-8
        QString utfchr = toUtf8(array);//在utfchr内存储utf-8编码字符串

        //输出字符与频率
        FrequencyOut << utfchr << " " << character[i].f << '/' << FileSize << '\n';
    }

    //清空主窗口内的数据，防止插入文件
    frequency.clear();
    character.clear();
    empty = 0;
    FrequencyFile.close();
    //结束计时
    endcompressed = clock();
    ui->statusbar->showMessage("压缩率：" + QString::number(SizeofCompressedFile/(double)Sizeoftextfile)
                                + " 压缩时间：" + QString::number(endcompressed-startload) + "ms");
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_addFrequencyFile_clicked(){//解压文件：字符频率统计结果
    //获取字符频率统计结果文件的地址
    QString FrequencyFileName = QFileDialog::getOpenFileName(this,
                                                              "请选择字符频率统计文件",
                                                              "C:",
                                                              "txt(*.txt) ;; all (*.*)");
    if(FrequencyFileName.isEmpty()){
        return ;
    }
    //防止同时选中字符频率统计结果和字符编码表，在解压时产生歧义
    ui->codeFile->clear();
    ui->frequencyFile->setText(FrequencyFileName);
}
void MainWindow::on_addCodeFile_clicked(){//解压文件：字符编码表
    //获取字符编码表文件的地址
    QString CodeFileName = QFileDialog::getOpenFileName(this,
                                                        "请选择字符编码表",
                                                        "C:",
                                                        "txt(*.txt) ;; all (*.*)");
    if(CodeFileName.isEmpty()){
        return ;
    }
    //防止同时选中字符频率统计结果和字符编码表，在解压时产生歧义
    ui->frequencyFile->clear();
    ui->codeFile->setText(CodeFileName);
}
void MainWindow::on_addCompressedFile_clicked(){//解压文件：待解压文件
    //获取待解压文件的地址
    QString CompressedFileName = QFileDialog::getOpenFileName(this,
                                                              "请选择待解压文件",
                                                              "C:",
                                                              "bin(*.bin) ;; all (*.*)");
    if(CompressedFileName.isEmpty()){
        return ;
    }
    ui->compressedFile->setText(CompressedFileName);
}
void MainWindow::on_Decompressed_clicked(){//解压文件
    //获取待解压文件的地址
    QString CompressedFileName = ui->compressedFile->text();
    if(CompressedFileName.isEmpty()){
        ui->statusbar->showMessage("未选中有效待解压文件");
        return ;
    }
    QFile CompressedFile(CompressedFileName);
    if(!CompressedFile.open(QIODevice::ReadOnly)){
        ui->statusbar->showMessage("打开待解压文件失败！");
        return ;
    }
    QDataStream in(&CompressedFile);

    //创建解压缩文件
    while(CompressedFileName.back() != '.') CompressedFileName.erase(CompressedFileName.end()-1);
    CompressedFileName.erase(CompressedFileName.end()-1);
    QFile DecompressedFile(CompressedFileName + "_decompressed.txt");
    if(!DecompressedFile.open(QIODevice::WriteOnly)){
        ui->statusbar->showMessage("新建解压缩文件失败！");
        CompressedFile.close();
        return ;
    }
    QTextStream DecompressedOut(&DecompressedFile);

    //尝试获取字符频率文件地址
    QString FrequencyFileName = ui->frequencyFile->text();
    if(FrequencyFileName.isEmpty()){//未选中字符频率统计文件，使用字符编码表解压
        int TempEmpty{0};
        QString CodeFileName = ui->codeFile->text();
        //也未选中字符编码表，解压失败
        if(CodeFileName.isEmpty()){
            ui->statusbar->showMessage("未选中参照文件！");
            DecompressedFile.close();
            CompressedFile.close();
            return ;
        }
        QFile CodeFile(CodeFileName);
        if(!CodeFile.open(QIODevice::ReadOnly)){
            ui->statusbar->showMessage("打开参照文件失败！");
            CompressedFile.close();
            DecompressedFile.close();
            return ;
        }

        QByteArray array;
        //获取压缩文件末尾无效二进制位数
        array = CodeFile.readLine();
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
        QString str = toUtf8(array);
        string temp = str.toStdString();
        while(isdigit(temp[0])){
            TempEmpty = TempEmpty * 10 + temp[0] - '0';
            temp.erase(temp.begin());
        }

        //新建树的头节点
        node *head = new node(Data(0));
        //逐行读取文件，获取字符及其编码
        do{
            array = CodeFile.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            QString str = toUtf8(array);
            string temp = str.toStdString();
            //以上操作与首次读取文件相同，故不再做说明

            //存储每行的字符
            unsigned ch{0};
            //由于'\r'与'\n'两个字符比较特殊，需要特殊处理
            if(temp[0] == '\r'){
                ch = temp[0];
                temp.erase(temp.begin());
                temp.erase(temp.begin());
            }
            else if(temp[0] == '\n'){
                ch = temp[0];
                array = CodeFile.readLine();
                auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
                QString str = toUtf8(array);
                temp = str.toStdString();
                temp.erase(temp.begin());
            }
            else{
                unsigned judge = (1 << 7);
                int size{0};// 该utf-8字符的长度
                while(temp[0] & judge){// 通过前缀1的数量判断字符长度
                    size++;
                    judge >>= 1;
                }

                if(size){
                    for(int j = 0; j < size; j++){
                        ch <<= 8;
                        ch += (unsigned char)temp[0];
                        temp.erase(temp.begin());
                    }
                    temp.erase(temp.begin());
                }
                else{
                    ch = temp[0];
                    temp.erase(temp.begin());
                    temp.erase(temp.begin());
                }
            }

            //找到合适的位置插入，如果找不到则沿路径新建
            //由于解压的时候不需要考虑非叶子节点的数据，因此可以不设置非叶子节点的数值
            node* f = head;
            for(auto e : temp){
                if(!isdigit(e)) break;
                if(e == '0'){
                    if(f->l == nullptr){
                        f->l = new node(Data(0));
                    }
                    f = f->l;
                }
                else{
                    if(f->r == nullptr){
                        f->r = new node(Data(0));
                    }
                    f = f->r;
                }
            }
            f->ch.c = ch;
        }while(!(CodeFile.atEnd()));
        CodeFile.close();

        if(head->l == nullptr && head->r == nullptr){
            ui->statusbar->showMessage("无法读取字符频率统计文件或当前流为空。\n请检查字符频率统计文件，或往流内添加文件");
            CompressedFile.close();
            DecompressedFile.close();
            return ;
        }

        //读取压缩文件
        unsigned judge = (1 << 31);
        unsigned a{0}, l{0};
        array.clear();
        //每次读入32位二进制数据进行解压
        in >> a;
        while(!in.atEnd()){
            //用于在Huffman树上进行查找的指针
            node* f = head;

            while(!f->ch.c){
                if(judge&a) f = f->r;
                else f = f->l;

                a <<= 1;
                l++;
                if(l == 32){
                    if(in.atEnd()) break;
                    l = 0;
                    in >> a;
                }
            }

            //如果找到最末的节点还查找不到该字符，则该字符不存在于编码表内，解压失败
            if(!f->ch.c){
                CompressedFile.close();
                DecompressedFile.close();
                ui->statusbar->showMessage("文件解压失败");
                return ;
            }
            else{
                //查找到该字符，将其转为utf-8字符输出
                unsigned utfch = f->ch.c;
                array.clear();
                while(utfch){
                    char c = utfch;
                    utfch >>= 8;
                    array.insert(0, c);
                }
                auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);//从二进制流转为QString的编码方式，这里为utf-8
                QString str = toUtf8(array);

                DecompressedOut << str;
            }
        }

        //最末的32位还可能存在有效数据
        if(l + TempEmpty != 32){
            if(l + TempEmpty > 32){//正常情况下不可能出现，只可能是解压失败
                CompressedFile.close();
                DecompressedFile.close();
                ui->statusbar->showMessage("文件解压失败");
                return ;
            }

            //将最后一位完全解压输出
            while(l + TempEmpty < 32){
                const node* f = head;
                while(f->l != nullptr){//跟上面一样进行查找
                    if(judge&a) f = f->r;
                    else f = f->l;

                    a <<= 1;
                    l++;
                    if(l + TempEmpty == 32) break;
                }

                if(f->l != nullptr){
                    CompressedFile.close();
                    DecompressedFile.close();
                    ui->statusbar->showMessage("文件解压失败");
                    return ;
                }
                else{
                    unsigned utfch = f->ch.c;
                    array.clear();
                    while(utfch){
                        char c = utfch;
                        utfch >>= 8;
                        array.insert(0, c);
                    }
                    auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);//从二进制流转为QString的编码方式，这里为utf-8
                    QString str = toUtf8(array);

                    DecompressedOut << str;
                }
            }
        }
    }
    else{//已选中字符频率统计文件，使用字符频率统计文件解压
        //读取字符频率文件
        vector<Data> fileCharacter;
        int TempEmpty{0};

        QFile FrequencyFile(FrequencyFileName);
        if(!FrequencyFile.open(QIODevice::ReadOnly)){
            ui->statusbar->showMessage("打开参照文件失败！");
            CompressedFile.close();
            DecompressedFile.close();
            return ;
        }
        QByteArray array;

        //获取压缩文件末尾无效二进制位数
        array = FrequencyFile.readLine();
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
        QString str = toUtf8(array);
        string temp = str.toStdString();
        while(isdigit(temp[0])){
            TempEmpty = TempEmpty * 10 + temp[0] - '0';
            temp.erase(temp.begin());
        }

        //逐行读取文件，获取字符及频率信息
        do{
            array = FrequencyFile.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            str = toUtf8(array);
            string temp = str.toStdString();

            //存储每行的字符
            unsigned ch{0};
            //由于'\r'与'\n'两个字符比较特殊，需要特殊处理
            if(temp[0] == '\r'){
                ch = temp[0];
                temp.erase(temp.begin());
                temp.erase(temp.begin());
            }
            else if(temp[0] == '\n'){
                ch = temp[0];
                array = FrequencyFile.readLine();
                auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
                QString str = toUtf8(array);
                temp = str.toStdString();
                temp.erase(temp.begin());
            }
            else{
                unsigned judge = (1 << 7);
                int size{0};// 该utf-8字符的长度
                while(temp[0] & judge){// 通过前缀1的数量判断字符长度
                    size++;
                    judge >>= 1;
                }

                if(size){
                    for(int j = 0; j < size; j++){
                        ch <<= 8;
                        ch += (unsigned char)temp[0];
                        temp.erase(temp.begin());
                    }
                    temp.erase(temp.begin());
                }
                else{
                    ch = temp[0];
                    temp.erase(temp.begin());
                    temp.erase(temp.begin());
                }
            }

            unsigned long long f{0};
            while(isdigit(temp[0])){
                f = f * 10 + temp[0] - '0';
                temp.erase(temp.begin());
            }
            fileCharacter.push_back(Data(ch, f));
        }while(!(FrequencyFile.atEnd()));
        FrequencyFile.close();

        if(fileCharacter.empty()){
            ui->statusbar->showMessage("无法读取字符频率统计文件或当前流为空。\n请检查字符频率统计文件，或往流内添加文件");
            CompressedFile.close();
            DecompressedFile.close();
            return ;
        }

        //建立静态Huffman树，便于解压
        HuffmanTree tree(fileCharacter);

        unsigned judge = (1 << 31);
        unsigned a{0}, l{0};
        array.clear();
        //下面的操作与上一个if条件内的输出一样，不写在if条件外是因为这个条件下的Huffman树只能在else块内建立，无法跳出该块进行
        in >> a;
        while(!in.atEnd()){
            const node* f = tree.head;

            while(f->l != nullptr){
                if(judge&a) f = f->r;
                else f = f->l;

                a <<= 1;
                l++;
                if(l == 32){
                    if(in.atEnd()) break;
                    l = 0;
                    in >> a;
                }
            }

            if(f->l != nullptr){
                CompressedFile.close();
                DecompressedFile.close();
                ui->statusbar->showMessage("文件解压失败");
                return ;
            }
            else{
                unsigned utfch = f->ch.c;
                array.clear();
                while(utfch){
                    char c = utfch;
                    utfch >>= 8;
                    array.insert(0, c);
                }
                auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
                QString str = toUtf8(array);

                DecompressedOut << str;
            }
        }

        if(l + TempEmpty != 32){
            if(l + TempEmpty > 32){
                CompressedFile.close();
                DecompressedFile.close();
                ui->statusbar->showMessage("文件解压失败");
                return ;
            }

            while(l + TempEmpty < 32){
                const node* f = tree.head;
                while(f->l != nullptr){
                    if(judge&a) f = f->r;
                    else f = f->l;

                    a <<= 1;
                    l++;
                    if(l + TempEmpty == 32) break;
                }

                if(f->l != nullptr){
                    CompressedFile.close();
                    DecompressedFile.close();
                    ui->statusbar->showMessage("文件解压失败");
                    return ;
                }
                else{
                    unsigned utfch = f->ch.c;
                    array.clear();
                    while(utfch){
                        char c = utfch;
                        utfch >>= 8;
                        array.insert(0, c);
                    }
                    auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
                    QString str = toUtf8(array);

                    DecompressedOut << str;
                }
            }
        }
    }

    DecompressedFile.close();
    ui->statusbar->showMessage("文件解压成功");
    CompressedFile.close();
}

void MainWindow::on_textFileListClear_clicked(){//适应性huffman编码：清空文件流
    character.clear();
    frequency.clear();
    adaptiveTree.clear();
    empty = 0;
    ui->textFileList->setText("当前流内无文件");
    ui->statusbar->showMessage("已清空文件流");
}
void MainWindow::on_reConstruct_toggled(){//适应性huffman编码：切换实现方式
    //清空文件流，防止干扰
    character.clear();
    frequency.clear();
    adaptiveTree.clear();
    empty = 0;

    ui->statusbar->showMessage("已清空文件流");
    ui->textFileList->setText("当前流内无文件");
}
void MainWindow::on_textFileListAppend_clicked(){//适应性huffman编码：添加文件
    unsigned FileSize{0};
    QString Filename = QFileDialog::getOpenFileName(this,
                                                    "请选择需要添加的文件",
                                                    "C:",
                                                    "txt(*.txt) ;; all (*.*)");
    if(Filename.isEmpty()){
        return ;
    }

    //存储每个字符对应的Huffman编码
    map<unsigned, string> HuffmanCode;
    //存储压缩文件末尾无效位数
    unsigned FileTemp{0};
    //存储文件字符频率
    Mymap FileFrequency;
    //存储文件字符
    vector<unsigned> FileCharacter;

    clock_t startload, enddecompressed;
    startload = clock();

    QFile textfile(Filename);
    if(!textfile.open(QIODevice::ReadOnly)){
        ui->statusbar->showMessage("打开文件失败");
        return ;
    }
    if(character.empty() && adaptiveTree.empty()){
        ui->textFileList->clear();
        ui->textFileList->append("当前流内文件：\n" + Filename);
    }
    else{
        ui->textFileList->append(Filename);
    }
    QByteArray array;

    if(ui->adaptiveConstruct->isChecked()){//动态Huffman树
        //读取文件并统计字符的操作在主窗口的构造函数内已经说明，这里不再赘述
        do{
            array = textfile.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            QString str = toUtf8(array);
            string temp = str.toStdString();

            for(int i = 0; i < temp.size(); i++){
                unsigned judge = (1 << 7);
                int size{0};
                while(temp[i] & judge){
                    size++;
                    judge >>= 1;
                }

                unsigned a{0};
                if(size){
                    for(int j = 0; j < size; j++){
                        a <<= 8;
                        a += (unsigned char)temp[i++];
                    }
                    i--;
                    FileFrequency[a]++;
                    FileSize += size;
                }
                else{
                    a = temp[i];
                    FileFrequency[a]++;
                    FileSize++;
                }

                adaptiveTree.insert(a);//往动态Huffman树内插入字符
                if(FileFrequency[a] == 1){
                    FileCharacter.push_back(a);
                }
            }
        }while(!(textfile.atEnd()));
        textfile.close();

        adaptiveTree.CreateCode(HuffmanCode);
    }
    else{
        //读取文件并统计字符的操作在主窗口的构造函数内已经说明，这里不再赘述
        do{
            array = textfile.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            QString str = toUtf8(array);
            string temp = str.toStdString();

            for(int i = 0; i < temp.size(); i++){
                unsigned judge = (1 << 7);
                int size{0};
                while(temp[i] & judge){
                    size++;
                    judge >>= 1;
                }

                unsigned a{0};
                if(size){
                    for(int j = 0; j < size; j++){
                        a <<= 8;
                        a += (unsigned char)temp[i++];
                    }
                    i--;
                    frequency[a]++;
                    FileFrequency[a]++;
                    FileSize += size;
                }
                else{
                    a = temp[i];
                    frequency[a]++;
                    FileFrequency[a]++;
                    FileSize++;
                }

                if(frequency[a] == 1) {
                    character.push_back(a);
                }
                if(FileFrequency[a] == 1){
                    FileCharacter.push_back(a);
                }
            }
        }while(!(textfile.atEnd()));
        textfile.close();

        for(int i = 0; i < character.size(); i++){
            character[i].f = frequency[character[i].c];
        }

        //静态构造Huffman树
        HuffmanTree tree(character);
        tree.CreateCode(HuffmanCode);
    }

    //压缩文件输出
    while(Filename.back() != '.') Filename.erase(Filename.end()-1);
    Filename.erase(Filename.end()-1);
    QFile CompressedFile(Filename + "_compressed.bin");
    CompressedFile.open(QIODevice::WriteOnly);
    QDataStream CompressedOut(&CompressedFile);
    textfile.open(QIODevice::ReadOnly);

    long long SizeofCompressedFile{0};
    unsigned a{0}, l{0};
    //再次读取文件，获知原文件字符顺序
    do{
        array = textfile.readLine();
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
        QString str = toUtf8(array);
        string temp = str.toStdString();
        //以上操作与首次读取文件相同，故不再做说明

        for(int i = 0; i < temp.size(); i++){
            unsigned judge = (1 << 7);
            int size{0};//该utf-8字符的长度
            while(temp[i] & judge){//通过前缀1的数量判断字符长度
                size++;
                judge >>= 1;
            }

            unsigned ch{0};
            if(size){
                for(int j = 0; j < size; j++){
                    ch <<= 8;
                    ch += (unsigned char)temp[i++];
                }
                i--;
            }
            else{
                ch = temp[i];
            }

            string code = HuffmanCode[ch];
            for(auto e : code){
                a <<= 1;
                if(e == '1') a++;
                l++;

                if(l == 32){
                    CompressedOut << a;
                    SizeofCompressedFile += 4;
                    a = l = 0;
                }
            }
        }
    }while(!(textfile.atEnd()));

    if(l != 32){
        while(l != 32){
            a <<= 1;
            FileTemp++;
            l++;
        }
        CompressedOut << a;
        SizeofCompressedFile += 4;
    }

    empty = (empty + FileTemp)%32;
    CompressedFile.close();
    textfile.close();

    //编码表输出
    QFile CodeFile(Filename + "_code.txt");
    CodeFile.open(QIODevice::WriteOnly);
    QTextStream CodeOut(&CodeFile);
    CodeOut << QString::number(FileTemp) << '\n';

    for(int i = 0; i < FileCharacter.size(); i++){
        unsigned utfch = FileCharacter[i];
        string code = HuffmanCode[utfch];
        array.clear();
        while(utfch){
            char c = utfch;
            utfch >>= 8;
            array.insert(0, c);
        }
        auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
        QString str = toUtf8(array);

        CodeOut << str << " " << QString::fromStdString(code) << '\n';
    }

    CodeFile.close();
    //结束计时
    enddecompressed = clock();
    ui->statusbar->showMessage("压缩率：" + QString::number(SizeofCompressedFile/(double)FileSize)
                                                                + " 压缩时间：" + QString::number(enddecompressed-startload) + "ms");
}

void MainWindow::on_addCmpFile_1_clicked(){//文本检测：对比文件1
    QString File1 = QFileDialog::getOpenFileName(this,
                                                 "请选择对比文件",
                                                 "C:",
                                                 "txt(*.txt) ;; all (*.*)");
    if(File1.isEmpty()){//未选中对比文件1
        return ;
    }
    ui->cmpFile_1->setText(File1);
    QString File2 = ui->cmpFile_2->text();
    if(!File2.isEmpty()){//已选中对比文件2，可以开始对比
        QFile cmpFile1(File1);
        if(!cmpFile1.open(QIODevice::ReadOnly)){//打开对比文件1，若打开失败则结束
            ui->statusbar->showMessage("对比文件1不存在！");
        }
        QFile cmpFile2(File2);
        if(!cmpFile2.open(QIODevice::ReadOnly)){//打开对比文件2，若打开失败则结束
            ui->statusbar->showMessage("对比文件2不存在！");
        }

        QByteArray array1, array2;
        do{
            array1 = cmpFile1.readLine();
            array2 = cmpFile2.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            QString str1 = toUtf8(array1);
            QString str2 = toUtf8(array2);
            string cmp1 = str1.toStdString();
            string cmp2 = str2.toStdString();

            if(MyHash(cmp1) != MyHash(cmp2)){//计算文本哈希值，若不同则结束函数，相同则继续检测
                cmpFile1.close();
                cmpFile2.close();
                ui->fileCmp->setText("不相同");
                return ;
            }
        }while(!cmpFile1.atEnd() && !cmpFile2.atEnd());

        //已检测文本相同但文本长度不同时则肯定不相同
        if(!cmpFile1.atEnd() || !cmpFile2.atEnd()) ui->fileCmp->setText("不相同");
        else ui->fileCmp->setText("相同");
        cmpFile1.close();
        cmpFile2.close();
    }
}
void MainWindow::on_addCmpFile_2_clicked(){//文本检测:对比文件2
    QString File2 = QFileDialog::getOpenFileName(this,
                                                 "请选择对比文件",
                                                 "C:",
                                                 "txt(*.txt) ;; all (*.*)");
    if(File2.isEmpty()){//未选中对比文件2
        return ;
    }
//以下操作与文本检测：对比文件1相同，不再赘述
    ui->cmpFile_2->setText(File2);
    QString File1 = ui->cmpFile_1->text();
    if(!File1.isEmpty()){
        QFile cmpFile1(File1);
        if(!cmpFile1.open(QIODevice::ReadOnly)){
            ui->statusbar->showMessage("对比文件1不存在！");
            return ;
        }
        QFile cmpFile2(File2);
        if(!cmpFile2.open(QIODevice::ReadOnly)){
            ui->statusbar->showMessage("对比文件2不存在！");
            return ;
        }

        QByteArray array1, array2;
        do{
            array1 = cmpFile1.readLine();
            array2 = cmpFile2.readLine();
            auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
            QString str1 = toUtf8(array1);
            QString str2 = toUtf8(array2);
            string cmp1 = str1.toStdString();
            string cmp2 = str2.toStdString();

            if(MyHash(cmp1) != MyHash(cmp2)){
                ui->fileCmp->setText("不相同");
                cmpFile1.close();
                cmpFile2.close();
                return ;
            }
        }while(!cmpFile1.atEnd() && !cmpFile2.atEnd());

        if(!cmpFile1.atEnd() || !cmpFile2.atEnd()) ui->fileCmp->setText("不相同");
        else ui->fileCmp->setText("相同");
        cmpFile1.close();
        cmpFile2.close();
    }
}
