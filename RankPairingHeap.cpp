#pragma once
//
// Rank-Pairing Heap — Tipo 2 (Haeupler, Sen & Tarjan — SIAM J. Comput. 2011)
//
// Representación: colección de half-trees (árboles binarios half-ordered).
//   left   -> primer hijo (hijo más a la izquierda en el árbol multi-vía)
//   right  -> siguiente hermano (para no-raíces) / siguiente raíz en la
//             lista circular (para raíces — las raíces de half-trees no
//             tienen hermano, así que el puntero right queda libre)
//   parent -> padre en el árbol binario (nullptr si es raíz)
//   rank   -> rango del nodo (entero no negativo)
//
// Regla de rango Tipo 2:
//   - Toda raíz es un 1-nodo: rank = rank(left) + 1.
//   - Todo hijo (no raíz) es un nodo (1,1), (1,2), (2,1),
//     (0,i) con i > 1, o (i,0) con i > 1.
//   donde (i,j) indica las diferencias de rango con los hijos izq y der.
//   Un hijo faltante se trata con rank = −1.
//
// Complejidades amortizadas:
//   insert, find_min, meld, decrease_key  :  O(1)
//   erase_min, erase                      :  O(log n)
//

#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cassert>
#include <cstdlib>

template<typename Tk, typename Tv>
class RankPairingHeap {
public:
    struct Node {
        Tk key;
        Tv* value;
        int rank = 0;
        Node* left   = nullptr;   // primer hijo
        Node* right  = nullptr;   // sig. hermano (no-raíz) / sig. raíz (raíz)
        Node* parent = nullptr;   // padre en el árbol binario

        Node(Tk k, Tv* v) : key(k), value(v) {}
    };

private:
    Node* min_root = nullptr;   // raíz con llave mínima (cabeza de lista circ.)
    size_t n = 0;

    static constexpr size_t MAX_RANK = 128;

    // -----------------------------------------------------------------
    //  Enlace (link) de dos half-trees de igual rango
    //  El ganador (menor llave) absorbe al perdedor como primer hijo.
    //  Retorna la nueva raíz con rango incrementado en 1.
    // -----------------------------------------------------------------
    static Node* link(Node* a, Node* b) {
        if (b->key < a->key) std::swap(a, b);
        // b pierde: el subárbol izq. de a pasa a ser el hermano der. de b
        b->right = a->left;
        if (a->left) a->left->parent = b;
        // b se convierte en el primer hijo de a
        a->left = b;
        b->parent = a;
        a->rank++;
        return a;
    }

    // -----------------------------------------------------------------
    //  Insertar una raíz en la lista circular de raíces en O(1)
    // -----------------------------------------------------------------
    void add_root(Node* r) {
        r->parent = nullptr;
        if (!min_root) {
            min_root = r;
            r->right = r;                  // auto-enlace circular
        } else {
            r->right = min_root->right;
            min_root->right = r;
            if (r->key < min_root->key) {
                min_root = r;
            }
        }
    }

    // -----------------------------------------------------------------
    //  Reducción de rango en cascada (regla Tipo 2)
    //  Tras un corte, actualiza los rangos ascendiendo hasta que no haya
    //  violación o se alcance una raíz.
    // -----------------------------------------------------------------
    void cascade_rank_fix(Node* p) {
        while (p != nullptr) {
            int old_rank = p->rank;
            int new_rank;
            int l = p->left ? p->left->rank : -1;

            if (p->parent == nullptr) {
                // p es una raíz: rank = rank(left) + 1
                new_rank = l + 1;
                if (new_rank < old_rank) p->rank = new_rank;
                break;   // no hay ancestro al que propagar
            }

            // p no es raíz: regla Tipo 2
            int r = p->right ? p->right->rank : -1;
            int diff = l - r;
            if (diff < 0) diff = -diff;          // |l − r|

            if (diff <= 1)
                new_rank = std::max(l, r) + 1;
            else
                new_rank = std::max(l, r);

            if (new_rank >= old_rank) break;      // invariante OK

            p->rank = new_rank;
            p = p->parent;
        }
    }

    // -----------------------------------------------------------------
    //  Validación recursiva del subárbol binario (para testing / debug)
    //  mw_parent_key = llave del padre en el árbol multi-vía.
    // -----------------------------------------------------------------
    bool validate_subtree(Node* x, Tk mw_parent_key, size_t& count) const {
        if (!x) { count = 0; return true; }
        count = 1;

        // Orden de montículo: llave ≥ llave del padre multi-vía
        if (x->key < mw_parent_key) return false;

        // Verificar regla de rango Tipo 2 para no-raíces
        int lr = x->left  ? x->left->rank  : -1;
        int rr = x->right ? x->right->rank : -1;
        int i_diff = x->rank - lr;
        int j_diff = x->rank - rr;

        if (i_diff < 0 || j_diff < 0) return false;

        bool valid = false;
        if (i_diff == 1 && j_diff == 1) valid = true;   // (1,1)
        if (i_diff == 1 && j_diff == 2) valid = true;   // (1,2)
        if (i_diff == 2 && j_diff == 1) valid = true;   // (2,1)
        if (i_diff == 0 && j_diff >  1) valid = true;   // (0,i), i>1
        if (j_diff == 0 && i_diff >  1) valid = true;   // (i,0), i>1
        if (!valid) return false;

        // Punteros padre consistentes
        if (x->left  && x->left->parent  != x) return false;
        if (x->right && x->right->parent != x) return false;

        // Hijo izquierdo: es hijo de x en el árbol multi-vía
        size_t lc = 0;
        if (!validate_subtree(x->left, x->key, lc)) return false;
        count += lc;

        // Hijo derecho: es hermano de x, comparte padre multi-vía
        size_t rc = 0;
        if (!validate_subtree(x->right, mw_parent_key, rc)) return false;
        count += rc;

        return true;
    }

public:
    RankPairingHeap() = default;

    RankPairingHeap(const RankPairingHeap&) = delete;
    RankPairingHeap& operator=(const RankPairingHeap&) = delete;

    RankPairingHeap(RankPairingHeap&& other) noexcept
        : min_root(other.min_root), n(other.n) {
        other.min_root = nullptr;
        other.n = 0;
    }

    RankPairingHeap& operator=(RankPairingHeap&& other) noexcept {
        if (this != &other) {
            clear();
            min_root = other.min_root;
            n = other.n;
            other.min_root = nullptr;
            other.n = 0;
        }
        return *this;
    }

    ~RankPairingHeap() { clear(); }

    // ========================== Consultas ============================

    bool   empty()    const { return min_root == nullptr; }
    size_t size()     const { return n; }

    Tv* find_min() const { return min_root ? min_root->value : nullptr; }
    Tv* get_min()  const { return find_min(); }

    Node* min_node() const { return min_root; }

    Tk min_key() const {
        if (!min_root) throw std::runtime_error("Heap is empty");
        return min_root->key;
    }

    // ==================== Operaciones principales ====================

    // --- Insertar un elemento. Retorna un puntero al nodo. ---
    Node* insert(Tk key, Tv* value) {
        Node* x = new Node(key, value);
        add_root(x);
        n++;
        return x;
    }

    // --- Fusionar otro heap en éste (destructivo para other). ---
    void meld(RankPairingHeap& other) {
        if (this == &other || !other.min_root) return;
        if (!min_root) {
            min_root = other.min_root;
            n = other.n;
        } else {
            // Concatenar las dos listas circulares de raíces
            Node* tmp = min_root->right;
            min_root->right = other.min_root->right;
            other.min_root->right = tmp;
            if (other.min_root->key < min_root->key)
                min_root = other.min_root;
            n += other.n;
        }
        other.min_root = nullptr;
        other.n = 0;
    }

    // --- Extraer el mínimo. Retorna el valor asociado. ---
    Tv* erase_min() {
        if (!min_root) return nullptr;

        Node* r = min_root;
        Tv* val = r->value;

        // 1) Recolectar las demás raíces (excluyendo r)
        std::vector<Node*> all_roots;
        if (r->right != r) {
            Node* curr = r->right;
            while (curr != r) {
                all_roots.push_back(curr);
                curr = curr->right;
            }
        }

        // 2) Desensamblar: cada nodo en la espina derecha de r->left
        //    se convierte en una nueva raíz de half-tree.
        Node* c = r->left;
        while (c != nullptr) {
            Node* next = c->right;
            c->right  = nullptr;
            c->parent = nullptr;
            c->rank   = (c->left ? c->left->rank + 1 : 0);
            all_roots.push_back(c);
            c = next;
        }

        delete r;
        n--;

        if (all_roots.empty()) {
            min_root = nullptr;
            return val;
        }

        // 3) Consolidar: enlazar raíces de igual rango mediante buckets
        std::array<Node*, MAX_RANK> buckets;
        buckets.fill(nullptr);

        for (Node* x : all_roots) {
            x->parent = nullptr;
            while (x->rank < (int)MAX_RANK && buckets[x->rank] != nullptr) {
                Node* y = buckets[x->rank];
                buckets[x->rank] = nullptr;
                x = link(x, y);
            }
            if (x->rank < (int)MAX_RANK)
                buckets[x->rank] = x;
        }

        // 4) Reconstruir la lista circular y encontrar el nuevo mínimo
        min_root = nullptr;
        for (size_t i = 0; i < MAX_RANK; ++i) {
            if (buckets[i] != nullptr) {
                Node* x = buckets[i];
                x->parent = nullptr;
                if (!min_root) {
                    min_root = x;
                    x->right = x;
                } else {
                    x->right = min_root->right;
                    min_root->right = x;
                    if (x->key < min_root->key)
                        min_root = x;
                }
            }
        }

        return val;
    }

    Tv* extract_min() { return erase_min(); }
    Tv* delete_min()  { return erase_min(); }

    // --- Decrementar la llave de un nodo a new_key (≤ key actual). ---
    void decrease_key(Node* x, Tk new_key) {
        if (!x) return;
        if (new_key > x->key)
            throw std::invalid_argument(
                "New key is greater than current key in decrease_key");

        x->key = new_key;

        if (x->parent == nullptr) {
            // x es una raíz: solo actualizar min_root si es necesario
            if (x->key < min_root->key)
                min_root = x;
            return;
        }

        // x no es raíz: cortar x de su padre en el árbol binario
        Node* p = x->parent;
        Node* y = x->right;          // hermano siguiente de x

        if (p->left == x)
            p->left = y;
        else
            p->right = y;
        if (y) y->parent = p;

        // x se convierte en raíz de half-tree
        x->right  = nullptr;
        x->parent = nullptr;
        x->rank   = (x->left ? x->left->rank + 1 : 0);

        add_root(x);

        // Cascada de reducción de rango desde el antiguo padre
        cascade_rank_fix(p);
    }

    void decrease_key_delta(Node* x, Tk delta) {
        if (delta < 0)
            throw std::invalid_argument(
                "Delta cannot be negative in decrease_key_delta");
        decrease_key(x, x->key - delta);
    }

    // --- Eliminar un nodo arbitrario. ---
    void erase(Node* x) {
        if (!x || !min_root) return;
        if (x == min_root) { erase_min(); return; }
        decrease_key(x, std::numeric_limits<Tk>::lowest());
        min_root = x;
        erase_min();
    }

    // --- Liberar toda la memoria. ---
    void clear() {
        if (!min_root) return;

        // Recolectar todas las raíces de la lista circular
        std::vector<Node*> roots;
        Node* curr = min_root;
        do {
            roots.push_back(curr);
            curr = curr->right;
        } while (curr != min_root);

        // Liberar cada half-tree mediante recorrido iterativo
        for (Node* r : roots) {
            std::vector<Node*> st;
            if (r->left) st.push_back(r->left);
            delete r;

            while (!st.empty()) {
                Node* x = st.back(); st.pop_back();
                if (x->left)  st.push_back(x->left);
                if (x->right) st.push_back(x->right);
                delete x;
            }
        }

        min_root = nullptr;
        n = 0;
    }

    // ============= Validación de invariantes (Testing / Debug) ==============

    bool validate_invariants() const {
        if (n == 0) return min_root == nullptr;
        if (!min_root) return false;

        size_t total_nodes = 0;
        size_t root_count  = 0;
        Node* curr = min_root;

        do {
            root_count++;
            if (root_count > n) return false;           // lista corrupta

            if (curr->parent != nullptr) return false;  // toda raíz: parent==null
            if (curr->key < min_root->key) return false;// min_root es mínima

            // Regla de rango para raíces: rank = rank(left) + 1
            int l = curr->left ? curr->left->rank : -1;
            if (curr->rank != l + 1) return false;

            // Puntero padre del hijo izquierdo
            if (curr->left && curr->left->parent != curr) return false;

            // Contar y validar el subárbol del half-tree
            size_t subtree_count = 1;   // la raíz misma
            if (curr->left) {
                size_t lc = 0;
                if (!validate_subtree(curr->left, curr->key, lc))
                    return false;
                subtree_count += lc;
            }
            total_nodes += subtree_count;

            curr = curr->right;
        } while (curr != min_root);

        return total_nodes == n;
    }
};
