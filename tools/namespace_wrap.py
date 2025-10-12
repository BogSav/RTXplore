from pathlib import Path
import re

root = Path(r"E:/RTXplore/engine/gfx")
skip_names = {"d3dx12.h", "HlslUtils.h"}
header_dir = root / "include" / "engine" / "gfx"
src_dir = root / "src"

files = list(header_dir.glob("*")) + list(src_dir.glob("*.cpp"))

for path in files:
    if path.name in skip_names:
        continue
    text = path.read_text()
    if re.search(r"namespace\s+engine::gfx\s*{", text):
        continue
    lines = text.splitlines()

    last_include_idx = -1
    for idx, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith("#include"):
            last_include_idx = idx
            continue
        if stripped == "":
            continue
        if stripped.startswith("#pragma once"):
            continue
        if stripped.startswith("//") or stripped.startswith("/*"):
            continue
        if last_include_idx == -1:
            break

    if last_include_idx == -1:
        insert_idx = 0
    else:
        insert_idx = last_include_idx + 1
        while insert_idx < len(lines) and lines[insert_idx].strip() == "":
            insert_idx += 1

    prefix = lines[:insert_idx]
    rest = lines[insert_idx:]

    new_lines = []
    new_lines.extend(prefix)
    if new_lines and new_lines[-1] != "":
        new_lines.append("")
    new_lines.append("namespace engine::gfx")
    new_lines.append("{")
    if rest and rest[0] != "":
        new_lines.append("")
    new_lines.extend(rest)
    if new_lines and new_lines[-1] != "":
        new_lines.append("")
    new_lines.append("}  // namespace engine::gfx")

    path.write_text("\n".join(new_lines) + "\n")
