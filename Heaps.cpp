//
// Created by andre on 11/11/26.
//
#include <iostream>
#include "FibonacciHeap.cpp"
#include "HollowHeap.cpp"
#include "PairHeap.cpp"
#include "RankPairingHeap.cpp"

using namespace std;

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

void hollow_heap_basic_test() {
    cout << "\n--- Hollow Heap Test ---" << endl;
    HollowHeap<int, int> h;
    int a[20];
    HollowHeap<int, int>::Node* ptrs[20];
    for (int i = 19; i >= 0; i--) {
        a[i] = i;
        ptrs[i] = h.insert(i, &a[i]);
    }
    h.erase(ptrs[0]);
    h.erase(ptrs[15]);

    while (!h.empty()) {
        cout << *h.find_min() << endl;
        h.erase_min();
    }
}

void pair_heap_basic_test() {
    cout << "\n--- Pairing Heap Test ---" << endl;
    PairHeap<int, int> ph;
    int a[20];
    PairHeap<int, int>::Node* ptrs[20];
    for (int i = 19; i >= 0; i--) {
        a[i] = i;
        ptrs[i] = ph.insert(i, &a[i]);
    }
    ph.erase(ptrs[0]);
    ph.erase(ptrs[15]);

    while (!ph.empty()) {
        cout << *ph.find_min() << endl;
        ph.erase_min();
    }
}

void rank_pairing_heap_basic_test() {
    cout << "\n--- Rank-Pairing Heap Test ---" << endl;
    RankPairingHeap<int, int> rp;
    int a[20];
    RankPairingHeap<int, int>::Node* ptrs[20];
    for (int i = 19; i >= 0; i--) {
        a[i] = i;
        ptrs[i] = rp.insert(i, &a[i]);
    }
    rp.erase(ptrs[0]);
    rp.erase(ptrs[15]);

    while (!rp.empty()) {
        cout << *rp.find_min() << endl;
        rp.erase_min();
    }
}

int main() {
    fibonacci_heap_basic_test();
    hollow_heap_basic_test();
    pair_heap_basic_test();
    rank_pairing_heap_basic_test();
    cout << "\nAll heap basic tests finished successfully." << endl;
    return 0;
}
