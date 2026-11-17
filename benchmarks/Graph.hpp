#pragma once
//
// Representación de Grafos Ponderados para Benchmarks de Dijkstra y Prim
//
#include <vector>
#include <cstddef>
#include <iostream>

template<typename Weight = int>
struct Edge {
    int to;
    Weight weight;
};

template<typename Weight = int>
class Graph {
private:
    int num_vertices;
    bool is_directed;
    std::vector<std::vector<Edge<Weight>>> adj;
    size_t edge_count = 0;

public:
    Graph(int v = 0, bool directed = false)
        : num_vertices(v), is_directed(directed), adj(v) {}

    int V() const { return num_vertices; }
    size_t E() const { return edge_count; }
    bool directed() const { return is_directed; }

    void add_edge(int u, int v, Weight w) {
        if (u < 0 || u >= num_vertices || v < 0 || v >= num_vertices) return;
        adj[u].push_back({v, w});
        edge_count++;
        if (!is_directed && u != v) {
            adj[v].push_back({u, w});
        }
    }

    const std::vector<Edge<Weight>>& neighbors(int u) const {
        return adj[u];
    }
};
