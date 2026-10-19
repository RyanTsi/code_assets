#include <algorithm_assets/graph/strong_components.hpp>
#include <algorithm_assets/numeric/fft.hpp>
#include <algorithm_assets/geometry/minkowski.hpp>
#include <algorithm_assets/algebra/modular.hpp>
int expansion_odr_probe() {
    return algorithm_assets::Modulus64(7).power(2, 3) == 1;
}
