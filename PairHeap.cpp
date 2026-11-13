//
// Pairing heap (Fredman, Sedgewick, Sleator, Tarjan - Algorithmica 1986)
//
// Representación: child/sibling (árbol binario) + prev.
//   child   -> hijo más joven (el último enlazado)
//   sibling -> siguiente hermano (más viejo)
//   prev    -> padre en el árbol binario: el hermano anterior, o el padre real
//              si el nodo es el primer hijo. Es el "tercer puntero" del paper
//              que hace posible decrease_key y delete en O(1) de corte.
//
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
using namespace std;

template<typename Tk, typename Tv>
struct PairHeap {

    struct Node {
        Tk key;
        Tv* value;
        Node* child = nullptr;
        Node* sibling = nullptr;
        Node* prev = nullptr;
        Node(Tk k, Tv* v) : key(k), value(v) {}
    };

    Node* root = nullptr;
    size_t n = 0;

    PairHeap() = default;
    PairHeap(const PairHeap&) = delete;
    PairHeap& operator=(const PairHeap&) = delete;
    ~PairHeap() { clear(); }

    bool empty() const { return root == nullptr; }
    size_t size() const { return n; }


    static Node* link(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (b->key < a->key) swap(a, b);
        b->sibling = a->child;
        if (a->child) a->child->prev = b;
        b->prev = a;
        a->child = b;
        return a;
    }

    Node* insert(Tk k, Tv* v) {
        Node* x = new Node(k, v);
        root = link(root, x);
        n++;
        return x;
    }

    void meld(PairHeap& other) {
        root = link(root, other.root);
        n += other.n;
        other.root = nullptr;
        other.n = 0;
    }

    Tv* find_min() const { return root ? root->value : nullptr; }

    static void cut(Node* x) {
        if (x->prev->child == x) x->prev->child = x->sibling;
        else                     x->prev->sibling = x->sibling;
        if (x->sibling) x->sibling->prev = x->prev;
        x->prev = x->sibling = nullptr;
    }

    void decrease_key(Node* x, Tk delta) {
        x->key -= delta;
        if (x == root) return;
        cut(x);
        root = link(root, x);
    }


    static Node* combine(Node* first) {
        if (!first) return nullptr;

        Node* pairs = nullptr;
        while (first) {
            Node* a = first;
            Node* b = a->sibling;
            if (b) { first = b->sibling; b->sibling = nullptr; b->prev = nullptr; }
            else   { first = nullptr; }
            a->sibling = nullptr;
            a->prev = nullptr;
            Node* t = link(a, b);
            t->sibling = pairs;
            pairs = t;
        }

        Node* res = pairs;
        pairs = pairs->sibling;
        res->sibling = nullptr;
        while (pairs) {
            Node* t = pairs;
            pairs = pairs->sibling;
            t->sibling = nullptr;
            res = link(t, res);
        }
        return res;
    }

    Tv* erase_min() {
        if (!root) return nullptr;
        Node* r = root;
        Tv* v = r->value;
        root = combine(r->child);
        if (root) root->prev = nullptr;
        delete r;
        n--;
        return v;
    }
    void erase(Node* x) {
        if (!x) return;
        if (x == root) { erase_min(); return; }
        cut(x);
        Node* sub = combine(x->child);
        x->child = nullptr;
        delete x;
        n--;
        root = link(root, sub);
    }

    void clear() {
        vector<Node*> st;
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
};

