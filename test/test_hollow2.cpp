#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <cassert>
#include <algorithm>
#include "../HollowHeap2.cpp"

void test_basic() {
    std::cout << "[Test Hollow2] Basic operations..." << std::endl;
    HollowHeap2<int, int> h;
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

    std::cout << "[Test Hollow2] Basic operations PASSED!" << std::endl;
}

void test_meld() {
    std::cout << "[Test Hollow2] Meld operations..." << std::endl;
    HollowHeap2<int, int> h1, h2;
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

    // Meld on empty heap (previously crashed in HollowHeap.cpp)
    HollowHeap2<int, int> empty1, h3;
    int e = 100;
    h3.insert(100, &e);
    empty1.meld(h3);
    assert(empty1.size() == 1);
    assert(*empty1.find_min() == 100);

    std::cout << "[Test Hollow2] Meld operations PASSED!" << std::endl;
}

void test_repeated_decrease_key() {
    std::cout << "[Test Hollow2] Repeated decrease_key on same handle..." << std::endl;
    HollowHeap2<int, int> h;
    int val = 999;
    auto node = h.insert(100, &val);

    // Decrease 1: 100 -> 80
    h.decrease_key(node, 80);
    assert(h.min_key() == 80);
    assert(*h.find_min() == 999);
    assert(h.validate_invariants());

    // Decrease 2 on the SAME node pointer (crucial for Dijkstra): 80 -> 50
    h.decrease_key(node, 50);
    assert(h.min_key() == 50);
    assert(*h.find_min() == 999);
    assert(h.validate_invariants());

    // Decrease 3: 50 -> 20
    h.decrease_key(node, 20);
    assert(h.min_key() == 20);
    assert(*h.find_min() == 999);
    assert(h.validate_invariants());

    // Extract min should yield val 999
    int* out = h.erase_min();
    assert(out != nullptr && *out == 999);
    assert(h.empty());
    assert(h.validate_invariants());

    std::cout << "[Test Hollow2] Repeated decrease_key PASSED!" << std::endl;
}

void test_lazy_erase() {
    std::cout << "[Test Hollow2] Lazy erase arbitrary nodes..." << std::endl;
    HollowHeap2<int, int> h;
    const int N = 20;
    std::vector<int> vals(N);
    std::vector<HollowHeap2<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i * 10;
        nodes[i] = h.insert(vals[i], &vals[i]);
    }

    // Erase min
    h.erase(nodes[0]);
    assert(h.size() == N - 1);
    assert(h.min_key() == 10);
    assert(h.validate_invariants());

    // Erase a node in the middle
    h.erase(nodes[10]);
    assert(h.size() == N - 2);
    assert(h.validate_invariants());

    // Erase another
    h.erase(nodes[15]);
    assert(h.size() == N - 3);
    assert(h.validate_invariants());

    while (!h.empty()) {
        h.erase_min();
        assert(h.validate_invariants());
    }

    std::cout << "[Test Hollow2] Lazy erase PASSED!" << std::endl;
}

void test_random_stress_unique_keys() {
    std::cout << "[Test Hollow2] Random stress test against std::priority_queue..." << std::endl;
    using KeyType = std::pair<int, int>;
    HollowHeap2<KeyType, int> hollow_heap;
    std::priority_queue<KeyType, std::vector<KeyType>, std::greater<KeyType>> pq;

    std::mt19937 rng(42);
    const int NUM_OPS = 10000;
    std::vector<int> active_ids;
    std::vector<int> current_priority(NUM_OPS + 10, 0);
    std::vector<HollowHeap2<KeyType, int>::Node*> nodes(NUM_OPS + 10, nullptr);
    std::vector<int> values(NUM_OPS + 10, 0);
    int next_id = 1;

    for (int step = 0; step < NUM_OPS; ++step) {
        int op = rng() % 100;
        if (op < 50 || hollow_heap.empty()) {
            // Insert
            int id = next_id++;
            int prio = (rng() % 100000) + 1000;
            current_priority[id] = prio;
            values[id] = id;
            KeyType k = {prio, id};
            nodes[id] = hollow_heap.insert(k, &values[id]);
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
                    hollow_heap.decrease_key(nodes[id], new_k);
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
                assert(!hollow_heap.empty());
                KeyType min_key = hollow_heap.min_key();
                assert(min_key == expected_min);
                int* val = hollow_heap.erase_min();
                assert(val != nullptr);
                int id = *val;
                assert(id == expected_min.second);
                current_priority[id] = -1; // mark deleted
                active_ids.erase(std::remove(active_ids.begin(), active_ids.end(), id), active_ids.end());
            }
        }

        if (step % 250 == 0) {
            assert(hollow_heap.validate_invariants());
        }
    }

    assert(hollow_heap.validate_invariants());
    std::cout << "[Test Hollow2] Random stress test PASSED (" << NUM_OPS << " operations)!" << std::endl;
}

int main() {
    test_basic();
    test_meld();
    test_repeated_decrease_key();
    test_lazy_erase();
    test_random_stress_unique_keys();
    std::cout << "\n>>> ALL HOLLOW HEAP 2 TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
