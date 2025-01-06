#include "adaptivehuffman.h"

AdaptiveHuffman::AdaptiveHuffman(){
    HuffNode* p = new HuffNode(0);//创建新的空节点，为插入做准备
    head = p;
    ch.push_back(p);
}

AdaptiveHuffman::~AdaptiveHuffman(){
    del(head);
    ch.clear();
}
void AdaptiveHuffman::del(HuffNode* p){
    if (p == nullptr) return;
    //析构时先delete子节点再delete该节点
    del(p->l);
    del(p->r);
    delete p;
}

bool AdaptiveHuffman::empty(){//由于适应性Huffman树头指针一直不为nullptr，需要判断头节点是否还有子节点来判断树是否为空
    return (head->l == nullptr);
}

void AdaptiveHuffman::insert(unsigned c){
    //遍历每个节点，若找到字符值等于c的节点则增加该节点频率值，否则将c当作新字符插入
    for (int i = 0; i < ch.size(); i++) {
        if (ch[i]->ch.c == c) {
            ch[i]->ch.f++;
            updateNode(ch[i], i);
            return;
        }
    }

    //作为新节点插入
    HuffNode* p = new HuffNode(c);
    p->p = ch[0];
    ch[0]->r = p;
    //c作为新字符插入时，作为原有空节点的子节点插入，因此还需要创建一个新的空节点
    ch[0]->l = new HuffNode(0);
    ch[0]->l->p = ch[0];
    //将新建的两个节点插入存储节点的数组内
    ch.insert(ch.begin(), p);
    ch.insert(ch.begin(), ch[1]->l);
    p->ch.f = 1;
    updateNode(p, 1);
}
void AdaptiveHuffman::updateNode(HuffNode* p, int w){
    HuffNode* f = ch[w];//f指向新插入的节点

    while (f->p != nullptr) {
        int s{ 0 };//记录在f的父节点左侧及在f右侧的节点中比f的频率更小的
        for (int i = w; i < ch.size(); i++) {
            if (ch[i]->ch.f < f->ch.f && f->p != ch[i]) s = i;
            if (ch[i]->ch.f > f->ch.f) break;
        }
        if (s) {
            HuffNode* temp = ch[s]->p;
            ch[s]->p = ch[w]->p;
            ch[w]->p = temp;
            if (ch[s]->p->l == ch[w]) ch[s]->p->l = ch[s];
            else ch[s]->p->r = ch[s];
            if (ch[w]->p->l == ch[s]) ch[w]->p->l = ch[w];
            else ch[w]->p->r = ch[w];
            swap(ch[s], ch[w]);
        }
        f = f->p;
        f->ch.f++;
        while (ch[w] != f) w++;
    }
}

//与静态Huffman树实现方式相同，故不再赘述
void AdaptiveHuffman::CreateCode(map<unsigned int, string> &code){
    helpCreate(head, code, "");
}
void AdaptiveHuffman::helpCreate(HuffNode *p, map<unsigned int, string> &code, string str){
    if(p == nullptr) return ;
    if(p->ch.c) code[p->ch.c] = str;
    else{
        helpCreate(p->l, code, str + "0");
        helpCreate(p->r, code, str + "1");
    }
}

//清空树时只清空头节点的子节点，头节点保留用于之后插入时使用
void AdaptiveHuffman::clear(){
    ch.clear();
    head->ch.f = 0;
    del(head->l);
    head->l = nullptr;
    del(head->r);
    head->r = nullptr;
    ch.push_back(head);
}
