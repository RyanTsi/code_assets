"""Validate independently owned asset pages and repository Markdown links."""

from __future__ import annotations

from html import unescape
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit

import catalog

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIRECTORIES = ("algorithms", "representations", "recipes")
SECTIONS = (
    "使用与设计",
    "输入与输出",
    "前置条件与约束",
    "状态与生命周期",
    "时间与空间",
    "正确性依据",
    "关联资产",
    "验证与来源",
)


def prose(text: str) -> str:
    """Ignore fenced examples: their strings and comments are not document links."""
    return re.sub(r"^```[^\n]*\n.*?^```[^\n]*$", "", text, flags=re.M | re.S)


def anchors(text: str) -> set[str]:
    content = prose(text)
    result = set(re.findall(r'<a\s+id="([^"]+)"', content))
    seen: dict[str, int] = {}
    for title in re.findall(r"^#{1,6}\s+(.+?)\s*#*\s*$", content, flags=re.M):
        title = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", title)
        title = re.sub(r"<[^>]+>", "", title)
        slug = re.sub(r"[^\w\- ]", "", unescape(title).lower()).replace(" ", "-")
        number = seen.get(slug, 0)
        result.add(slug + (f"-{number}" if number else ""))
        seen[slug] = number + 1
    return result


def check_link(page: Path, destination: str) -> None:
    url = urlsplit(destination)
    if url.scheme or url.netloc:
        return
    target = (page.parent / unquote(url.path)).resolve() if url.path else page
    if not target.is_relative_to(ROOT) or not target.exists():
        raise ValueError(f"{page.relative_to(ROOT)}: missing/nonlocal link {destination}")
    if url.fragment and target.suffix == ".md":
        if unquote(url.fragment) not in anchors(target.read_text(encoding="utf-8")):
            raise ValueError(f"{page.relative_to(ROOT)}: missing anchor {destination}")


def check() -> tuple[int, int]:
    if {p.name for p in (ROOT / "docs").glob("*.md")} != {"README.md"}:
        raise ValueError(
            "docs/ root may only contain README.md; project material belongs in docs/project/"
        )
    assets = catalog.load_assets()
    catalog.validate(assets)
    owned = {catalog.local_file(a["documentation"]) for a in assets}
    actual = {
        page.resolve()
        for directory in ASSET_DIRECTORIES
        for page in (ROOT / "docs" / directory).glob("*.md")
    }
    if owned != actual:
        raise ValueError("Asset document inventory has missing or unowned pages")
    index = (ROOT / "docs/README.md").read_text(encoding="utf-8")
    for asset in assets:
        page = catalog.local_file(asset["documentation"])
        text = page.read_text(encoding="utf-8")
        if re.findall(r"^资产 ID：`([^`]+)`", text, flags=re.M) != [asset["id"]]:
            raise ValueError(f"{page.relative_to(ROOT)}: asset identity missing or mismatched")
        for section in SECTIONS:
            if f"## {section}\n" not in text:
                raise ValueError(f"{page.relative_to(ROOT)}: missing section {section}")
        if f"({page.relative_to(ROOT / 'docs').as_posix()})" not in index:
            raise ValueError(f"{asset['id']}: missing from docs/README.md")
        if f"#include <{asset['header'].removeprefix('include/')}>" not in text:
            raise ValueError(f"{asset['id']}: missing public include example")
    pages = sorted((ROOT / "docs").rglob("*.md"))
    pages += sorted(ROOT.glob("*.md"))
    pages += [ROOT / "third_party/README.md"]
    pages += [
        page
        for directory in ("contracts", "recipes", "benchmarks", "gaps", "tests", "judge")
        for page in (ROOT / directory).glob("*.md")
    ]
    for page in pages:
        text = prose(page.read_text(encoding="utf-8"))
        # Destinations with parentheses must use angle brackets in Markdown.
        for destination in re.findall(r"\[[^\]\n]+\]\(<?([^\s<>]+?)>?\)", text):
            check_link(page, destination)
    return len(assets), len(pages)


if __name__ == "__main__":
    count, pages = check()
    print(f"Validated {count} independently owned asset documents and links in {pages} pages")
