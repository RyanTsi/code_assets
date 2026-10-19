"""Check or format maintained code; preserve upstream and historical evidence."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

import black

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED = ("third_party/", "provenance/", "judge/history/", "judge/generated/")


def run(command: list[str], **kwargs) -> None:
    subprocess.run(command, cwd=ROOT, check=True, **kwargs)


def executable(name: str, override: str | None = None) -> str:
    found = shutil.which(override or name)
    if found:
        return found
    if name == "clang-format" and os.name == "nt":
        base = Path(os.environ.get("ProgramFiles", "C:/Program Files"))
        candidates = sorted(
            (base / "Microsoft Visual Studio").glob("*/*/VC/Tools/Llvm/bin/clang-format.exe")
        )
        if candidates:
            return str(candidates[-1])
    raise SystemExit(f"Missing {name}; see docs/project/FORMATTING.md")


def code_fences(path: Path, clang_format: str) -> str:
    text = path.read_text(encoding="utf-8")

    def replace(match: re.Match) -> str:
        code = subprocess.run(
            [clang_format, "--style=file", "--assume-filename=example.cpp"],
            cwd=ROOT,
            input=match[2],
            text=True,
            capture_output=True,
            check=True,
            encoding="utf-8",
        ).stdout
        return match[1] + code + match[3]

    return re.sub(r"(^```(?:cpp|c\+\+)\s*\n)(.*?)(^```[ \t]*$)", replace, text, flags=re.M | re.S)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--write", action="store_true", help="Apply formatting; default is check only"
    )
    parser.add_argument("--clang-format", help="Path to clang-format 19.1.5")
    args = parser.parse_args()

    inventory = (
        subprocess.check_output(
            ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"], cwd=ROOT
        )
        .decode("utf-8")
        .split("\0")
    )
    files = sorted({p for p in inventory if p and not p.startswith(EXCLUDED)})
    cpp = [p for p in files if Path(p).suffix in {".hpp", ".cpp"}]
    python = [p for p in files if p.endswith(".py")]
    web = [
        p
        for p in files
        if Path(p).suffix in {".js", ".css", ".html", ".json"} and p != "visualization/index.html"
    ]
    cmake = [p for p in files if p == "CMakeLists.txt" or p.endswith((".cmake", ".cmake.in"))]
    markdown = [p for p in files if p.endswith(".md")]

    clang_format = executable("clang-format", args.clang_format)
    version = subprocess.check_output([clang_format, "--version"], text=True)
    if not re.search(r"\b19\.1\.5\b", version):
        raise SystemExit("Use clang-format 19.1.5 for reproducible formatting")
    run([clang_format, "-i" if args.write else "--dry-run", "--Werror", *cpp])
    if black.__version__ != "24.10.0":
        raise SystemExit("Use Black 24.10.0 for reproducible formatting")
    black_config = black.parse_pyproject_toml(str(ROOT / "pyproject.toml"))
    mode = black.Mode(
        line_length=black_config["line_length"],
        target_versions={black.TargetVersion.PY310},
    )
    stale_python = []
    for name in python:
        path = ROOT / name
        original = path.read_text(encoding="utf-8")
        try:
            formatted = black.format_file_contents(original, fast=False, mode=mode)
        except black.NothingChanged:
            continue
        if args.write:
            path.write_text(formatted, encoding="utf-8", newline="\n")
        else:
            stale_python.append(name)
    if stale_python:
        raise SystemExit("Unformatted Python files: " + ", ".join(stale_python))

    prettier = ROOT / ".cache/format-node/node_modules/prettier/bin/prettier.cjs"
    if prettier.exists():
        prettier_command = [executable("node"), str(prettier)]
    else:
        prettier_command = [executable("prettier")]
    if subprocess.check_output([*prettier_command, "--version"], text=True).strip() != "3.6.2":
        raise SystemExit("Use Prettier 3.6.2 for reproducible formatting")
    run([*prettier_command, "--write" if args.write else "--check", *web])

    environment = os.environ.copy()
    cache = ROOT / ".cache/format-python"
    if cache.exists():
        environment["PYTHONPATH"] = str(cache) + os.pathsep + environment.get("PYTHONPATH", "")
    cmake_version = subprocess.check_output(
        [sys.executable, "-m", "cmakelang.format", "--version"], text=True, env=environment
    ).strip()
    if cmake_version != "0.6.13":
        raise SystemExit("Use cmakelang 0.6.13 for reproducible formatting")
    run(
        [
            sys.executable,
            "-m",
            "cmakelang.format",
            "--in-place" if args.write else "--check",
            *cmake,
        ],
        env=environment,
    )

    stale = []
    count = 0
    for name in markdown:
        path = ROOT / name
        original = path.read_text(encoding="utf-8")
        if not re.search(r"^```(?:cpp|c\+\+)\s*$", original, re.M):
            continue
        count += 1
        formatted = code_fences(path, clang_format)
        if formatted != original:
            if args.write:
                path.write_text(formatted, encoding="utf-8", newline="\n")
            else:
                stale.append(name)
    if stale:
        raise SystemExit("Unformatted C++ documentation examples: " + ", ".join(stale))
    print(
        f"{'Formatted' if args.write else 'Checked'} {len(cpp)} C++ files, "
        f"{len(python)} Python files, {len(web)} web/JSON files, "
        f"{len(cmake)} CMake files and C++ examples in {count} documents."
    )


if __name__ == "__main__":
    main()
