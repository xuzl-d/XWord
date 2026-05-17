# XWord — C++ docx 生成库

纯头文件接口、ABI 稳定的 .docx 文档生成库。基于 MSVC v141 工具集编译为 DLL，供其他项目二进制依赖。

## 功能概览

- **页面设置** — 纸张大小（A4/Letter）、方向、页边距
- **标题** — 1–6 级标题，自定义字体/颜色/间距/对齐，自动编号（第1章 / 1.1 / 1.1.1）
- **目录** — TOC 域，指定大纲级别范围
- **段落** — 首行缩进、对齐、富文本（加粗/斜体/下划线/字号/颜色）
- **表格** — 网格边框、表头行、单元格合并、单元格内嵌图片和公式
- **图片** — PNG/EMF/WMF/SVG，自动尺寸检测，按章节编号（图1-1/图2-1）
- **列表** — 无序列表和有序列表，多级缩进
- **LaTeX 公式** — 行内和显示模式，转译希腊字母/分式/根号/矩阵/上下标
- **页眉页脚** — 文字或富文本（含页码域 `PAGE` / `NUMPAGES`）
- **模板引擎** — 加载 docx 模板，`${key}` 占位替换，`{%if%}/{%else%}/{%endif%}` 条件块

## 快速开始

```cpp
#include "xword/xword.hpp"

int main() {
    using namespace xword;

    Document doc;
    doc.setPage(Page().setSize(PageSize::A4));

    // 标题
    doc.setHeadingStyle(1, HeadingStyle()
        .setFont("SimHei").setFontSize(26).setBold(true)
        .setColor("1F4E79").setAlignment(Alignment::Center));
    doc.enableHeadingNumbering();
    doc.setHeadingNumFormat(HeadingNumFormat::Chapter);

    doc.addHeading("概述", 1);
    doc.addParagraph("这是一个自动首行缩进的段落。");

    // 富文本
    doc.addParagraph()
        .addRun("加粗 ", RunStyle().bold())
        .addRun("红色 ", RunStyle().color("FF0000"))
        .addRun("组合", RunStyle().bold().italic().fontSize(14));

    // 表格
    auto& tbl = doc.addTable(3, 2);
    tbl.setStyle(TableStyle::Grid).setCaption("示例表");
    tbl.cell(0, 0).addParagraph("名称");
    tbl.cell(0, 1).addParagraph("数值");
    tbl.cell(1, 0).addParagraph("A");
    tbl.cell(1, 1).addParagraph("100");

    // 图片（支持按章节自动编号）
    doc.enableImageNumbering("图", CaptionNumStyle::ByChapter);
    doc.enableTableNumbering("表", CaptionNumStyle::ByChapter);
    doc.addImage("chart.png").setCaption("趋势图");

    // LaTeX 公式
    doc.addDisplayEquation("E = mc^2");
    doc.addDisplayEquation(
        xword::format("\\frac{a}{b} = %.3f", 1.0 / 3.0));

    // 页眉页脚
    doc.setHeader("XWord 演示文档");
    doc.setFooter()
        .addRun("第 ").addPageNumber()
        .addRun(" 页 / 共 ").addPageCount().addRun(" 页")
        .setAlignment(Alignment::Center);

    doc.save("output.docx");
    return 0;
}
```

## 模板引擎

```cpp
// 1. 在 Word 中做好模板，直接写入占位符：
//    尊敬的${name}：  /  {%if detail%}详细信息{%else%}摘要{%endif%}

Document doc;
doc.open("template.docx");
doc.set("name", "张三");
doc.set("detail", true);
doc.save("output.docx");
```

**占位符语法**

| 标记 | 含义 |
|---|---|
| `${key}` | 替换为 set() 设置的 value |
| `{%if key%}` | 条件开始——独占一段 |
| `{%else%}` | else 分支——独占一段（可选） |
| `{%endif%}` | 条件结束——独占一段 |

**真值判断**：`"false"` / `"0"` / `""` / key 不存在 → false；其余 → true

条件块可跨多段（包括表格）。当前不支持嵌套 `{%if%}`。

## 作为外部依赖使用

### 产物结构

```
dist/
├── bin/xword.dll         # 运行时库
├── lib/xword.lib         # 导入库（隐式链接）
└── include/xword/        # 公共头文件（9 个 .hpp）
```

### CMake 集成

```cmake
# 假设 dist/ 在项目根目录
add_library(xword SHARED IMPORTED)
set_target_properties(xword PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/dist/bin/xword.dll"
    IMPORTED_IMPLIB   "${CMAKE_SOURCE_DIR}/dist/lib/xword.lib"
)
target_include_directories(xword INTERFACE "${CMAKE_SOURCE_DIR}/dist/include")
target_link_libraries(my_app PRIVATE xword)
```

### ABI 兼容性

- 所有公共类均采用 **Pimpl** 模式，类布局仅含一个 `unique_ptr`，成员变更不影响 ABI
- DLL 使用 **MSVC v141** (VS2017) 工具集编译
- 消费方需使用相同 MSVC 主版本和相同 CRT 链接方式（`/MD` Release 或 `/MDd` Debug）
- Debug 版 DLL 文件名为 `xword**d**.dll`

## 编译

```bash
# 要求：CMake 3.14+, Visual Studio 2019 (含 v141 工具集)
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -T v141
cmake --build build --config Release
cmake --install build --prefix dist --config Release
```

## API 参考

详见头文件注释，每个公开方法和类均附有中文说明。入口头文件为 `xword/xword.hpp`。

| 头文件 | 说明 |
|---|---|
| `Document.hpp` | `Document` 文档构建器、`BulletList` 列表 |
| `Paragraph.hpp` | `Paragraph` 段落（文本运行、字段、公式） |
| `Table.hpp` | `Table` 表格、`Cell` 单元格、`CellImage` 描述符 |
| `Image.hpp` | `Image` 图片 |
| `Equation.hpp` | `Equation` LaTeX 公式 |
| `Run.hpp` | `RunStyle` 文本格式（值类型） |
| `Types.hpp` | 枚举、`Page`、`HeadingStyle` 等值类型 |
| `Format.hpp` | `xword::format()` — printf 风格字符串格式化 |
| `xword.hpp` | 总头文件 |

## 许可证

MIT
