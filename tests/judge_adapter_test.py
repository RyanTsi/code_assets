"""Exercise generated submissions as actual stdin/stdout programs."""

import itertools
import random
import subprocess
import sys

kmp, bit, mst, xor = sys.argv[1:]
rng = random.Random(20261014)


def run(executable, text, expected):
    result = subprocess.run([executable], input=text, text=True, capture_output=True, check=True)
    assert result.stdout.split() == str(expected).split(), (
        executable,
        text,
        result.stdout,
        expected,
    )


run(kmp, "ABABABC\nABA\n", "1 3 0 0 1")
run(bit, "5 5\n1 5 4 2 3\n1 1 3\n2 2 5\n1 3 -1\n1 4 2\n2 1 4\n", "14 16")
run(mst, "4 5\n1 2 2\n1 3 2\n1 4 3\n2 3 4\n3 4 3\n", "7")
run(mst, "3 1\n1 2 1\n", "orz")
run(xor, "2\n5 3\n", "6")
for _ in range(25):
    text = "".join(rng.choice("ABC") for _ in range(80))
    pattern = "".join(rng.choice("ABC") for _ in range(rng.randint(1, 9)))
    hits = [i + 1 for i in range(len(text) - len(pattern) + 1) if text.startswith(pattern, i)]
    borders = [
        max((k for k in range(i) if pattern[:k] == pattern[i - k : i]), default=0)
        for i in range(1, len(pattern) + 1)
    ]
    run(kmp, text + "\n" + pattern + "\n", " ".join(map(str, hits + borders)))
    values = [rng.randint(-100, 100) for _ in range(20)]
    original = values[:]
    operations, answers = [], []
    for __ in range(80):
        first = rng.randrange(20)
        if rng.randrange(2):
            delta = rng.randint(-100, 100)
            operations.append(f"1 {first+1} {delta}")
            values[first] += delta
        else:
            last = rng.randrange(first + 1, 21)
            operations.append(f"2 {first+1} {last}")
            answers.append(sum(values[first:last]))
    run(
        bit,
        "20 80\n" + " ".join(map(str, original)) + "\n" + "\n".join(operations) + "\n",
        " ".join(map(str, answers)),
    )
    numbers = [rng.randrange(1 << 64) for _ in range(8)]
    span = {0}
    for value in numbers:
        span |= {x ^ value for x in span}
    run(xor, "8\n" + " ".join(map(str, numbers)) + "\n", max(span))
    n = 4
    edges = [
        (a, b, rng.randint(1, 100))
        for a, b in itertools.combinations(range(n), 2)
        if rng.randrange(3)
    ]
    best = None
    for subset in itertools.combinations(edges, n - 1):
        components = list(range(n))
        for a, b, _weight in subset:
            old, new = components[b], components[a]
            components = [new if c == old else c for c in components]
        if len(set(components)) == 1:
            weight = sum(edge[2] for edge in subset)
            best = weight if best is None else min(best, weight)
    data = f"{n} {len(edges)}\n" + "\n".join(f"{a+1} {b+1} {w}" for a, b, w in edges) + "\n"
    run(mst, data, "orz" if best is None else best)
print("Generated C++20 programs: samples and 100 randomized stdin/stdout oracles passed")
