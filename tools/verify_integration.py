"""Compile the actual algorithm tests as external source and installed consumers."""

import argparse
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def run(*arguments):
    subprocess.run(arguments, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--generator", default=None)
    args = parser.parse_args()
    build = (ROOT / args.build_dir).resolve()
    prefix = build / "integration-install"
    run("cmake", "--install", str(build), "--config", "Release", "--prefix", str(prefix))
    source = build / "integration-consumer"
    source.mkdir(parents=True, exist_ok=True)
    standalone = []
    for index, header in enumerate(sorted((ROOT / "include").rglob("*.hpp"))):
        path = source / f"header_{index}.cpp"
        path.write_text(
            f"#include <{header.relative_to(ROOT / 'include').as_posix()}>\n", encoding="utf-8"
        )
        standalone.append(f'"{path.as_posix()}"')
    cmake = f"""cmake_minimum_required(VERSION 3.20)
project(ExternalAssetConsumer LANGUAGES CXX)
if(ASSETS_SOURCE_DIR)
  add_subdirectory("${{ASSETS_SOURCE_DIR}}" assets)
else()
  find_package(AlgorithmAssets 0.1 CONFIG REQUIRED)
endif()
add_executable(external_tests "{(ROOT / 'tests/algorithms_test.cpp').as_posix()}"
  {' '.join(standalone)})
target_link_libraries(external_tests PRIVATE algorithm_assets::eigen)
set_target_properties(external_tests PROPERTIES CXX_EXTENSIONS OFF)
enable_testing()
add_test(NAME external_algorithms COMMAND external_tests)
add_executable(external_geometry "{(ROOT / 'tests/geometry_test.cpp').as_posix()}"
  "{(ROOT / 'tests/geometry_odr.cpp').as_posix()}")
target_link_libraries(external_geometry PRIVATE algorithm_assets::algorithm_assets)
set_target_properties(external_geometry PROPERTIES CXX_EXTENSIONS OFF)
add_test(NAME external_geometry COMMAND external_geometry)
add_executable(external_repository "{(ROOT / 'tests/repository_assets_test.cpp').as_posix()}"
  "{(ROOT / 'tests/repository_assets_odr.cpp').as_posix()}")
target_link_libraries(external_repository PRIVATE algorithm_assets::algorithm_assets)
set_target_properties(external_repository PROPERTIES CXX_EXTENSIONS OFF)
add_test(NAME external_repository COMMAND external_repository)
add_executable(external_expansion "{(ROOT / 'tests/repository_expansion_test.cpp').as_posix()}"
  "{(ROOT / 'tests/repository_expansion_odr.cpp').as_posix()}")
target_link_libraries(external_expansion PRIVATE algorithm_assets::algorithm_assets)
set_target_properties(external_expansion PROPERTIES CXX_EXTENSIONS OFF)
add_test(NAME external_expansion COMMAND external_expansion)
add_executable(external_combinations "{(ROOT / 'tests/problem_combinations_test.cpp').as_posix()}"
  "{(ROOT / 'tests/problem_combinations_odr.cpp').as_posix()}")
target_link_libraries(external_combinations PRIVATE algorithm_assets::algorithm_assets)
set_target_properties(external_combinations PROPERTIES CXX_EXTENSIONS OFF)
add_test(NAME external_combinations COMMAND external_combinations)
add_executable(external_mesh3d "{(ROOT / 'tests/mesh3d_test.cpp').as_posix()}"
  "{(ROOT / 'tests/mesh3d_odr.cpp').as_posix()}")
target_link_libraries(external_mesh3d PRIVATE algorithm_assets::eigen)
set_target_properties(external_mesh3d PROPERTIES CXX_EXTENSIONS OFF)
add_test(NAME external_mesh3d COMMAND external_mesh3d)
"""
    (source / "CMakeLists.txt").write_text(cmake, encoding="utf-8")
    for mode in ("installed", "source"):
        output = build / f"integration-{mode}"
        configure = ["cmake", "-S", str(source), "-B", str(output), "-DCMAKE_BUILD_TYPE=Release"]
        if args.generator:
            configure += ["-G", args.generator]
        if mode == "installed":
            configure.append(f"-DCMAKE_PREFIX_PATH={prefix.as_posix()}")
        else:
            configure.append(f"-DASSETS_SOURCE_DIR={ROOT.as_posix()}")
        run(*configure)
        run("cmake", "--build", str(output), "--config", "Release", "--parallel", "2")
        run("ctest", "--test-dir", str(output), "-C", "Release", "--output-on-failure")
    print("Source/installed consumers, standalone headers, and multi-TU algorithm tests passed")


if __name__ == "__main__":
    main()
