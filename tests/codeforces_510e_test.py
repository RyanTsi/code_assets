"""Independent cycle-partition oracle and special-output checker for the CF adapter."""
import itertools
import random
import subprocess
import sys
from functools import cache
from pathlib import Path


def is_prime(value):
    return value >= 2 and all(value % divisor for divisor in range(2, int(value ** 0.5) + 1))


def possible(ages):
    # General compatibility graph: no parity assumption, no flow/matching implementation.
    n = len(ages)
    adjacent = [[i != j and is_prime(ages[i] + ages[j]) for j in range(n)] for i in range(n)]
    cycles = set()
    for start in range(n):
        @cache
        def paths(mask, last):
            if mask.bit_count() >= 3 and adjacent[last][start]:
                cycles.add(mask)
            for following in range(start + 1, n):
                if not mask >> following & 1 and adjacent[last][following]:
                    paths(mask | (1 << following), following)
        paths(1 << start, start)

    @cache
    def cover(remaining):
        if remaining == 0:
            return True
        first = remaining & -remaining
        return any(mask & first and mask & remaining == mask and cover(remaining ^ mask) for mask in cycles)
    return cover((1 << n) - 1)


def check_output(ages, output, expected):
    tokens = output.split()
    if tokens == ['Impossible']:
        assert not expected, ('False Impossible', ages)
        return
    assert expected, ('Accepted impossible instance', ages, output)
    numbers = list(map(int, tokens))
    assert numbers, 'Missing output'
    count, cursor, used = numbers[0], 1, []
    assert 1 <= count <= len(ages) // 3, ('Invalid table count', output)
    for _ in range(count):
        assert cursor < len(numbers), 'Missing table'
        length = numbers[cursor]
        cursor += 1
        assert length >= 3, ('Two-person or empty ring', output)
        ring = numbers[cursor:cursor + length]
        assert len(ring) == length and all(1 <= vertex <= len(ages) for vertex in ring)
        cursor += length
        used.extend(ring)
        assert all(is_prime(ages[u - 1] + ages[v - 1]) for u, v in zip(ring, ring[1:] + ring[:1])), (ages, ring)
    assert cursor == len(numbers), 'Trailing tokens'
    assert sorted(used) == list(range(1, len(ages) + 1)), ('Duplicate/missing vertex', output)


def main():
    executable = Path(sys.argv[1]).resolve()
    checked = 0

    def run(ages, expected=None):
        nonlocal checked
        data = f'{len(ages)}\n' + ' '.join(map(str, ages)) + '\n'
        process = subprocess.run([str(executable)], input=data, text=True, capture_output=True, check=True, timeout=10)
        check_output(ages, process.stdout, possible(ages) if expected is None else expected)
        checked += 1

    # Official examples: check semantics, not the particular arrangement printed by the statement.
    run([3, 4, 8, 9], True)
    run([2] * 5, False)
    run(list(range(2, 14)), True)
    run(list(range(2, 26)), True)
    for ages in itertools.product(range(2, 6), repeat=4):
        run(list(ages))
    rng = random.Random(510)
    for _ in range(240):
        n = rng.randrange(3, 10)
        # Balanced parity often produces feasible inputs; also test unconstrained inputs.
        ages = [rng.randrange(2, 40) for _ in range(n)]
        if n % 2 == 0 and rng.randrange(2):
            ages = [2 * rng.randrange(1, 20) + i % 2 for i in range(n)]
            rng.shuffle(ages)
        run(ages)
    # Degree-one graph cannot satisfy degree two; repeated values remain distinct people.
    run([2, 3, 4, 11], False)
    run([2, 2, 3, 3], True)
    run([2, 3, 4], False)
    run([9999] * 100 + [10000] * 100, False)  # 19999 = 7 * 2857
    run([2] * 100 + [3] * 100, True)  # Dense maximum n
    run([10000] * 100 + [9973] * 100, True)  # Largest sieve/query region; 19973 prime
    run(list(range(2, 202)), True)

    # Mutation checks: malformed witnesses must be rejected by the checker itself.
    for invalid in ['1\n2 1 2\n', '1\n4 1 2 1 4\n', '1\n4 1 2 3 4 99\n', 'Impossible\n']:
        try:
            check_output([2, 2, 3, 3], invalid, True)
        except AssertionError:
            pass
        else:
            raise AssertionError(('Checker accepted invalid witness', invalid))
    print(f'CF510E: {checked} cases passed (4 samples, 256 exhaustive, 240 randomized, 7 targeted); independent cycle oracle and witness checker')


if __name__ == '__main__':
    main()
