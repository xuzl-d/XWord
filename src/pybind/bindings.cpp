#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "xword/xword.hpp"

namespace py = pybind11;
using namespace xword;

PYBIND11_MODULE(_native, m) {
    m.doc() = "XWord .docx document generator (native C++ backend)";

    // ═══════════════════════════════════════════════════════════
    //  Enums
    // ═══════════════════════════════════════════════════════════

    py::enum_<Alignment>(m, "Alignment")
        .value("LEFT", Alignment::Left)
        .value("CENTER", Alignment::Center)
        .value("RIGHT", Alignment::Right)
        .value("JUSTIFY", Alignment::Justify);

    py::enum_<TableStyle>(m, "TableStyle")
        .value("NONE", TableStyle::None)
        .value("GRID", TableStyle::Grid)
        .value("LIGHT", TableStyle::Light);

    py::enum_<PageSize>(m, "PageSize")
        .value("A4", PageSize::A4)
        .value("LETTER", PageSize::Letter);

    py::enum_<Orientation>(m, "Orientation")
        .value("PORTRAIT", Orientation::Portrait)
        .value("LANDSCAPE", Orientation::Landscape);

    py::enum_<ListType>(m, "ListType")
        .value("BULLET", ListType::Bullet)
        .value("ORDERED", ListType::Ordered);

    py::enum_<HeadingNumFormat>(m, "HeadingNumFormat")
        .value("DECIMAL", HeadingNumFormat::Decimal)
        .value("CHAPTER", HeadingNumFormat::Chapter);

    py::enum_<EquationMode>(m, "EquationMode")
        .value("INLINE", EquationMode::Inline)
        .value("DISPLAY", EquationMode::Display);

    py::enum_<CaptionNumStyle>(m, "CaptionNumStyle")
        .value("SEQUENTIAL", CaptionNumStyle::Sequential)
        .value("BY_CHAPTER", CaptionNumStyle::ByChapter);

    py::enum_<SectionBreakType>(m, "SectionBreakType")
        .value("NEXT_PAGE", SectionBreakType::NextPage)
        .value("CONTINUOUS", SectionBreakType::Continuous)
        .value("EVEN_PAGE", SectionBreakType::EvenPage)
        .value("ODD_PAGE", SectionBreakType::OddPage);

    // ═══════════════════════════════════════════════════════════
    //  Value types
    // ═══════════════════════════════════════════════════════════

    py::class_<PageMargins>(m, "PageMargins")
        .def(py::init<>())
        .def_readwrite("top", &PageMargins::top)
        .def_readwrite("bottom", &PageMargins::bottom)
        .def_readwrite("left", &PageMargins::left)
        .def_readwrite("right", &PageMargins::right);

    py::class_<Page>(m, "Page")
        .def(py::init<>())
        .def("set_size", &Page::setSize)
        .def("set_orientation", &Page::setOrientation)
        .def("set_margins", &Page::setMargins,
             py::arg("top"), py::arg("bottom"), py::arg("left"), py::arg("right"))
        .def_readwrite("size", &Page::size)
        .def_readwrite("orientation", &Page::orientation)
        .def_readwrite("margins", &Page::margins);

    py::class_<HeadingStyle>(m, "HeadingStyle")
        .def(py::init<>())
        .def("set_font", &HeadingStyle::setFont)
        .def("set_font_size", &HeadingStyle::setFontSize)
        .def("set_bold", &HeadingStyle::setBold, py::arg("b") = true)
        .def("set_italic", &HeadingStyle::setItalic, py::arg("i") = true)
        .def("set_color", &HeadingStyle::setColor)
        .def("set_line_spacing", &HeadingStyle::setLineSpacing)
        .def("set_space_before", &HeadingStyle::setSpaceBefore)
        .def("set_space_after", &HeadingStyle::setSpaceAfter)
        .def("set_alignment", &HeadingStyle::setAlignment)
        .def_readwrite("font", &HeadingStyle::font)
        .def_readwrite("font_size", &HeadingStyle::fontSize)
        .def_readwrite("bold", &HeadingStyle::bold)
        .def_readwrite("italic", &HeadingStyle::italic)
        .def_readwrite("color", &HeadingStyle::color)
        .def_readwrite("line_spacing", &HeadingStyle::lineSpacing)
        .def_readwrite("space_before", &HeadingStyle::spaceBefore)
        .def_readwrite("space_after", &HeadingStyle::spaceAfter);

    py::class_<RunStyle>(m, "RunStyle")
        .def(py::init<>())
        .def("bold", [](RunStyle& s, bool on) -> RunStyle& { return s.bold(on); },
             py::arg("on") = true)
        .def("italic", [](RunStyle& s, bool on) -> RunStyle& { return s.italic(on); },
             py::arg("on") = true)
        .def("underline", [](RunStyle& s, bool on) -> RunStyle& { return s.underline(on); },
             py::arg("on") = true)
        .def("font_size", [](RunStyle& s, double pt) -> RunStyle& { return s.fontSize(pt); })
        .def("color", [](RunStyle& s, const std::string& c) -> RunStyle& { return s.color(c); })
        .def("font", [](RunStyle& s, const std::string& n) -> RunStyle& { return s.font(n); });

    // ═══════════════════════════════════════════════════════════
    //  CellImage
    // ═══════════════════════════════════════════════════════════

    py::class_<CellImage>(m, "CellImage")
        .def(py::init<>())
        .def("set_caption", &CellImage::setCaption)
        .def_readwrite("filepath", &CellImage::filepath)
        .def_readwrite("width", &CellImage::width)
        .def_readwrite("height", &CellImage::height)
        .def_readwrite("skipped", &CellImage::skipped)
        .def_readwrite("caption", &CellImage::caption);

    // ═══════════════════════════════════════════════════════════
    //  Equation
    // ═══════════════════════════════════════════════════════════

    py::class_<Equation>(m, "Equation")
        .def(py::init<const std::string&, EquationMode>(),
             py::arg("latex"), py::arg("mode") = EquationMode::Inline)
        .def("set_mode", &Equation::setMode)
        .def("set_style", &Equation::setStyle)
        .def("latex", &Equation::latex)
        .def("mode", &Equation::mode)
        .def("to_xml", &Equation::toXml);

    // ═══════════════════════════════════════════════════════════
    //  Paragraph
    // ═══════════════════════════════════════════════════════════

    py::class_<Paragraph>(m, "Paragraph")
        .def(py::init<>())
        .def("add_run",
             py::overload_cast<const std::string&>(&Paragraph::addRun),
             py::return_value_policy::reference_internal)
        .def("add_run",
             py::overload_cast<const std::string&, const RunStyle&>(&Paragraph::addRun),
             py::return_value_policy::reference_internal)
        .def("add_equation", &Paragraph::addEquation,
             py::return_value_policy::reference_internal)
        .def("add_page_number", &Paragraph::addPageNumber,
             py::return_value_policy::reference_internal)
        .def("add_page_count", &Paragraph::addPageCount,
             py::return_value_policy::reference_internal)
        .def("add_footnote_ref", &Paragraph::addFootnoteRef,
             py::return_value_policy::reference_internal)
        .def("set_alignment", &Paragraph::setAlignment,
             py::return_value_policy::reference_internal)
        .def("set_first_line_indent", &Paragraph::setFirstLineIndent,
             py::return_value_policy::reference_internal)
        .def("set_first_line_indent_chars", &Paragraph::setFirstLineIndentChars,
             py::return_value_policy::reference_internal,
             py::arg("chars"), py::arg("font_size_pt") = 12)
        .def("set_spacing_after", &Paragraph::setSpacingAfter,
             py::return_value_policy::reference_internal)
        .def("set_spacing_before", &Paragraph::setSpacingBefore,
             py::return_value_policy::reference_internal)
        .def("set_default_run_style", &Paragraph::setStyle,
             py::return_value_policy::reference_internal)
        .def("to_xml", &Paragraph::toXml);

    // ═══════════════════════════════════════════════════════════
    //  Image
    // ═══════════════════════════════════════════════════════════

    py::class_<Image>(m, "Image")
        .def(py::init<const std::string&>())
        .def("set_size", &Image::setSize)
        .def("set_alignment", &Image::setAlignment)
        .def("set_caption", &Image::setCaption)
        .def_property_readonly("filepath", &Image::filepath)
        .def_property_readonly("width", &Image::width)
        .def_property_readonly("height", &Image::height)
        .def_property_readonly("alignment", &Image::alignment)
        .def_property_readonly("has_alignment", &Image::hasAlignment)
        .def_property_readonly("caption", &Image::caption)
        .def_property_readonly("skipped", &Image::skipped);

    // ═══════════════════════════════════════════════════════════
    //  Cell
    // ═══════════════════════════════════════════════════════════

    py::class_<Cell>(m, "Cell")
        .def(py::init<>())
        .def("add_paragraph",
             py::overload_cast<const std::string&>(&Cell::addParagraph),
             py::return_value_policy::reference_internal,
             py::arg("text") = "")
        .def("add_paragraph",
             py::overload_cast<const std::string&, const RunStyle&>(&Cell::addParagraph),
             py::return_value_policy::reference_internal)
        .def("add_equation", &Cell::addEquation,
             py::return_value_policy::reference_internal)
        .def("add_image",
             py::overload_cast<const std::string&>(&Cell::addImage),
             py::return_value_policy::reference_internal)
        .def("add_image",
             py::overload_cast<const std::string&, int, int>(&Cell::addImage),
             py::return_value_policy::reference_internal);

    // ═══════════════════════════════════════════════════════════
    //  Table
    // ═══════════════════════════════════════════════════════════

    py::class_<Table>(m, "Table")
        .def(py::init<int, int>())
        .def("set_header_row", &Table::setHeaderRow,
             py::return_value_policy::reference_internal)
        .def("set_style", &Table::setBorderStyle,
             py::return_value_policy::reference_internal)
        .def("set_run_style", &Table::setStyle,
             py::return_value_policy::reference_internal)
        .def("set_caption", &Table::setCaption,
             py::return_value_policy::reference_internal)
        .def("caption", &Table::caption)
        .def("cell", py::overload_cast<int, int>(&Table::cell),
             py::return_value_policy::reference_internal)
        .def("merge_cells", &Table::mergeCells,
             py::return_value_policy::reference_internal)
        .def("rows", &Table::rows)
        .def("cols", &Table::cols)
        .def("set_column_width", &Table::setColumnWidth,
             py::return_value_policy::reference_internal)
        .def("set_column_widths", &Table::setColumnWidths,
             py::return_value_policy::reference_internal)
        .def("to_xml", &Table::toXml);

    // ═══════════════════════════════════════════════════════════
    //  BulletList
    // ═══════════════════════════════════════════════════════════

    py::class_<BulletList>(m, "BulletList")
        .def(py::init<ListType>(), py::arg("type") = ListType::Bullet)
        .def("add_item", &BulletList::addItem,
             py::return_value_policy::reference_internal)
        .def("set_level", &BulletList::setLevel,
             py::return_value_policy::reference_internal)
        .def("set_num_id", &BulletList::setNumId,
             py::return_value_policy::reference_internal)
        .def("num_id", &BulletList::numId)
        .def("to_xml", &BulletList::toXml);

    // ═══════════════════════════════════════════════════════════
    //  Document
    // ═══════════════════════════════════════════════════════════

    py::class_<Document>(m, "Document")
        .def(py::init<>())
        // Page settings
        .def("set_page", &Document::setPage,
             py::return_value_policy::reference_internal)
        .def("set_default_paragraph_indent", &Document::setDefaultParagraphIndent,
             py::return_value_policy::reference_internal,
             py::arg("chars") = 2, py::arg("font_size_pt") = 12)
        // Body run style
        .def("set_body_font", &Document::setBodyFont,
             py::return_value_policy::reference_internal,
             py::arg("east_asia"), py::arg("ascii") = "", py::arg("h_ansi") = "")
        .def("set_body_font_size", &Document::setBodyFontSize,
             py::return_value_policy::reference_internal)
        .def("set_body_line_spacing", &Document::setBodyLineSpacing,
             py::return_value_policy::reference_internal)
        .def("set_body_run_style", &Document::setBodyRunStyle,
             py::return_value_policy::reference_internal)
        .def("set_display_equation_style", &Document::setDisplayEquationStyle,
             py::return_value_policy::reference_internal)
        .def("set_table_run_style", &Document::setTableRunStyle,
             py::return_value_policy::reference_internal)
        // Headings
        .def("add_heading", &Document::addHeading,
             py::return_value_policy::reference_internal)
        .def("add_heading_no_num", &Document::addHeadingNoNum,
             py::return_value_policy::reference_internal)
        .def("set_heading_style", &Document::setHeadingStyle,
             py::return_value_policy::reference_internal)
        .def("enable_heading_numbering", &Document::enableHeadingNumbering,
             py::return_value_policy::reference_internal)
        .def("disable_heading_numbering", &Document::disableHeadingNumbering,
             py::return_value_policy::reference_internal)
        .def("set_heading_num_format", &Document::setHeadingNumFormat,
             py::return_value_policy::reference_internal)
        // TOC
        .def("add_toc", &Document::addTOC,
             py::return_value_policy::reference_internal,
             py::arg("levels") = "1-3", py::arg("title") = "")
        // Paragraphs
        .def("add_paragraph",
             py::overload_cast<const std::string&>(&Document::addParagraph),
             py::return_value_policy::reference_internal,
             py::arg("text") = "")
        // Images
        .def("add_image",
             py::overload_cast<const std::string&>(&Document::addImage),
             py::return_value_policy::reference_internal)
        .def("enable_image_numbering", &Document::enableImageNumbering,
             py::return_value_policy::reference_internal,
             py::arg("prefix") = u8"图",
             py::arg("style") = CaptionNumStyle::Sequential)
        .def("disable_image_numbering", &Document::disableImageNumbering,
             py::return_value_policy::reference_internal)
        // Tables
        .def("add_table", &Document::addTable,
             py::return_value_policy::reference_internal)
        .def("enable_table_numbering", &Document::enableTableNumbering,
             py::return_value_policy::reference_internal,
             py::arg("prefix") = u8"表",
             py::arg("style") = CaptionNumStyle::Sequential)
        .def("disable_table_numbering", &Document::disableTableNumbering,
             py::return_value_policy::reference_internal)
        // Lists
        .def("add_bullet_list", &Document::addBulletList,
             py::return_value_policy::reference_internal)
        .def("add_ordered_list", &Document::addOrderedList,
             py::return_value_policy::reference_internal)
        // Sections
        .def("add_section_break", &Document::addSectionBreak,
             py::return_value_policy::reference_internal,
             py::arg("type") = SectionBreakType::NextPage)
        .def("enable_title_page", &Document::enableTitlePage,
             py::return_value_policy::reference_internal)
        // Footnotes
        .def("add_footnote", &Document::addFootnote)
        // Equations
        .def("add_equation", &Document::addEquation,
             py::return_value_policy::reference_internal)
        .def("add_display_equation", &Document::addDisplayEquation,
             py::return_value_policy::reference_internal)
        // Header / Footer — string overload
        .def("set_header",
             py::overload_cast<const std::string&>(&Document::setHeader),
             py::return_value_policy::reference_internal)
        .def("set_footer",
             py::overload_cast<const std::string&>(&Document::setFooter),
             py::return_value_policy::reference_internal)
        // Header / Footer — builder overload (no args, returns Paragraph&)
        .def("set_header",
             py::overload_cast<>(&Document::setHeader),
             py::return_value_policy::reference_internal)
        .def("set_footer",
             py::overload_cast<>(&Document::setFooter),
             py::return_value_policy::reference_internal)
        .def("clear_header", &Document::clearHeader)
        .def("clear_footer", &Document::clearFooter)
        // Template engine
        .def("open", &Document::open)
        .def("set_var",
             [](Document& self, const std::string& key, const std::string& v) -> Document& {
                 return self.set(key, v);
             }, py::return_value_policy::reference_internal)
        .def("set_var_bool",
             [](Document& self, const std::string& key, bool v) -> Document& {
                 return self.set(key, v);
             }, py::return_value_policy::reference_internal)
        .def("set_var_int",
             [](Document& self, const std::string& key, int v) -> Document& {
                 return self.set(key, v);
             }, py::return_value_policy::reference_internal)
        .def("set_var_float",
             [](Document& self, const std::string& key, double v, int precision) -> Document& {
                 return self.set(key, v, precision);
             }, py::return_value_policy::reference_internal,
             py::arg("key"), py::arg("value"), py::arg("precision") = 2)
        // Save
        .def("save", &Document::save);
}
