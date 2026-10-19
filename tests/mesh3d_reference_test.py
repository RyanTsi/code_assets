"""Independent FEM gradient assembly, SciPy solve, and VTU topology/field verification."""

from pathlib import Path
import subprocess
import sys
import xml.etree.ElementTree as ET

import numpy as np
from scipy.io import mmread
from scipy.sparse.linalg import spsolve

exe = Path(sys.argv[1]).resolve()
directory = Path(sys.argv[2]).resolve()
directory.mkdir(parents=True, exist_ok=True)
process = subprocess.run([str(exe), str(directory)], check=True, capture_output=True, text=True)
points = []
faces = []
for line in (directory / "surface.obj").read_text().splitlines():
    tokens = line.split()
    if not tokens:
        continue
    if tokens[0] == "v":
        points.append([float(x) for x in tokens[1:4]])
    if tokens[0] == "f":
        faces.append([int(x.split("/")[0]) - 1 for x in tokens[1:]])
points = np.array(points)
faces = np.array(faces)
n = len(points)
reference = np.zeros((n, n))
for face in faces:
    p = points[face]
    normal = np.cross(p[1] - p[0], p[2] - p[0])
    area = np.linalg.norm(normal) / 2
    gradients = np.array(
        [
            np.cross(normal, p[2] - p[1]),
            np.cross(normal, p[0] - p[2]),
            np.cross(normal, p[1] - p[0]),
        ]
    ) / np.dot(normal, normal)
    reference[np.ix_(face, face)] += area * (gradients @ gradients.T)
matrix = mmread(directory / "stiffness.mtx").tocsr()
np.testing.assert_allclose(matrix.toarray(), reference, atol=2e-13, rtol=2e-13)
np.testing.assert_allclose(matrix @ np.ones(n), 0, atol=2e-13)
field = np.genfromtxt(directory / "solution.csv", delimiter=",", names=True)
fixed = np.flatnonzero(
    (points[:, 0] == 0) | (points[:, 0] == 1) | (points[:, 1] == 0) | (points[:, 1] == 1)
)
free = np.setdiff1d(np.arange(n), fixed)
values = points[:, 0] + 2 * points[:, 1]
solution = spsolve(matrix[free][:, free], -matrix[free][:, fixed] @ values[fixed])
np.testing.assert_allclose(field["value"][free], solution, atol=2e-12, rtol=2e-12)
np.testing.assert_allclose(field["value"], values, atol=2e-12, rtol=2e-12)
piece = ET.parse(directory / "analysis.vtu").find("UnstructuredGrid/Piece")
assert piece.attrib["NumberOfPoints"] == "5" and piece.attrib["NumberOfCells"] == "2"
arrays = {
    item.attrib["Name"]: np.fromstring(item.text, sep=" ")
    for item in piece.findall("Cells/DataArray")
}
np.testing.assert_array_equal(arrays["offsets"], [4, 8])
np.testing.assert_array_equal(arrays["types"], [10, 10])
vertices = np.fromstring(piece.find("Points/DataArray").text, sep=" ").reshape(-1, 3)
cells = arrays["connectivity"].astype(int).reshape(-1, 4)
volume = 0.0
for cell in cells:
    p = vertices[cell]
    v = np.linalg.det((p[1:] - p[0]).T) / 6
    assert v > 0
    volume += v
np.testing.assert_allclose(volume, 1 / 3)
np.testing.assert_array_equal(
    np.fromstring(piece.find("PointData/DataArray").text, sep=" "), [0, 1, 2, 3, 4]
)
np.testing.assert_array_equal(np.fromstring(piece.find("CellData/DataArray").text, sep=" "), [3, 5])
print(process.stdout.strip())
print(
    f"Independent reference: {n} vertices/{len(faces)} triangles; FEM stiffness max error {np.max(np.abs(matrix.toarray()-reference)):.3g}; SciPy solution and VTU cells/fields passed"
)
