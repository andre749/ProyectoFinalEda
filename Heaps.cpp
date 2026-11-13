//
// Created by andre on 11/11/26.
//
#include<iostream>
#include "HollowHeap.cpp"
#include "PairHeap.cpp"
using namespace std;
void hollow_heap_basic_test() {
    HollowHeap<int,int> h;
    int a[20];
    HollowHeap<int,int>::Node* ptrs[20];
    for (int i=19;i>=0;i--) {
        a[i]=i;
        ptrs[i]=h.insert(i,&a[i]);
    }
    h.erase(ptrs[0]);
    h.erase(ptrs[15]);

    // h.erase(ptrs[15]);
    // h.erase(ptrs[15]);

    while (!h.empty()){
        cout<<*h.min()<<endl;
        h.delete_min();
    }
}

int main() {
    hollow_heap_basic_test();
    cout<<"hello world";
}