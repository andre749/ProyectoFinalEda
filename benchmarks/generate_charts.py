#!/usr/bin/env python3
"""Generate publication-quality charts from benchmark CSV data."""

import csv
import os
import matplotlib
matplotlib.use('Agg')  # Non-interactive backend
import matplotlib.pyplot as plt
import numpy as np

# Read CSV data
csv_path = os.path.join(os.path.dirname(__file__), 'benchmark_results.csv')
rows = []
with open(csv_path, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        rows.append(row)

# Color palette (colorblind-friendly)
COLORS = {
    'Fibonacci': '#4477AA',
    'Hollow': '#CC6677',
    'Pairing': '#228B22',
    'Rank-Pairing': '#EE7733',
}
HEAPS = ['Fibonacci', 'Hollow', 'Pairing', 'Rank-Pairing']
COL_MAP = {
    'Fibonacci': 'FibonacciHeap_ms',
    'Hollow': 'HollowHeap_ms',
    'Pairing': 'PairHeap_ms',
    'Rank-Pairing': 'RankPairingHeap_ms',
}

plt.rcParams.update({
    'font.size': 11,
    'font.family': 'serif',
    'axes.labelsize': 12,
    'axes.titlesize': 13,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'figure.dpi': 200,
    'savefig.bbox': 'tight',
    'savefig.pad_inches': 0.1,
})

out_dir = os.path.join(os.path.dirname(__file__), '..', 'figures')
os.makedirs(out_dir, exist_ok=True)

# ============================================================
# Chart 1: Dijkstra — Sparse + Grid instances
# ============================================================
dijkstra_sparse = [r for r in rows if r['Algorithm'] == 'Dijkstra' and ('Sparse' in r['GraphType'] or 'Grid' in r['GraphType'])]
labels = [r['GraphType'] for r in dijkstra_sparse]
x = np.arange(len(labels))
bar_w = 0.18

fig, ax = plt.subplots(figsize=(10, 5.5))
for i, heap in enumerate(HEAPS):
    vals = [float(r[COL_MAP[heap]]) for r in dijkstra_sparse]
    bars = ax.bar(x + i * bar_w, vals, bar_w, label=heap, color=COLORS[heap], edgecolor='black', linewidth=0.4)
    for bar, v in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1.5,
                f'{v:.1f}', ha='center', va='bottom', fontsize=7, rotation=90)

ax.set_xlabel('Instancia de Grafo')
ax.set_ylabel('Tiempo de Ejecución (ms)')
ax.set_title('Dijkstra: Grafos Dispersos y Mallas 2D')
ax.set_xticks(x + 1.5 * bar_w)
ax.set_xticklabels(labels, rotation=15, ha='right')
ax.legend(loc='upper left', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
ax.set_ylim(0, max(float(r[COL_MAP[h]]) for r in dijkstra_sparse for h in HEAPS) * 1.25)
fig.savefig(os.path.join(out_dir, 'dijkstra_sparse_grid.png'))
fig.savefig(os.path.join(out_dir, 'dijkstra_sparse_grid.pdf'))
plt.close(fig)
print("Chart 1: dijkstra_sparse_grid OK")

# ============================================================
# Chart 2: Prim — Dense instances
# ============================================================
prim_dense = [r for r in rows if r['Algorithm'] == 'Prim' and 'Dense' in r['GraphType']]
labels2 = [r['GraphType'] for r in prim_dense]
x2 = np.arange(len(labels2))
bar_w2 = 0.18

fig, ax = plt.subplots(figsize=(7, 5))
for i, heap in enumerate(HEAPS):
    vals = [float(r[COL_MAP[heap]]) for r in prim_dense]
    bars = ax.bar(x2 + i * bar_w2, vals, bar_w2, label=heap, color=COLORS[heap], edgecolor='black', linewidth=0.4)
    for bar, v in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.3,
                f'{v:.2f}', ha='center', va='bottom', fontsize=9)

ax.set_xlabel('Instancia de Grafo')
ax.set_ylabel('Tiempo Prim (ms)')
ax.set_title('Prim: Grafos Densos')
ax.set_xticks(x2 + 1.5 * bar_w2)
ax.set_xticklabels(labels2)
ax.legend(loc='upper left', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
ax.set_ylim(0, max(float(r[COL_MAP[h]]) for r in prim_dense for h in HEAPS) * 1.3)
fig.savefig(os.path.join(out_dir, 'prim_dense.png'))
fig.savefig(os.path.join(out_dir, 'prim_dense.pdf'))
plt.close(fig)
print("Chart 2: prim_dense OK")

# ============================================================
# Chart 3: Prim — Sparse + Grid instances
# ============================================================
prim_sparse = [r for r in rows if r['Algorithm'] == 'Prim' and ('Sparse' in r['GraphType'] or 'Grid' in r['GraphType'])]
labels3 = [r['GraphType'] for r in prim_sparse]
x3 = np.arange(len(labels3))

fig, ax = plt.subplots(figsize=(10, 5.5))
for i, heap in enumerate(HEAPS):
    vals = [float(r[COL_MAP[heap]]) for r in prim_sparse]
    bars = ax.bar(x3 + i * bar_w, vals, bar_w, label=heap, color=COLORS[heap], edgecolor='black', linewidth=0.4)
    for bar, v in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1.5,
                f'{v:.1f}', ha='center', va='bottom', fontsize=7, rotation=90)

ax.set_xlabel('Instancia de Grafo')
ax.set_ylabel('Tiempo de Ejecución (ms)')
ax.set_title('Prim: Grafos Dispersos y Mallas 2D')
ax.set_xticks(x3 + 1.5 * bar_w)
ax.set_xticklabels(labels3, rotation=15, ha='right')
ax.legend(loc='upper left', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
ax.set_ylim(0, max(float(r[COL_MAP[h]]) for r in prim_sparse for h in HEAPS) * 1.25)
fig.savefig(os.path.join(out_dir, 'prim_sparse_grid.png'))
fig.savefig(os.path.join(out_dir, 'prim_sparse_grid.pdf'))
plt.close(fig)
print("Chart 3: prim_sparse_grid OK")

# ============================================================
# Chart 4: Dijkstra — Dense instances
# ============================================================
dijkstra_dense = [r for r in rows if r['Algorithm'] == 'Dijkstra' and 'Dense' in r['GraphType']]
labels4 = [r['GraphType'] for r in dijkstra_dense]
x4 = np.arange(len(labels4))

fig, ax = plt.subplots(figsize=(7, 5))
for i, heap in enumerate(HEAPS):
    vals = [float(r[COL_MAP[heap]]) for r in dijkstra_dense]
    bars = ax.bar(x4 + i * bar_w2, vals, bar_w2, label=heap, color=COLORS[heap], edgecolor='black', linewidth=0.4)
    for bar, v in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.15,
                f'{v:.2f}', ha='center', va='bottom', fontsize=9)

ax.set_xlabel('Instancia de Grafo')
ax.set_ylabel('Tiempo Dijkstra (ms)')
ax.set_title('Dijkstra: Grafos Densos')
ax.set_xticks(x4 + 1.5 * bar_w2)
ax.set_xticklabels(labels4)
ax.legend(loc='upper left', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
ax.set_ylim(0, max(float(r[COL_MAP[h]]) for r in dijkstra_dense for h in HEAPS) * 1.3)
fig.savefig(os.path.join(out_dir, 'dijkstra_dense.png'))
fig.savefig(os.path.join(out_dir, 'dijkstra_dense.pdf'))
plt.close(fig)
print("Chart 4: dijkstra_dense OK")

# ============================================================
# Chart 5: Speedup relativo vs Fibonacci Heap
# ============================================================
all_labels = [f"{r['Algorithm']}\n{r['GraphType']}" for r in rows]
x5 = np.arange(len(all_labels))

fig, ax = plt.subplots(figsize=(14, 5.5))
bar_w5 = 0.22
for i, heap in enumerate(['Hollow', 'Pairing', 'Rank-Pairing']):
    speedups = [float(r['FibonacciHeap_ms']) / float(r[COL_MAP[heap]]) for r in rows]
    bars = ax.bar(x5 + i * bar_w5, speedups, bar_w5, label=heap, color=COLORS[heap], edgecolor='black', linewidth=0.4)
    for bar, v in zip(bars, speedups):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.05,
                f'{v:.1f}x', ha='center', va='bottom', fontsize=6, rotation=90)

ax.axhline(y=1.0, color='gray', linestyle='--', linewidth=0.8, label='Línea base (Fibonacci)')
ax.set_xlabel('Instancia')
ax.set_ylabel('Speedup vs Fibonacci Heap')
ax.set_title('Factor de Aceleración Relativo al Fibonacci Heap')
ax.set_xticks(x5 + bar_w5)
ax.set_xticklabels(all_labels, rotation=45, ha='right', fontsize=7)
ax.legend(loc='upper right', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
fig.savefig(os.path.join(out_dir, 'speedup_vs_fibonacci.png'))
fig.savefig(os.path.join(out_dir, 'speedup_vs_fibonacci.pdf'))
plt.close(fig)
print("Chart 5: speedup_vs_fibonacci OK")

# ============================================================
# Chart 6: Decrease-key count comparison
# ============================================================
dk_cols = {
    'Fibonacci': 'DecKeys_Fib',
    'Hollow': 'DecKeys_Hollow',
    'Pairing': 'DecKeys_Pair',
    'Rank-Pairing': 'DecKeys_RP',
}
dk_labels = [f"{r['Algorithm']}\n{r['GraphType']}" for r in rows]
x6 = np.arange(len(dk_labels))

fig, ax = plt.subplots(figsize=(14, 5))
bar_w6 = 0.18
for i, heap in enumerate(HEAPS):
    vals = [int(r[dk_cols[heap]]) for r in rows]
    ax.bar(x6 + i * bar_w6, vals, bar_w6, label=heap, color=COLORS[heap], edgecolor='black', linewidth=0.3, alpha=0.85)

ax.set_xlabel('Instancia')
ax.set_ylabel('Número de decrease-key')
ax.set_title('Conteo de Operaciones decrease-key por Instancia')
ax.set_xticks(x6 + 1.5 * bar_w6)
ax.set_xticklabels(dk_labels, rotation=45, ha='right', fontsize=7)
ax.legend(loc='upper left', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
fig.savefig(os.path.join(out_dir, 'decrease_key_counts.png'))
fig.savefig(os.path.join(out_dir, 'decrease_key_counts.pdf'))
plt.close(fig)
print("Chart 6: decrease_key_counts OK")

# ============================================================
# Chart 7: Memory footprint comparison (theoretical, per node)
# ============================================================
structs = ['Fibonacci\nHeap', 'Pairing\nHeap', 'Rank-Pairing\nHeap', 'Hollow\nHeap']
sizes = [64, 40, 48, 64]  # 24 + 40 for HollowHeap
colors_mem = [COLORS['Fibonacci'], COLORS['Pairing'], COLORS['Rank-Pairing'], COLORS['Hollow']]

fig, ax = plt.subplots(figsize=(7, 4.5))
bars = ax.bar(structs, sizes, color=colors_mem, edgecolor='black', linewidth=0.5, width=0.55)
for bar, v in zip(bars, sizes):
    label = f'{v} B' if v != 64 or bar == bars[0] else '24+40 B'
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1,
            label, ha='center', va='bottom', fontsize=11, fontweight='bold')
ax.texts[-1].set_text('24+40 B')

ax.axhline(y=64, color='red', linestyle=':', linewidth=1, alpha=0.5, label='Tamaño línea L1 (64 B)')
ax.set_ylabel('Bytes por Nodo')
ax.set_title('Huella de Memoria por Nodo (64-bit)')
ax.legend(loc='upper right', framealpha=0.9)
ax.grid(axis='y', alpha=0.3)
ax.set_ylim(0, 80)
fig.savefig(os.path.join(out_dir, 'memory_footprint.png'))
fig.savefig(os.path.join(out_dir, 'memory_footprint.pdf'))
plt.close(fig)
print("Chart 7: memory_footprint OK")

print(f"\nAll charts saved to: {os.path.abspath(out_dir)}")
