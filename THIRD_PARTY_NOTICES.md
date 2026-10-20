# License scope and third-party notices

The MIT license in [LICENSE](LICENSE) applies to the original implementation,
tools, tests, examples, contracts and documentation contributed to AlgorithmAssets,
except where a file or the notices below specify otherwise. Individual contributors
retain copyright in their contributions. The project license grants only rights
that the relevant copyright holders can grant.

## Eigen 3.4.0

The files in `third_party/eigen3/` are unmodified upstream Eigen headers and
license texts. They are not relicensed under MIT by AlgorithmAssets.

Eigen is primarily licensed under MPL-2.0; certain files have different or
additional terms. Consult each file's notices and the upstream `COPYING.*` files
for the applicable terms and copyright holders. The presence of a license text
does not mean that every Eigen file is governed by that license.

The pinned source, archive hash and individual file hashes are recorded in
`provenance/dependencies/eigen-3.4.0.json`. AlgorithmAssets enables its Eigen backend
explicitly with `EIGEN_MPL2_ONLY`; this configuration does not replace upstream
licensing or waive distribution obligations. Additional external backends are
not enabled by this integration.

In the source checkout, the upstream license texts are in `third_party/eigen3/`.
In a CMake installation, they are in `share/AlgorithmAssets/licenses/eigen3/`
relative to the installation prefix (or the configured CMake data directory).
The redistributed Eigen headers are installed alongside the library headers.
Retain the applicable upstream notices and make covered source available as
required by the relevant upstream terms when distributing software using Eigen.

## Reference material and historical evidence

The root MIT license does not grant new rights over third-party source excerpts,
problem statements, platform screenshots or other external material retained for
reference or validation. In particular, the archived input template
`provenance/geometry2d-template.cpp.txt`, external material under `provenance/`,
and judge evidence under `judge/evidence/` retain their original rights and terms.
Public availability, a source URL or a recorded hash is not a license grant.
Original metadata written for this project is covered by the project MIT license.

The MIT license applies to project-authored portions of judge adapters and bundles;
it does not override notices or rights in any incorporated third-party material.
Historical judge snapshots remain unchanged and retain their recorded provenance.
No new rights to external reference repositories are granted by this project.

Do not treat the project license as permission to relicense third-party files.
Keep source attribution and applicable notices when reusing or redistributing them.
