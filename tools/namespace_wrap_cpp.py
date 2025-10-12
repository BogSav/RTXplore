from pathlib import Path
import re

src_dir = Path(r"E:/RTXplore/engine/gfx/src")

for path in src_dir.glob("*.cpp"):
    text = path.read_text(encoding="utf-8")
    text = text.replace("\r\n", "\n")
    text = text.lstrip("\ufeff").lstrip("ï»¿")

    text = re.sub(r"(?:\n}\s*// namespace engine::gfx\s*)+$", "\n", text)
    text = re.sub(r"^(?:\s*namespace\s+engine::gfx\s*\{\s*)+", "", text)

    lines = text.splitlines()
    if lines:
        lines[0] = lines[0].lstrip("\ufeff").lstrip("ï»¿")

    prelude_lines = []
    idx = 0
    while idx < len(lines):
        stripped = lines[idx].lstrip()
        normalized = stripped.lstrip("\ufeff").lstrip("ï»¿")
        if normalized.startswith("#") or stripped == "":
            line_to_add = lines[idx]
            if line_to_add.startswith("\ufeff") or line_to_add.startswith("ï»¿"):
                line_to_add = line_to_add.lstrip("\ufeff").lstrip("ï»¿")
            prelude_lines.append(line_to_add)
            idx += 1
            continue
        break

    body_lines = lines[idx:]
    body_text = "\n".join(body_lines).strip("\n")

    new_lines = []
    new_lines.extend(prelude_lines)
    if new_lines and new_lines[-1].strip() != "":
        new_lines.append("")
    new_lines.append("namespace engine::gfx")
    new_lines.append("{")
    if body_text:
        new_lines.append("")
        new_lines.extend(body_text.splitlines())
        if new_lines and new_lines[-1].strip() != "":
            new_lines.append("")
    new_lines.append("}  // namespace engine::gfx")

    path.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
