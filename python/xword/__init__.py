"""XWord — .docx (Office Open XML) document generator.

Uses pybind11 native C++ backend for maximum performance.
"""

from ._native import (
    # Enums
    Alignment,
    TableStyle,
    PageSize,
    Orientation,
    ListType,
    HeadingNumFormat,
    EquationMode,
    CaptionNumStyle,
    SectionBreakType,
    # Value types
    PageMargins,
    Page,
    HeadingStyle,
    RunStyle,
    # Core classes
    Document,
    Paragraph,
    Image,
    Equation,
    Table,
    Cell,
    CellImage,
    BulletList,
)
