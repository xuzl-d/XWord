# XWord 2.0

C++17 / Python `.docx` 生成库，输出 OOXML Transitional。生成过程不需要安装 Office；实际分页、目录页码和原生参考文献格式由 Word 更新域后计算。

## 功能

- 分页符、换栏、段前分页、与下段同页、段内不分页和孤行控制。
- 五种分节；自定义页面、分栏、节页码；默认／首页／偶数页的多段、图片、表格页眉页脚。
- 富文本脚注、尾注、书签、超链接、交叉引用、SEQ 图表题注及图表目录。
- Word 原生文献来源、CITATION／BIBLIOGRAPHY 域，默认 IEEE。
- 命名段落／字符／表格样式，明确的格式继承，精细表格格式、合并与跨页控制。
- 段内／浮动图片、裁剪、替代文字、SVG 回退入口，LaTeX 到 OMML 公式。
- 普通范围批注、核心及自定义文档属性。
- XML 节点级模板替换：跨 run 变量、条件块、段落／表格／图片／列表／公式块。
- 可检查的保存错误和警告、部件级关系验证、临时文件成功提交。

完整范围与后续缺口见 [能力矩阵](docs/capabilities.md)。接口入口见 [使用手册](docs/api.md)、[C++／Python 接口对照](docs/api-reference.md)及 [2.0 迁移说明](docs/migration-2.0.md)。

## C++ 示例

```cpp
#include "xword/xword.hpp"
using namespace xword;

Document doc;
doc.enableHeadingNumbering().enableTableNumbering();
doc.currentSection().setPage(Page()).setTitlePage();
doc.currentSection().header(HeaderFooterType::First).addParagraph("封面");
doc.setFooter().addPageNumber().addRun(" / ").addPageCount();

doc.addHeadingParagraph("结果", 1)
   .setBookmark(TargetId("results", TargetKind::Heading));
doc.addParagraph("正文").setKeepTogether().setWidowControl();
doc.addParagraph("注释").addFootnoteRef(doc.addFootnote("脚注内容"));

doc.addSection().setPage(Page().setOrientation(Orientation::Landscape));
auto& table = doc.addTable(2, 2);
table.setBorderStyle(TableStyle::Grid).setHeaderRows(1).setCaption("统计表");
table.cell(0, 0).addParagraph("名称");
table.cell(1, 0).addParagraph("样本");

auto result = doc.saveDetailed("report.docx");
if (!result.success) { /* result.error.code / part / message */ }
```

综合示例在 [examples/demo_v2.cpp](examples/demo_v2.cpp)。

## 构建与验证

```powershell
cmake -S . -B build/release -G "Visual Studio 17 2022" -A x64
cmake --build build/release --config Release
ctest --test-dir build/release -C Release --output-on-failure
cmake --install build/release --prefix dist --config Release
```

要求 CMake 3.14+、C++17。使用 v141 工具集时，在首次配置命令追加 `-T v141`；消费者与库使用一致的工具集／CRT 配置。2.0 改变 ABI，必须重新编译消费者。Debug DLL 为 `xwordd.dll`。

库依赖固定的 pugixml 1.14 与仓库内 miniz，普通 C++ 构建不联网。输出 `bin/xword.dll`、`lib/xword.lib` 和 `include/xword/`。

官方 schema 校验是独立开发工具，不是生成库的运行依赖：

```powershell
dotnet run --project tools/OpenXmlValidator -- build/release/v2_features.docx
```

工具默认使用 .NET 8 和 Open XML SDK 3.0.1。客户端验收、校验边界及生成工件见 [验证说明](docs/validation.md)。

## Python

从源码构建匹配当前解释器的扩展，不再分发仓库中的固定 CPython `.pyd`：

```powershell
python -m pip install .
```

```python
import xword as x

doc = x.Document()
doc.add_heading("报告", 1)
doc.add_paragraph("正文").set_keep_together()
doc.current_section().footer().add_paragraph().add_page_number()
result = doc.save_detailed("report.docx")
if not result.success:
    raise RuntimeError(result.error.message)
```

直接用 CMake 构建 Python 时启用 `XWORD_BUILD_PYTHON`，通过 `pybind11_DIR` 指定 pybind11 2.13.6 的 CMake 配置目录，通过 `Python_EXECUTABLE` 固定解释器。更换解释器或从旧版 FindPython 配置迁移时使用新的构建目录。Python wheel 包含扩展和相邻运行库。

已有 CMake 原生产物且不安装额外打包工具时，也可离线制作匹配当前解释器的 wheel：

```powershell
cmake --install build/release --prefix build/stage --config Release --component python
python tools/package_built_wheel.py build/stage build/wheels
```

## 模板

`${key}` 用于文本替换；块内容占位符须独占一个段落。条件标记 `{%if key%}`、`{%else%}`、`{%endif%}` 各占一个段落。空字符串、`0`、`false` 或未设置变量为假，非空内容块为真。

```cpp
Document doc;
if (doc.open("template.docx")) {
    doc.set("name", "张三");
    doc.set("detail", true);
    doc.setTable("results", 2, 2).cell(0, 0).addParagraph("结果");
    doc.save("filled.docx");
}
```

模板模式与从零生成模式分开使用。嵌套条件、循环和任意 docx 编辑不属于当前接口。
