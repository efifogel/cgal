"""Generates a closed, outward oriented triangle mesh of a terrain block with
integer coordinates, used to test all_intersections.cpp on degenerate input.

The top surface interpolates random integer heights (1-3) over an N x N grid,
with a random diagonal in every cell; it has saddle vertices and flat regions.
The vertical walls and the bottom (z = 0, triangulated grid) are flat. With
integer coordinates, lines through integer points (all_intersections -l grid)
often lie in planes of faces or run along edges.

Usage: python3 generate_terrain.py N seed output.off
Example (the mesh terrain12.off): python3 generate_terrain.py 12 1 terrain12.off
"""

import random
import sys

n = int(sys.argv[1])
seed = int(sys.argv[2])
output = sys.argv[3]

random.seed(seed)
heights = [[random.choice([1, 2, 2, 3]) for j in range(n + 1)] for i in range(n + 1)]

points = []
indices = {}


def vertex(i, j, top):
    """Returns the index of the top or bottom vertex above grid point (i, j)."""
    key = (i, j, top)
    if key not in indices:
        indices[key] = len(points)
        points.append((i, j, heights[i][j] if top else 0))
    return indices[key]


triangles = []

# Top (counterclockwise seen from above) and bottom (reversed) of every cell.
for i in range(n):
    for j in range(n):
        corners = ((i, j), (i + 1, j), (i + 1, j + 1), (i, j + 1))
        for top in (True, False):
            a, b, c, d = (vertex(*p, top) for p in corners)
            cell = [(a, b, c), (a, c, d)] if random.random() < 0.5 else [(a, b, d), (b, c, d)]
            for t in cell:
                triangles.append(t if top else t[::-1])

# Walls along the boundary loop (counterclockwise seen from above).
loop = ([(i, 0) for i in range(n)] + [(n, j) for j in range(n)] +
        [(i, n) for i in range(n, 0, -1)] + [(0, j) for j in range(n, 0, -1)])
for k in range(len(loop)):
    p, q = loop[k], loop[(k + 1) % len(loop)]
    pb, qb, qt, pt = vertex(*p, False), vertex(*q, False), vertex(*q, True), vertex(*p, True)
    triangles.append((pb, qb, qt))
    triangles.append((pb, qt, pt))

with open(output, "w") as f:
    f.write("OFF\n%d %d 0\n" % (len(points), len(triangles)))
    for p in points:
        f.write("%d %d %d\n" % p)
    for t in triangles:
        f.write("3 %d %d %d\n" % t)
