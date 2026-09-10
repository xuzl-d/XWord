# 2.0 能力与 OpenXML 对照

“支持”表示实现对应 OOXML 生成接口并有自动化覆盖；最终 Word 客户端行为仍需按 [验证说明](validation.md) 验收。库不实现自动排版引擎。

| 能力 | OpenXML 对应 | 自动检查 | 边界 |
|---|---|---|---|
| 分页／换栏 | br、pageBreakBefore、keepNext、keepLines、widowControl | test_v2、test_python | 实际页数由 Word 决定 |
| 五种分节／纸张／分栏／节页码 | sectPr、type、pgSz、pgMar、cols、pgNumType | test_v2、OpenXmlValidator | 连续节纸张变化可能另起页 |
| 首／偶／默认页眉页脚与继承 | header/footerReference、titlePg、evenAndOddHeaders | test_v2 | 未创建槽位按 Word 规则继承 |
| 富文本脚注尾注 | footnotes/endnotes、Reference、numFmt/numRestart/pos | test_v2、test_python | 引用仅允许主文档 |
| 书签／链接／交叉引用 | bookmarkStart/End、hyperlink、REF/PAGEREF | test_v2、test_python | 页码缓存不由库计算 |
| 标题／目录／图表题注 | Heading1—9、TOC、SEQ、STYLEREF | test_v2、demo_v2 | 最终域结果需 Word 更新 |
| 原生文献 | bibliography custom XML、CITATION/BIBLIOGRAPHY | test_v2 六类来源、OpenXmlValidator | 默认 IEEE；不运行 Word XSL，不写全局来源库 |
| 段落／字符／表格样式 | styles、basedOn、pPr/rPr/tblPr | test_v2、test_python | 未实现主题编辑器 |
| 多级富文本列表 | abstractNum、num、lvl、numPr | test_v2 | 层级 0—8 |
| 表格格式／合并／跨页 | tblGrid、tcPr、vMerge/gridSpan、tblHeader/cantSplit | test_v2 | 不计算表格物理分页 |
| 有序嵌套内容 | p/tbl/drawing/OMML | test_v2、test_basic | 不提供任意 DOM 编辑 |
| 段内／浮动图片 | inline/anchor、wrap、srcRect、docPr | test_v2、test_python | 无紧密轮廓环绕、无 SVG 栅格化 |
| 公式 | OMML oMath/oMathPara | test_eq_dump、test_basic、OpenXmlValidator | 现有 LaTeX 子集，非完整 TeX |
| 普通批注 | comments、commentRangeStart/End/Reference | test_v2、test_python | 无线程回复和修订 |
| 核心／自定义属性 | docProps/core、custom、vt | test_v2、test_python | 日期采用 UTC ISO 8601 |
| 模板变量／条件／块 | XML 文本及块节点、OPC 关系 | test_basic、test_v2 | 无循环与嵌套条件 |
| 包与保存 | ZIP、Content Types、Relationships | test_v2、OpenXmlValidator | 不提供签名／加密 |
| Python 一致性 | 同一 C++ 引擎 | test_python 逐部件 XML 比较 | wheel 按解释器构建 |

## 后续缺口

按优先级保留以下路线，未计入本轮已实现范围：

1. 报告模板循环、嵌套条件、索引、更多题注类别、GB/T 7714／CSL 文献格式。
2. 原生图表与嵌入工作簿、形状和文本框、SmartArt、OLE、复杂轮廓环绕。
3. 内容控件、修订及接受／拒绝、现代线程批注、文档合并、完整读取和往返编辑。
4. Word／LibreOffice 自动刷新、PDF 导出、实际页码提取、字体嵌入、保护、签名和加密。

规范参考：[WordprocessingML 结构](https://learn.microsoft.com/en-us/office/open-xml/word/structure-of-a-wordprocessingml-document)、[SectionType](https://learn.microsoft.com/en-us/dotnet/api/documentformat.openxml.wordprocessing.sectiontype)、[参考文献来源](https://learn.microsoft.com/en-us/office/vba/word/concepts/working-with-word/working-with-bibliographies)、[文档验证](https://learn.microsoft.com/en-us/office/open-xml/word/how-to-validate-a-word-processing-document)。
