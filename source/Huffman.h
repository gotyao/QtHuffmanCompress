#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <bits/stdc++.h>
using namespace std;

struct Data{//存储字符及其频率
    unsigned c;//字符
    unsigned long long f;//频率

    Data()
        :c{0}, f{0}
    { }
    Data(unsigned a, unsigned long long b = 0)
        :c{a}, f{b}
    { }
};

//静态Huffman树的节点
struct node {
    Data ch;
    node* l, * r;
    node(Data a)
        :ch{ a }, l{ nullptr }, r{ nullptr }
    { }
};

//自定义优先队列，对节点按照频率进行升序排序
//为了使代码易读，不使用第0个数组成员。因此根节点下标为1，第i个节点的子节点下标分别为2*i、2*i+1，父节点的下标为i/2。
class PriorityQueue {
    vector<node*> a;//存储每个节点
public:
    PriorityQueue();
    ~PriorityQueue();
    //沿用堆的操作
    void sink(int k);
    void swim(int k);
    //入队和出队操作
    void enqueue(node* p);
    node* dequeue();
    //返回有效节点的个数
    size_t size();
};

//静态Huffman树
class HuffmanTree{
public:
    node* head;
    //通过存放了字符及其频率的数组建立静态Huffman树
    HuffmanTree(const vector<Data>& a);
    ~HuffmanTree();
    //析构函数辅助函数
    void del(node* p);
    //将每个字符的编码存储到code内
    void CreateCode(map<unsigned, string>& code);
    //CreateCode()函数的辅助函数
    void helpCreate(node* p, map<unsigned, string>& code, string str);
};

#endif // HUFFMAN_H
