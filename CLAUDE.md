# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & test

```bash
# Configure (requires VS 2019 with v141 toolset)
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -T v141
cmake --build build --config Release

# Run tests
ctest --test-dir build -C Release

# Install to dist/
cmake --install build --prefix dist --config Release

# Python bindings (requires the built .pyd in python/xword/)
pip install -e .
```

## Architecture

XWord is a **C++17 .docx (OOXML) generator** — a shared library (DLL) that builds valid OPC/ZIP packages containing `word/document.xml`, styles, numbering, footnotes, headers/footers, media, and relationships. It ships with **pybind11 Python bindings** so the same engine is usable from Python.

### Public API (headers in `include/xword/`)

Every public class uses **Pimpl** (`std::unique_ptr<Impl>`) — class layout is a single pointer, so member changes don't break ABI.

- **`Document`** — top-level builder: page setup, headings, paragraphs, tables, images, lists, equations, headers/footers, TOC, footnotes, section breaks, template engine. Entry point for all document creation.
- **`Paragraph`** — text container with runs (`addRun`), fields (PAGE/NUMPAGES), inline equations, footnote refs. Fluent API throughout.
- **`Table`** / **`Cell`** / **`CellImage`** — grid-border tables, header rows, cell merging, cell-level images/equations.
- **`Image`** — PNG/EMF/WMF/SVG embedding with auto size detection and caption numbering.
- **`Equation`** — LaTeX → OMML translator (Greek letters, fractions, roots, sums, integrals, matrices, accents, trig functions).
- **`BulletList`** — bullet and ordered lists with multi-level indent.
- **`Types.hpp`** — enums (`Alignment`, `TableStyle`, `PageSize`, `Orientation`, `HeadingNumFormat`, `CaptionNumStyle`, `SectionBreakType`) and value types (`Page`, `PageMargins`, `HeadingStyle`).
- **`RunStyle`** (`Run.hpp`) — value type for text formatting (bold, italic, underline, font, size, color).
- **`Format.hpp`** — `xword::format()` printf-style string helper.
- **`xword.hpp`** — umbrella header.

### Source layout (`src/`)

Each public class has its own `.cpp` with the `Impl` struct defined there. `src/internal/ZipWriter.cpp`/`ZipReader.cpp` handle the OPC package (ZIP container) via miniz (static lib in `thirdparty/miniz/`). `src/pybind/bindings.cpp` maps the full C++ API to Python with pybind11.

### Template engine

`Document::open("template.docx")` loads an existing .docx as a template. `set(key, value)` fills `${key}` placeholders; `{%if key%}` / `{%else%}` / `{%endif%}` control conditional blocks. No nested ifs. Truth is `"false"`/`"0"`/`""` → false, else true. The template XML is parsed and rewritten, then re-zipped on save.

### Python package (`python/xword/`)

Thin wrapper: `__init__.py` re-exports everything from the compiled `_native` module. The native `.pyd` is declared as package data in `pyproject.toml`. The pybind build is handled separately by `python/pybind_build/CMakeLists.txt`.

### DLL ABI contract

Built with MSVC v141 (VS2017 toolset). Consumers must use the same MSVC major version and CRT link mode (`/MD` Release, `/MDd` Debug). Debug DLL is named `xwordd.dll`.
