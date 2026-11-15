#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <cassert>
#include <algorithm>
#include "../RankPairingHeap.cpp"

void test_basic() {
    std::cout << "[Test RP-Heap] Basic operations..." << std::endl;
    RankPairingHeap<int, int> h;
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

    // Extract on empty heap returns nullptr
    assert(h.erase_min() == nullptr);

    std::cout << "[Test RP-Heap] Basic operations PASSED!" << std::endl;
}

void test_meld() {
    std::cout << "[Test RP-Heap] Meld operations..." << std::endl;
    RankPairingHeap<int, int> h1, h2;
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

    // Meld with empty heap
    RankPairingHeap<int, int> empty_heap;
    h1.meld(empty_heap);
    assert(h1.size() == 4);
    assert(h1.validate_invariants());

    empty_heap.meld(h1);
    assert(h1.empty());
    assert(empty_heap.size() == 4);
    assert(*empty_heap.find_min() == 5);
    assert(empty_heap.validate_invariants());

    // Self-meld should be a safe no-op
    empty_heap.meld(empty_heap);
    assert(empty_heap.size() == 4);
    assert(empty_heap.validate_invariants());

    // Extract all
    std::vector<int> extracted;
    while (!empty_heap.empty()) {
        extracted.push_back(*empty_heap.erase_min());
        assert(empty_heap.validate_invariants());
    }
    std::vector<int> expected = {5, 15, 25, 30};
    assert(extracted == expected);

    std::cout << "[Test RP-Heap] Meld operations PASSED!" << std::endl;
}

void test_cascading_rank_fix() {
    std::cout << "[Test RP-Heap] Cascading rank fix verification (Type-2 rule)..." << std::endl;
    RankPairingHeap<int, int> h;
    const int N = 32;
    std::vector<int> vals(N);
    std::vector<RankPairingHeap<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i;
        nodes[i] = h.insert(i, &vals[i]);
    }
    assert(h.validate_invariants());

    // Consolidate trees into higher ranks by extracting min (0)
    int* m0 = h.erase_min();
    assert(*m0 == 0);
    assert(h.validate_invariants());

    // Now trees have formed with rank > 0.
    // Decrease keys and verify cascading rank changes and invariant preservation
    for (int i = N - 1; i >= 1; --i) {
        h.decrease_key(nodes[i], -i);
        assert(h.validate_invariants());
    }

    // Test decrease_key_delta
    h.decrease_key_delta(nodes[1], 100);
    assert(h.validate_invariants());

    // Verify exception handling for invalid decreases
    bool threw_exception = false;
    try {
        h.decrease_key(nodes[1], 1000); // larger key
    } catch (const std::invalid_argument&) {
        threw_exception = true;
    }
    assert(threw_exception);

    threw_exception = false;
    try {
        h.decrease_key_delta(nodes[1], -10); // negative delta
    } catch (const std::invalid_argument&) {
        threw_exception = true;
    }
    assert(threw_exception);

    // Extract all in order
    int prev = -999999;
    while (!h.empty()) {
        int k = h.min_key();
        assert(k >= prev);
        prev = k;
        h.erase_min();
        assert(h.validate_invariants());
    }

    std::cout << "[Test RP-Heap] Cascading rank fix PASSED!" << std::endl;
}

void test_erase_arbitrary() {
    std::cout << "[Test RP-Heap] Erase arbitrary nodes..." << std::endl;
    RankPairingHeap<int, int> h;
    const int N = 30;
    std::vector<int> vals(N);
    std::vector<RankPairingHeap<int, int>::Node*> nodes(N);

    for (int i = 0; i < N; ++i) {
        vals[i] = i * 10;
        nodes[i] = h.insert(vals[i], &vals[i]);
    }
    assert(h.validate_invariants());

    // Consolidate by extracting min
    h.erase_min(); // removes 0
    assert(h.validate_invariants());

    // Erase a node in the middle
    h.erase(nodes[10]);
    assert(h.size() == N - 2);
    assert(h.validate_invariants());

    // Erase another
    h.erase(nodes[5]);
    assert(h.size() == N - 3);
    assert(h.validate_invariants());

    // Erase min via erase
    h.erase(nodes[1]);
    assert(h.size() == N - 4);
    assert(h.validate_invariants());

    // Erase leaf or root nodes
    h.erase(nodes[20]);
    h.erase(nodes[29]);
    assert(h.validate_invariants());

    // Erase nullptr does nothing
    h.erase(nullptr);
    assert(h.validate_invariants());

    while (!h.empty()) {
        h.erase_min();
        assert(h.validate_invariants());
    }

    std::cout << "[Test RP-Heap] Erase arbitrary nodes PASSED!" << std::endl;
}

void test_move_and_clear() {
    std::cout << "[Test RP-Heap] Move semantics and clear..." << std::endl;
    RankPairingHeap<int, int> h1;
    int v1 = 1, v2 = 2, v3 = 3;
    h1.insert(1, &v1);
    h1.insert(2, &v2);
    h1.insert(3, &v3);

    // Move constructor
    RankPairingHeap<int, int> h2 = std::move(h1);
    assert(h1.empty());
    assert(h1.size() == 0);
    assert(h2.size() == 3);
    assert(*h2.find_min() == 1);
    assert(h2.validate_invariants());

    // Move assignment
    RankPairingHeap<int, int> h3;
    h3 = std::move(h2);
    assert(h2.empty());
    assert(h3.size() == 3);
    assert(*h3.find_min() == 1);
    assert(h3.validate_invariants());

    // Self move-assignment
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wself-move"
    h3 = std::move(h3);
    #pragma GCC diagnostic pop
    assert(h3.size() == 3);
    assert(h3.validate_invariants());

    // Clear
    h3.clear();
    assert(h3.empty());
    assert(h3.size() == 0);
    assert(h3.find_min() == nullptr);
    assert(h3.validate_invariants());

    // Can insert after clear
    int v4 = 42;
    h3.insert(42, &v4);
    assert(h3.size() == 1);
    assert(*h3.find_min() == 42);
    assert(h3.validate_invariants());

    std::cout << "[Test RP-Heap] Move semantics and clear PASSED!" << std::endl;
}

void test_random_stress_unique_keys() {
    std::cout << "[Test RP-Heap] Random stress test with unique keys against std::priority_queue..." << std::endl;
    using KeyType = std::pair<int, int>;
    RankPairingHeap<KeyType, int> rp_heap;
    std::priority_queue<KeyType, std::vector<KeyType>, std::greater<KeyType>> pq;

    std::mt19937 rng(42);
    const int NUM_OPS = 20000;
    std::vector<int> active_ids;
    std::vector<int> current_priority(NUM_OPS + 10, 0);
    std::vector<RankPairingHeap<KeyType, int>::Node*> nodes(NUM_OPS + 10, nullptr);
    std::vector<int> values(NUM_OPS + 10, 0);
    int next_id = 1;

    for (int step = 0; step < NUM_OPS; ++step) {
        int op = rng() % 100;
        if (op < 50 || rp_heap.empty()) {
            // Insert (50%)
            int id = next_id++;
            int prio = (rng() % 100000) + 1000;
            current_priority[id] = prio;
            values[id] = id;
            KeyType k = {prio, id};
            nodes[id] = rp_heap.insert(k, &values[id]);
            pq.push(k);
            active_ids.push_back(id);
        } else if (op < 75) {
            // Decrease key (25%)
            if (!active_ids.empty()) {
                size_t idx = rng() % active_ids.size();
                int id = active_ids[idx];
                if (current_priority[id] > 5) {
                    int new_prio = rng() % current_priority[id];
                    current_priority[id] = new_prio;
                    KeyType new_k = {new_prio, id};
                    rp_heap.decrease_key(nodes[id], new_k);
                    pq.push(new_k);
                }
            }
        } else if (op < 95) {
            // Extract min (20%)
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
                assert(!rp_heap.empty());
                KeyType min_key = rp_heap.min_key();
                assert(min_key == expected_min);
                int* val = rp_heap.erase_min();
                assert(val != nullptr);
                int id = *val;
                assert(id == expected_min.second);
                current_priority[id] = -1; // mark deleted
                active_ids.erase(std::remove(active_ids.begin(), active_ids.end(), id), active_ids.end());
            }
        } else {
            // Erase arbitrary node (5%)
            if (!active_ids.empty()) {
                size_t idx = rng() % active_ids.size();
                int id = active_ids[idx];
                current_priority[id] = -1; // mark deleted
                rp_heap.erase(nodes[id]);
                active_ids.erase(active_ids.begin() + idx);
            }
        }

        if (step % 250 == 0) {
            assert(rp_heap.validate_invariants());
        }
    }

    assert(rp_heap.validate_invariants());
    std::cout << "[Test RP-Heap] Random stress test PASSED (" << NUM_OPS << " operations)!" << std::endl;
}

int main() {
    test_basic();
    test_meld();
    test_cascading_rank_fix();
    test_erase_arbitrary();
    test_move_and_clear();
    test_random_stress_unique_keys();
    std::cout << "\n>>> ALL RANK-PAIRING HEAP TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}
