"""Independent stdin/stdout oracles for three problem-derived recipes."""

import itertools
import random
import subprocess
import sys
from collections import deque
from pathlib import Path

directory = Path(sys.argv[1])
rng = random.Random(1015)
counts = {"822c": 0, "1822f": 0, "abl_d": 0}


def run(name, data):
    stem = {"822c": "judge_cf822c", "1822f": "judge_cf1822f", "abl_d": "judge_abl_d"}[name]
    executable = directory / (stem + (".exe" if (directory / (stem + ".exe")).exists() else ""))
    return subprocess.run(
        [str(executable.resolve())],
        input=data,
        text=True,
        capture_output=True,
        check=True,
        timeout=20,
    ).stdout.split()


def intervals_case(intervals, duration, expected=None):
    if expected is None:
        costs = [
            a[2] + b[2]
            for a, b in itertools.combinations(intervals, 2)
            if (a[1] < b[0] or b[1] < a[0]) and a[1] - a[0] + b[1] - b[0] + 2 == duration
        ]
        expected = min(costs, default=-1)
    data = f"{len(intervals)} {duration}\n" + "".join(
        f"{l} {r} {cost}\n" for l, r, cost in intervals
    )
    assert run("822c", data) == [str(expected)], (intervals, duration, expected)
    counts["822c"] += 1


def subsequence_case(values, gap, expected=None):
    if expected is None:
        dp = [1] * len(values)
        for i in range(len(values)):
            for j in range(i):
                if abs(values[i] - values[j]) <= gap:
                    dp[i] = max(dp[i], dp[j] + 1)
        expected = max(dp, default=0)
    assert run("abl_d", f"{len(values)} {gap}\n" + " ".join(map(str, values)) + "\n") == [
        str(expected)
    ]
    counts["abl_d"] += 1


intervals_case([(1, 3, 4), (1, 2, 5), (5, 6, 1), (1, 2, 4)], 5, 5)
intervals_case([(4, 6, 3), (2, 4, 1), (3, 5, 4)], 2, -1)
intervals_case([(1, 2, 1), (2, 3, 1)], 4, -1)  # Closed endpoint collision
intervals_case([(1, 2, 10**9), (3, 4, 10**9)], 4, 2 * 10**9)
subsequence_case([1, 5, 4, 3, 8, 6, 9, 7, 2, 4], 3, 7)
subsequence_case([0, 100, 101], 1, 2)
subsequence_case([0], 0, 1)
subsequence_case([300000, 0, 300000], 300000, 3)

tree_cases = [
    (3, 2, 3, [(1, 0), (2, 0)], 2),
    (5, 4, 1, [(1, 0), (3, 1), (4, 3), (2, 3)], 12),
    (6, 5, 3, [(3, 0), (5, 0), (1, 5), (4, 0), (2, 1)], 17),
    (10, 6, 4, [(0, 2), (0, 8), (8, 6), (6, 5), (5, 3), (8, 1), (1, 7), (7, 4), (4, 9)], 32),
]
for _ in range(160):
    n = rng.randrange(2, 25)
    intervals = [
        (rng.randrange(1, 50), rng.randrange(1, 12), rng.randrange(1, 1000)) for _ in range(n)
    ]
    intervals_case(
        [(l, l + length - 1, cost) for l, length, cost in intervals], rng.randrange(2, 25)
    )
    subsequence_case([rng.randrange(300001) for _ in range(n)], rng.randrange(300001))
    k, c = rng.randrange(1, 10**9 + 1), rng.randrange(1, 10**9 + 1)
    edges = [(v, rng.randrange(v)) for v in range(1, n)]
    adjacent = [[] for _ in range(n)]
    for u, v in edges:
        adjacent[u].append(v)
        adjacent[v].append(u)
    all_distances = []
    for source in range(n):
        distances = [-1] * n
        distances[source] = 0
        queue = deque([source])
        while queue:
            u = queue.popleft()
            for v in adjacent[u]:
                if distances[v] == -1:
                    distances[v] = distances[u] + 1
                    queue.append(v)
        all_distances.append(distances)
    answer = max(k * max(all_distances[v]) - c * all_distances[0][v] for v in range(n))
    tree_cases.append((n, k, c, edges, answer))

intervals_case([(1, 1, 1)] * 100000 + [(3, 3, 1)] * 100000, 2, 2)
subsequence_case([0, 300000] * 150000, 0, 150000)
subsequence_case(list(range(300000)), 1, 300000)
data = (
    str(len(tree_cases))
    + "\n"
    + "".join(
        f"{n} {k} {c}\n" + "".join(f"{u + 1} {v + 1}\n" for u, v in edges)
        for n, k, c, edges, expected in tree_cases
    )
)
assert run("1822f", data) == [str(expected) for n, k, c, edges, expected in tree_cases]
large_tree = "1\n200000 1000000000 1000000000\n" + "".join(
    f"{i} {i + 1}\n" for i in range(1, 200000)
)
assert run("1822f", large_tree) == [str(199999 * 10**9)]
counts["1822f"] = len(tree_cases) + 1
print(
    "Three judge adapters: samples, independent oracles, maximum sizes passed; cases=" + str(counts)
)
