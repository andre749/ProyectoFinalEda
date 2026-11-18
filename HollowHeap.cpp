#pragma once
// Hollow Heap (multi-root con soporte para multiple decrease-key)
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cassert>

using namespace std;

template<typename Tk, typename Tv>
class HollowHeap {
private:
    struct DagNode;

public:
    // nodo visible para el usuario (se mantiene valido tras decrease_key)
    struct Node {
        Tk key;
        Tv* value;
        DagNode* dag_node = nullptr;

        Node(Tk k, Tv* v) : key(k), value(v) {}
    };

private:
    // nodo interno del dag (puede quedar hollow)
    struct DagNode {
        Node* item = nullptr; // nullptr si el nodo es hueco
        Tk key;
        size_t rank = 0;
        DagNode* child = nullptr;
        DagNode* next = nullptr;

        DagNode(Node* it, Tk k) : item(it), key(k) {}
    };

    DagNode* min_root = nullptr;
    size_t n = 0;

    static constexpr size_t MAX_RANK = 128;

    // enlace de dos raices del mismo rango
    static DagNode* link(DagNode* u, DagNode* v) {
        if (v->key < u->key) swap(u, v);
        v->next = u->child;
        u->child = v;
        u->rank++;
        return u;
    }

    // agrega una raiz a la lista circular
    void add_root(DagNode* r) {
        if (!min_root) {
            min_root = r;
            min_root->next = min_root;
        } else {
            r->next = min_root->next;
            min_root->next = r;
            if (r->key < min_root->key) min_root = r;
        }
    }

    bool validate_tree(DagNode* u, size_t& live_count) const {
        if (u->item) {
            live_count++;
            if (u->item->dag_node != u) return false;
            if (u->item->key != u->key) return false;
        }

        DagNode* c = u->child;
        while (c) {
            if (c->key < u->key) return false;
            if (!validate_tree(c, live_count)) return false;
            c = c->next;
        }
        return true;
    }

public:
    HollowHeap() = default;

    HollowHeap(const HollowHeap&) = delete;
    HollowHeap& operator=(const HollowHeap&) = delete;

    HollowHeap(HollowHeap&& other) noexcept : min_root(other.min_root), n(other.n) {
        other.min_root = nullptr;
        other.n = 0;
    }

    HollowHeap& operator=(HollowHeap&& other) noexcept {
        if (this != &other) {
            clear();
            min_root = other.min_root;
            n = other.n;
            other.min_root = nullptr;
            other.n = 0;
        }
        return *this;
    }

    ~HollowHeap() { clear(); }

    bool empty() const { return min_root == nullptr; }
    size_t size() const { return n; }

    Tv* find_min() const {
        return (min_root && min_root->item) ? min_root->item->value : nullptr;
    }
    Tv* get_min() const { return find_min(); }

    Node* min_node() const {
        return min_root ? min_root->item : nullptr;
    }
    Node* min_node_handle() const {
        return min_node();
    }

    Tk min_key() const {
        if (!min_root || !min_root->item) throw runtime_error("cola vacia");
        return min_root->key;
    }

    Node* insert(Tk key, Tv* value = nullptr) {
        Node* handle = new Node(key, value);
        DagNode* u = new DagNode(handle, key);
        handle->dag_node = u;
        add_root(u);
        n++;
        return handle;
    }

    void meld(HollowHeap& other) {
        if (!other.min_root) return;
        if (!min_root) {
            min_root = other.min_root;
            n = other.n;
        } else {
            DagNode* next1 = min_root->next;
            DagNode* next2 = other.min_root->next;
            min_root->next = next2;
            other.min_root->next = next1;
            if (other.min_root->key < min_root->key) {
                min_root = other.min_root;
            }
            n += other.n;
        }
        other.min_root = nullptr;
        other.n = 0;
    }

    Tv* erase_min() {
        if (!min_root) return nullptr;

        Node* min_item = min_root->item;
        Tv* val = min_item ? min_item->value : nullptr;
        if (min_item) delete min_item;
        min_root->item = nullptr; // queda hueco
        n--;

        // 1. juntamos todas las raices
        vector<DagNode*> L;
        DagNode* curr = min_root;
        do {
            DagNode* nxt = curr->next;
            curr->next = nullptr;
            L.push_back(curr);
            curr = nxt;
        } while (curr != min_root && curr != nullptr);

        // 2. desmantelamos las raices huecas
        vector<DagNode*> live_roots;
        for (size_t i = 0; i < L.size(); ++i) {
            DagNode* u = L[i];
            if (!u->item) {
                // sus hijos suben a raices y borramos el nodo hueco
                DagNode* c = u->child;
                while (c) {
                    DagNode* nxt_c = c->next;
                    c->next = nullptr;
                    L.push_back(c);
                    c = nxt_c;
                }
                delete u;
            } else {
                live_roots.push_back(u);
            }
        }

        if (live_roots.empty()) {
            min_root = nullptr;
            return val;
        }

        // 3. enlazamos por rango con cubetas
        array<DagNode*, MAX_RANK> A;
        A.fill(nullptr);
        size_t max_r = 0;

        for (DagNode* u : live_roots) {
            size_t r = u->rank;
            while (r < MAX_RANK && A[r] != nullptr) {
                DagNode* v = A[r];
                A[r] = nullptr;
                u = link(u, v);
                r++;
            }
            if (r < MAX_RANK) {
                A[r] = u;
                if (r > max_r) max_r = r;
            }
        }

        // 4. reconstruimos la lista circular con el nuevo minimo
        min_root = nullptr;
        for (size_t r = 0; r <= max_r; ++r) {
            if (!A[r]) continue;
            DagNode* u = A[r];
            if (!min_root) {
                min_root = u;
                min_root->next = min_root;
            } else {
                u->next = min_root->next;
                min_root->next = u;
                if (u->key < min_root->key) min_root = u;
            }
        }

        return val;
    }

    Tv* extract_min() { return erase_min(); }
    Tv* delete_min()  { return erase_min(); }

    void decrease_key(Node* handle, Tk new_key) {
        if (!handle) return;
        if (new_key > handle->key) {
            throw invalid_argument("la llave nueva es mayor que la actual");
        }
        handle->key = new_key;
        DagNode* u = handle->dag_node;
        if (!u) return;

        // si ya es el minimo se actualiza directo
        if (u == min_root) {
            u->key = new_key;
            return;
        }

        // se crea un nuevo nodo vivo y el viejo queda hueco
        DagNode* v = new DagNode(handle, new_key);
        handle->dag_node = v;
        u->item = nullptr;

        if (u->rank > 2 && u->child && u->child->next) {
            v->rank = u->rank - 2;
            v->child = u->child->next->next;
            u->child->next->next = nullptr;
            u->rank = 2;
        } else {
            v->rank = 0;
            v->child = nullptr;
        }

        add_root(v);
    }

    void decrease_key_delta(Node* handle, Tk delta) {
        if (delta < 0) throw invalid_argument("delta no puede ser negativo");
        decrease_key(handle, handle->key - delta);
    }

    void erase(Node* handle) {
        if (!handle || !min_root) return;
        if (handle->dag_node == min_root) {
            erase_min();
            return;
        }
        // marcado perezoso como hueco
        DagNode* u = handle->dag_node;
        if (u) u->item = nullptr;
        delete handle;
        n--;
    }

    void clear() {
        if (!min_root) return;

        vector<DagNode*> st;
        DagNode* curr = min_root;
        do {
            DagNode* nxt = curr->next;
            curr->next = nullptr;
            st.push_back(curr);
            curr = nxt;
        } while (curr != min_root && curr != nullptr);

        while (!st.empty()) {
            DagNode* u = st.back();
            st.pop_back();

            DagNode* c = u->child;
            while (c) {
                DagNode* nxt_c = c->next;
                c->next = nullptr;
                st.push_back(c);
                c = nxt_c;
            }

            if (u->item) {
                delete u->item;
                u->item = nullptr;
            }
            delete u;
        }

        min_root = nullptr;
        n = 0;
    }

    bool validate_invariants() const {
        if (n == 0) return min_root == nullptr;
        if (!min_root || !min_root->item) return false;
        if (min_root->key != min_root->item->key) return false;

        size_t live_count = 0;
        vector<DagNode*> roots;
        DagNode* curr = min_root;
        do {
            roots.push_back(curr);
            if (curr->item && curr->key < min_root->key) return false;
            curr = curr->next;
        } while (curr != min_root && curr != nullptr);

        for (DagNode* r : roots) {
            if (!validate_tree(r, live_count)) return false;
        }

        return live_count == n;
    }
};
