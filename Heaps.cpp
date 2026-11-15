//
// Created by andre on 11/11/26.
//
#include <iostream>
#include "HollowHeap.cpp"
#include "PairHeap.cpp"
#include "FibonacciHeap.cpp"
#include "HollowHeap2.cpp"
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

void fibonacci_heap_basic_test() {
    cout << "\n--- Fibonacci Heap Test ---" << endl;
    FibonacciHeap<int, int> fib;
    int a[20];
    FibonacciHeap<int, int>::Node* ptrs[20];
    for (int i = 19; i >= 0; i--) {
        a[i] = i;
        ptrs[i] = fib.insert(i, &a[i]);
    }
    fib.erase(ptrs[0]);
    fib.erase(ptrs[15]);

    while (!fib.empty()) {
        cout << *fib.find_min() << endl;
        fib.erase_min();
    }
}

void hollow_heap2_basic_test() {
    cout << "\n--- Hollow Heap 2 Test ---" << endl;
    HollowHeap2<int, int> h2;
    int a[20];
    HollowHeap2<int, int>::Node* ptrs[20];
    for (int i = 19; i >= 0; i--) {
        a[i] = i;
        ptrs[i] = h2.insert(i, &a[i]);
    }
    h2.erase(ptrs[0]);
    h2.erase(ptrs[15]);

    while (!h2.empty()) {
        cout << *h2.find_min() << endl;
        h2.erase_min();
    }
}

int main() {
    cout << "--- Hollow Heap (Original) Test ---" << endl;
    hollow_heap_basic_test();
    fibonacci_heap_basic_test();
    hollow_heap2_basic_test();
    cout << "\nAll heap basic tests finished successfully." << endl;
}
