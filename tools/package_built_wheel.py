"""Package a CMake-installed native build using only the Python standard library.

This is an offline release fallback. The regular source build uses pyproject.toml.
Run with the same interpreter used by CMake, after installing COMPONENT python.
"""
import base64
import csv
import hashlib
import importlib.machinery
import io
from pathlib import Path
import re
import sys
import sysconfig
import zipfile

if len(sys.argv) != 3:
    raise SystemExit("Usage: package_built_wheel.py <install-prefix> <wheel-directory>")
root = Path(__file__).resolve().parents[1]
package = Path(sys.argv[1]).resolve() / "xword"
suffix = importlib.machinery.EXTENSION_SUFFIXES[0]
if not (package / ("_native" + suffix)).is_file():
    raise SystemExit("Installed native module does not match this interpreter")
version = re.search(r'^version = "([^"]+)"', (root / "pyproject.toml").read_text(), re.M).group(1)
interpreter = f"cp{sys.version_info.major}{sys.version_info.minor}"
platform = sysconfig.get_platform().replace("-", "_").replace(".", "_")
tag = f"{interpreter}-{interpreter}-{platform}"
info = f"xword_docx-{version}.dist-info"
files = {
    "xword/" + path.relative_to(package).as_posix(): path.read_bytes()
    for path in package.rglob("*")
    if path.is_file() and "__pycache__" not in path.parts and path.suffix != ".pyc"
}
files[f"{info}/METADATA"] = (
    "Metadata-Version: 2.1\nName: xword-docx\n"
    f"Version: {version}\nSummary: C++17 / Python Office Open XML document generator\n"
    "Requires-Python: >=3.8\nLicense: MIT\nDescription-Content-Type: text/markdown\n\n"
    + (root / "README.md").read_text(encoding="utf-8")
).encode("utf-8")
files[f"{info}/WHEEL"] = (
    "Wheel-Version: 1.0\nGenerator: xword-offline-packager\nRoot-Is-Purelib: false\n"
    f"Tag: {tag}\n"
).encode()
files[f"{info}/licenses/pugixml-LICENSE.md"] = (root / "thirdparty/pugixml/LICENSE.md").read_bytes()
files[f"{info}/licenses/miniz-source-license.txt"] = (root / "thirdparty/miniz/miniz.c").read_bytes().split(b"#include", 1)[0]
record = io.StringIO(newline="")
writer = csv.writer(record, lineterminator="\n")
for name, data in sorted(files.items()):
    digest = base64.urlsafe_b64encode(hashlib.sha256(data).digest()).rstrip(b"=").decode()
    writer.writerow((name, "sha256=" + digest, len(data)))
writer.writerow((f"{info}/RECORD", "", ""))
files[f"{info}/RECORD"] = record.getvalue().encode()
output = Path(sys.argv[2]).resolve()
output.mkdir(parents=True, exist_ok=True)
wheel = output / f"xword_docx-{version}-{tag}.whl"
with zipfile.ZipFile(wheel, "w", zipfile.ZIP_DEFLATED) as archive:
    for name, data in sorted(files.items()):
        archive.writestr(name, data)
print(wheel)
