"""Native binding ownership, feature behavior and C++/Python package parity."""
import gc
import importlib
import os
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
import zipfile

module_dir = Path(sys.argv[1]).resolve()
sys.path.insert(0, str(module_dir))
dll_handle = os.add_dll_directory(str(module_dir)) if os.name == "nt" else None
x = importlib.import_module("xword" if (module_dir / "xword").exists() else "_native")
assert x.__version__ == "2.0.0"


def save(document, path):
    result = document.save_detailed(str(path))
    assert result.success, (result.error.part, result.error.message)


def parity_document():
    doc = x.Document()
    doc.enable_heading_numbering().enable_table_numbering("Table")
    doc.set_body_font("SimSun", "Times New Roman")
    doc.add_heading_paragraph("Parity", 1).set_bookmark(x.TargetId("head", x.TargetKind.Heading))
    doc.add_paragraph("Text").add_run("bold", x.RunStyle().bold()).add_break(x.BreakType.Line)
    doc.add_paragraph().add_reference(x.TargetId("table", x.TargetKind.Table), x.ReferenceKind.Number)
    table = doc.add_table(2, 2)
    table.set_header_rows(1).set_border_style(x.TableStyle.Grid).set_caption("Values").set_bookmark(x.TargetId("table", x.TargetKind.Table))
    table.cell(0, 0).add_paragraph("A")
    table.cell(1, 1).add_paragraph("B")
    doc.add_paragraph("Note").add_footnote_ref(doc.add_footnote("Footnote"))
    doc.add_section(x.SectionBreakType.Continuous).set_page_numbering(x.NumberFormat.LowerRoman, 1)
    doc.current_section().footer().add_paragraph().add_page_number()
    doc.add_paragraph("Last")
    return doc


save(parity_document(), "v2_python_parity.docx")


def canonical_parts(path):
    with zipfile.ZipFile(path) as package:
        return {
            name: ET.canonicalize(package.read(name).decode("utf-8"))
            if name.endswith((".xml", ".rels")) else package.read(name)
            for name in package.namelist()
        }


assert canonical_parts("v2_python_parity.docx") == canonical_parts("v2_cpp_parity.docx")

# References returned through each ownership chain keep every parent alive.
paragraph = x.Document().add_table(1, 1).cell(0, 0).add_paragraph("owned")
gc.collect()
paragraph.add_run(" alive").set_keep_together()
assert "alive" in paragraph.to_xml()
header = x.Document().current_section().header().add_paragraph("header")
gc.collect()
assert "header" in header.to_xml()
note = x.Document().add_endnote()
gc.collect()
note.add_paragraph("endnote")

doc = x.Document()
style = x.ParagraphStyle()
style.based_on = "Normal"
style.run = x.RunStyle().bold().east_asia_font("SimSun").font_size(12)
style.keep_next = x.Toggle.On
style.left_indent = x.Length.cm(1)
style.hanging = x.Length.pt(10)
style.before = x.Length.pt(6)
style.line_spacing = 15
style.line_rule = x.LineRule.Exact
tab = x.TabStop()
tab.position = x.Length.cm(4)
tab.alignment = x.TabAlignment.Right
tab.leader = x.TabLeader.Dot
style.tabs = [tab]
style.border = x.Border()
doc.register_paragraph_style("PythonStyle", style)
doc.add_paragraph("formatted").set_style_id("PythonStyle").add_run("off", x.RunStyle().bold(False).strike().highlight("yellow").subscript())
doc.add_paragraph().set_paragraph_style(style).set_line_spacing(1.5).set_first_line_indent_chars(2).set_right_indent(x.Length.cm(1)).set_hanging_indent(x.Length.pt(12)).add_tab_stop(tab).set_shading("EEEEEE")
doc.add_paragraph().set_page_break_before().set_keep_with_next().set_widow_control(False).add_tab().add_page_count().add_section_page_count()
source = x.BibliographySource()
source.tag, source.title, source.year = "python", "Python source", "2026"
author = x.Author()
author.first, author.last = "Ada", "Lovelace"
source.authors = [author]
doc.add_source(source).set_bibliography_style("IEEE")
doc.add_paragraph().add_citation("python", "2")
doc.add_bibliography("References")
doc.add_paragraph().add_endnote_ref(doc.add_endnote("Tail"))
comment_id = doc.add_comment("Review", "Tester", "2026-09-10T00:00:00Z")
doc.add_paragraph().start_comment(comment_id).add_run("marked").end_comment(comment_id)
doc.set_even_and_odd_headers()
section = doc.current_section()
section.set_title_page().header(x.HeaderFooterType.First).add_table(1, 1).cell(0, 0).add_paragraph("title table")
section.header(x.HeaderFooterType.Even).add_paragraph("even")
section.footer().add_paragraph().add_page_number()
properties = x.DocumentProperties()
properties.title = "Python features"
prop = x.CustomProperty()
prop.value, prop.type = "true", x.PropertyType.Boolean
doc.set_properties(properties).set_custom_property("Verified", prop)
save(doc, "v2_python_features.docx")

template = x.Document()
assert template.open("v2_empty.docx")
assert template.save("v2_python_template.docx")

invalid = x.Document()
invalid.add_paragraph().add_reference(x.TargetId("unknown"))
assert invalid.save_detailed("python_invalid.docx").error.code == x.SaveError.InvalidReference
try:
    x.Table(0, 1)
    raise AssertionError("Invalid table was accepted")
except ValueError:
    pass
print("Python API, ownership and C++ parity checks passed")
