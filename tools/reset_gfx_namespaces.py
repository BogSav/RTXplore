from pathlib import Path
import re

src_dir = Path(r"E:/RTXplore/engine/gfx/src")

for path in src_dir.glob("*.cpp"):
    text = path.read_text(encoding="utf-8")
    text = text.replace("\r\n", "\n")
    text = text.lstrip("\ufeff").lstrip("ï»¿")

    text = re.sub(r"\n?\}\s*//\s*namespace\s+engine::gfx\s*", "\n", text)
    text = re.sub(r"\s*namespace\s+engine::gfx\s*\{\s*", "\n", text)

    lines = [line.rstrip("\n") for line in text.split("\n")]

    while lines and lines[0].strip() == "":
        lines.pop(0)

    prelude = []
    idx = 0
    while idx < len(lines):
        stripped = lines[idx].strip()
        if stripped == "" or stripped.startswith("#include") or stripped.startswith("#pragma"):
            prelude.append(lines[idx])
            idx += 1
            continue
        break

    body_lines = lines[idx:]
    body_text = "\n".join(body_lines).strip("\n")

    new_lines = []
    if prelude:
        new_lines.append("\n".join(prelude).rstrip())
        new_lines.append("")

    new_lines.append("namespace engine::gfx")
    new_lines.append("{")

    if body_text:
        new_lines.append("")
        new_lines.append(body_text)
        new_lines.append("")

    new_lines.append("}  // namespace engine::gfx")

    result = "\n".join([line for line in new_lines if line is not None]) + "\n"
    path.write_text(result, encoding="utf-8")
