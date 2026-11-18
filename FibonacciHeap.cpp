#pragma once
// Fibonacci Heap
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cassert>

using namespace std;

template<typename Tk, typename Tv>
class FibonacciHeap {
public:
    struct Node {
        Tk key;
        Tv* value;
        size_t degree = 0;
        bool mark = false;
        Node* parent = nullptr;
        Node* child = nullptr;
        Node* left = nullptr;
        Node* right = nullptr;

        Node(Tk k, Tv* v) : key(k), value(v), left(this), right(this) {}
    };

private:
    Node* min = nullptr;
    size_t n = 0;

    static constexpr size_t MAX_DEGREE = 128;

    // inserta un nodo en la lista circular a la izquierda de head
    static void list_add(Node* head, Node* node) {
        if (!head || !node) return;
        node->left = head->left;
        node->right = head;
        head->left->right = node;
        head->left = node;
    }

    // saca un nodo de su lista circular
    static void list_remove(Node* node) {
        if (!node) return;
        node->left->right = node->right;
        node->right->left = node->left;
        node->left = node;
        node->right = node;
    }

    // concatena dos listas circulares
    static void list_concat(Node* h1, Node* h2) {
        if (!h1 || !h2) return;
        Node* l1 = h1->left;
        Node* l2 = h2->left;
        h1->left = l2;
        l2->right = h1;
        h2->left = l1;
        l1->right = h2;
    }

    // hace que y sea hijo de x
    void heap_link(Node* y, Node* x) {
        list_remove(y);
        y->parent = x;
        if (!x->child) {
            x->child = y;
            y->left = y;
            y->right = y;
        } else {
            list_add(x->child, y);
        }
        x->degree++;
        y->mark = false;
    }

    // consolidacion de raices con mismo grado usando cubetas en stack
    void consolidate() {
        array<Node*, MAX_DEGREE> A;
        A.fill(nullptr);

        vector<Node*> roots;
        if (min) {
            Node* curr = min;
            do {
                roots.push_back(curr);
                curr = curr->right;
            } while (curr != min);
        }

        for (Node* w : roots) {
            Node* x = w;
            size_t d = x->degree;
            while (d < MAX_DEGREE && A[d] != nullptr) {
                Node* y = A[d];
                if (x->key > y->key) swap(x, y);
                heap_link(y, x);
                A[d] = nullptr;
                d++;
            }
            if (d < MAX_DEGREE) A[d] = x;
        }

        min = nullptr;
        for (size_t i = 0; i < MAX_DEGREE; ++i) {
            if (!A[i]) continue;
            A[i]->left = A[i];
            A[i]->right = A[i];
            if (!min) {
                min = A[i];
            } else {
                list_add(min, A[i]);
                if (A[i]->key < min->key) min = A[i];
            }
        }
    }

    // corta el nodo x de su padre p y lo sube a la lista de raices
    void cut(Node* x, Node* p) {
        if (x == x->right) {
            p->child = nullptr;
        } else {
            if (p->child == x) p->child = x->right;
            list_remove(x);
        }
        p->degree--;
        x->parent = nullptr;
        x->mark = false;
        x->left = x;
        x->right = x;
        list_add(min, x);
        if (x->key < min->key) min = x;
    }

    // cascada de cortes si el ancestro ya estaba marcado
    void cascading_cut(Node* y) {
        Node* z = y->parent;
        if (z) {
            if (!y->mark) {
                y->mark = true;
            } else {
                cut(y, z);
                cascading_cut(z);
            }
        }
    }

    bool validate_subtree(Node* u, size_t& count) const {
        count = 1;
        if (!u->child) return u->degree == 0;

        size_t child_count = 0;
        Node* c = u->child;
        do {
            if (c->parent != u) return false;
            if (c->right->left != c || c->left->right != c) return false;
            if (c->key < u->key) return false;

            size_t sub_count = 0;
            if (!validate_subtree(c, sub_count)) return false;
            count += sub_count;
            child_count++;
            c = c->right;
        } while (c != u->child);

        return child_count == u->degree;
    }

public:
    FibonacciHeap() = default;

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    FibonacciHeap(FibonacciHeap&& other) noexcept : min(other.min), n(other.n) {
        other.min = nullptr;
        other.n = 0;
    }

    FibonacciHeap& operator=(FibonacciHeap&& other) noexcept {
        if (this != &other) {
            clear();
            min = other.min;
            n = other.n;
            other.min = nullptr;
            other.n = 0;
        }
        return *this;
    }

    ~FibonacciHeap() {
        clear();
    }

    bool empty() const { return min == nullptr; }
    size_t size() const { return n; }

    Tv* find_min() const { return min ? min->value : nullptr; }
    Tv* get_min() const { return find_min(); }
    Node* min_node() const { return min; }

    Tk min_key() const {
        if (!min) throw runtime_error("cola vacia");
        return min->key;
    }

    Node* insert(Tk key, Tv* value = nullptr) {
        Node* x = new Node(key, value);
        if (!min) {
            min = x;
        } else {
            list_add(min, x);
            if (x->key < min->key) min = x;
        }
        n++;
        return x;
    }

    void meld(FibonacciHeap& other) {
        if (!other.min) return;
        if (!min) {
            min = other.min;
            n = other.n;
        } else {
            list_concat(min, other.min);
            if (other.min->key < min->key) min = other.min;
            n += other.n;
        }
        other.min = nullptr;
        other.n = 0;
    }

    Tv* erase_min() {
        Node* z = min;
        if (!z) return nullptr;

        // subimos todos los hijos de z a la lista de raices
        if (z->child) {
            vector<Node*> children;
            Node* c = z->child;
            do {
                children.push_back(c);
                c = c->right;
            } while (c != z->child);

            for (Node* child : children) {
                child->parent = nullptr;
                child->mark = false;
                child->left = child;
                child->right = child;
                list_add(min, child);
            }
            z->child = nullptr;
        }

        Tv* val = z->value;
        if (z == z->right) {
            min = nullptr;
        } else {
            min = z->right;
            list_remove(z);
            consolidate();
        }
        delete z;
        n--;
        return val;
    }

    Tv* extract_min() { return erase_min(); }
    Tv* delete_min() { return erase_min(); }

    void decrease_key(Node* x, Tk new_key) {
        if (!x) return;
        if (new_key > x->key) {
            throw invalid_argument("la llave nueva es mayor que la actual");
        }
        x->key = new_key;
        Node* p = x->parent;
        if (p && x->key < p->key) {
            cut(x, p);
            cascading_cut(p);
        }
        if (x->key < min->key) min = x;
    }

    void decrease_key_delta(Node* x, Tk delta) {
        if (delta < 0) throw invalid_argument("delta no puede ser negativo");
        decrease_key(x, x->key - delta);
    }

    void erase(Node* x) {
        if (!x || !min) return;
        if (x == min) {
            erase_min();
            return;
        }
        decrease_key(x, numeric_limits<Tk>::lowest());
        erase_min();
    }

    void clear() {
        if (!min) return;
        vector<Node*> st;
        Node* curr = min;
        vector<Node*> roots;
        do {
            roots.push_back(curr);
            curr = curr->right;
        } while (curr != min);

        for (Node* r : roots) st.push_back(r);

        while (!st.empty()) {
            Node* u = st.back();
            st.pop_back();
            if (u->child) {
                Node* c = u->child;
                vector<Node*> kids;
                do {
                    kids.push_back(c);
                    c = c->right;
                } while (c != u->child);
                for (Node* k : kids) st.push_back(k);
                u->child = nullptr;
            }
            delete u;
        }
        min = nullptr;
        n = 0;
    }

    bool validate_invariants() const {
        if (n == 0) return min == nullptr;
        if (!min) return false;

        size_t total_nodes = 0;
        Node* curr = min;
        do {
            if (curr->parent != nullptr) return false;
            if (curr->mark) return false;
            if (curr->right->left != curr || curr->left->right != curr) return false;
            if (curr->key < min->key) return false;

            size_t tree_nodes = 0;
            if (!validate_subtree(curr, tree_nodes)) return false;
            total_nodes += tree_nodes;
            curr = curr->right;
        } while (curr != min);

        return total_nodes == n;
    }
};
