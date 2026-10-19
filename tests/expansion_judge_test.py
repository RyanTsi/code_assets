"""Run the eight generated judge programs; validate outputs independently."""

import itertools
import math
import random
import subprocess
import sys
from pathlib import Path

directory = Path(sys.argv[1])
rng = random.Random(1014)


def run(problem, text):
    exe = directory / f"judge_p{problem}.exe"
    if not exe.exists():
        exe = directory / f"judge_p{problem}"
    return subprocess.run(
        [str(exe)], input=text, text=True, capture_output=True, check=True
    ).stdout.split()


def exact(problem, text, expected):
    actual = run(problem, text)
    assert actual == str(expected).split(), (problem, actual, expected)


exact(3376, "3 2 1 3\n1 2 7\n2 3 5\n", 5)
exact(3386, "2 2 3\n1 1\n1 2\n2 2\n", 2)
exact(3803, "1 1\n1 2\n3 4\n", "3 10 8")
exact(1776, "2 5\n3 2 2\n4 3 1\n", 7)
exact(3389, "2\n0 2 4\n1 3 7\n", "1.00 2.00")
exact(3389, "2\n1 2 3\n2 4 6\n", "No Solution")
exact(4782, "1 2\n1 0 1 0\n1 1 1 1\n", "IMPOSSIBLE")
for _ in range(20):
    edges = [(rng.randrange(4), rng.randrange(4), rng.randrange(15)) for _ in range(10)]
    minimum = min(
        sum(w for u, v, w in edges if mask >> u & 1 and not (mask >> v & 1))
        for mask in range(16)
        if mask & 1 and not (mask & 8)
    )
    data = "4 10 1 4\n" + "\n".join(f"{u+1} {v+1} {w}" for u, v, w in edges) + "\n"
    exact(3376, data, minimum)
    clauses = [
        (rng.randrange(5), rng.randrange(2), rng.randrange(5), rng.randrange(2)) for _ in range(12)
    ]
    solutions = [
        mask
        for mask in range(32)
        if all((mask >> u & 1) == x or (mask >> v & 1) == y for u, x, v, y in clauses)
    ]
    tokens = run(
        4782, "5 12\n" + "\n".join(f"{u+1} {x} {v+1} {y}" for u, x, v, y in clauses) + "\n"
    )
    if solutions:
        assert tokens[0] == "POSSIBLE" and len(tokens) == 6
        values = list(map(int, tokens[1:]))
        assert all(values[u] == x or values[v] == y for u, x, v, y in clauses)
    else:
        assert tokens == ["IMPOSSIBLE"]
    matching_edges = [(u, v) for u in range(4) for v in range(4) if rng.randrange(2)]
    best = 0
    for r in range(5):
        for subset in itertools.combinations(matching_edges, r):
            if len({u for u, v in subset}) == r and len({v for u, v in subset}) == r:
                best = max(best, r)
    exact(
        3386,
        f"4 4 {len(matching_edges)}\n"
        + "\n".join(f"{u+1} {v+1}" for u, v in matching_edges)
        + "\n",
        best,
    )
    a = [rng.randrange(10) for _ in range(15)]
    b = [rng.randrange(10) for _ in range(9)]
    product = [0] * 23
    for i, x in enumerate(a):
        for j, y in enumerate(b):
            product[i + j] += x * y
    exact(
        3803,
        "14 8\n" + " ".join(map(str, a)) + "\n" + " ".join(map(str, b)) + "\n",
        " ".join(map(str, product)),
    )
    queries = [(rng.randint(-8, 8), rng.randint(-8, 8)) for _ in range(20)]
    data = (
        "4 4 20\n0 0\n2 0\n2 2\n0 2\n0 0\n3 0\n3 3\n0 3\n"
        + "\n".join(f"{x} {y}" for x, y in queries)
        + "\n"
    )
    expected = [int(-3 <= x <= 2 and -3 <= y <= 2) for x, y in queries]
    exact(4557, data, " ".join(map(str, expected)))
    points = [rng.randint(-50, 50) for _ in range(20)]
    tokens = list(map(float, run(1742, "20\n" + "\n".join(f"{x} 0" for x in points) + "\n")))
    assert (
        abs(tokens[0] - (max(points) - min(points)) / 2) < 1e-9
        and abs(tokens[1] - (max(points) + min(points)) / 2) < 1e-9
        and abs(tokens[2]) < 1e-9
    )
    items = [(rng.randrange(20), rng.randint(1, 9), rng.randrange(5)) for _ in range(5)]
    best = max(
        sum(k * v for k, (v, w, c) in zip(counts, items))
        for counts in itertools.product(*(range(c + 1) for v, w, c in items))
        if sum(k * w for k, (v, w, c) in zip(counts, items)) <= 20
    )
    exact(1776, "5 20\n" + "\n".join(f"{v} {w} {c}" for v, w, c in items) + "\n", best)
    n = 5
    matrix = [[rng.randint(-3, 3) for _ in range(n)] for __ in range(n)]
    for i in range(n):
        matrix[i][i] += 20
    known = [rng.randint(-10, 10) for _ in range(n)]
    right = [sum(a * b for a, b in zip(row, known)) for row in matrix]
    tokens = run(
        3389,
        "5\n" + "\n".join(" ".join(map(str, row + [r])) for row, r in zip(matrix, right)) + "\n",
    )
    assert len(tokens) == n and all(abs(float(x) - y) < 1e-7 for x, y in zip(tokens, known))
print("Eight generated submissions: boundaries and 160 randomized stdin/stdout checks passed")
