#include "Huffman.h"

PriorityQueue::PriorityQueue() {
    //在头部加入空节点
    a.push_back(nullptr);
}

PriorityQueue::~PriorityQueue() {
    a.clear();
}

void PriorityQueue::sink(int k) {
    while (2 * k + 1 <= a.size()) {
        int j = 2 * k;
        if (j + 1 < a.size() && a[j]->ch.f > a[j + 1]->ch.f) j++;
        if (a[k]->ch.f <= a[j]->ch.f) break;
        swap(a[k], a[j]);
        k = j;
    }
}

void PriorityQueue::swim(int k) {
    while (k > 1 && a[k / 2]->ch.f > a[k]->ch.f) {
        swap(a[k / 2], a[k]);
        k = k / 2;
    }
}

void PriorityQueue::enqueue(node* p) {
    a.push_back(p);
    //将新插入节点上浮，恢复堆
    swim(a.size() - 1);
}

node* PriorityQueue::dequeue() {
    if (a.size() == 1) return nullptr;
    node* re = a[1];
    //先交换以便在单次下沉操作内恢复堆
    swap(a[1], a[a.size() - 1]);
    a.pop_back();
    //通过下沉恢复堆
    sink(1);
    return re;
}

size_t PriorityQueue::size() {
    //由于首个节点没有实际意义，结果需要-1
    return a.size() - 1;
}

HuffmanTree::HuffmanTree(const vector<Data>& a){
    head = nullptr;
    PriorityQueue pq;
    //先将每个字符入队
    for(auto e:a) {
        node* p = new node(e);
        pq.enqueue(p);
    }
    while (pq.size() > 1) {
        //逐步构造树
        node* sum = new node(Data());
        sum->l = pq.dequeue();
        sum->r = pq.dequeue();
        sum->ch.f = sum->l->ch.f + sum->r->ch.f;
        head = sum;
        pq.enqueue(sum);
    }
    head = pq.dequeue();
}

HuffmanTree::~HuffmanTree(){
    del(head);
}
void HuffmanTree::del(node* p){
    if (p->l != nullptr) del(p->l);
    if (p->r != nullptr) del(p->r);
    delete p;
}

void HuffmanTree::CreateCode(map<unsigned int, string>& code){
    helpCreate(head, code, "");
}
void HuffmanTree::helpCreate(node *p, map<unsigned int, string>& code, string str){
    if(p == nullptr) return ;
    if(p->ch.c){
        code[p->ch.c] = str;
    }
    else{
        helpCreate(p->l, code, str + "0");
        helpCreate(p->r, code, str + "1");
    }
}