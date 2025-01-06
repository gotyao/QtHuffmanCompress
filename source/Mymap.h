#ifndef MYMAP_H
#define MYMAP_H
#include <bits/stdc++.h>
using namespace std;

//链表内存储的信息
struct Pair {
    unsigned key;//字符的utf-8编码
    unsigned long long val;//字符的频率

    Pair(unsigned a = 0, unsigned long long b = 0) {
        key = a;
        val = b;
    }
};

//散列链表
class Mymap {
    vector<vector<Pair>> m;
    int N;
public:
    Mymap();
    ~Mymap() { }
    //通过key访问val
    unsigned long long& operator[](unsigned a);
    void clear();
};

unsigned MyHash(string str);
#endif // MYMAP_H
