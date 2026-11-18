# Comparación Experimental de Colas de Prioridad

Proyecto final para el análisis, diseño e investigación experimental de cuatro estructuras de datos de cola de prioridad avanzadas: **Fibonacci Heap**, **Hollow Heap**, **Pairing Heap** y **Rank-Pairing Heap**, con evaluación sobre los algoritmos de **Dijkstra** (caminos mínimos) y **Prim** (árbol generador mínimo) en diversas topologías de grafos.

**Autores:**
* Sánchez Domínguez, Matías
* Contreras Valera, André

**Institución:** Universidad de Ingeniería y Tecnología (UTEC) — Ciencia de la Computación  
**Curso:** Estructuras de Datos Avanzadas (EDA)

---

## 1. Estructuras de Datos Implementadas

Todas las estructuras comparten una misma interfaz homogénea basada en plantillas `template<typename Tk, typename Tv>`:

| Estructura | Archivo | Representación | Inserción | Find-Min | Extract-Min | Decrease-Key | Meld |
| :--- | :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **Fibonacci Heap** | `FibonacciHeap.cpp` | Listas circulares doblemente enlazadas, marcas y cortes en cascada | $O(1)$ | $O(1)$ | $O(\log n)$ | $O(1)$ | $O(1)$ |
| **Pairing Heap** | `PairHeap.cpp` | Árbol binario *child / sibling / prev*, combinación en dos pasadas (*two-pass*) | $O(1)$ | $O(1)$ | $O(\log n)$ | $o(\log n)$ | $O(1)$ |
| **Rank-Pairing Heap** | `RankPairingHeap.cpp` | Colección de *half-trees*, regla de rango Tipo 2 | $O(1)$ | $O(1)$ | $O(\log n)$ | $O(1)$ | $O(1)$ |
| **Hollow Heap** | `HollowHeap.cpp` | DAG multi-raíz con eliminación perezosa (*lazy deletion*) | $O(1)$ | $O(1)$ | $O(\log n)$ | $O(1)$ | $O(1)$ |

### Interfaz Unificada

```cpp
Node* insert(Tk key, Tv* value = nullptr);
Tv*   find_min() const;
Tv*   erase_min();           // Retorna el valor y elimina la menor llave (alias: extract_min)
void  decrease_key(Node* x, Tk new_key);
void  erase(Node* x);
void  meld(Heap& other);
bool  empty() const;
size_t size() const;
void  clear();
bool  validate_invariants() const;
```

---

## 2. Estructura del Proyecto

```
ProyectoFinalEda/
├── FibonacciHeap.cpp          # Implementación de Fibonacci Heap
├── HollowHeap.cpp             # Implementación de Hollow Heap (Multi-Root 1-Parent)
├── PairHeap.cpp               # Implementación de Pairing Heap (Two-Pass)
├── RankPairingHeap.cpp        # Implementación de Rank-Pairing Heap (Tipo 2)
├── Heaps.cpp                  # Ejecutable principal de verificación básica
├── CMakeLists.txt             # Configuración de compilación CMake
├── analisis_experimental.tex  # Reporte formal y artículo experimental en LaTeX
│
├── test/                      # Suite de pruebas unitarias y de estrés
│   ├── test_fibo.cpp          # Pruebas y estrés de Fibonacci Heap
│   ├── test_hollow.cpp        # Pruebas y estrés de Hollow Heap
│   ├── test_pairheap.cpp      # Pruebas y estrés de Pairing Heap
│   └── test_rankpairing.cpp   # Pruebas y estrés de Rank-Pairing Heap
│
├── benchmarks/                # Infraestructura experimental
│   ├── Graph.hpp              # Estructura de grafo ponderado
│   ├── GraphGenerator.hpp     # Generadores sintéticos (dispersos, densos y mallas 2D)
│   ├── Algorithms.hpp         # Dijkstra y Prim instrumentados con checksums
│   ├── benchmark.cpp          # Suite de ejecución de benchmarks
│   ├── benchmark_results.csv  # Datos brutos de tiempos y operaciones
│   └── generate_charts.py     # Generador de gráficos con matplotlib
│
└── figures/                   # Gráficos generados para el reporte
    ├── dijkstra_sparse_grid.pdf/.png
    ├── dijkstra_dense.pdf/.png
    ├── prim_sparse_grid.pdf/.png
    ├── prim_dense.pdf/.png
    ├── speedup_vs_fibonacci.pdf/.png
    ├── decrease_key_counts.pdf/.png
    └── memory_footprint.pdf/.png
```

---

## 3. Compilación y Pruebas Unitarias

Cada estructura cuenta con su propio ejecutable de pruebas unitarias y pruebas de estrés aleatorias (10 000 a 20 000 operaciones validadas contra `std::priority_queue`):

```powershell
# Compilar y ejecutar pruebas de Fibonacci Heap
g++ -O3 -std=c++17 -Wall -Wextra test/test_fibo.cpp -o test_fibo.exe
.\test_fibo.exe

# Compilar y ejecutar pruebas de Hollow Heap
g++ -O3 -std=c++17 -Wall -Wextra test/test_hollow.cpp -o test_hollow.exe
.\test_hollow.exe

# Compilar y ejecutar pruebas de Pairing Heap
g++ -O3 -std=c++17 -Wall -Wextra test/test_pairheap.cpp -o test_pairheap.exe
.\test_pairheap.exe

# Compilar y ejecutar pruebas de Rank-Pairing Heap
g++ -O3 -std=c++17 -Wall -Wextra test/test_rankpairing.cpp -o test_rankpairing.exe
.\test_rankpairing.exe

# Ejecutar test conjunto básico
g++ -O3 -std=c++17 -Wall -Wextra Heaps.cpp -o Heaps.exe
.\Heaps.exe
```

---

## 4. Ejecución de Benchmarks Experimentales

La suite de benchmarks evalúa los algoritmos de Dijkstra y Prim sobre diferentes topologías:
* **Grafos Dispersos Conexos:** $|V| \in [5\,000, 50\,000]$, $|E|$ hasta $375\,000$.
* **Grafos Densos:** $|V| \in [1\,000, 2\,000]$, $p \in [0.20, 0.25]$, $|E| \approx 501\,686$.
* **Mallas Regulares 2D:** $100 \times 100$ ($|V| = 10\,000$) y $200 \times 200$ ($|V| = 40\,000$).

### Ejecutar benchmarks:

```powershell
g++ -O3 -std=c++17 -Wall -Wextra benchmarks/benchmark.cpp -o benchmark.exe
.\benchmark.exe
```

Los resultados se exportan automáticamente en formato de tabla y a `benchmarks/benchmark_results.csv`.

### Generación de Gráficos:

```powershell
python benchmarks/generate_charts.py
```

Genera todas las figuras vectoriales (`.pdf`) y rasterizadas (`.png`) en el directorio `figures/`.

---

## 5. Reporte Formal en LaTeX

El reporte experimental detallado se encuentra en `analisis_experimental.tex` y contiene:
1. **Marco Teórico y Complejidades Asintóticas** en peor caso y amortizadas.
2. **Representación en Memoria y Arquitectura:** Comparativa de bytes por nodo en 64 bits (Fibonacci: 64B, Pairing: 40B, Rank-Pairing: 48B, Hollow: 24B + 40B).
3. **Análisis de Jerarquía de Caché:** Impacto de la dispersión de punteros (*pointer chasing*), fallos de caché fríos por cortes en cascada, empaquetamiento en líneas de caché L1 de 64 bytes y consolidación mediante cubetas en memoria de pila (`std::array<Node*, 128>`).
4. **Discusión (Teoría vs. Práctica):** Por qué el Pairing Heap domina en tiempo real a pesar de su cota subóptima teórica, y la competitividad de Rank-Pairing Heap en grafos con alta densidad de aristas.
