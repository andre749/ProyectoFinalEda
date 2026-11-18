#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <cassert>
#include <algorithm>
#include "../PairHeap.cpp"

void test_basic() {
    std::cout << "[Test PairHeap] Basic operations..." << std::endl;
    PairHeap<int, int> h;
    assert(h.empty());
    assert(h.size() == 0);
    assert(h.find_min() == nullptr);
    assert(h.get_min() == nullptr);
    assert(h.min_node() == nullptr);
    assert(h.validate_invariants());

    int v1 = 10, v2 = 20, v3 = 5;
    auto n1 = h.insert(10, &v1);
    assert(!h.empty());
    assert(h.size() == 1);
    assert(*h.find_min() == 10);
    assert(h.min_key() == 10);
    assert(h.min_node() == n1);
    assert(h.validate_invariants());

    auto n2 = h.insert(20, &v2);
    (void)n2;
    auto n3 = h.insert(5, &v3);
    assert(h.size() == 3);
    assert(*h.find_min() == 5);
    assert(h.min_key() == 5);
    assert(h.min_node() == n3);
    assert(h.validate_invariants());

    // Extract min
    int* min_val = h.erase_min();
    assert(min_val != nullptr && *min_val == 5);
    assert(h.size() == 2);
    assert(*h.find_min() == 10);
    assert(h.min_key() == 10);
    assert(h.validate_invariants());

    min_val = h.extract_min();
    assert(min_val != nullptr && *min_val == 10);
    assert(h.size() == 1);
    assert(*h.find_min() == 20);
    assert(h.min_key() == 20);
    assert(h.validate_invariants());

    min_val = h.delete_min();
    assert(min_val != nullptr && *min_val == 20);
    assert(h.empty());
    assert(h.size() == 0);
    assert(h.find_min() == nullptr);
    assert(h.validate_invariants());

    // Extract on empty returns nullptr
    assert(h.erase_min() == nullptr);

    std::cout << "[Test PairHeap] Basic operations PASSED!" << std::endl;
}

void test_meld() {
    std::cout << "[Test PairHeap] Meld operations..." << std::endl;
    PairHeap<int, int> h1, h2;
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

    std::cout << "[Test PairHeap] Meld operations PASSED!" << std::endl;
}

void test_decrease_key() {
    std::cout << "[Test PairHeap] Decrease key operations..." << std::endl;
    PairHeap<int, int> h;
    const int N = 20;
    std::vector<int> vals(N);
    std::vector<PairHeap<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i * 10;
        nodes[i] = h.insert(vals[i], &vals[i]);
    }
    assert(h.validate_invariants());

    // Extract 0
    h.erase_min();
    assert(h.validate_invariants());

    // Decrease nodes
    h.decrease_key(nodes[15], 2);
    assert(h.min_key() == 2);
    assert(h.validate_invariants());

    h.decrease_key_delta(nodes[10], 50); // was 100 -> now 50
    assert(h.validate_invariants());

    // Multiple decrease_key on same node
    h.decrease_key(nodes[15], 1);
    assert(h.min_key() == 1);
    assert(h.validate_invariants());

    // Invalid decrease throws
    bool threw = false;
    try {
        h.decrease_key(nodes[15], 100);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::cout << "[Test PairHeap] Decrease key PASSED!" << std::endl;
}

void test_erase_arbitrary() {
    std::cout << "[Test PairHeap] Erase arbitrary nodes..." << std::endl;
    PairHeap<int, int> h;
    const int N = 30;
    std::vector<int> vals(N);
    std::vector<PairHeap<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i * 10;
        nodes[i] = h.insert(vals[i], &vals[i]);
    }

    h.erase_min();
    assert(h.validate_invariants());

    h.erase(nodes[10]);
    assert(h.size() == N - 2);
    assert(h.validate_invariants());

    h.erase(nodes[5]);
    assert(h.size() == N - 3);
    assert(h.validate_invariants());

    h.erase(nodes[1]);
    assert(h.size() == N - 4);
    assert(h.validate_invariants());

    while (!h.empty()) {
        h.erase_min();
        assert(h.validate_invariants());
    }

    std::cout << "[Test PairHeap] Erase arbitrary nodes PASSED!" << std::endl;
}

void test_random_stress_unique_keys() {
    std::cout << "[Test PairHeap] Random stress test against std::priority_queue..." << std::endl;
    using KeyType = std::pair<int, int>;
    PairHeap<KeyType, int> pair_heap;
    std::priority_queue<KeyType, std::vector<KeyType>, std::greater<KeyType>> pq;

    std::mt19937 rng(1337);
    const int NUM_OPS = 20000;
    std::vector<int> active_ids;
    std::vector<int> current_priority(NUM_OPS + 10, 0);
    std::vector<PairHeap<KeyType, int>::Node*> nodes(NUM_OPS + 10, nullptr);
    std::vector<int> values(NUM_OPS + 10, 0);
    int next_id = 1;

    for (int step = 0; step < NUM_OPS; ++step) {
        int op = rng() % 100;
        if (op < 50 || pair_heap.empty()) {
            // Insert
            int id = next_id++;
            int prio = (rng() % 100000) + 1000;
            current_priority[id] = prio;
            values[id] = id;
            KeyType k = {prio, id};
            nodes[id] = pair_heap.insert(k, &values[id]);
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
                    pair_heap.decrease_key(nodes[id], new_k);
                    pq.push(new_k);
                }
            }
        } else if (op < 95) {
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
                assert(!pair_heap.empty());
                KeyType min_key = pair_heap.min_key();
                assert(min_key == expected_min);
                int* val = pair_heap.erase_min();
                assert(val != nullptr);
                int id = *val;
                assert(id == expected_min.second);
                current_priority[id] = -1;
                active_ids.erase(std::remove(active_ids.begin(), active_ids.end(), id), active_ids.end());
            }
        } else {
            // Erase arbitrary
            if (!active_ids.empty()) {
                size_t idx = rng() % active_ids.size();
                int id = active_ids[idx];
                current_priority[id] = -1;
                pair_heap.erase(nodes[id]);
                active_ids.erase(active_ids.begin() + idx);
            }
        }

        if (step % 250 == 0) {
            assert(pair_heap.validate_invariants());
        }
    }

    assert(pair_heap.validate_invariants());
    std::cout << "[Test PairHeap] Random stress test PASSED (" << NUM_OPS << " operations)!" << std::endl;
}

int main() {
    test_basic();
    test_meld();
    test_decrease_key();
    test_erase_arbitrary();
    test_random_stress_unique_keys();
    std::cout << "\n>>> ALL PAIRING HEAP TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
