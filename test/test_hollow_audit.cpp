#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include <random>
#include "../HollowHeap.cpp"

void test_stress() {
    std::cout << "[Audit Test] Running test_stress..." << std::endl;
    HollowHeap<int, int> h;
    const int N = 100;
    std::vector<int> vals(N);
    std::vector<HollowHeap<int, int>::Node*> nodes(N, nullptr);

    for (int i = 0; i < N; ++i) {
        vals[i] = i;
        nodes[i] = h.insert(i, &vals[i]);
    }

    for (int i = 0; i < N; ++i) {
        assert(!h.empty());
        int m = *h.min();
        assert(m == i);
        h.delete_min();
    }
    assert(h.empty());
    std::cout << "[Audit Test] Sequential insert and delete_min passed!" << std::endl;
}

void test_stress_random() {
    std::cout << "[Audit Test] Running test_stress_random..." << std::endl;
    std::mt19937 rng(42);
    HollowHeap<int, int> h;
    const int N = 200;
    std::vector<int> vals(N);
    std::vector<HollowHeap<int, int>::Node*> nodes(N, nullptr);

    std::vector<int> keys(N);
    for (int i = 0; i < N; ++i) {
        keys[i] = rng() % 1000;
        vals[i] = i;
        nodes[i] = h.insert(keys[i], &vals[i]);
    }

    int prev = -1;
    for (int i = 0; i < N; ++i) {
        assert(!h.empty());
        int v = *h.min();
        int k = keys[v];
        assert(prev == -1 || k >= prev);
        prev = k;
        h.delete_min();
    }
    assert(h.empty());
    std::cout << "[Audit Test] Random keys delete_min passed!" << std::endl;
}

void test_stress_decrease() {
    std::cout << "[Audit Test] Running test_stress_decrease..." << std::endl;
    HollowHeap<int, int> h;
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    h.insert(100, &v1);
    h.insert(200, &v2);
    auto n3 = h.insert(300, &v3);
    auto n4 = h.insert(400, &v4);

    h.decrease_key(n4, 350); // 400 - 350 = 50 -> new min
    assert(*h.min() == 4);
    h.delete_min(); // extracts 4

    h.decrease_key(n3, 250); // 300 - 250 = 50 -> new min
    assert(*h.min() == 3);
    h.delete_min(); // extracts 3

    assert(*h.min() == 1);
    h.delete_min();

    assert(*h.min() == 2);
    h.delete_min();
    assert(h.empty());
    std::cout << "[Audit Test] Single decrease_key passed!" << std::endl;
}

void audit_double_decrease_issue() {
    std::cout << "[Audit Warning] Checking double decrease on same node handle..." << std::endl;
    HollowHeap<int, int> h;
    int v = 100;
    auto n = h.insert(100, &v);
    h.decrease_key(n, 20); // becomes 80. Old node n is now hollow (n->item = nullptr)!
    // If client decreases key on n again (standard in Dijkstra when a vertex distance relaxes twice):
    h.decrease_key(n, 30); // Uses n, creating a new Node with n->item (which is now null)!
    // The second decrease is lost because it's treated as a hollow node.
    std::cout << "[Audit Warning] Documented: Handle becomes invalid after first decrease_key." << std::endl;
}

int main() {
    test_stress();
    test_stress_random();
    test_stress_decrease();
    audit_double_decrease_issue();
    std::cout << "\nAll Hollow Heap audit tests executed.\n" << std::endl;
    return 0;
}
