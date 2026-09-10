# C++／Python 接口对照

由实际编译的 Python 绑定生成。使用、单位、默认行为和限制见 [使用手册](api.md)。

C++ 参数与重载以 `include/xword/` 头文件为准。Python 字符串对应 UTF-8；C++ 宽字符串重载共享同一实现。`CellImage` 为 `Image` 别名。

## Alignment

`Left`, `Center`, `Right`, `Justify`

## Author

值字段：`first`, `last`, `middle`, `corporate`。

### `Author.__init__` → C++ `Author`

```text
__init__(self: Author) -> None
```

## BibliographySource

值字段：`tag`, `title`, `type`, `authors`, `year`, `month`, `day`, `publisher`, `city`, `journal`, `volume`, `issue`, `pages`, `doi`, `url`, `accessed_year`, `accessed_month`, `accessed_day`, `language`。

### `BibliographySource.__init__` → C++ `BibliographySource`

```text
__init__(self: BibliographySource) -> None
```

## Border

值字段：`style`, `color`, `width`, `space`。

### `Border.__init__` → C++ `Border`

```text
__init__(self: Border) -> None
```

## BreakType

`Line`, `Page`, `Column`

## BulletList

### `BulletList.__init__` → C++ `BulletList`

```text
__init__(self: BulletList, type: ListType = <ListType.Bullet: 0>) -> None
```

### `BulletList.add_item` → C++ `addItem`

```text
add_item(self: BulletList, arg0: str) -> BulletList
```

### `BulletList.add_item_paragraph` → C++ `addItemParagraph`

```text
add_item_paragraph(self: BulletList, text: str = '', level: int = -1) -> Paragraph
```

### `BulletList.continue_from` → C++ `continueFrom`

```text
continue_from(self: BulletList, arg0: BulletList) -> BulletList
```

### `BulletList.num_id` → C++ `numId`

```text
num_id(self: BulletList) -> int
```

### `BulletList.set_format` → C++ `setFormat`

```text
set_format(self: BulletList, format: NumberFormat, text: str = '%1.') -> BulletList
```

### `BulletList.set_level` → C++ `setLevel`

```text
set_level(self: BulletList, arg0: int) -> BulletList
```

### `BulletList.set_num_id` → C++ `setNumId`

```text
set_num_id(self: BulletList, arg0: int) -> BulletList
```

### `BulletList.set_start` → C++ `setStart`

```text
set_start(self: BulletList, arg0: int) -> BulletList
```

### `BulletList.to_xml` → C++ `toXml`

```text
to_xml(self: BulletList) -> str
```

### `BulletList.type` → C++ `type`

```text
type(self: BulletList) -> ListType
```

## CaptionNumStyle

`Sequential`, `ByChapter`

## Cell

### `Cell.__init__` → C++ `Cell`

```text
__init__(self: Cell) -> None
```

### `Cell.set_border` → C++ `setBorder`

```text
set_border(self: Cell, arg0: Border) -> Cell
```

### `Cell.set_margins` → C++ `setMargins`

```text
set_margins(self: Cell, arg0: Length, arg1: Length, arg2: Length, arg3: Length) -> Cell
```

### `Cell.set_shading` → C++ `setShading`

```text
set_shading(self: Cell, arg0: str) -> Cell
```

### `Cell.set_v_align` → C++ `setVAlign`

```text
set_v_align(self: Cell, arg0: VAlignment) -> Cell
```

### `Cell.v_align` → C++ `vAlign`

```text
v_align(self: Cell) -> VAlignment
```

同时继承：`Content` 的接口。

## CellImage

### `CellImage.__init__` → C++ `CellImage`

```text
__init__(self: Image, arg0: str) -> None
```

### `CellImage.alignment` → C++ `见头文件／继承接口`

```text
alignment(self: Image) -> Alignment
```

### `CellImage.caption` → C++ `见头文件／继承接口`

```text
caption(self: Image) -> str
```

### `CellImage.filepath` → C++ `见头文件／继承接口`

```text
filepath(self: Image) -> str
```

### `CellImage.height` → C++ `见头文件／继承接口`

```text
height(self: Image) -> int
```

### `CellImage.set_alignment` → C++ `见头文件／继承接口`

```text
set_alignment(self: Image, arg0: Alignment) -> Image
```

### `CellImage.set_alt_text` → C++ `见头文件／继承接口`

```text
set_alt_text(self: Image, description: str, title: str = '') -> Image
```

### `CellImage.set_bookmark` → C++ `见头文件／继承接口`

```text
set_bookmark(self: Image, arg0: TargetId) -> Image
```

### `CellImage.set_caption` → C++ `见头文件／继承接口`

```text
set_caption(self: Image, arg0: str) -> Image
```

### `CellImage.set_crop` → C++ `见头文件／继承接口`

```text
set_crop(self: Image, arg0: float, arg1: float, arg2: float, arg3: float) -> Image
```

### `CellImage.set_dimensions` → C++ `见头文件／继承接口`

```text
set_dimensions(self: Image, arg0: Length, arg1: Length) -> Image
```

### `CellImage.set_keep_aspect_ratio` → C++ `见头文件／继承接口`

```text
set_keep_aspect_ratio(self: Image, on: bool = True) -> Image
```

### `CellImage.set_position` → C++ `见头文件／继承接口`

```text
set_position(self: Image, x: Length, y: Length, horizontal: PositionRelative = <PositionRelative.Margin: 1>, vertical: PositionRelative = <PositionRelative.Paragraph: 3>) -> Image
```

### `CellImage.set_size` → C++ `见头文件／继承接口`

```text
set_size(self: Image, arg0: int, arg1: int) -> Image
```

### `CellImage.set_svg_fallback` → C++ `见头文件／继承接口`

```text
set_svg_fallback(self: Image, arg0: str) -> Image
```

### `CellImage.set_wrap` → C++ `见头文件／继承接口`

```text
set_wrap(self: Image, arg0: ImageWrap) -> Image
```

### `CellImage.skipped` → C++ `见头文件／继承接口`

```text
skipped(self: Image) -> bool
```

### `CellImage.width` → C++ `见头文件／继承接口`

```text
width(self: Image) -> int
```

## Column

值字段：`width`, `space`。

### `Column.__init__` → C++ `Column`

```text
__init__(self: Column) -> None
```

## Content

### `Content.__init__` → C++ `Content`

```text
__init__(self: Content) -> None
```

### `Content.add_bullet_list` → C++ `addBulletList`

```text
add_bullet_list(self: Content) -> BulletList
```

### `Content.add_equation` → C++ `addEquation`

```text
add_equation(self: Content, arg0: str) -> Paragraph
```

### `Content.add_image` → C++ `addImage`

```text
add_image(*args, **kwargs)
Overloaded function.

1. add_image(self: Content, arg0: str) -> Image

2. add_image(self: Content, arg0: str, arg1: int, arg2: int) -> Image
```

### `Content.add_math` → C++ `addMath`

```text
add_math(self: Content, latex: str, mode: EquationMode = <EquationMode.Display: 1>) -> Equation
```

### `Content.add_ordered_list` → C++ `addOrderedList`

```text
add_ordered_list(self: Content) -> BulletList
```

### `Content.add_paragraph` → C++ `addParagraph`

```text
add_paragraph(*args, **kwargs)
Overloaded function.

1. add_paragraph(self: Content, text: str = '') -> Paragraph

2. add_paragraph(self: Content, arg0: str, arg1: RunStyle) -> Paragraph
```

### `Content.add_table` → C++ `addTable`

```text
add_table(self: Content, arg0: int, arg1: int) -> xword::Table
```

### `Content.empty` → C++ `empty`

```text
empty(self: Content) -> bool
```

### `Content.get_style` → C++ `getStyle`

```text
get_style(self: Content) -> RunStyle
```

### `Content.set_style` → C++ `setStyle`

```text
set_style(self: Content, arg0: RunStyle) -> None
```

### `Content.to_xml` → C++ `toXml`

```text
to_xml(self: Content) -> str
```

## CustomProperty

值字段：`value`, `type`。

### `CustomProperty.__init__` → C++ `CustomProperty`

```text
__init__(self: CustomProperty) -> None
```

## Diagnostic

值字段：`code`, `part`, `message`。

### `Diagnostic.__init__` → C++ `Diagnostic`

```text
__init__(self: Diagnostic) -> None
```

## Document

### `Document.__init__` → C++ `Document`

```text
__init__(self: Document) -> None
```

### `Document.add_bibliography` → C++ `addBibliography`

```text
add_bibliography(self: Document, title: str = '') -> Document
```

### `Document.add_bullet_list` → C++ `addBulletList`

```text
add_bullet_list(self: Document) -> BulletList
```

### `Document.add_comment` → C++ `addComment`

```text
add_comment(self: Document, text: str, author: str, date: str = '') -> int
```

### `Document.add_display_equation` → C++ `addDisplayEquation`

```text
add_display_equation(self: Document, arg0: str) -> Equation
```

### `Document.add_endnote` → C++ `addEndnote`

```text
add_endnote(*args, **kwargs)
Overloaded function.

1. add_endnote(self: Document) -> Note

2. add_endnote(self: Document, arg0: str) -> int
```

### `Document.add_equation` → C++ `addEquation`

```text
add_equation(self: Document, arg0: str) -> Equation
```

### `Document.add_figure_toc` → C++ `addFigureTOC`

```text
add_figure_toc(self: Document, title: str = '') -> Document
```

### `Document.add_footnote` → C++ `addFootnote`

```text
add_footnote(*args, **kwargs)
Overloaded function.

1. add_footnote(self: Document) -> Note

2. add_footnote(self: Document, arg0: str) -> int
```

### `Document.add_heading` → C++ `addHeading`

```text
add_heading(self: Document, arg0: str, arg1: int) -> Document
```

### `Document.add_heading_no_num` → C++ `addHeadingNoNum`

```text
add_heading_no_num(self: Document, arg0: str, arg1: int) -> Document
```

### `Document.add_heading_paragraph` → C++ `addHeadingParagraph`

```text
add_heading_paragraph(self: Document, text: str, level: int, numbered: bool = True) -> Paragraph
```

### `Document.add_image` → C++ `addImage`

```text
add_image(self: Document, arg0: str) -> Image
```

### `Document.add_ordered_list` → C++ `addOrderedList`

```text
add_ordered_list(self: Document) -> BulletList
```

### `Document.add_page_break` → C++ `addPageBreak`

```text
add_page_break(self: Document) -> Document
```

### `Document.add_paragraph` → C++ `addParagraph`

```text
add_paragraph(self: Document, text: str = '') -> Paragraph
```

### `Document.add_section` → C++ `addSection`

```text
add_section(self: Document, type: SectionBreakType = <SectionBreakType.NextPage: 0>) -> Section
```

### `Document.add_section_break` → C++ `addSectionBreak`

```text
add_section_break(self: Document, type: SectionBreakType = <SectionBreakType.NextPage: 0>) -> Document
```

### `Document.add_source` → C++ `addSource`

```text
add_source(self: Document, arg0: BibliographySource) -> Document
```

### `Document.add_table` → C++ `addTable`

```text
add_table(self: Document, arg0: int, arg1: int) -> Table
```

### `Document.add_table_toc` → C++ `addTableTOC`

```text
add_table_toc(self: Document, title: str = '') -> Document
```

### `Document.add_toc` → C++ `addTOC`

```text
add_toc(self: Document, levels: str = '1-3', title: str = '') -> Document
```

### `Document.clear_footer` → C++ `clearFooter`

```text
clear_footer(self: Document) -> None
```

### `Document.clear_header` → C++ `clearHeader`

```text
clear_header(self: Document) -> None
```

### `Document.current_section` → C++ `currentSection`

```text
current_section(self: Document) -> Section
```

### `Document.disable_heading_numbering` → C++ `disableHeadingNumbering`

```text
disable_heading_numbering(self: Document) -> Document
```

### `Document.disable_image_numbering` → C++ `disableImageNumbering`

```text
disable_image_numbering(self: Document) -> Document
```

### `Document.disable_table_numbering` → C++ `disableTableNumbering`

```text
disable_table_numbering(self: Document) -> Document
```

### `Document.enable_heading_numbering` → C++ `enableHeadingNumbering`

```text
enable_heading_numbering(self: Document) -> Document
```

### `Document.enable_image_numbering` → C++ `enableImageNumbering`

```text
enable_image_numbering(self: Document, prefix: str = '图', style: CaptionNumStyle = <CaptionNumStyle.Sequential: 0>) -> Document
```

### `Document.enable_table_numbering` → C++ `enableTableNumbering`

```text
enable_table_numbering(self: Document, prefix: str = '表', style: CaptionNumStyle = <CaptionNumStyle.Sequential: 0>) -> Document
```

### `Document.enable_title_page` → C++ `enableTitlePage`

```text
enable_title_page(self: Document) -> Document
```

### `Document.endnote` → C++ `endnote`

```text
endnote(self: Document, arg0: int) -> Note
```

### `Document.footnote` → C++ `footnote`

```text
footnote(self: Document, arg0: int) -> Note
```

### `Document.open` → C++ `open`

```text
open(self: Document, arg0: str) -> bool
```

### `Document.register_character_style` → C++ `registerCharacterStyle`

```text
register_character_style(self: Document, id: str, style: RunStyle, based_on: str = '') -> Document
```

### `Document.register_paragraph_style` → C++ `registerParagraphStyle`

```text
register_paragraph_style(self: Document, arg0: str, arg1: ParagraphStyle) -> Document
```

### `Document.register_table_style` → C++ `registerTableStyle`

```text
register_table_style(self: Document, arg0: str, arg1: TableStyleDefinition) -> Document
```

### `Document.save` → C++ `save`

```text
save(self: Document, arg0: str) -> bool
```

### `Document.save_detailed` → C++ `saveDetailed`

```text
save_detailed(self: Document, path: str, options: SaveOptions = <默认值对象>) -> SaveResult
```

### `Document.set_bibliography_style` → C++ `setBibliographyStyle`

```text
set_bibliography_style(self: Document, style: str = 'IEEE') -> Document
```

### `Document.set_body_font` → C++ `setBodyFont`

```text
set_body_font(self: Document, east_asia: str, ascii: str = '', h_ansi: str = '') -> Document
```

### `Document.set_body_font_size` → C++ `setBodyFontSize`

```text
set_body_font_size(self: Document, arg0: float) -> Document
```

### `Document.set_body_line_spacing` → C++ `setBodyLineSpacing`

```text
set_body_line_spacing(self: Document, arg0: float) -> Document
```

### `Document.set_body_run_style` → C++ `setBodyRunStyle`

```text
set_body_run_style(self: Document, arg0: RunStyle) -> Document
```

### `Document.set_bullet_list` → C++ `setBulletList`

```text
set_bullet_list(self: Document, arg0: str) -> BulletList
```

### `Document.set_custom_property` → C++ `setCustomProperty`

```text
set_custom_property(self: Document, arg0: str, arg1: CustomProperty) -> Document
```

### `Document.set_default_paragraph_indent` → C++ `setDefaultParagraphIndent`

```text
set_default_paragraph_indent(self: Document, chars: float = 2, font_size_pt: int = 12) -> Document
```

### `Document.set_display_equation` → C++ `setDisplayEquation`

```text
set_display_equation(self: Document, arg0: str, arg1: str) -> Equation
```

### `Document.set_display_equation_style` → C++ `setDisplayEquationStyle`

```text
set_display_equation_style(self: Document, arg0: RunStyle) -> Document
```

### `Document.set_equation` → C++ `setEquation`

```text
set_equation(self: Document, arg0: str, arg1: str) -> Equation
```

### `Document.set_even_and_odd_headers` → C++ `setEvenAndOddHeaders`

```text
set_even_and_odd_headers(self: Document, on: bool = True) -> Document
```

### `Document.set_footer` → C++ `setFooter`

```text
set_footer(*args, **kwargs)
Overloaded function.

1. set_footer(self: Document) -> Paragraph

2. set_footer(self: Document, arg0: str) -> Document
```

### `Document.set_header` → C++ `setHeader`

```text
set_header(*args, **kwargs)
Overloaded function.

1. set_header(self: Document) -> Paragraph

2. set_header(self: Document, arg0: str) -> Document
```

### `Document.set_heading_num_format` → C++ `setHeadingNumFormat`

```text
set_heading_num_format(self: Document, arg0: HeadingNumFormat) -> Document
```

### `Document.set_heading_style` → C++ `setHeadingStyle`

```text
set_heading_style(self: Document, arg0: int, arg1: HeadingStyle) -> Document
```

### `Document.set_image` → C++ `setImage`

```text
set_image(self: Document, arg0: str, arg1: str) -> Image
```

### `Document.set_ordered_list` → C++ `setOrderedList`

```text
set_ordered_list(self: Document, arg0: str) -> BulletList
```

### `Document.set_page` → C++ `setPage`

```text
set_page(self: Document, arg0: Page) -> Document
```

### `Document.set_paragraph` → C++ `setParagraph`

```text
set_paragraph(self: Document, key: str, text: str = '') -> Paragraph
```

### `Document.set_properties` → C++ `setProperties`

```text
set_properties(self: Document, arg0: DocumentProperties) -> Document
```

### `Document.set_table` → C++ `setTable`

```text
set_table(self: Document, arg0: str, arg1: int, arg2: int) -> Table
```

### `Document.set_table_run_style` → C++ `setTableRunStyle`

```text
set_table_run_style(self: Document, arg0: RunStyle) -> Document
```

### `Document.set_var` → C++ `set`

```text
set_var(self: Document, arg0: str, arg1: str) -> Document
```

### `Document.set_var_bool` → C++ `set`

```text
set_var_bool(self: Document, arg0: str, arg1: bool) -> Document
```

### `Document.set_var_float` → C++ `set`

```text
set_var_float(self: Document, key: str, value: float, precision: int = 2) -> Document
```

### `Document.set_var_int` → C++ `set`

```text
set_var_int(self: Document, arg0: str, arg1: int) -> Document
```

## DocumentProperties

值字段：`title`, `subject`, `creator`, `keywords`, `description`, `last_modified_by`, `language`。

### `DocumentProperties.__init__` → C++ `DocumentProperties`

```text
__init__(self: DocumentProperties) -> None
```

## EndnotePosition

`DocumentEnd`, `SectionEnd`

## Equation

### `Equation.__init__` → C++ `Equation`

```text
__init__(self: Equation, latex: str, mode: EquationMode = <EquationMode.Inline: 0>) -> None
```

### `Equation.get_style` → C++ `getStyle`

```text
get_style(self: Equation) -> RunStyle
```

### `Equation.latex` → C++ `latex`

```text
latex(self: Equation) -> str
```

### `Equation.mode` → C++ `mode`

```text
mode(self: Equation) -> EquationMode
```

### `Equation.set_mode` → C++ `setMode`

```text
set_mode(self: Equation, arg0: EquationMode) -> Equation
```

### `Equation.set_style` → C++ `setStyle`

```text
set_style(self: Equation, arg0: RunStyle) -> Equation
```

### `Equation.to_xml` → C++ `toXml`

```text
to_xml(self: Equation) -> str
```

## EquationMode

`Inline`, `Display`

## FootnotePosition

`PageBottom`, `BeneathText`

## HeaderFooterType

`Default`, `First`, `Even`

## HeadingNumFormat

`Decimal`, `Chapter`

## HeadingStyle

值字段：`font`, `font_size`, `bold`, `italic`, `color`, `line_spacing`, `space_before`, `space_after`, `alignment`, `has_alignment`。

### `HeadingStyle.__init__` → C++ `HeadingStyle`

```text
__init__(self: HeadingStyle) -> None
```

### `HeadingStyle.set_alignment` → C++ `setAlignment`

```text
set_alignment(self: HeadingStyle, arg0: Alignment) -> HeadingStyle
```

### `HeadingStyle.set_bold` → C++ `setBold`

```text
set_bold(self: HeadingStyle, on: bool = True) -> HeadingStyle
```

### `HeadingStyle.set_color` → C++ `setColor`

```text
set_color(self: HeadingStyle, arg0: str) -> HeadingStyle
```

### `HeadingStyle.set_font` → C++ `setFont`

```text
set_font(self: HeadingStyle, arg0: str) -> HeadingStyle
```

### `HeadingStyle.set_font_size` → C++ `setFontSize`

```text
set_font_size(self: HeadingStyle, arg0: int) -> HeadingStyle
```

### `HeadingStyle.set_italic` → C++ `setItalic`

```text
set_italic(self: HeadingStyle, on: bool = True) -> HeadingStyle
```

### `HeadingStyle.set_line_spacing` → C++ `setLineSpacing`

```text
set_line_spacing(self: HeadingStyle, arg0: float) -> HeadingStyle
```

### `HeadingStyle.set_space_after` → C++ `setSpaceAfter`

```text
set_space_after(self: HeadingStyle, arg0: float) -> HeadingStyle
```

### `HeadingStyle.set_space_before` → C++ `setSpaceBefore`

```text
set_space_before(self: HeadingStyle, arg0: float) -> HeadingStyle
```

## ImageWrap

`Inline`, `Square`, `TopBottom`, `BehindText`, `InFrontOfText`

## Length

### `Length.__init__` → C++ `Length`

```text
__init__(self: Length) -> None
```

### `Length.cm` → C++ `cm`

```text
cm(arg0: float) -> Length
```

### `Length.dxa` → C++ `dxa`

```text
dxa(self: Length) -> int
```

### `Length.emu` → C++ `emu`

```text
emu(self: Length) -> int
```

### `Length.inch` → C++ `inch`

```text
inch(arg0: float) -> Length
```

### `Length.mm` → C++ `mm`

```text
mm(arg0: float) -> Length
```

### `Length.pt` → C++ `pt`

```text
pt(arg0: float) -> Length
```

### `Length.twips` → C++ `twips`

```text
twips(arg0: int) -> Length
```

## LineRule

`Auto`, `Exact`, `AtLeast`

## ListType

`Bullet`, `Ordered`

## Note

### `Note.__init__` → C++ `Note`

```text
Initialize self.  See help(type(self)) for accurate signature.
```

### `Note.id` → C++ `id`

```text
id(self: Note) -> int
```

同时继承：`Content` 的接口。

## NoteOptions

值字段：`format`, `start`, `restart`, `footnote_position`, `endnote_position`。

### `NoteOptions.__init__` → C++ `NoteOptions`

```text
__init__(self: NoteOptions) -> None
```

## NoteRestart

`Continuous`, `EachSection`, `EachPage`

## NumberFormat

`Decimal`, `UpperRoman`, `LowerRoman`, `UpperLetter`, `LowerLetter`, `Bullet`

## Orientation

`Portrait`, `Landscape`

## Page

值字段：`size`, `orientation`, `margins`, `custom_width`, `custom_height`, `header_distance`, `footer_distance`, `gutter`。

### `Page.__init__` → C++ `Page`

```text
__init__(self: Page) -> None
```

### `Page.set_custom_size` → C++ `setCustomSize`

```text
set_custom_size(self: Page, arg0: Length, arg1: Length) -> Page
```

### `Page.set_footer_distance` → C++ `setFooterDistance`

```text
set_footer_distance(self: Page, arg0: Length) -> Page
```

### `Page.set_gutter` → C++ `setGutter`

```text
set_gutter(self: Page, arg0: Length) -> Page
```

### `Page.set_header_distance` → C++ `setHeaderDistance`

```text
set_header_distance(self: Page, arg0: Length) -> Page
```

### `Page.set_margins` → C++ `setMargins`

```text
set_margins(self: Page, arg0: float, arg1: float, arg2: float, arg3: float) -> Page
```

### `Page.set_orientation` → C++ `setOrientation`

```text
set_orientation(self: Page, arg0: Orientation) -> Page
```

### `Page.set_size` → C++ `setSize`

```text
set_size(self: Page, arg0: PageSize) -> Page
```

## PageMargins

值字段：`top`, `bottom`, `left`, `right`。

### `PageMargins.__init__` → C++ `PageMargins`

```text
__init__(self: PageMargins) -> None
```

## PageSize

`A4`, `Letter`

## Paragraph

### `Paragraph.__init__` → C++ `Paragraph`

```text
__init__(self: Paragraph) -> None
```

### `Paragraph.add_break` → C++ `addBreak`

```text
add_break(self: Paragraph, type: BreakType = <BreakType.Line: 0>) -> Paragraph
```

### `Paragraph.add_citation` → C++ `addCitation`

```text
add_citation(self: Paragraph, tag: str, pages: str = '') -> Paragraph
```

### `Paragraph.add_endnote_ref` → C++ `addEndnoteRef`

```text
add_endnote_ref(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.add_equation` → C++ `addEquation`

```text
add_equation(self: Paragraph, arg0: str) -> Paragraph
```

### `Paragraph.add_field` → C++ `addField`

```text
add_field(self: Paragraph, instruction: str, cached: str = '') -> Paragraph
```

### `Paragraph.add_footnote_ref` → C++ `addFootnoteRef`

```text
add_footnote_ref(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.add_hyperlink` → C++ `addHyperlink`

```text
add_hyperlink(self: Paragraph, text: str, url: str, style: RunStyle = <默认值对象>) -> Paragraph
```

### `Paragraph.add_image` → C++ `addImage`

```text
add_image(self: Paragraph, arg0: str) -> Image
```

### `Paragraph.add_page_count` → C++ `addPageCount`

```text
add_page_count(self: Paragraph) -> Paragraph
```

### `Paragraph.add_page_number` → C++ `addPageNumber`

```text
add_page_number(self: Paragraph) -> Paragraph
```

### `Paragraph.add_reference` → C++ `addReference`

```text
add_reference(self: Paragraph, target: TargetId, kind: ReferenceKind = <ReferenceKind.Text: 0>, hyperlink: bool = True) -> Paragraph
```

### `Paragraph.add_run` → C++ `addRun`

```text
add_run(*args, **kwargs)
Overloaded function.

1. add_run(self: Paragraph, arg0: str) -> Paragraph

2. add_run(self: Paragraph, arg0: str, arg1: RunStyle) -> Paragraph
```

### `Paragraph.add_section_page_count` → C++ `addSectionPageCount`

```text
add_section_page_count(self: Paragraph) -> Paragraph
```

### `Paragraph.add_tab` → C++ `addTab`

```text
add_tab(self: Paragraph) -> Paragraph
```

### `Paragraph.add_tab_stop` → C++ `addTabStop`

```text
add_tab_stop(self: Paragraph, arg0: TabStop) -> Paragraph
```

### `Paragraph.end_bookmark` → C++ `endBookmark`

```text
end_bookmark(self: Paragraph, arg0: TargetId) -> Paragraph
```

### `Paragraph.end_comment` → C++ `endComment`

```text
end_comment(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.get_style` → C++ `getStyle`

```text
get_style(self: Paragraph) -> RunStyle
```

### `Paragraph.set_alignment` → C++ `setAlignment`

```text
set_alignment(self: Paragraph, arg0: Alignment) -> Paragraph
```

### `Paragraph.set_bookmark` → C++ `setBookmark`

```text
set_bookmark(self: Paragraph, arg0: TargetId) -> Paragraph
```

### `Paragraph.set_border` → C++ `setBorder`

```text
set_border(self: Paragraph, arg0: Border) -> Paragraph
```

### `Paragraph.set_default_run_style` → C++ `setStyle`

```text
set_default_run_style(self: Paragraph, arg0: RunStyle) -> Paragraph
```

### `Paragraph.set_first_line_indent` → C++ `setFirstLineIndent`

```text
set_first_line_indent(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.set_first_line_indent_chars` → C++ `setFirstLineIndentChars`

```text
set_first_line_indent_chars(self: Paragraph, chars: float, font_size_pt: int = 12) -> Paragraph
```

### `Paragraph.set_hanging_indent` → C++ `setHangingIndent`

```text
set_hanging_indent(self: Paragraph, arg0: Length) -> Paragraph
```

### `Paragraph.set_keep_together` → C++ `setKeepTogether`

```text
set_keep_together(self: Paragraph, on: bool = True) -> Paragraph
```

### `Paragraph.set_keep_with_next` → C++ `setKeepWithNext`

```text
set_keep_with_next(self: Paragraph, on: bool = True) -> Paragraph
```

### `Paragraph.set_left_indent` → C++ `setLeftIndent`

```text
set_left_indent(self: Paragraph, arg0: Length) -> Paragraph
```

### `Paragraph.set_line_spacing` → C++ `setLineSpacing`

```text
set_line_spacing(self: Paragraph, value: float, rule: LineRule = <LineRule.Auto: 0>) -> Paragraph
```

### `Paragraph.set_numbering` → C++ `setNumbering`

```text
set_numbering(self: Paragraph, id: int, level: int = 0) -> Paragraph
```

### `Paragraph.set_page_break_before` → C++ `setPageBreakBefore`

```text
set_page_break_before(self: Paragraph, on: bool = True) -> Paragraph
```

### `Paragraph.set_paragraph_style` → C++ `setParagraphStyle`

```text
set_paragraph_style(self: Paragraph, arg0: ParagraphStyle) -> Paragraph
```

### `Paragraph.set_right_indent` → C++ `setRightIndent`

```text
set_right_indent(self: Paragraph, arg0: Length) -> Paragraph
```

### `Paragraph.set_shading` → C++ `setShading`

```text
set_shading(self: Paragraph, arg0: str) -> Paragraph
```

### `Paragraph.set_spacing_after` → C++ `setSpacingAfter`

```text
set_spacing_after(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.set_spacing_before` → C++ `setSpacingBefore`

```text
set_spacing_before(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.set_style` → C++ `setStyle`

```text
set_style(self: Paragraph, arg0: RunStyle) -> Paragraph
```

### `Paragraph.set_style_id` → C++ `setStyleId`

```text
set_style_id(self: Paragraph, arg0: str) -> Paragraph
```

### `Paragraph.set_widow_control` → C++ `setWidowControl`

```text
set_widow_control(self: Paragraph, on: bool = True) -> Paragraph
```

### `Paragraph.start_bookmark` → C++ `startBookmark`

```text
start_bookmark(self: Paragraph, arg0: TargetId) -> Paragraph
```

### `Paragraph.start_comment` → C++ `startComment`

```text
start_comment(self: Paragraph, arg0: int) -> Paragraph
```

### `Paragraph.to_xml` → C++ `toXml`

```text
to_xml(self: Paragraph) -> str
```

## ParagraphStyle

值字段：`based_on`, `next`, `shading`, `run`, `alignment`, `left_indent`, `right_indent`, `first_line`, `hanging`, `before`, `after`, `first_line_chars`, `line_spacing`, `line_rule`, `page_break_before`, `keep_next`, `keep_lines`, `widow_control`, `tabs`, `border`。

### `ParagraphStyle.__init__` → C++ `ParagraphStyle`

```text
__init__(self: ParagraphStyle) -> None
```

## PositionRelative

`Page`, `Margin`, `Column`, `Paragraph`

## PropertyType

`String`, `Number`, `Boolean`, `Date`

## ReferenceKind

`Text`, `Number`, `Page`

## RunStyle

### `RunStyle.__init__` → C++ `RunStyle`

```text
__init__(self: RunStyle) -> None
```

### `RunStyle.baseline` → C++ `baseline`

```text
baseline(self: RunStyle) -> RunStyle
```

### `RunStyle.bold` → C++ `bold`

```text
bold(self: RunStyle, on: bool = True) -> RunStyle
```

### `RunStyle.color` → C++ `color`

```text
color(self: RunStyle, arg0: str) -> RunStyle
```

### `RunStyle.east_asia_font` → C++ `eastAsiaFont`

```text
east_asia_font(self: RunStyle, arg0: str) -> RunStyle
```

### `RunStyle.font` → C++ `font`

```text
font(self: RunStyle, arg0: str) -> RunStyle
```

### `RunStyle.font_size` → C++ `fontSize`

```text
font_size(self: RunStyle, arg0: float) -> RunStyle
```

### `RunStyle.has_formatting` → C++ `hasFormatting`

```text
has_formatting(self: RunStyle) -> bool
```

### `RunStyle.highlight` → C++ `highlight`

```text
highlight(self: RunStyle, arg0: str) -> RunStyle
```

### `RunStyle.inherit_bold` → C++ `inheritBold`

```text
inherit_bold(self: RunStyle) -> RunStyle
```

### `RunStyle.inherit_italic` → C++ `inheritItalic`

```text
inherit_italic(self: RunStyle) -> RunStyle
```

### `RunStyle.inherit_underline` → C++ `inheritUnderline`

```text
inherit_underline(self: RunStyle) -> RunStyle
```

### `RunStyle.italic` → C++ `italic`

```text
italic(self: RunStyle, on: bool = True) -> RunStyle
```

### `RunStyle.set_style_id` → C++ `setStyleId`

```text
set_style_id(self: RunStyle, arg0: str) -> RunStyle
```

### `RunStyle.strike` → C++ `strike`

```text
strike(self: RunStyle, on: bool = True) -> RunStyle
```

### `RunStyle.subscript` → C++ `subscript`

```text
subscript(self: RunStyle) -> RunStyle
```

### `RunStyle.superscript` → C++ `superscript`

```text
superscript(self: RunStyle) -> RunStyle
```

### `RunStyle.underline` → C++ `underline`

```text
underline(self: RunStyle, on: bool = True) -> RunStyle
```

## SaveError

`None`, `InvalidArgument`, `InvalidXml`, `InvalidReference`, `MissingResource`, `IoError`

## SaveOptions

值字段：`missing_images_are_errors`。

### `SaveOptions.__init__` → C++ `SaveOptions`

```text
__init__(self: SaveOptions) -> None
```

## SaveResult

值字段：`success`, `error`, `warnings`。

### `SaveResult.__init__` → C++ `SaveResult`

```text
__init__(self: SaveResult) -> None
```

## Section

### `Section.__init__` → C++ `Section`

```text
Initialize self.  See help(type(self)) for accurate signature.
```

### `Section.clear_footer` → C++ `clearFooter`

```text
clear_footer(self: Section, type: HeaderFooterType = <HeaderFooterType.Default: 0>) -> Section
```

### `Section.clear_header` → C++ `clearHeader`

```text
clear_header(self: Section, type: HeaderFooterType = <HeaderFooterType.Default: 0>) -> Section
```

### `Section.footer` → C++ `footer`

```text
footer(self: Section, type: HeaderFooterType = <HeaderFooterType.Default: 0>) -> Content
```

### `Section.header` → C++ `header`

```text
header(self: Section, type: HeaderFooterType = <HeaderFooterType.Default: 0>) -> Content
```

### `Section.link_footer_to_previous` → C++ `linkFooterToPrevious`

```text
link_footer_to_previous(self: Section, type: HeaderFooterType = <HeaderFooterType.Default: 0>) -> Section
```

### `Section.link_header_to_previous` → C++ `linkHeaderToPrevious`

```text
link_header_to_previous(self: Section, type: HeaderFooterType = <HeaderFooterType.Default: 0>) -> Section
```

### `Section.page` → C++ `page`

```text
page(self: Section) -> Page
```

### `Section.set_column_widths` → C++ `setColumnWidths`

```text
set_column_widths(self: Section, columns: list[Column], separator: bool = False) -> Section
```

### `Section.set_columns` → C++ `setColumns`

```text
set_columns(self: Section, count: int, space: Length = <默认值对象>, separator: bool = False) -> Section
```

### `Section.set_endnote_options` → C++ `setEndnoteOptions`

```text
set_endnote_options(self: Section, arg0: NoteOptions) -> Section
```

### `Section.set_footnote_options` → C++ `setFootnoteOptions`

```text
set_footnote_options(self: Section, arg0: NoteOptions) -> Section
```

### `Section.set_page` → C++ `setPage`

```text
set_page(self: Section, arg0: Page) -> Section
```

### `Section.set_page_numbering` → C++ `setPageNumbering`

```text
set_page_numbering(self: Section, format: NumberFormat = <NumberFormat.Decimal: 0>, start: int = 0) -> Section
```

### `Section.set_title_page` → C++ `setTitlePage`

```text
set_title_page(self: Section, on: bool = True) -> Section
```

## SectionBreakType

`NextPage`, `Continuous`, `EvenPage`, `OddPage`, `NextColumn`

## SourceType

`Book`, `JournalArticle`, `ConferenceProceedings`, `Report`, `InternetSite`, `Misc`

## TabAlignment

`Left`, `Center`, `Right`, `Decimal`, `Bar`

## TabLeader

`None`, `Dot`, `Hyphen`, `Underscore`

## TabStop

值字段：`position`, `alignment`, `leader`。

### `TabStop.__init__` → C++ `TabStop`

```text
__init__(self: TabStop) -> None
```

## Table

### `Table.__init__` → C++ `Table`

```text
__init__(self: Table, arg0: int, arg1: int) -> None
```

### `Table.caption` → C++ `caption`

```text
caption(self: Table) -> str
```

### `Table.cell` → C++ `cell`

```text
cell(self: Table, arg0: int, arg1: int) -> Cell
```

### `Table.cols` → C++ `cols`

```text
cols(self: Table) -> int
```

### `Table.get_column_width` → C++ `getColumnWidth`

```text
get_column_width(self: Table, arg0: int) -> float
```

### `Table.get_style` → C++ `getStyle`

```text
get_style(self: Table) -> RunStyle
```

### `Table.merge_cells` → C++ `mergeCells`

```text
merge_cells(self: Table, arg0: int, arg1: int, arg2: int, arg3: int) -> Table
```

### `Table.rows` → C++ `rows`

```text
rows(self: Table) -> int
```

### `Table.set_auto_fit` → C++ `setAutoFit`

```text
set_auto_fit(self: Table, on: bool = True) -> Table
```

### `Table.set_bookmark` → C++ `setBookmark`

```text
set_bookmark(self: Table, arg0: TargetId) -> Table
```

### `Table.set_border` → C++ `setBorder`

```text
set_border(self: Table, arg0: Border) -> Table
```

### `Table.set_border_style` → C++ `setBorderStyle`

```text
set_border_style(self: Table, arg0: TableStyle) -> Table
```

### `Table.set_caption` → C++ `setCaption`

```text
set_caption(self: Table, arg0: str) -> Table
```

### `Table.set_cell_margins` → C++ `setCellMargins`

```text
set_cell_margins(self: Table, arg0: Length, arg1: Length, arg2: Length, arg3: Length) -> Table
```

### `Table.set_column_width` → C++ `setColumnWidth`

```text
set_column_width(self: Table, arg0: int, arg1: float) -> Table
```

### `Table.set_column_widths` → C++ `setColumnWidths`

```text
set_column_widths(self: Table, arg0: list[float]) -> Table
```

### `Table.set_header_row` → C++ `setHeaderRow`

```text
set_header_row(self: Table, arg0: int) -> Table
```

### `Table.set_header_rows` → C++ `setHeaderRows`

```text
set_header_rows(self: Table, arg0: int) -> Table
```

### `Table.set_row_allow_split` → C++ `setRowAllowSplit`

```text
set_row_allow_split(self: Table, row: int, allow: bool = True) -> Table
```

### `Table.set_row_height` → C++ `setRowHeight`

```text
set_row_height(self: Table, row: int, height: Length, exact: bool = False) -> Table
```

### `Table.set_run_style` → C++ `setStyle`

```text
set_run_style(self: Table, arg0: RunStyle) -> Table
```

### `Table.set_shading` → C++ `setShading`

```text
set_shading(self: Table, arg0: str) -> Table
```

### `Table.set_style` → C++ `setStyle`

```text
set_style(self: Table, arg0: RunStyle) -> Table
```

### `Table.set_style_id` → C++ `setStyleId`

```text
set_style_id(self: Table, arg0: str) -> Table
```

### `Table.set_v_align` → C++ `setVAlign`

```text
set_v_align(self: Table, arg0: VAlignment) -> Table
```

### `Table.set_width` → C++ `setWidth`

```text
set_width(self: Table, arg0: Length) -> Table
```

### `Table.to_xml` → C++ `toXml`

```text
to_xml(self: Table) -> str
```

## TableStyle

`None`, `Grid`, `Light`

## TableStyleDefinition

值字段：`based_on`, `shading`, `run`, `border`, `cell_margin`。

### `TableStyleDefinition.__init__` → C++ `TableStyleDefinition`

```text
__init__(self: TableStyleDefinition) -> None
```

## TargetId

值字段：`name`, `kind`。

### `TargetId.__init__` → C++ `TargetId`

```text
__init__(self: TargetId, name: str = '', kind: TargetKind = <TargetKind.Bookmark: 0>) -> None
```

## TargetKind

`Bookmark`, `Paragraph`, `Heading`, `Figure`, `Table`

## Toggle

`Inherit`, `Off`, `On`

## VAlignment

`Top`, `Center`, `Bottom`
