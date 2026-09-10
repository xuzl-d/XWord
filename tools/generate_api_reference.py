"""Generate the checked-in Python/C++ method and value-field inventory.

Run with the matching Python interpreter: script.py <native-module-directory>.
"""
import importlib
import os
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
module_dir = Path(sys.argv[1]).resolve()
sys.path.insert(0, str(module_dir))
handle = os.add_dll_directory(str(module_dir)) if os.name == "nt" else None
module = importlib.import_module("_native")
bindings = (root / "src/pybind/bindings.cpp").read_text(encoding="utf-8")
rows = ["# C++／Python 接口对照", "", "由实际编译的 Python 绑定生成。使用、单位、默认行为和限制见 [使用手册](api.md)。", "", "C++ 参数与重载以 `include/xword/` 头文件为准。Python 字符串对应 UTF-8；C++ 宽字符串重载共享同一实现。`CellImage` 为 `Image` 别名。", ""]
seen = set()
for name in sorted(dir(module)):
    value = getattr(module, name)
    if name.startswith("_") or not isinstance(value, type) or id(value) in seen:
        continue
    seen.add(id(value))
    rows += [f"## {name}", ""]
    if hasattr(value, "__members__"):
        rows += [", ".join(f"`{key}`" for key in value.__members__), ""]
        continue
    fields = [key for key, attr in value.__dict__.items() if isinstance(attr, property) and not key.startswith("_")]
    if fields:
        rows += ["值字段：" + ", ".join(f"`{field}`" for field in fields) + "。", ""]
    for member in sorted(value.__dict__):
        if member.startswith("_") and member != "__init__":
            continue
        attr = getattr(value, member)
        if not callable(attr) or not getattr(attr, "__doc__", None):
            continue
        match = re.search(r'\.def(?:_static)?\("' + re.escape(member) + r'",[^\n]*&' + re.escape(name) + r'::(\w+)', bindings)
        lambda_match = re.search(r'\.def\("' + re.escape(member) + r'",\s*\[\]\(' + re.escape(name) + r'&[^\n]*return \w+\.(\w+)\(', bindings)
        cpp = match.group(1) if match else (lambda_match.group(1) if lambda_match else (name if member == "__init__" else "见头文件／继承接口"))
        doc = attr.__doc__.replace("_native.", "").strip()
        doc = re.sub(r'<[^>]+ object at 0x[0-9a-fA-F]+>', '<默认值对象>', doc)
        rows += [f"### `{name}.{member}` → C++ `{cpp}`", "", "```text", doc, "```", ""]
    bases = [base.__name__ for base in value.__bases__ if base.__module__ == "_native"]
    if bases:
        rows += ["同时继承：" + ", ".join(f"`{base}`" for base in bases) + " 的接口。", ""]
(root / "docs/api-reference.md").write_text("\n".join(rows), encoding="utf-8")
