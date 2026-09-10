# 从 1.x 迁移到 2.0

2.0 改变 C++ ABI、部分值类型布局和低级接口。重新编译所有消费者，使用匹配架构、工具集和 CRT 的 DLL／导入库。Python 从源码构建与当前解释器匹配的 wheel，不再依赖仓库中 CPython 3.13 的预编译文件。

| 1.x 用法／行为 | 2.0 用法／行为 |
|---|---|
| `setPage`、`addSectionBreak` | 保留；推荐 `currentSection().setPage(...)`、`addSection(...)` |
| 默认页眉页脚的单个 Paragraph | 旧便捷接口保留；`section.header/footer(type)` 返回 Content |
| `clearHeader`／`clearFooter` 通过省略引用导致继续继承 | 明确创建本节空白；继承用 `linkHeader/FooterToPrevious` |
| 首行“字符”缩进换算为固定 twips | `setFirstLineIndentChars` 写字符单位；固定尺寸用 `setFirstLineIndent` |
| `bold(false)` 与“未设置”无法区分 | 显式关闭；恢复继承用 `inheritBold()`，italic／underline 同理 |
| 标题 1—6 级 | 扩展到 1—9；`addHeadingParagraph` 返回可设置书签／格式的段落 |
| 图表题注为固化文本编号 | SEQ 域、缓存结果和可引用书签；Word 可重新编号 |
| `CellImage` 暴露 filepath／rId 等字段 | `CellImage` 为 Image 别名；使用 Image 方法，不直接管理媒体关系 |
| 单元格分别存图片和段落 | 按调用顺序输出；可添加嵌套表格 |
| `cell.images()`／`paragraphs()` 等存储访问 | 从推荐接口移除；添加时保留返回引用 |
| `setHeaderRow(n)` 可以指定离散行 | 从第一行到 n 的连续表头；推荐 `setHeaderRows(count)` |
| 列表 `setLevel` 影响整份列表 | 影响后续条目；单项层级用 `addItemParagraph(text, level)` |
| 未检查的表格索引、尺寸和合并 | 无效输入抛异常，重叠合并拒绝 |
| `save()` 布尔结果 | 保留；推荐 `saveDetailed()` 获取位置与原因 |
| Python `table.set_style(TableStyle.Grid)` | 改为 `set_border_style`；`set_style` 统一接收 RunStyle，`set_run_style` 为别名 |
| Python 消费 move-only 内容参数 | 使用 `set_paragraph/table/image/...` 工厂；不从 Python 隐式移动对象 |

字符串使用 UTF-8；已有宽字符串文字／图片路径重载保留。中文输出路径统一由 Unicode 文件接口处理。新 Length API 显式标注物理单位，旧数值接口仍按手册中的原单位解释。

Document 在成功 `open()` 后重置已构造状态。已有内容对象引用不可继续使用。模板模式使用 `set*` 系列；从零生成使用 `add*` 系列，不把两条模式混用。

`Paragraph::toXml()` 等输出是内部诊断片段；媒体、书签和引用要经过完整包生成流程解析。关系 ID 和媒体文件名不再是可依赖的输出契约。
