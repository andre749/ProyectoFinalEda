#pragma once
//
// Hollow Heap 2 (Multi-Root 1-Parent Hollow Heap)
// Basado en Hansen, Kaplan, Tarjan & Zwick (ACM TALG 2017)
// Implementación optimizada con punteros intrínsecos, gestión RAII y soporte
// para múltiples decrease_key transparentes (para Dijkstra y Prim).
//
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cassert>

template<typename Tk, typename Tv>
class HollowHeap2 {
private:
    struct DagNode;

public:
    // Manejador expuesto al usuario / cliente (Dijkstra, Prim).
    // Conserva su identidad a través de sucesivos decrease_key sin requerir
    // reasignaciones manuales por parte del llamador.
    struct Node {
        Tk key;
        Tv* value;
        DagNode* dag_node = nullptr;

        Node(Tk k, Tv* v) : key(k), value(v) {}
    };

private:
    struct DagNode {
        Node* item = nullptr; // nullptr indica que el nodo es hueco (hollow)
        Tk key;
        size_t rank = 0;
        DagNode* child = nullptr;
        DagNode* next = nullptr;

        DagNode(Node* it, Tk k) : item(it), key(k), rank(0), child(nullptr), next(nullptr) {}
    };

    DagNode* min_node = nullptr;
    size_t n = 0;

    static constexpr size_t MAX_RANK = 128;

    // Enlace ordenado por rango entre dos raíces vivas del mismo rango.
    // El nodo con menor llave absorbe al otro como su primer hijo en O(1).
    static DagNode* link(DagNode* u, DagNode* v) {
        if (v->key < u->key) {
            std::swap(u, v);
        }
        v->next = u->child;
        u->child = v;
        u->rank++;
        return u;
    }

    // Inserta una raíz en la lista circular de raíces en O(1)
    void add_root(DagNode* r) {
        if (!min_node) {
            min_node = r;
            min_node->next = min_node;
        } else {
            r->next = min_node->next;
            min_node->next = r;
            if (r->key < min_node->key) {
                min_node = r;
            }
        }
    }

    bool validate_tree(DagNode* u, size_t& live_count) const {
        if (u->item) {
            live_count++;
            if (u->item->dag_node != u) return false;
            if (u->item->key != u->key) return false;
        }

        DagNode* c = u->child;
        while (c != nullptr) {
            if (c->key < u->key) return false; // Min-heap invariant
            if (!validate_tree(c, live_count)) return false;
            c = c->next;
        }
        return true;
    }

public:
    HollowHeap2() = default;

    HollowHeap2(const HollowHeap2&) = delete;
    HollowHeap2& operator=(const HollowHeap2&) = delete;

    HollowHeap2(HollowHeap2&& other) noexcept : min_node(other.min_node), n(other.n) {
        other.min_node = nullptr;
        other.n = 0;
    }

    HollowHeap2& operator=(HollowHeap2&& other) noexcept {
        if (this != &other) {
            clear();
            min_node = other.min_node;
            n = other.n;
            other.min_node = nullptr;
            other.n = 0;
        }
        return *this;
    }

    ~HollowHeap2() {
        clear();
    }

    // --- Consultas ---

    bool empty() const {
        return min_node == nullptr;
    }

    size_t size() const {
        return n;
    }

    Tv* find_min() const {
        return (min_node && min_node->item) ? min_node->item->value : nullptr;
    }

    Tv* get_min() const {
        return find_min();
    }

    Node* min_node_handle() const {
        return min_node ? min_node->item : nullptr;
    }

    Tk min_key() const {
        if (!min_node || !min_node->item) throw std::runtime_error("Heap is empty");
        return min_node->key;
    }

    // --- Mutaciones Principales ---

    Node* insert(Tk key, Tv* value) {
        Node* handle = new Node(key, value);
        DagNode* u = new DagNode(handle, key);
        handle->dag_node = u;
        add_root(u);
        n++;
        return handle;
    }

    void meld(HollowHeap2& other) {
        if (!other.min_node) return;
        if (!min_node) {
            min_node = other.min_node;
            n = other.n;
        } else {
            DagNode* next1 = min_node->next;
            DagNode* next2 = other.min_node->next;
            min_node->next = next2;
            other.min_node->next = next1;
            if (other.min_node->key < min_node->key) {
                min_node = other.min_node;
            }
            n += other.n;
        }
        other.min_node = nullptr;
        other.n = 0;
    }

    Tv* erase_min() {
        if (!min_node) return nullptr;

        Node* min_item = min_node->item;
        Tv* val = min_item ? min_item->value : nullptr;
        if (min_item) {
            delete min_item;
        }
        min_node->item = nullptr; // Se vuelve hollow
        n--;

        // 1. Recolectar todas las raíces actuales
        std::vector<DagNode*> L;
        DagNode* curr = min_node;
        do {
            DagNode* nxt = curr->next;
            curr->next = nullptr;
            L.push_back(curr);
            curr = nxt;
        } while (curr != min_node && curr != nullptr);

        // 2. Desmantelar todas las raíces hollow y liberar su memoria
        std::vector<DagNode*> live_roots;
        for (size_t i = 0; i < L.size(); ++i) {
            DagNode* u = L[i];
            if (u->item == nullptr) {
                // Nodo hueco: sus hijos ascienden a raíces y u se libera
                DagNode* c = u->child;
                while (c != nullptr) {
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
            min_node = nullptr;
            return val;
        }

        // 3. Consolidación por rangos (Ranked Linking) con arreglo estático en stack
        std::array<DagNode*, MAX_RANK> A;
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

        // 4. Reconstruir la lista circular de raíces vivas y encontrar el nuevo min_node
        min_node = nullptr;
        for (size_t r = 0; r <= max_r; ++r) {
            if (A[r] != nullptr) {
                DagNode* u = A[r];
                if (!min_node) {
                    min_node = u;
                    min_node->next = min_node;
                } else {
                    u->next = min_node->next;
                    min_node->next = u;
                    if (u->key < min_node->key) {
                        min_node = u;
                    }
                }
            }
        }

        return val;
    }

    Tv* extract_min() {
        return erase_min();
    }

    Tv* delete_min() {
        return erase_min();
    }

    void decrease_key(Node* handle, Tk new_key) {
        if (!handle) return;
        if (new_key > handle->key) {
            throw std::invalid_argument("New key is greater than current key in decrease_key");
        }
        handle->key = new_key;
        DagNode* u = handle->dag_node;
        if (!u) return;

        // Si ya es la raíz mínima, se actualiza in-place en O(1)
        if (u == min_node) {
            u->key = new_key;
            return;
        }

        // Multi-root hollow heap decrease-key (Hansen et al. 2017)
        DagNode* v = new DagNode(handle, new_key);
        handle->dag_node = v;
        u->item = nullptr; // El nodo previo queda hollow

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
        if (delta < 0) {
            throw std::invalid_argument("Delta cannot be negative in decrease_key_delta");
        }
        decrease_key(handle, handle->key - delta);
    }

    void erase(Node* handle) {
        if (!handle || !min_node) return;
        if (handle->dag_node == min_node) {
            erase_min();
            return;
        }
        // Eliminación perezosa en O(1): marca como hollow y libera el identificador
        DagNode* u = handle->dag_node;
        if (u) {
            u->item = nullptr;
        }
        delete handle;
        n--;
    }

    void clear() {
        if (!min_node) return;

        std::vector<DagNode*> st;
        DagNode* curr = min_node;
        do {
            DagNode* nxt = curr->next;
            curr->next = nullptr;
            st.push_back(curr);
            curr = nxt;
        } while (curr != min_node && curr != nullptr);

        while (!st.empty()) {
            DagNode* u = st.back();
            st.pop_back();

            DagNode* c = u->child;
            while (c != nullptr) {
                DagNode* nxt_c = c->next;
                c->next = nullptr;
                st.push_back(c);
                c = nxt_c;
            }

            if (u->item != nullptr) {
                delete u->item;
                u->item = nullptr;
            }
            delete u;
        }

        min_node = nullptr;
        n = 0;
    }

    // --- Validación de Invariantes Estructurales ---

    bool validate_invariants() const {
        if (n == 0) {
            return min_node == nullptr;
        }
        if (!min_node) return false;
        if (!min_node->item) return false;
        if (min_node->key != min_node->item->key) return false;

        size_t live_count = 0;
        std::vector<DagNode*> roots;
        DagNode* curr = min_node;
        do {
            roots.push_back(curr);
            if (curr->item && curr->key < min_node->key) {
                return false;
            }
            curr = curr->next;
        } while (curr != min_node && curr != nullptr);

        for (DagNode* r : roots) {
            if (!validate_tree(r, live_count)) return false;
        }

        return live_count == n;
    }
};
