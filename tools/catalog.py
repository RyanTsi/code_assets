"""Small, fail-closed catalog matcher. No natural-language or novel-plan search."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = {
    "schema_version", "id", "kind", "name", "header", "documentation", "problem",
    "parameters", "requires", "capabilities", "input", "output", "constraints",
    "complexity", "memory_lower_bound", "relations", "evidence", "provenance",
}
CAPABILITIES = {
    "persistence", "checkpoint", "rollback", "undo", "redo", "arbitrary_deletion",
    "concurrent_reads", "concurrent_writes",
}
RELATIONS = {"uses", "alternative", "composes_into", "representation_for"}
STATUSES = {"supported", "unsupported", "conditional", "unknown"}
COMMON_QUERY_FIELDS = {
    "problem", "n", "m", "q", "value_bytes", "capabilities", "max_memory_bytes", "max_time_ms",
}


def load_assets() -> list[dict]:
    return [json.loads(path.read_text(encoding="utf-8"))
            for path in sorted((ROOT / "contracts").glob("*.json"))]


def local_file(relative: str) -> Path:
    path = (ROOT / relative.split("#", 1)[0]).resolve()
    if not path.is_relative_to(ROOT) or not path.is_file():
        raise ValueError(f"Missing or nonlocal asset file: {relative}")
    return path


def validate(assets: list[dict]) -> None:
    if not assets:
        raise ValueError("The catalog contains no assets")
    ids = [asset.get("id") for asset in assets]
    if len(set(ids)) != len(ids):
        raise ValueError("Duplicate asset IDs")
    documentation_owners: dict[str, str] = {}
    for asset in assets:
        missing = REQUIRED - asset.keys()
        if missing:
            raise ValueError(f"{asset.get('id')}: missing {sorted(missing)}")
        if asset["schema_version"] != 1 or asset["kind"] not in {"algorithm", "representation", "recipe"}:
            raise ValueError(f"{asset['id']}: unsupported schema or kind")
        if "display_name" in asset and (not isinstance(asset["display_name"], str) or not asset["display_name"].strip()):
            raise ValueError(f"{asset['id']}: display_name must be nonempty text")
        if not CAPABILITIES.issubset(asset["capabilities"]):
            raise ValueError(f"{asset['id']}: missing capability declarations")
        if any(value not in STATUSES for value in asset["capabilities"].values()):
            raise ValueError(f"{asset['id']}: invalid capability status")
        if not isinstance(asset["requires"], dict) or not isinstance(asset["parameters"], list):
            raise ValueError(f"{asset['id']}: invalid matching specification")
        if any(not isinstance(p, str) or not p for p in asset["parameters"]) or len(set(asset["parameters"])) != len(asset["parameters"]):
            raise ValueError(f"{asset['id']}: invalid parameter names")
        for rule in asset["requires"].values():
            if isinstance(rule, dict) and (len(rule) != 1 or
                                           next(iter(rule)) not in {"one_of", "subset_of"}):
                raise ValueError(f"{asset['id']}: unsupported predicate")
        if not isinstance(asset["constraints"].get("max_n"), int):
            raise ValueError(f"{asset['id']}: max_n must be an integer")
        local_file(asset["header"])
        documentation = asset["documentation"]
        if not isinstance(documentation, str) or "#" in documentation:
            raise ValueError(f"{asset['id']}: documentation must be an independent page, not a section")
        page = local_file(documentation)
        directory = {"algorithm": "algorithms", "representation": "representations",
                     "recipe": "recipes"}[asset["kind"]]
        if page.suffix != ".md" or page.parent != ROOT / "docs" / directory:
            raise ValueError(f"{asset['id']}: documentation must be under docs/{directory}/")
        page_key = str(page).casefold()
        if page_key in documentation_owners:
            raise ValueError(f"{asset['id']}: documentation already owned by {documentation_owners[page_key]}")
        documentation_owners[page_key] = asset["id"]
        local_file(asset["evidence"]["tests"])
        local_file(asset["evidence"]["example"])
        for relation in asset["relations"]:
            if relation["type"] not in RELATIONS or relation["target"] not in ids:
                raise ValueError(f"{asset['id']}: invalid relation {relation}")
            if "condition" in relation and (not isinstance(relation["condition"], str) or not relation["condition"].strip()):
                raise ValueError(f"{asset['id']}: relation condition must be nonempty text")
        model = asset["memory_lower_bound"]
        if not isinstance(model.get("constant"), int) or model["constant"] < 0:
            raise ValueError(f"{asset['id']}: invalid memory constant")
        for term in model["terms"]:
            if not isinstance(term["coefficient"], int) or term["coefficient"] < 0:
                raise ValueError(f"{asset['id']}: invalid memory coefficient")
            if not term["factors"] or any(factor not in set(asset["parameters"]) | {"value_bytes"}
                                          for factor in term["factors"]):
                raise ValueError(f"{asset['id']}: invalid memory factors")
    # Dependency cycles are different from harmless bidirectional alternatives.
    dependencies = {asset["id"]: [relation["target"] for relation in asset["relations"]
                                   if relation["type"] == "uses"] for asset in assets}
    completed: set[str] = set()

    def visit(node: str, stack: set[str]) -> None:
        if node in stack:
            raise ValueError(f"Dependency cycle through {node}")
        if node not in completed:
            for child in dependencies[node]:
                visit(child, stack | {node})
            completed.add(node)

    for node in dependencies:
        visit(node, set())


def strictly_equal(first, second) -> bool:
    return type(first) is type(second) and first == second


def satisfies(value, expected) -> bool:
    if not isinstance(expected, dict):
        return strictly_equal(value, expected)
    if "one_of" in expected:
        return any(strictly_equal(value, option) for option in expected["one_of"])
    return (isinstance(value, list) and
            all(any(strictly_equal(item, option) for option in expected["subset_of"])
                for item in value))


def match(query: dict, assets: list[dict]) -> dict:
    if not isinstance(query, dict) or not isinstance(query.get("problem"), str):
        raise ValueError("A query must supply a string problem ID")
    numeric_fields = {"n", "m", "q", "value_bytes", "max_memory_bytes", "max_time_ms"}
    numeric_fields.update(parameter for asset in assets for parameter in asset["parameters"])
    for key in numeric_fields:
        value = query.get(key)
        if value is not None and (type(value) is not int or value < 0):
            raise ValueError(f"{key} must be a nonnegative integer")
    if query.get("value_bytes") == 0:
        raise ValueError("value_bytes must be positive")
    requested = query.get("capabilities", {})
    if not isinstance(requested, dict) or any(value not in {"supported", "unsupported"}
                                            for value in requested.values()):
        raise ValueError("Requested capabilities must be supported or unsupported")

    evaluations = []
    for asset in assets:
        if asset["problem"] != query["problem"]:
            continue
        missing, rejected = [], []
        for key in query.keys() - COMMON_QUERY_FIELDS - asset["requires"].keys() - set(asset["parameters"]):
            rejected.append(f"No registered rule can verify requirement '{key}'")
        for key, expected in asset["requires"].items():
            if key not in query or query[key] is None:
                missing.append(key)
            elif not satisfies(query[key], expected):
                rejected.append(f"{key}={query[key]!r} does not satisfy {expected!r}")
        for key in asset["parameters"]:
            if query.get(key) is None:
                missing.append(key)
        if query.get("n") is not None and query["n"] > asset["constraints"]["max_n"]:
            rejected.append("n exceeds the implementation's declared integer range")
        for capability, wanted in requested.items():
            actual = asset["capabilities"].get(capability, "unknown")
            if actual in {"unknown", "conditional"}:
                missing.append(f"capabilities.{capability}: {actual}")
            elif actual != wanted:
                rejected.append(f"capabilities.{capability} is {actual}, requested {wanted}")
        lower_bound = asset["memory_lower_bound"]["constant"]
        bound_known = True
        for term in asset["memory_lower_bound"]["terms"]:
            value = term["coefficient"]
            for factor in term["factors"]:
                if query.get(factor) is None:
                    bound_known = False
                    if query.get("max_memory_bytes") is not None:
                        missing.append(factor)
                    break
                value *= query[factor]
            else:
                lower_bound += value
        over_budget = (not rejected and not missing and bound_known and
                       query.get("max_memory_bytes") is not None and
                       lower_bound > query["max_memory_bytes"])
        evaluations.append({"id": asset["id"], "kind": asset["kind"],
                            "missing": sorted(set(missing)), "rejected": rejected,
                            "over_budget": over_budget,
                            "payload_lower_bound_bytes": lower_bound if bound_known else None,
                            "complexity": asset["complexity"], "header": asset["header"]})
    eligible = [item for item in evaluations
                if not item["missing"] and not item["rejected"] and not item["over_budget"]]
    singles = [item for item in eligible if item["kind"] != "recipe"]
    candidates = singles or eligible
    if candidates:
        status = "found_algorithm" if singles else "found_recipe"
    elif any(item["missing"] and not item["rejected"] for item in evaluations):
        status = "needs_information"
    elif any(item["over_budget"] for item in evaluations):
        status = "budget_exceeded"
    else:
        status = "not_covered"
    return {
        "catalog_version": "0.1.0-pilot-5", "status": status, "candidates": candidates,
        "evaluations": evaluations,
        "resource_status": "unknown" if candidates else "not_applicable",
        "scope": "All registered implementations and recipes for this problem ID; no novel composition search",
        "limitation": "A metadata match assumes the declared semantic facts. Payload lower bounds can reject budgets; they cannot certify peak memory or latency.",
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["validate", "query"])
    parser.add_argument("query_file", nargs="?")
    args = parser.parse_args()
    assets = load_assets()
    validate(assets)
    if args.command == "validate":
        print(f"Validated {len(assets)} asset contracts and their file/dependency links")
    else:
        if not args.query_file:
            parser.error("query requires a JSON file")
        query = json.loads(Path(args.query_file).read_text(encoding="utf-8-sig"))
        print(json.dumps(match(query, assets), ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
