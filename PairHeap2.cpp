#pragma once
//
// Pairing Heap 2 (Fredman, Sedgewick, Sleator, Tarjan - Algorithmica 1986)
// Implementación optimizada con interfaz uniforme para benchmarks experimentales
//
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cassert>

template<typename Tk, typename Tv>
class PairHeap2 {
public:
    struct Node {
        Tk key;
        Tv* value;
        Node* child   = nullptr;
        Node* sibling = nullptr;
        Node* prev    = nullptr; // anterior hermano, o padre si es el primer hijo

        Node(Tk k, Tv* v) : key(k), value(v) {}
    };

private:
    Node* root = nullptr;
    size_t n = 0;

    // Enlace de dos montículos de apareamiento
    static Node* link(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (b->key < a->key) std::swap(a, b);
        b->sibling = a->child;
        if (a->child) a->child->prev = b;
        b->prev = a;
        a->child = b;
        return a;
    }

    // Corte de un subárbol x de su posición actual
    static void cut(Node* x) {
        if (x->prev->child == x) x->prev->child = x->sibling;
        else                     x->prev->sibling = x->sibling;
        if (x->sibling)          x->sibling->prev = x->prev;
        x->prev = nullptr;
        x->sibling = nullptr;
    }

    // Combinación de hermanos en dos pasadas (Two-Pass Pairing)
    static Node* combine(Node* first) {
        if (!first) return nullptr;

        Node* pairs = nullptr;
        // Pasada 1: de izquierda a derecha en pares
        while (first) {
            Node* a = first;
            Node* b = a->sibling;
            if (b) {
                first = b->sibling;
                b->sibling = nullptr;
                b->prev = nullptr;
            } else {
                first = nullptr;
            }
            a->sibling = nullptr;
            a->prev = nullptr;
            Node* t = link(a, b);
            t->sibling = pairs;
            pairs = t;
        }

        // Pasada 2: de derecha a izquierda acumulando
        Node* res = pairs;
        pairs = pairs->sibling;
        res->sibling = nullptr;
        while (pairs) {
            Node* t = pairs;
            pairs = pairs->sibling;
            t->sibling = nullptr;
            res = link(t, res);
        }
        if (res) res->prev = nullptr;
        return res;
    }

    bool validate_node(Node* u, Node* parent, size_t& count) const {
        if (!u) return true;
        count = 1;
        if (parent && u->key < parent->key) return false;

        Node* prev_ptr = u;
        Node* c = u->child;
        while (c) {
            if (c->prev != prev_ptr) return false;
            if (c->key < u->key) return false;
            size_t child_count = 0;
            if (!validate_node(c, u, child_count)) return false;
            count += child_count;
            prev_ptr = c;
            c = c->sibling;
        }
        return true;
    }

public:
    PairHeap2() = default;

    PairHeap2(const PairHeap2&) = delete;
    PairHeap2& operator=(const PairHeap2&) = delete;

    PairHeap2(PairHeap2&& other) noexcept : root(other.root), n(other.n) {
        other.root = nullptr;
        other.n = 0;
    }

    PairHeap2& operator=(PairHeap2&& other) noexcept {
        if (this != &other) {
            clear();
            root = other.root;
            n = other.n;
            other.root = nullptr;
            other.n = 0;
        }
        return *this;
    }

    ~PairHeap2() { clear(); }

    // ========================== Consultas ============================

    bool empty() const { return root == nullptr; }
    size_t size() const { return n; }

    Tv* find_min() const { return root ? root->value : nullptr; }
    Tv* get_min() const { return find_min(); }

    Node* min_node() const { return root; }

    Tk min_key() const {
        if (!root) throw std::runtime_error("Heap is empty");
        return root->key;
    }

    // ==================== Operaciones principales ====================

    Node* insert(Tk key, Tv* value) {
        Node* x = new Node(key, value);
        root = link(root, x);
        if (root) root->prev = nullptr;
        n++;
        return x;
    }

    void meld(PairHeap2& other) {
        if (this == &other || !other.root) return;
        root = link(root, other.root);
        if (root) root->prev = nullptr;
        n += other.n;
        other.root = nullptr;
        other.n = 0;
    }

    Tv* erase_min() {
        if (!root) return nullptr;
        Node* r = root;
        Tv* val = r->value;
        root = combine(r->child);
        if (root) root->prev = nullptr;
        delete r;
        n--;
        return val;
    }

    Tv* extract_min() { return erase_min(); }
    Tv* delete_min()  { return erase_min(); }

    void decrease_key(Node* x, Tk new_key) {
        if (!x) return;
        if (new_key > x->key) {
            throw std::invalid_argument("New key is greater than current key in decrease_key");
        }
        x->key = new_key;
        if (x == root) return;
        cut(x);
        root = link(root, x);
        if (root) root->prev = nullptr;
    }

    void decrease_key_delta(Node* x, Tk delta) {
        if (delta < 0) {
            throw std::invalid_argument("Delta cannot be negative in decrease_key_delta");
        }
        decrease_key(x, x->key - delta);
    }

    void erase(Node* x) {
        if (!x || !root) return;
        if (x == root) { erase_min(); return; }
        cut(x);
        Node* sub = combine(x->child);
        x->child = nullptr;
        delete x;
        n--;
        root = link(root, sub);
        if (root) root->prev = nullptr;
    }

    void clear() {
        std::vector<Node*> st;
        if (root) st.push_back(root);
        while (!st.empty()) {
            Node* x = st.back(); st.pop_back();
            if (x->child)   st.push_back(x->child);
            if (x->sibling) st.push_back(x->sibling);
            delete x;
        }
        root = nullptr;
        n = 0;
    }

    // ============= Validación de invariantes (Testing / Debug) ==============

    bool validate_invariants() const {
        if (n == 0) return root == nullptr;
        if (!root) return false;
        if (root->prev != nullptr) return false;
        if (root->sibling != nullptr) return false;

        size_t count = 0;
        if (!validate_node(root, nullptr, count)) return false;
        return count == n;
    }
};
