cmake_minimum_required(VERSION 3.20)

project(AlgorithmAssets VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(algorithm_assets INTERFACE)
add_library(algorithm_assets::algorithm_assets ALIAS algorithm_assets)
target_compile_features(algorithm_assets INTERFACE cxx_std_20)
target_include_directories(algorithm_assets INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(ALGORITHM_ASSETS_BUILD_EXAMPLES "Build usage examples" OFF)
if(ALGORITHM_ASSETS_BUILD_EXAMPLES)
  foreach(example IN ITEMS include_library first_algorithms geometry2d repository_assets repository_expansion problem_combinations audio_spectrum)
    add_executable(algorithm_assets_${example} examples/${example}.cpp)
    target_link_libraries(algorithm_assets_${example} PRIVATE algorithm_assets::algorithm_assets)
    set_target_properties(algorithm_assets_${example} PROPERTIES CXX_EXTENSIONS OFF)
  endforeach()
endif()

option(ALGORITHM_ASSETS_BUILD_BENCHMARKS "Build reproducible local benchmarks" OFF)
if(ALGORITHM_ASSETS_BUILD_BENCHMARKS)
  add_executable(algorithm_assets_geometry_benchmark benchmarks/geometry_benchmark.cpp)
  target_link_libraries(algorithm_assets_geometry_benchmark PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_geometry_benchmark PROPERTIES CXX_EXTENSIONS OFF)
endif()

option(ALGORITHM_ASSETS_BUILD_TESTS "Build correctness and catalog tests" OFF)
if(ALGORITHM_ASSETS_BUILD_TESTS)
  enable_testing()
  add_executable(algorithm_assets_tests tests/algorithms_test.cpp)
  file(GLOB_RECURSE public_headers CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
  foreach(header IN LISTS public_headers)
    string(MAKE_C_IDENTIFIER ${header} header_id)
    set(header_source ${CMAKE_CURRENT_BINARY_DIR}/header_checks/${header_id}.cpp)
    file(WRITE ${header_source} "#include <${header}>\n")
    target_sources(algorithm_assets_tests PRIVATE ${header_source})
  endforeach()
  target_link_libraries(algorithm_assets_tests PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_tests PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(algorithm_assets_tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  add_test(NAME algorithms COMMAND algorithm_assets_tests)
  add_executable(algorithm_assets_geometry_tests tests/geometry_test.cpp tests/geometry_odr.cpp)
  target_link_libraries(algorithm_assets_geometry_tests PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_geometry_tests PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(algorithm_assets_geometry_tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  add_test(NAME geometry COMMAND algorithm_assets_geometry_tests)
  add_executable(algorithm_assets_repository_tests tests/repository_assets_test.cpp tests/repository_assets_odr.cpp)
  target_link_libraries(algorithm_assets_repository_tests PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_repository_tests PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(algorithm_assets_repository_tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  add_test(NAME repository_assets COMMAND algorithm_assets_repository_tests)
  add_executable(algorithm_assets_expansion_tests tests/repository_expansion_test.cpp tests/repository_expansion_odr.cpp)
  target_link_libraries(algorithm_assets_expansion_tests PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_expansion_tests PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(algorithm_assets_expansion_tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  add_test(NAME repository_expansion COMMAND algorithm_assets_expansion_tests)
  foreach(problem IN ITEMS 3375 3374 3366 3812 3376 4782 3386 3803 4557 1742 3389 1776)
    add_executable(judge_p${problem} judge/generated/luogu_p${problem}.cpp)
    target_compile_features(judge_p${problem} PRIVATE cxx_std_20)
    set_target_properties(judge_p${problem} PROPERTIES CXX_EXTENSIONS OFF)
  endforeach()
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_executable(judge_cf510e judge/generated/codeforces_510e.cpp)
  target_compile_features(judge_cf510e PRIVATE cxx_std_20)
  set_target_properties(judge_cf510e PROPERTIES CXX_EXTENSIONS OFF)
  add_test(NAME codeforces_510e COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/codeforces_510e_test.py $<TARGET_FILE:judge_cf510e>)
  add_executable(algorithm_assets_combination_tests tests/problem_combinations_test.cpp tests/problem_combinations_odr.cpp)
  target_link_libraries(algorithm_assets_combination_tests PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_combination_tests PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(algorithm_assets_combination_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
  add_test(NAME problem_combinations COMMAND algorithm_assets_combination_tests)
  add_executable(algorithm_assets_audio_tests tests/audio_spectrum_test.cpp)
  target_link_libraries(algorithm_assets_audio_tests PRIVATE algorithm_assets::algorithm_assets)
  set_target_properties(algorithm_assets_audio_tests PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(algorithm_assets_audio_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
  add_test(NAME audio_consumer_composition COMMAND algorithm_assets_audio_tests)
  if(TARGET algorithm_assets_audio_spectrum)
    execute_process(COMMAND ${Python3_EXECUTABLE} -c "import numpy, scipy"
      RESULT_VARIABLE audio_reference_dependencies OUTPUT_QUIET ERROR_QUIET)
    if(audio_reference_dependencies EQUAL 0)
      add_test(NAME audio_scipy_reference COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/audio_spectrum_reference_test.py
        $<TARGET_FILE:algorithm_assets_audio_spectrum>)
    endif()
  endif()
  foreach(spec IN ITEMS "cf822c;codeforces_822c" "cf1822f;codeforces_1822f" "abl_d;atcoder_abl_d")
    list(GET spec 0 target_name)
    list(GET spec 1 source_name)
    add_executable(judge_${target_name} judge/generated/${source_name}.cpp)
    target_compile_features(judge_${target_name} PRIVATE cxx_std_20)
    set_target_properties(judge_${target_name} PROPERTIES CXX_EXTENSIONS OFF)
  endforeach()
  add_test(NAME combination_judge_adapters COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/combination_judge_test.py $<TARGET_FILE_DIR:judge_cf822c>)
  add_test(NAME source_import_ledger COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/repository_import.py)
  add_test(NAME source_import_integrity COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/import_ledger_test.py)
  add_test(NAME expansion_judge_adapters COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/expansion_judge_test.py $<TARGET_FILE_DIR:judge_p3376>)
  add_test(NAME judge_bundles COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/bundle_submission.py --check)
  add_test(NAME judge_adapters COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/judge_adapter_test.py
    $<TARGET_FILE:judge_p3375> $<TARGET_FILE:judge_p3374>
    $<TARGET_FILE:judge_p3366> $<TARGET_FILE:judge_p3812>)
  add_test(NAME catalog_validation COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/catalog.py validate)
  add_test(NAME catalog_matching COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/catalog_test.py)
  add_test(NAME graph_catalog COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/graph_test.py)
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS algorithm_assets EXPORT AlgorithmAssetsTargets)
install(EXPORT AlgorithmAssetsTargets
  FILE AlgorithmAssetsTargets.cmake
  NAMESPACE algorithm_assets::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AlgorithmAssets
)

configure_package_config_file(
  cmake/AlgorithmAssetsConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/AlgorithmAssetsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AlgorithmAssets
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/AlgorithmAssetsConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/AlgorithmAssetsConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/AlgorithmAssetsConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AlgorithmAssets
)
