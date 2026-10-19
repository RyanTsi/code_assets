"""Check that the redistributed Eigen files exactly match the pinned upstream snapshot."""
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
record=json.loads((ROOT/'provenance/dependencies/eigen-3.4.0.json').read_text(encoding='utf-8'))
directory=ROOT/'third_party/eigen3'
actual={p.relative_to(directory).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
        for p in directory.rglob('*') if p.is_file()}
if actual!=record['files']:
    raise SystemExit('Eigen vendor inventory/hash mismatch; do not silently modify upstream files')
print(f"Eigen {record['version']}: {len(actual)} unmodified files, archive SHA-256 {record['archive_sha256']}")
