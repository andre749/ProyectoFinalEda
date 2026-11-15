#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <cassert>
#include <algorithm>
#include "../FibonacciHeap.cpp"

void test_basic() {
    std::cout << "[Test Fibo] Basic operations..." << std::endl;
    FibonacciHeap<int, int> h;
    assert(h.empty());
    assert(h.size() == 0);
    assert(h.find_min() == nullptr);
    assert(h.validate_invariants());

    int v1 = 10, v2 = 20, v3 = 5;
    h.insert(10, &v1);
    assert(!h.empty());
    assert(h.size() == 1);
    assert(*h.find_min() == 10);
    assert(h.validate_invariants());

    h.insert(20, &v2);
    h.insert(5, &v3);
    assert(h.size() == 3);
    assert(*h.find_min() == 5);
    assert(h.validate_invariants());

    // Extract min
    int* min_val = h.erase_min();
    assert(min_val != nullptr && *min_val == 5);
    assert(h.size() == 2);
    assert(*h.find_min() == 10);
    assert(h.validate_invariants());

    min_val = h.erase_min();
    assert(min_val != nullptr && *min_val == 10);
    assert(h.size() == 1);
    assert(*h.find_min() == 20);
    assert(h.validate_invariants());

    min_val = h.erase_min();
    assert(min_val != nullptr && *min_val == 20);
    assert(h.empty());
    assert(h.size() == 0);
    assert(h.find_min() == nullptr);
    assert(h.validate_invariants());

    std::cout << "[Test Fibo] Basic operations PASSED!" << std::endl;
}

void test_meld() {
    std::cout << "[Test Fibo] Meld operations..." << std::endl;
    FibonacciHeap<int, int> h1, h2;
    int a = 15, b = 25, c = 5, d = 30;
    h1.insert(15, &a);
    h1.insert(25, &b);
    assert(h1.validate_invariants());

    h2.insert(5, &c);
    h2.insert(30, &d);
    assert(h2.validate_invariants());

    h1.meld(h2);
    assert(h1.size() == 4);
    assert(h2.empty());
    assert(*h1.find_min() == 5);
    assert(h1.validate_invariants());

    // Extract all
    std::vector<int> extracted;
    while (!h1.empty()) {
        extracted.push_back(*h1.erase_min());
        assert(h1.validate_invariants());
    }
    std::vector<int> expected = {5, 15, 25, 30};
    assert(extracted == expected);

    std::cout << "[Test Fibo] Meld operations PASSED!" << std::endl;
}

void test_cascading_cut() {
    std::cout << "[Test Fibo] Cascading cut verification..." << std::endl;
    FibonacciHeap<int, int> h;
    const int N = 16;
    std::vector<int> vals(N);
    std::vector<FibonacciHeap<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i;
        nodes[i] = h.insert(i, &vals[i]);
    }
    assert(h.validate_invariants());

    // Consolidate trees into higher degrees by extracting 0
    int* m0 = h.erase_min();
    assert(*m0 == 0);
    assert(h.validate_invariants());

    // Now trees have formed with degrees > 0.
    // Decrease keys and verify cascading cuts and invariant preservation
    for (int i = N - 1; i >= 1; --i) {
        h.decrease_key(nodes[i], -i);
        assert(h.validate_invariants());
    }

    // Now extract all in order
    int prev = -999999;
    while (!h.empty()) {
        int k = h.min_key();
        assert(k >= prev);
        prev = k;
        h.erase_min();
        assert(h.validate_invariants());
    }

    std::cout << "[Test Fibo] Cascading cut PASSED!" << std::endl;
}

void test_erase_arbitrary() {
    std::cout << "[Test Fibo] Erase arbitrary nodes..." << std::endl;
    FibonacciHeap<int, int> h;
    const int N = 20;
    std::vector<int> vals(N);
    std::vector<FibonacciHeap<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i * 10;
        nodes[i] = h.insert(vals[i], &vals[i]);
    }

    // Consolidate by extracting min
    h.erase_min(); // removes 0
    assert(h.validate_invariants());

    // Erase a node in the middle
    h.erase(nodes[10]);
    assert(h.validate_invariants());

    // Erase another
    h.erase(nodes[5]);
    assert(h.validate_invariants());

    // Erase min via erase
    h.erase(nodes[1]);
    assert(h.validate_invariants());

    while (!h.empty()) {
        h.erase_min();
        assert(h.validate_invariants());
    }

    std::cout << "[Test Fibo] Erase arbitrary nodes PASSED!" << std::endl;
}

void test_random_stress_unique_keys() {
    std::cout << "[Test Fibo] Random stress test with unique keys against std::priority_queue..." << std::endl;
    using KeyType = std::pair<int, int>;
    FibonacciHeap<KeyType, int> fib_heap;
    std::priority_queue<KeyType, std::vector<KeyType>, std::greater<KeyType>> pq;

    std::mt19937 rng(1337);
    const int NUM_OPS = 10000;
    std::vector<int> active_ids;
    std::vector<int> current_priority(NUM_OPS + 10, 0);
    std::vector<FibonacciHeap<KeyType, int>::Node*> nodes(NUM_OPS + 10, nullptr);
    std::vector<int> values(NUM_OPS + 10, 0);
    int next_id = 1;

    for (int step = 0; step < NUM_OPS; ++step) {
        int op = rng() % 100;
        if (op < 50 || fib_heap.empty()) {
            // Insert
            int id = next_id++;
            int prio = (rng() % 100000) + 1000;
            current_priority[id] = prio;
            values[id] = id;
            KeyType k = {prio, id};
            nodes[id] = fib_heap.insert(k, &values[id]);
            pq.push(k);
            active_ids.push_back(id);
        } else if (op < 75) {
            // Decrease key
            if (!active_ids.empty()) {
                size_t idx = rng() % active_ids.size();
                int id = active_ids[idx];
                if (current_priority[id] > 5) {
                    int new_prio = rng() % current_priority[id];
                    current_priority[id] = new_prio;
                    KeyType new_k = {new_prio, id};
                    fib_heap.decrease_key(nodes[id], new_k);
                    pq.push(new_k);
                }
            }
        } else {
            // Extract min
            while (!pq.empty()) {
                auto top = pq.top();
                if (current_priority[top.second] == top.first) {
                    break;
                }
                pq.pop();
            }
            if (!pq.empty()) {
                auto expected_min = pq.top();
                pq.pop();
                assert(!fib_heap.empty());
                KeyType min_key = fib_heap.min_key();
                assert(min_key == expected_min);
                int* val = fib_heap.erase_min();
                assert(val != nullptr);
                int id = *val;
                assert(id == expected_min.second);
                current_priority[id] = -1; // mark deleted
                active_ids.erase(std::remove(active_ids.begin(), active_ids.end(), id), active_ids.end());
            }
        }

        if (step % 250 == 0) {
            assert(fib_heap.validate_invariants());
        }
    }

    assert(fib_heap.validate_invariants());
    std::cout << "[Test Fibo] Random stress test PASSED (" << NUM_OPS << " operations)!" << std::endl;
}

int main() {
    test_basic();
    test_meld();
    test_cascading_cut();
    test_erase_arbitrary();
    test_random_stress_unique_keys();
    std::cout << "\n>>> ALL FIBONACCI HEAP TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
