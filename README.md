# Comparación Experimental de Colas de Prioridad

Proyecto final para el análisis y comparación experimental de colas de prioridad: **Fibonacci Heap**, **Hollow Heap**, **Pairing Heap** y **Rank-Pairing Heap**, con aplicaciones a los algoritmos de Dijkstra y Prim.

---

## Estructuras Implementadas

1. **Fibonacci Heap (`FibonacciHeap.cpp`)**: Implementación basada en Fredman & Tarjan (1987) y la especificación de `fibo.md`. Utiliza listas doblemente enlazadas circulares, cortes en cascada, verificación de invariantes y consolidación eficiente mediante arreglo estático en memoria de pila.
2. **Pairing Heap Original (`PairHeap.cpp`)**: Representación clásica *child/sibling/prev* en árbol binario con cortes y combinación en dos pasadas.
3. **Pairing Heap Optimizado (`PairHeap2.cpp`)**: Implementación corregida y estandarizada con la interfaz unificada del proyecto (`decrease_key(Node* x, Tk new_key)` en lugar de delta, corrección de puntero nulo en raíz tras `erase`, validación de invariantes y semántica de movimiento).
4. **Hollow Heap Original (`HollowHeap.cpp`)**: Implementación inicial de montículo hollow con múltiples raíces.
5. **Hollow Heap Optimizado (`HollowHeap2.cpp`)**: Reimplementación robusta y eficiente de montículos hollow (*Multi-Root 1-Parent*) basada en Hansen, Kaplan, Tarjan & Zwick (2017).
6. **Rank-Pairing Heap (`RankPairingHeap.cpp`)**: Implementación de montículos *Rank-Pairing* (Tipo 2) basada en Haeupler, Sen & Tarjan (2011). Emplea representación en *half-trees* binarios con a lo más dos punteros por nodo interno (`left` y `right`), enlace $O(1)$, reducción de rangos en cascada con regla Tipo 2 ($O(1)$ amortizado para `decrease_key`), y consolidación por cubetas estáticas en $O(\log n)$ durante `erase_min`.

---

## Auditoría y Hallazgos sobre `HollowHeap` (Original)

Durante la auditoría teórica y empírica del archivo `HollowHeap.cpp`, se identificaron discrepancias teóricas, problemas de estabilidad y cuellos de botella de rendimiento frente a la especificación de Hansen et al. (2017):

### 1. Pérdida del elemento tras múltiples `decrease_key` (Crítico para Dijkstra/Prim)
* **Problema:** En la teoría de Hollow Heaps, al decrementar una llave se crea un nodo nuevo y el nodo anterior se convierte en hueco (`node->item = nullptr`). En la implementación original, `decrease_key(Node* node, Tk delta)` retorna `void` y no devuelve ni actualiza la referencia del usuario. Si un algoritmo de grafos relaja dos o más veces la distancia de un mismo vértice, la segunda llamada opera sobre el nodo viejo que ya tiene `item == nullptr`, creando un nuevo nodo con `item = nullptr` que es descartado como hueco y provocando la pérdida del dato.
* **Prueba:** Caso documentado en `test/test_hollow_audit.cpp` (`audit_double_decrease_issue`).

### 2. *Crash* por puntero nulo (*Null Pointer Dereference*) en `meld` con montículo vacío
* **Problema:** Si `this->roots_number == 0`, `last_root` es `nullptr`. En la línea 175 de `HollowHeap.cpp`:
  ```cpp
  last_root->next = other->roots;
  ```
  se intenta desreferenciar `last_root`, provocando una falla de segmentación inmediata (`Segmentation Fault`). Además, si `other->roots_number == 0`, se retorna antes de ejecutar `delete other;`, fugando la memoria del objeto montículo.

### 3. Fuga de memoria y sobreasignación dinámica por enlace (*Anti-patrón de asignación*)
* **Problema:** 
  - Emplea una estructura envoltorio auxiliar `List<Node>`. Cada enlace de nodos en `absorb` invoca `new List<Node>`, cada inserción de raíz invoca `new List<Node>`, y en `delete_min()` se crea dinámicamente una instancia completa `new HollowHeap()` por cada raíz hueca encontrada.
  - La clase carece de un destructor `~HollowHeap()`.
  - En `_insert_root()`, cuando `root->is_hollow()` es `true`, los hijos se procesan pero `delete root;` nunca se ejecuta.

### 4. Condición de parada incompleta en el recorrido de raíces huecas de `delete_min()`
* **Problema:** El bucle `while (it != old_last)` nunca procesa a `old_last`. Si `old_last` quedó en estado *hollow* (por ejemplo, tras un `erase()` sobre la última raíz), no se desmantela en la pasada de limpieza y es ingresado indebidamente al vector de enlazado por rango, donde un nodo hueco puede llegar a absorber o ser absorbido por un nodo con valor vivo.

### 5. Comprobación de límites inválida en el vector de rangos
* **Problema:** En `recursive_insert_in_vector()`, se evalúa:
  ```cpp
  if (rank >= v.capacity()) v.resize(rank + 1);
  ```
  En C++, `v.capacity()` puede ser mayor que `v.size()`. Si `v.size() <= rank < v.capacity()`, `v.resize()` no se ejecuta y la indexación posterior `v[rank]` genera un acceso fuera de los límites (*out-of-bounds* / comportamiento indefinido).

---

## Soluciones implementadas en `HollowHeap2.cpp` y `PairHeap2.cpp`

Para resolver todas las limitaciones anteriores y homogeneizar las estructuras:
* **`HollowHeap2.cpp`**: Punteros intrínsecos `child`/`next`, separación entre `Node` (item) y `DagNode` para soportar múltiples `decrease_key` consecutivos sin perder identidad, arreglo estático en pila para enlazado por rangos y desmantelamiento exhaustivo de nodos huecos con RAII.
* **`PairHeap2.cpp`**: Adopción de la firma estándar `decrease_key(Node* x, Tk new_key)` en lugar de delta, corrección de puntero colgante en `root->prev` tras `erase()`, métodos auxiliares de consulta (`min_key()`, `get_min()`, `min_node()`, `extract_min()`) y validación de invariantes.

---

## Ejecución de Pruebas Unitarias

Las pruebas unitarias y de estrés se encuentran organizadas en el directorio `test/`:

```powershell
# Compilar y ejecutar pruebas de Fibonacci Heap
g++ -O3 -std=c++17 -Wall -Wextra test/test_fibo.cpp -o test_fibo.exe
.\test_fibo.exe

# Compilar y ejecutar pruebas de Hollow Heap 2
g++ -O3 -std=c++17 -Wall -Wextra test/test_hollow2.cpp -o test_hollow2.exe
.\test_hollow2.exe

# Compilar y ejecutar pruebas de Pairing Heap 2
g++ -O3 -std=c++17 -Wall -Wextra test/test_pairheap.cpp -o test_pairheap.exe
.\test_pairheap.exe

# Compilar y ejecutar pruebas de Rank-Pairing Heap
g++ -O3 -std=c++17 -Wall -Wextra test/test_rankpairing.cpp -o test_rankpairing.exe
.\test_rankpairing.exe

# Compilar y ejecutar pruebas de auditoría sobre Hollow Heap Original
g++ -std=c++17 -Wall -Wextra test/test_hollow_audit.cpp -o test_hollow_audit.exe
.\test_hollow_audit.exe

# Ejecutar test conjunto básico
g++ -std=c++17 -Wall -Wextra Heaps.cpp -o Heaps.exe
.\Heaps.exe
```

---

## Benchmarks Experimentales (Dijkstra y Prim)

El directorio `benchmarks/` contiene la suite de benchmarking que evalúa las cuatro estructuras sobre algoritmos de grafos (Dijkstra y Prim) bajo distintas topologías de red:
* **Grafos Conexos Dispersos** ($|V| \in [5k, 50k]$, $|E| \in [25k, 375k]$)
* **Grafos Densos** ($|V| \in [1k, 2k]$, densidad hasta 25%, $|E| \approx 500k$)
* **Mallas Regulares 2D (Grid Graphs)** ($100 \times 100$, $200 \times 200$)

### Cómo ejecutar los benchmarks:

```powershell
g++ -O3 -std=c++17 -Wall -Wextra benchmarks/benchmark.cpp -o benchmark.exe
.\benchmark.exe
```

Los resultados detallados de tiempos de ejecución y conteo de operaciones se exportan automáticamente en formato tabular y en `benchmarks/benchmark_results.csv`.
