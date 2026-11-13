//
// Created by andre on 11/11/26.
//
#include <iostream> // no me dejabaa usar size t xdddddd
#include <vector>
using namespace std;
template<typename T>
struct List {
    T* node;
    List* next;
    List(T* n):node(n),next(nullptr){}

};

template<typename Tk,typename Ti>

struct HollowHeap {

    struct Node {
        size_t rank;
        List<Node>* kids;      // linked list ordered by rank in decreasing order
        Ti* item;
        Tk key;
        Node(Tk k,Ti* item):rank(0),kids(nullptr),item(item),key(k) {}
         void absorb(Node* loser) {
        rank++;
        List<Node>* temp=kids;
        kids=new List<Node>(loser);
        kids->next=temp;
        }

        bool is_hollow() {
            return !item;
        }


    };



    List<Node>* roots, *last_root;   //circular linked list pointing to the minimun nodes
    size_t roots_number;

    HollowHeap():roots(nullptr),last_root(nullptr),roots_number(0){}

    Node* _ranked_link(Node* a, Node* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->rank!=b->rank) {
            cout<<"Ranked link entre nodos de diferentes rangos";
            return nullptr;
        }
        if (a->key<b->key) {
            a->absorb(b);
            return a;
        }
        b->absorb(a);
        return b;
    }
    void _insert_root(Node* root) {
        if (root->is_hollow()) {
            auto current_kid=root->kids;
            while (current_kid) {
                auto prev=current_kid;
                _insert_root(current_kid->node);
                current_kid=current_kid->next;
                delete prev;
            }
            return;
        }

        auto ln=new List<Node>(root);
        roots_number++;
        if (roots_number==1) {
            roots=ln;
            roots->next=roots;
            last_root=ln;
            return;
        }
        if (root->key<roots->node->key) {
            ln->next=roots;
            roots=ln;
            last_root->next=ln;
            return;
        }
        last_root->next=ln;
        ln->next=roots;
        last_root=ln;

    }

    Node* insert(Tk key,Ti* item) {
        auto node=new Node(key,item);
        _insert_root(node);
        return node;
    }

    Ti* min() {
        return roots->node->item;
    }

    bool empty() const {
        return roots_number==0;
    }

    HollowHeap* pop_root(List<Node>*prev,List<Node>* root) {
        //root->node->item=nullptr;
        auto new_hh=new HollowHeap();
        new_hh->_insert_root(root->node);

        roots_number--;
        prev->next=root->next;
        if (root==roots) roots=root->next;
        if (root==last_root) last_root=prev;
        delete root->node;
        delete root;
        return new_hh;
    }

    void recursive_insert_in_vector(vector<Node*> &v,Node* node) {
        auto rank=node->rank;
        if (rank>=v.capacity())v.resize(rank+1);
        if (v[rank]) {
            recursive_insert_in_vector(v,_ranked_link(v[rank],node));
            v[rank]=nullptr;
            return;
        }
        v[rank]=node;
        //_insert_root(node);
    }
    void delete_min() {
        auto it=roots,
        old_last=last_root
        , prev=last_root;
        roots->node->item=nullptr;
        vector<HollowHeap*> heaps;
        //heaps.push_back(this);
        while (it!=old_last) {   //elimino todas las roots hollows
            if (it->node->is_hollow()) {
                heaps.push_back(pop_root(prev,it));
                it=prev->next;
            }else {
                prev=it;
                it=it->next;
            }
        }

        for (auto heap:heaps) meld(heap);
        vector<Node*> v(roots_number,nullptr);

         it=roots;
         recursive_insert_in_vector(v,last_root->node);
        old_last=last_root;

        int cont=1;
        while (it!=last_root) {
            recursive_insert_in_vector(v,it->node);
            prev=it;
            it=it->next;
            delete prev;
            cont++;
        }
        roots=nullptr;
        last_root=nullptr;
        roots_number=0;

        delete old_last;
        for (auto n:v) if (n) _insert_root(n);

    }

    void meld(HollowHeap* other) {
        if (!other or other->roots_number==0) return ;
        roots_number+=other->roots_number;
        last_root->next=other->roots;
        other->last_root->next=roots;
        if (roots->node->key<other->roots->node->key) {
            last_root=other->last_root;
        }else {
           roots=other->roots;
        }
        delete other;
    }

    void erase(Node* node) {
        if (node==roots->node) {
            return delete_min();
        }
        node->item=nullptr;
    }

    void decrease_key(Node* node, Tk delta) {
        if (delta<0) {
            cout<<"Tienes que decrementar un valor positivo de la llave/n";
            return;
        }
        auto new_node=new Node(node->key-delta,node->item);
        node->item=nullptr;
        auto cutting_point=node->kids;
        if (cutting_point) {
            cutting_point=cutting_point->next;
        }
        if (cutting_point) {
            new_node->kids=cutting_point->next;
            cutting_point->next=nullptr;
        }
        if (new_node->kids) {
            new_node->rank=1+new_node->kids->node->rank;
        }
        _insert_root(new_node);
    }


};



