# XWord development guide

XWord 2.0 is a C++17 / Python OOXML Transitional generator. Ordinary generation
does not require Office. Word performs layout and evaluates layout-dependent
fields and native bibliography formatting.

## Build and test

```powershell
cmake -S . -B build/release -G "Visual Studio 17 2022" -A x64
cmake --build build/release --config Release
ctest --test-dir build/release -C Release --output-on-failure
cmake --install build/release --prefix dist --config Release
```

Use `-T v141` in a fresh build directory to check the supported legacy toolset.
Debug and Release consumers must use the matching DLL and CRT. 2.0 changes ABI;
rebuild consumers rather than replacing a 1.x DLL in place.

For Python set `XWORD_BUILD_PYTHON=ON`, `pybind11_DIR` (2.13.6), and
`Python_EXECUTABLE`. Use a fresh build directory when changing interpreters.
The root project builds both the DLL and extension. Install COMPONENT python
to obtain a self-contained package. `pyproject.toml` supplies the regular
source-wheel backend; `tools/package_built_wheel.py` packages an already built
CMake install without additional Python build dependencies.

CTest covers basic generation/templates, equations, v2 features, regressions,
and Python ownership/package parity when enabled. Checks must evaluate in
Release and must not show CRT assertion dialogs. Run the independent
`tools/OpenXmlValidator` .NET tool on generated fixtures before release.

## Architecture

- `Document` owns ordered section bodies and document-level styles, annotations,
  sources and metadata. Programmatic generation and template filling are separate
  modes; unsupported mode combinations must fail explicitly.
- `Content` is the shared ordered block model for bodies, cells, headers/footers
  and notes. Objects have stable addresses while more content is appended.
- `Section` owns page geometry, columns, numbering and three header/footer slots.
  Absent slots inherit, created empty slots explicitly suppress inherited content.
- `Paragraph`, `Image`, `Table` and `BulletList` serialize structured fragments.
  Internal `xw` placeholders are resolved only by the complete package writer.
- `internal/Package` allocates part-local relationships and unique resources,
  resolves fields and references, validates structure, and commits a temporary ZIP.
- pugixml 1.14 is vendored under `thirdparty/pugixml`; miniz handles ZIP files.
  Template replacement operates on XML nodes and preserves unmodified parts.
- Formatting uses explicit `Length` units and tri-state inheritance. Public value
  types are not ABI-stable across major versions. Business APIs use camelCase in
  C++ and snake_case in Python, with `reference_internal` for owned references.

## Documentation

Use `docs/api.md`, `docs/api-reference.md`, `docs/migration-2.0.md`,
`docs/capabilities.md` and `docs/validation.md` as the current API/scope reference.
Regenerate the method inventory with `tools/generate_api_reference.py` using the
matching compiled Python module. Do not claim schema validation proves visual
layout, page counts or compatibility with Word versions that were not tested.
