//
// Suite de Benchmarking Experimental para Colas de Prioridad
// Comparación entre FibonacciHeap, HollowHeap, PairHeap y RankPairingHeap
//
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>

#include "../FibonacciHeap.cpp"
#include "../HollowHeap.cpp"
#include "../PairHeap.cpp"
#include "../RankPairingHeap.cpp"

#include "Graph.hpp"
#include "GraphGenerator.hpp"
#include "Algorithms.hpp"

struct BenchmarkRow {
    std::string algorithm;
    std::string graph_type;
    int V;
    size_t E;
    double fib_time;
    double hollow_time;
    double pair_time;
    double rp_time;
    size_t dec_keys_fib;
    size_t dec_keys_hollow;
    size_t dec_keys_pair;
    size_t dec_keys_rp;
};

void run_test_case(const std::string& name, const Graph<int>& G, std::vector<BenchmarkRow>& results, std::ofstream& csv) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << ">>> Caso: " << name << " (|V|=" << G.V() << ", |E|=" << G.E() << ") <<<" << std::endl;
    std::cout << "========================================================" << std::endl;

    // --- 1. DIJKSTRA ---
    {
        std::cout << "  Ejecutando Dijkstra..." << std::flush;
        auto res_fib  = GraphAlgorithms<FibonacciHeap<int, int>>::dijkstra(G, 0);
        auto res_hh   = GraphAlgorithms<HollowHeap<int, int>>::dijkstra(G, 0);
        auto res_ph   = GraphAlgorithms<PairHeap<int, int>>::dijkstra(G, 0);
        auto res_rp   = GraphAlgorithms<RankPairingHeap<int, int>>::dijkstra(G, 0);
        std::cout << " OK!" << std::endl;

        // Validar coherencia algorítmica exacta: todas deben producir la misma suma de distancias
        assert(res_fib.checksum == res_hh.checksum);
        assert(res_fib.checksum == res_ph.checksum);
        assert(res_fib.checksum == res_rp.checksum);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "    [Dijkstra] Fib: " << res_fib.time_ms << " ms | "
                  << "Hollow: " << res_hh.time_ms << " ms | "
                  << "Pair: " << res_ph.time_ms << " ms | "
                  << "RankPairing: " << res_rp.time_ms << " ms | "
                  << "Checksum: " << res_fib.checksum << std::endl;

        results.push_back({"Dijkstra", name, G.V(), G.E(),
                           res_fib.time_ms, res_hh.time_ms, res_ph.time_ms, res_rp.time_ms,
                           res_fib.decrease_key_count, res_hh.decrease_key_count,
                           res_ph.decrease_key_count, res_rp.decrease_key_count});

        csv << "Dijkstra," << name << "," << G.V() << "," << G.E() << ","
            << res_fib.time_ms << "," << res_hh.time_ms << "," << res_ph.time_ms << "," << res_rp.time_ms << ","
            << res_fib.decrease_key_count << "," << res_hh.decrease_key_count << ","
            << res_ph.decrease_key_count << "," << res_rp.decrease_key_count << "\n";
    }

    // --- 2. PRIM (MST) ---
    {
        std::cout << "  Ejecutando Prim..." << std::flush;
        auto res_fib  = GraphAlgorithms<FibonacciHeap<int, int>>::prim(G);
        auto res_hh   = GraphAlgorithms<HollowHeap<int, int>>::prim(G);
        auto res_ph   = GraphAlgorithms<PairHeap<int, int>>::prim(G);
        auto res_rp   = GraphAlgorithms<RankPairingHeap<int, int>>::prim(G);
        std::cout << " OK!" << std::endl;

        // Validar coherencia del peso del MST exacto
        assert(res_fib.checksum == res_hh.checksum);
        assert(res_fib.checksum == res_ph.checksum);
        assert(res_fib.checksum == res_rp.checksum);

        std::cout << "    [Prim]     Fib: " << res_fib.time_ms << " ms | "
                  << "Hollow: " << res_hh.time_ms << " ms | "
                  << "Pair: " << res_ph.time_ms << " ms | "
                  << "RankPairing: " << res_rp.time_ms << " ms | "
                  << "MST-Weight: " << res_fib.checksum << std::endl;

        results.push_back({"Prim", name, G.V(), G.E(),
                           res_fib.time_ms, res_hh.time_ms, res_ph.time_ms, res_rp.time_ms,
                           res_fib.decrease_key_count, res_hh.decrease_key_count,
                           res_ph.decrease_key_count, res_rp.decrease_key_count});

        csv << "Prim," << name << "," << G.V() << "," << G.E() << ","
            << res_fib.time_ms << "," << res_hh.time_ms << "," << res_ph.time_ms << "," << res_rp.time_ms << ","
            << res_fib.decrease_key_count << "," << res_hh.decrease_key_count << ","
            << res_ph.decrease_key_count << "," << res_rp.decrease_key_count << "\n";
    }
}

int main() {
    std::cout << "========================================================\n"
              << "   BENCHMARK EXPERIMENTAL: COLAS DE PRIORIDAD (EDA)\n"
              << "========================================================\n";

    std::vector<BenchmarkRow> results;
    std::ofstream csv("benchmarks/benchmark_results.csv");
    csv << "Algorithm,GraphType,V,E,FibonacciHeap_ms,HollowHeap_ms,PairHeap_ms,RankPairingHeap_ms,DecKeys_Fib,DecKeys_Hollow,DecKeys_Pair,DecKeys_RP\n";

    // 1. Grafos Conexos Dispersos Pequeños y Medianos
    {
        auto g1 = GraphGenerator::generate_connected_sparse(5000, 10, 1000, 101);
        run_test_case("Sparse-5k", g1, results, csv);

        auto g2 = GraphGenerator::generate_connected_sparse(20000, 12, 1000, 202);
        run_test_case("Sparse-20k", g2, results, csv);

        auto g3 = GraphGenerator::generate_connected_sparse(50000, 15, 1000, 303);
        run_test_case("Sparse-50k", g3, results, csv);
    }

    // 2. Grafos Densos (donde decrease_key es intensivo)
    {
        auto gd1 = GraphGenerator::generate_dense(1000, 0.20, 1000, 404);
        run_test_case("Dense-1k (p=0.20)", gd1, results, csv);

        auto gd2 = GraphGenerator::generate_dense(2000, 0.25, 1000, 505);
        run_test_case("Dense-2k (p=0.25)", gd2, results, csv);
    }

    // 3. Malla 2D (Grid Graphs, simulación de redes geométricas)
    {
        auto grid1 = GraphGenerator::generate_grid_2d(100, 100, 1000, 606); // 10,000 nodos
        run_test_case("Grid-100x100", grid1, results, csv);

        auto grid2 = GraphGenerator::generate_grid_2d(200, 200, 1000, 707); // 40,000 nodos
        run_test_case("Grid-200x200", grid2, results, csv);
    }

    csv.close();

    // Imprimir Tabla Markdown Resumen
    std::cout << "\n\n========================================================================================\n";
    std::cout << "### TABLA RESUMEN DE TIEMPOS DE EJECUCIÓN (ms)\n";
    std::cout << "========================================================================================\n\n";

    std::cout << "| Algoritmo | Instancia de Grafo | |V| | |E| | Fibonacci (ms) | Hollow (ms) | Pairing (ms) | Rank-Pairing (ms) |\n";
    std::cout << "| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: |\n";

    for (const auto& r : results) {
        std::cout << "| " << r.algorithm
                  << " | " << r.graph_type
                  << " | " << r.V
                  << " | " << r.E
                  << " | " << std::fixed << std::setprecision(2) << r.fib_time
                  << " | " << r.hollow_time
                  << " | " << r.pair_time
                  << " | " << r.rp_time
                  << " |\n";
    }

    std::cout << "\nResultados exportados exitosamente a 'benchmarks/benchmark_results.csv'.\n";
    return 0;
}
