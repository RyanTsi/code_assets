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

option(ALGORITHM_ASSETS_BUILD_EXAMPLES "Build the include example" OFF)
if(ALGORITHM_ASSETS_BUILD_EXAMPLES)
  add_executable(algorithm_assets_include_example examples/include_library.cpp)
  target_link_libraries(algorithm_assets_include_example PRIVATE
    algorithm_assets::algorithm_assets
  )
  set_target_properties(algorithm_assets_include_example PROPERTIES CXX_EXTENSIONS OFF)
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
