#include "Mymap.h"

Mymap::Mymap(){
    N = 100;
    //散列表初始化
    m.resize(N, vector<Pair>());
}

unsigned long long &Mymap::operator[](unsigned int a) {
    unsigned i = a % 100;//散列表查找位置
    //顺序查找对应的表
    for (int j = 0; j < m[i].size(); j++) {
        if (m[i][j].key == a) {
            //查找到键值为a的节点，返回节点val值的引用
            unsigned long long& re = m[i][j].val;
            return re;
        }
    }

    //查找不到键值为a的节点，新建一个
    m[i].push_back(Pair(a, 0));
    unsigned long long& re = (m[i].end() - 1)->val;
    return re;
}

void Mymap::clear(){
    m.clear();
    N = 100;
    m.resize(N, vector<Pair>());
}

//BKDHash
unsigned MyHash(string str){
    unsigned seed = 131;
    unsigned hash = 0;

    for (auto c : str) {
        hash = hash * seed + c;
    }

    return (hash & 0x7fffffff); // %(2^32)
}
