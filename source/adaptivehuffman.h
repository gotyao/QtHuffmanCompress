#ifndef ADAPTIVEHUFFMAN_H
#define ADAPTIVEHUFFMAN_H
#include "Huffman.h"
using namespace std;

//动态Huffman树的节点
struct HuffNode {
    Data ch;//存储节点信息
    HuffNode* l;
    HuffNode* r;
    HuffNode* p;//增加p指针指向节点的父节点，方便更新节点信息

    HuffNode(Data a) {
        ch = a;
        l = r = p = nullptr;
    }
};

class AdaptiveHuffman {
    vector<HuffNode*> ch;//将节点从左到右、从下到上排序
    HuffNode* head;
public:
    AdaptiveHuffman();
    ~AdaptiveHuffman();
    //析构函数辅助函数
    void del(HuffNode* p);
    //判断树空
    bool empty();
    //如果c在树内存在则增加c的频率，如果不存在则在树内新建存放c的节点
    void insert(unsigned c);
    //插入后更新每个节点的信息，并根据需要交换节点
    void updateNode(HuffNode* p, int w);
    //将每个字符的编码存储到code内
    void CreateCode(map<unsigned, string>& code);
    //CreateCode()函数的辅助函数
    void helpCreate(HuffNode* p, map<unsigned, string>& code, string str);
    //清空树，只剩空的根节点
    void clear();
};

#endif // ADAPTIVEHUFFMAN_H
