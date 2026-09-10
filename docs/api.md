# XWord 2.0 使用手册

全部 Python 方法、参数及对应 C++ 方法见 [接口对照](api-reference.md)。公开 C++ 头文件位于 `include/xword/`，总入口为 `xword/xword.hpp`。

## 对象与所有权

| 对象 | 作用 | 创建入口 |
|---|---|---|
| `Document` | 设置、正文、来源及保存 | 构造函数 |
| `Section` | 每节页面、分栏、页码和页眉页脚 | `currentSection()`、`addSection()` |
| `Content` | 按插入顺序持有段落、表格、图片、列表、公式 | 节页眉页脚；内部正文／模板块；`Cell`、`Note` 的基类 |
| `Paragraph` | 文本、字段、公式、图片、范围标记和段落格式 | `addParagraph()`、`addHeadingParagraph()` |
| `Table`／`Cell` | 表格及有序单元格内容 | `addTable(rows, cols)`、`cell(row, col)` |
| `Note` | 带 ID 的富文本脚注／尾注 | 无参数 `addFootnote()`／`addEndnote()` |
| `Image` | 图片资源、尺寸、定位和题注 | `addImage(path)` |
| `BulletList` | 多级、富文本列表 | `addBulletList()`／`addOrderedList()` |
| `Equation` | LaTeX 转 OMML | 文档公式接口、`Content::addMath()` |

父对象持有内容对象。向同一容器继续添加内容不会使已有引用失效；Python 绑定通过 `reference_internal` 保持父对象存活。销毁文档、成功 `open()` 重置文档或显式移动对象后，不再使用旧引用。对象不可复制，文档不可移动；值类型可复制。

`setHeader()`／`setFooter()` 替换当前默认内容，旧内容引用保持存活但不再输出。节的 `header()`／`footer()` 返回可继续追加的容器。一个文档不要由多个线程同时修改或保存；不同文档可独立生成。

## 单位、默认值与格式继承

| 输入 | 单位／默认 |
|---|---|
| `Length::pt/cm/mm/inch/twips` | 明确单位；`dxa()` 转 twips，`emu()` 转 EMU |
| 旧 `Page::setMargins(top,bottom,left,right)` | 厘米，默认各 2.54 cm |
| 页面 | 默认 A4 纵向；页眉页脚距离各 36 pt；装订线 0 |
| `setCustomSize(width,height)` | 两个 `Length` 指定最终物理宽高；方向另设，不隐式交换自定义宽高 |
| 旧图片 `setSize(width,height)` | 96 DPI 像素；0 为自动；只设一边按固有比例推导另一边 |
| `setDimensions(width,height)` | 明确物理尺寸；均须大于 0 |
| 旧段前后间距、首行缩进整数接口 | twips；间距 -1 表示继承 |
| `setFirstLineIndentChars(chars, fontSizePt)` | 2.0 输出字符缩进；兼容参数 `fontSizePt` 不再参与计算 |
| 字号、`HeadingStyle` 段前后间距 | pt |
| `setLineSpacing(value, Auto)` | 行距倍数；`Exact`／`AtLeast` 时为 pt |
| 表格 `setColumnWidth(s)` | 正数比例，不是物理宽度；`setWidth(Length)` 设置表宽 |
| 图片裁剪 | 每边 0≤比例<1，左右／上下总和各小于 1 |
| 页码起始值 | 0 表示续编；正整数表示重启 |

`RunStyle` 的 `bold/italic/underline(false)` 显式关闭格式；`inheritBold/Italic/Underline()` 恢复继承。行内格式优先于段落默认字符格式，未指定部分继续继承 Word 命名样式。字体、颜色、字号只覆盖被指定的部分。`getStyle()` 返回可修改的默认格式引用。

`ParagraphStyle` 中可选长度／对齐字段为空时继承；`Toggle::Inherit/Off/On` 用于分页类布尔格式。首行、字符首行、悬挂缩进通过段落 setter 互斥设置。

命名段落、字符和表格样式分别用 `registerParagraphStyle`、`registerCharacterStyle`、`registerTableStyle` 注册，然后使用 `setStyleId`。`basedOn` 必须存在且不能形成循环。内置 Normal、Heading1—9、Caption、ListParagraph、FootnoteText／Reference、EndnoteText／Reference、CommentText、Hyperlink。

## 分页和分节

```cpp
doc.addPageBreak();
doc.addParagraph().addBreak(BreakType::Line)
                  .addBreak(BreakType::Page)
                  .addBreak(BreakType::Column);
doc.addParagraph("标题后说明").setPageBreakBefore()
   .setKeepWithNext().setKeepTogether().setWidowControl();

auto& section = doc.addSection(SectionBreakType::NextPage);
section.setPage(Page().setOrientation(Orientation::Landscape));
section.setColumns(2, Length::cm(1), true);
section.setPageNumbering(NumberFormat::Decimal, 1);
```

`SectionBreakType` 包含 NextPage、Continuous、OddPage、EvenPage、NextColumn。新节继承页面和分栏配置、页眉页脚链接、标题页设置及注释选项，页码格式继承且默认续编。`Document::setPage()` 始终设置当前节。

不等宽分栏用 `setColumnWidths(vector<Column>, separator)`；每栏包含 width／space，最后一栏 space 不计入总宽。图片和表格自动宽度限制为当前栏宽，不等宽栏使用最窄栏，避免在较窄栏溢出。连续分节涉及纸张或方向变化时，Word 可自行另起一页。

## 页眉页脚

每类页眉和页脚分别有 Default、First、Even 三个槽位：

- 不创建槽位或调用 `linkHeaderToPrevious(type)`／`linkFooterToPrevious(type)`：继承上一节。
- `header(type)`／`footer(type)`：创建本节独立内容，继续调用时追加。
- `clearHeader(type)`／`clearFooter(type)`：创建本节独立空白内容，阻断继承。

`setTitlePage(true)` 按节开启首页不同；`Document::setEvenAndOddHeaders(true)` 按文档开启奇偶页不同。Default 用作普通页／奇数页。开启后，未设置的 First／Even 按 Word 的同类型继承规则处理。

容器支持段落、图片、表格、列表与公式；脚注／尾注引用及批注范围仅允许出现在主文档正文中。页码字段用 `addPageNumber()`、`addPageCount()`、`addSectionPageCount()`。

## 注释、范围和引用

```cpp
auto& note = doc.addFootnote();
note.addParagraph("解释：").addRun("重点", RunStyle().bold());
doc.addParagraph("正文").addFootnoteRef(note.id());
doc.addParagraph("尾注").addEndnoteRef(doc.addEndnote("尾注内容"));

TargetId target("result_table", TargetKind::Table);
doc.addParagraph("见表 ").addReference(target, ReferenceKind::Number);
doc.addTable(2, 2).setCaption("结果").setBookmark(target);
```

`addFootnote(text)`／`addEndnote(text)` 返回正整数 ID；无参数版本返回 `Note&`。`footnote(id)`／`endnote(id)` 可继续添加内容。

`NoteOptions` 支持 Decimal／Roman／Letter 编号、start、Continuous／EachSection／EachPage 重启。尾注禁止 EachPage；脚注位置为 PageBottom／BeneathText，尾注位置为 DocumentEnd／SectionEnd。通过节设置对应选项。

`TargetId(name, kind)` 的 kind 为 Bookmark、Paragraph、Heading、Figure、Table。书签名称以 ASCII 字母或 `_` 开头，只包含字母、数字、下划线，总长度不超过 40；带图表编号的目标预留 `_n` 后缀，建议原名不超过 38 字符。名称全局唯一，引用时必须匹配目标类型。

段落 `setBookmark` 覆盖整段；`startBookmark/endBookmark` 可标记更小范围或跨正文段落的范围。允许前向引用，保存时解析并检查。`ReferenceKind` 支持 Text、Number、Page；Number 要求目标确实有编号。`addHyperlink(text, url)` 支持外部链接，`#name` 链接文档内书签。

图表编号用 `enableImageNumbering`／`enableTableNumbering`，题注使用 SEQ；ByChapter 同时使用 STYLEREF，并要求文档按 Heading1 自动编号。`addFigureTOC`／`addTableTOC` 输出对应图表目录域。Word 刷新后显示最终结果。

`addComment(text, author, date)` 返回从 0 开始的 ID。正文段落用 `startComment/endComment` 标记范围；日期省略时取当前 UTC，显式日期使用 `YYYY-MM-DDTHH:MM:SSZ`。当前实现普通批注，不包含线程回复和修订。

## 原生参考文献

```cpp
BibliographySource source;
source.tag = "Doe2026";
source.type = SourceType::Book;
source.title = "Engineering Notes";
source.authors.push_back({"Jane", "Doe", "", ""});
source.year = "2026";
source.publisher = "Example Press";
doc.addSource(source).setBibliographyStyle("IEEE");
doc.addParagraph("相关论述").addCitation("Doe2026", "12-15");
doc.addBibliography("参考文献");
```

支持 Book、JournalArticle、ConferenceProceedings、Report、InternetSite、Misc。tag／title 必填，tag 唯一。按来源类型填写 authors、year/month/day、publisher/city、journal/volume/issue/pages、doi/url、accessedYear/Month/Day；language 默认 1033。团体作者填 `Author::corporate`，一个来源不能混用团体和个人作者。

来源写入文档 custom XML，正文使用 CITATION，文献列表使用 BIBLIOGRAPHY；默认 Word 内置 IEEE，可用 `setBibliographyStyle` 选择其他已安装的 Word 样式名。DOI 写入标准号字段。库不运行 Word 的 XSL 文献排版器，不伪造最终格式化结果；首次打开须在 Word 更新域。Word 全局来源库不被修改。

## 表格、列表与图片

表格行列为零基索引，列数 1—63。`setHeaderRows(n)` 设置从第一行开始的连续重复表头；旧 `setHeaderRow(lastRow)` 等价于 `setHeaderRows(lastRow+1)`。`setRowAllowSplit(row,false)` 阻止该行拆分，`setRowHeight(row,length,exact)` 设置最小／固定高度。超过单页高度的行仍由 Word 决定排版。

`mergeCells(r1,c1,r2,c2)` 使用闭区间，拒绝越界、倒置和重叠合并；主单元格保留内容，被覆盖单元格不输出。`setBorderStyle` 使用预设边框，`setBorder` 使用详细 Border，`setShading` 和 `setCellMargins` 设置表格默认。Cell 可覆盖边框、底纹、边距、垂直对齐。表格／单元格保留内容插入顺序，也支持嵌套表格。

列表 `setLevel` 影响后续项目；`addItemParagraph(text,level)` 返回 Paragraph，支持富文本和段落格式。层级范围 0—8。`setStart`、`setFormat(format,text)` 定义起始值及 `%1.` 等编号文本，`continueFrom(previous)` 共享前一列表编号；新列表默认重新开始。`setNumId` 是兼容性低级接口，优先使用 `continueFrom`。

图片支持 PNG、JPEG、GIF、BMP、TIFF、EMF、WMF、SVG 部件及尺寸探测。缺失图片默认跳过并记录警告；`SaveOptions::missingImagesAreErrors=true` 将其变为错误。

`Document/Content::addImage` 添加独立图片段落；`Paragraph::addImage` 为段内图片，不能设置块题注。`setWrap` 支持 Inline、Square、TopBottom、BehindText、InFrontOfText；`setPosition` 横向参考 Page／Margin／Column，纵向参考 Page／Margin／Paragraph。`setKeepAspectRatio` 写入编辑锁定，单边尺寸自动推导时始终保留比例；同时指定两边表示指定最终宽高。`setCrop` 设可见区域，`setAltText` 设无障碍说明。SVG 可用 `setSvgFallback` 提供调用方生成的栅格图片；库不做 SVG 栅格化。

## 模板与保存

`open(path)` 成功时重置已有构建状态并切换模板模式，失败返回 false。文本、条件及内容块填充使用 `set`／`setParagraph/Table/Image/BulletList/OrderedList/Equation/DisplayEquation`。同一块 key 的多次调用按顺序追加；块 key 优先于同名标量。

模板模式保留原文档的节、注释、来源及属性；不提供修改这些全局结构的接口。从零生成专用操作在模板模式会明确抛出异常，避免覆盖原部件或静默丢弃设置。新块的直接格式、图片题注和新增命名样式可使用；已存在的来源和注释可在新块中引用。

普通变量只修改 `w:t` 文本节点，跨 run 占位符保留首段文字的格式，替换文本会正确转义。条件标记独占段落，不支持嵌套；块占位符也须独占段落，整段由内容块替换。原节属性保留。正文、页眉页脚中的占位符可以替换；未修改部件保留原始字节。

`save(path)` 返回 bool；推荐 `saveDetailed(path, options)`，其结果包含 success、error(code/part/message)、warnings。错误包括 InvalidArgument、InvalidXml、InvalidReference、MissingResource、IoError。参数 setter 对明显错误抛出 `std::invalid_argument`／`std::out_of_range`，Python 映射为 ValueError／IndexError；保存期间的错误收集到结果。

保存校验包关系和引用，写入同目录临时文件，再替换目标。失败不替换已有成品；同一文档重复保存不会累加关系或编号。实际布局、页面总数、分页引用和 Word 原生文献域缓存由 Word 更新；`settings.xml` 及域 dirty 标记会请求更新，但客户端是否自动执行由客户端决定。

`toXml()` 只供诊断，可能包含内部占位节点及未分配关系，不能单独拿来组成成品包。业务应用使用 `saveDetailed()`。
