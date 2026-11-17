#pragma once
//
// Implementación Genérica de Dijkstra y Prim con Instrumentación de Operaciones
//
#include <vector>
#include <chrono>
#include <limits>
#include <cstdint>
#include "Graph.hpp"

struct AlgorithmResult {
    double time_ms = 0.0;
    size_t insert_count = 0;
    size_t decrease_key_count = 0;
    size_t extract_min_count = 0;
    long long checksum = 0; // Suma de distancias o peso total del MST
};

template<typename PriorityQueue>
class GraphAlgorithms {
public:
    // Algoritmo de Dijkstra para Caminos Mínimos desde una Fuente Única
    static AlgorithmResult dijkstra(const Graph<int>& G, int source = 0) {
        int V = G.V();
        const int INF = std::numeric_limits<int>::max();
        std::vector<int> dist(V, INF);
        std::vector<int> vertex_ids(V);
        for (int i = 0; i < V; ++i) vertex_ids[i] = i;

        using NodeHandle = typename PriorityQueue::Node*;
        std::vector<NodeHandle> handles(V, nullptr);

        PriorityQueue pq;
        AlgorithmResult res;

        auto t_start = std::chrono::high_resolution_clock::now();

        dist[source] = 0;
        handles[source] = pq.insert(0, &vertex_ids[source]);
        res.insert_count++;

        while (!pq.empty()) {
            int* min_ptr = pq.erase_min();
            res.extract_min_count++;
            if (!min_ptr) break;

            int u = *min_ptr;
            handles[u] = nullptr; // Marcado como finalizado

            for (const auto& edge : G.neighbors(u)) {
                int v = edge.to;
                int w = edge.weight;

                if (dist[u] != INF && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    if (handles[v] == nullptr) {
                        // Si aún no está en la cola y no ha sido finalizado
                        handles[v] = pq.insert(dist[v], &vertex_ids[v]);
                        res.insert_count++;
                    } else {
                        // Relajación mediante decrease_key
                        pq.decrease_key(handles[v], dist[v]);
                        res.decrease_key_count++;
                    }
                }
            }
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        res.time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        // Checksum: suma de distancias alcanzables
        long long sum = 0;
        for (int i = 0; i < V; ++i) {
            if (dist[i] != INF) sum += dist[i];
        }
        res.checksum = sum;

        return res;
    }

    // Algoritmo de Prim para Árbol Generador Mínimo (MST)
    static AlgorithmResult prim(const Graph<int>& G) {
        int V = G.V();
        const int INF = std::numeric_limits<int>::max();
        std::vector<int> key(V, INF);
        std::vector<bool> in_mst(V, false);
        std::vector<int> vertex_ids(V);
        for (int i = 0; i < V; ++i) vertex_ids[i] = i;

        using NodeHandle = typename PriorityQueue::Node*;
        std::vector<NodeHandle> handles(V, nullptr);

        PriorityQueue pq;
        AlgorithmResult res;

        auto t_start = std::chrono::high_resolution_clock::now();

        // Inicializar con el vértice 0
        key[0] = 0;
        for (int i = 0; i < V; ++i) {
            handles[i] = pq.insert(key[i], &vertex_ids[i]);
            res.insert_count++;
        }

        long long total_mst_weight = 0;

        while (!pq.empty()) {
            int* min_ptr = pq.erase_min();
            res.extract_min_count++;
            if (!min_ptr) break;

            int u = *min_ptr;
            in_mst[u] = true;
            handles[u] = nullptr;
            if (key[u] != INF) {
                total_mst_weight += key[u];
            }

            for (const auto& edge : G.neighbors(u)) {
                int v = edge.to;
                int w = edge.weight;

                if (!in_mst[v] && handles[v] != nullptr && w < key[v]) {
                    key[v] = w;
                    pq.decrease_key(handles[v], w);
                    res.decrease_key_count++;
                }
            }
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        res.time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        res.checksum = total_mst_weight;

        return res;
    }
};
