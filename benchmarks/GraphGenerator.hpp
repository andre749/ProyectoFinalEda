#pragma once
//
// Generador de Grafos Sintéticos para Benchmarking
//
#include <random>
#include <vector>
#include <algorithm>
#include "Graph.hpp"

class GraphGenerator {
public:
    // Genera un grafo conexo disperso con E ≈ V * avg_degree
    static Graph<int> generate_connected_sparse(int V, int avg_degree, int max_weight = 1000, unsigned seed = 42) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> weight_dist(1, max_weight);
        std::uniform_int_distribution<int> node_dist(0, V - 1);

        Graph<int> G(V, false);

        // 1. Árbol generador aleatorio para garantizar conexidad
        std::vector<int> perm(V);
        for (int i = 0; i < V; ++i) perm[i] = i;
        std::shuffle(perm.begin(), perm.end(), rng);

        for (int i = 1; i < V; ++i) {
            std::uniform_int_distribution<int> parent_dist(0, i - 1);
            int parent_idx = parent_dist(rng);
            int u = perm[i];
            int p = perm[parent_idx];
            G.add_edge(u, p, weight_dist(rng));
        }

        // 2. Agregar aristas adicionales hasta alcanzar el grado promedio
        long long target_edges = (long long)V * avg_degree / 2;
        while ((long long)G.E() < target_edges) {
            int u = node_dist(rng);
            int v = node_dist(rng);
            if (u != v) {
                G.add_edge(u, v, weight_dist(rng));
            }
        }

        return G;
    }

    // Genera un grafo conexo denso con densidad d ∈ (0, 1]
    static Graph<int> generate_dense(int V, double density, int max_weight = 1000, unsigned seed = 42) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> weight_dist(1, max_weight);
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

        Graph<int> G(V, false);

        // Garantizar conexidad con un árbol
        for (int i = 1; i < V; ++i) {
            std::uniform_int_distribution<int> p_dist(0, i - 1);
            int p = p_dist(rng);
            G.add_edge(i, p, weight_dist(rng));
        }

        // Agregar aristas aleatorias según densidad
        for (int u = 0; u < V; ++u) {
            for (int v = u + 1; v < V; ++v) {
                if (prob_dist(rng) < density) {
                    G.add_edge(u, v, weight_dist(rng));
                }
            }
        }

        return G;
    }

    // Genera una malla regular 2D (Grid) de R filas x C columnas
    static Graph<int> generate_grid_2d(int R, int C, int max_weight = 1000, unsigned seed = 42) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> weight_dist(1, max_weight);

        int V = R * C;
        Graph<int> G(V, false);

        auto idx = [C](int r, int c) { return r * C + c; };

        for (int r = 0; r < R; ++r) {
            for (int c = 0; c < C; ++c) {
                int u = idx(r, c);
                if (r + 1 < R) {
                    G.add_edge(u, idx(r + 1, c), weight_dist(rng));
                }
                if (c + 1 < C) {
                    G.add_edge(u, idx(r, c + 1), weight_dist(rng));
                }
            }
        }

        return G;
    }
};
