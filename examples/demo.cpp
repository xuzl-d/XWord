#include "xword/xword.hpp"
#include <iostream>
#include <filesystem>
#include <cmath>
#include <string>

int main() {
    using namespace xword;

    Document doc;

    // ---- Page settings ----
    doc.setPage(Page()
        .setSize(PageSize::A4)
        .setOrientation(Orientation::Portrait)
        .setMargins(2.54, 2.54, 3.0, 3.0));

    // ---- Header / Footer ----
    doc.setHeader("XWord 演示文档");
    doc.setFooter()
        .addRun("第 ").addPageNumber()
        .addRun(" 页 / 共 ").addPageCount().addRun(" 页")
        .setAlignment(Alignment::Center);

    // ---- Heading style customization ----
    doc.setHeadingStyle(1, HeadingStyle()
        .setFont("SimHei").setFontSize(26).setBold(true)
        .setColor("1F4E79").setLineSpacing(1.5)
        .setSpaceBefore(12).setSpaceAfter(6)
        .setAlignment(Alignment::Center));  // article title centered
    doc.setHeadingStyle(2, HeadingStyle()
        .setFont("SimHei").setFontSize(22).setBold(true)
        .setColor("2E75B6").setLineSpacing(1.3).setSpaceBefore(10).setSpaceAfter(4));
    doc.setHeadingStyle(3, HeadingStyle()
        .setFont("SimHei").setFontSize(18).setBold(true)
        .setColor("333333").setSpaceBefore(8).setSpaceAfter(4));

    // ---- Enable heading auto-numbering with chapter format ----
    doc.enableHeadingNumbering();
    doc.setHeadingNumFormat(HeadingNumFormat::Chapter);  // 第1章, 1.1, 1.1.1

    // ---- Caption numbering: by chapter (图1-1, 图1-2, 图2-1, 表1-1, ...) ----
    doc.enableImageNumbering("图", CaptionNumStyle::ByChapter);
    doc.enableTableNumbering("表", CaptionNumStyle::ByChapter);

    // ---- TOC ----
    doc.addTOC("1-3", "目录");

    // ---- Headings ----
    doc.addHeading("XWord 概述", 1);
    doc.addHeading("富文本段落", 2);

    // ---- Set default indent for all body paragraphs ----
    doc.setDefaultParagraphIndent(2);  // default: 2 chars first-line indent

    // ---- Simple paragraph (auto-indented by default) ----
    doc.addParagraph("这是一个自动首行缩进的段落。通过 setDefaultParagraphIndent(2) 设置后，"
                     "所有正文段落都默认缩进两个字符，无需每个段落单独设置。");
    doc.addParagraph("这是另一个段落，同样自动带有首行缩进。");

    // ---- Rich text paragraph ----
    doc.addParagraph()
        .addRun("这段文字包含：")
        .addRun("加粗文字 ", RunStyle().bold())
        .addRun("斜体文字 ", RunStyle().italic())
        .addRun("红色文字 ", RunStyle().color("FF0000"))
        .addRun("下划线文字 ", RunStyle().underline())
        .addRun("大字 ", RunStyle().fontSize(20))
        .addRun("组合效果", RunStyle().bold().italic().color("0070C0").fontSize(14))
        .setAlignment(Alignment::Center);

    // ---- Alignment examples ----
    doc.addParagraph("左对齐段落（默认）。").setAlignment(Alignment::Left);
    doc.addParagraph("居中对齐段落。").setAlignment(Alignment::Center);
    doc.addParagraph("右对齐段落。").setAlignment(Alignment::Right);

    // ---- Heading 2 ----
    doc.addHeading("表格", 2);

    // ---- Table ----
    doc.addParagraph("下面是一个带边框的学生成绩表：");

    auto& table = doc.addTable(5, 4);
    table.setHeaderRow(0).setStyle(TableStyle::Grid).setCaption("学生成绩表");

    table.cell(0, 0).addParagraph("姓名", RunStyle().bold());
    table.cell(0, 1).addParagraph("语文");
    table.cell(0, 2).addParagraph("数学");
    table.cell(0, 3).addParagraph("英语");

    table.cell(1, 0).addParagraph("张三");
    table.cell(1, 1).addParagraph("95");
    table.cell(1, 2).addParagraph("88");
    table.cell(1, 3).addParagraph("92");

    table.cell(2, 0).addParagraph("李四");
    table.cell(2, 1).addParagraph("78");
    table.cell(2, 2).addParagraph("85");
    table.cell(2, 3).addParagraph("80");

    table.cell(3, 0).addParagraph("王五");
    table.cell(3, 1).addParagraph("90");
    table.cell(3, 2).addParagraph("95");
    table.cell(3, 3).addParagraph("88");

    table.cell(4, 0).addParagraph("平均");
    table.cell(4, 1).addParagraph("87.7");
    table.cell(4, 2).addParagraph("89.3");
    table.cell(4, 3).addParagraph("86.7");

    // Merge header cells to show total
    table.mergeCells(0, 1, 0, 3); // Will redo - let's keep it simple

    // ---- Heading 3 ----
    doc.addHeading("合并单元格表格", 2);

    auto& table2 = doc.addTable(3, 3);
    table2.setStyle(TableStyle::Grid).setCaption("合并示例");

    // Merge first row entirely
    table2.mergeCells(0, 0, 0, 2);
    table2.cell(0, 0).addParagraph("标题行（跨3列合并）");

    table2.mergeCells(1, 0, 2, 0);
    table2.cell(1, 0).addParagraph("纵向\n合并");

    table2.cell(1, 1).addParagraph("单元格 1,1");
    table2.cell(1, 2).addParagraph("单元格 1,2");
    table2.cell(2, 1).addParagraph("单元格 2,1");
    table2.cell(2, 2).addParagraph("单元格 2,2");

    // ---- Table with equations ----
    doc.addHeading("表格中的公式", 3);

    auto& table3 = doc.addTable(3, 2);
    table3.setStyle(TableStyle::Grid).setHeaderRow(0).setCaption("常见公式");

    table3.cell(0, 0).addParagraph("名称");
    table3.cell(0, 1).addParagraph("公式");

    table3.cell(1, 0).addParagraph("求和");
    table3.cell(1, 1).addEquation("\\sum_{i=1}^{n} x_i+y_i");

    table3.cell(2, 0).addParagraph("积分");
    table3.cell(2, 1).addEquation("\\int_{0}^{\\infty} e^{-x^2} dx = \\frac{\\sqrt{\\pi}}{2}");

    doc.addParagraph("表格中的公式可以和文字混排：");
    auto& table4 = doc.addTable(2, 2);
    table4.setStyle(TableStyle::Grid).setCaption("公式混排");
    table4.cell(0, 0).addParagraph()
        .addRun("当 ")
        .addEquation("\\Delta = b^2 - 4ac > 0")
        .addRun(" 时");
    table4.cell(0, 1).addParagraph()
        .addRun("解为 ")
        .addEquation("x = \\frac{-b \\pm \\sqrt{\\Delta}}{2a}");
    table4.cell(1, 0).addParagraph("勾股定理");
    table4.cell(1, 1).addEquation("a^2 + b^2 = c^2");

    // ---- Lists ----
    doc.addHeading("列表", 2);

    doc.addParagraph("无序列表：");
    doc.addBulletList()
        .addItem("第一项：项目介绍")
        .addItem("第二项：技术方案")
        .addItem("第三项：实施计划");

    doc.addParagraph("有序列表：");
    doc.addOrderedList()
        .addItem("安装依赖库")
        .addItem("编译项目")
        .addItem("运行测试");

    doc.addHeading("插图", 2);
    doc.addParagraph("插入图片测试（按章节自动编号：图1-1、图1-2 ...）：");
    doc.addImage("Board001.png")
       .setCaption("面板示意图")
       .setAlignment(Alignment::Center);

    doc.addImage("test.emf")
       .setCaption("矢量示意图 （自动适应页面宽度）")
       .setAlignment(Alignment::Center);

    // ---- Heading 5: Equations ----
    doc.addHeading("LaTeX 公式", 2);

    doc.addParagraph("下面展示各种 LaTeX 公式示例。");

    doc.addParagraph("行内公式（与文字混排）：");
    doc.addParagraph()
        .addRun("一元二次方程 ")
        .addEquation("ax^2 + bx + c = 0")
        .addRun(" 的求根公式为 ")
        .addEquation("x = \\frac{-b \\pm \\sqrt{b^2 - 4ac}}{2a}")
        .addRun("。");

    doc.addParagraph()
        .addRun("当 ")
        .addEquation("\\Delta = b^2 - 4ac > 0")
        .addRun(" 时，方程有两个不同的实根。");

    doc.addParagraph("显示公式（独行居中）：");
    doc.addDisplayEquation("E = mc^2");
    doc.addDisplayEquation("\\sum_{i=1}^{n} x_i = x_1 + x_2 + \\cdots + x_n");
    doc.addDisplayEquation("\\int_{0}^{\\infty} e^{-x^2} dx = \\frac{\\sqrt{\\pi}}{2}");

    // printf-style helper: inject computed values into a LaTeX template
    // without {{}} escaping noise.
    double a_ = 1.0, b_ = -5.0, c_ = 6.0;
    double delta = b_ * b_ - 4 * a_ * c_;
    doc.addParagraph("带计算结果的公式（xword::format）：");
    doc.addDisplayEquation(
        xword::format("\\Delta = b^2 - 4ac = %.2f", delta));
    doc.addDisplayEquation(
        xword::format("x_1 = \\frac{-b + \\sqrt{\\Delta}}{2a} = %.3f",
                      (-b_ + std::sqrt(delta)) / (2 * a_)));

    doc.addParagraph("矩阵公式：");
    doc.addDisplayEquation(
        "\\begin{pmatrix} a_{11} & a_{12} & a_{13} \\\\ "
        "a_{21} & a_{22} & a_{23} \\\\ "
        "a_{31} & a_{32} & a_{33} \\end{pmatrix}");

    doc.addParagraph("希腊字母与符号：");
    doc.addEquation("\\alpha + \\beta = \\gamma, \\quad \\Delta x \\to 0, \\quad \\infty");
    doc.addParagraph();

    doc.addParagraph("积分与定积分：");
    doc.addDisplayEquation("\\int_{a}^{b} f(x)dx = F(b) - F(a)");

    doc.addParagraph("三角函数求导：");
    doc.addEquation("\\frac{d}{dx}\\sin x = \\cos x, \\quad \\frac{d}{dx}\\cos x = -\\sin x");

    doc.addParagraph();

    doc.addParagraph("带 accent 的符号：");
    doc.addEquation("\\bar{x} = \\frac{1}{n}\\sum_{i=1}^{n} x_i, \\quad \\hat{y} = \\vec{a} \\cdot \\vec{b}");

    //第2章
    doc.addHeading("章节测试", 1);
    doc.addHeading("钢铁是怎么练成的", 2);
    doc.addParagraph("我也不知道");

    // 第2章的图与表，验证章内编号重置（图2-1、表2-1）
    doc.addImage(u8"平面图.emf").setCaption(u8"第二章首图");
    // 故意添加一个不存在的图片：库会跳过它，文档仍合法可打开
    doc.addImage("nonexistent_xyz.png").setCaption("缺失图片测试");
    auto& tableCh2 = doc.addTable(2, 2);
    tableCh2.setStyle(TableStyle::Grid).setCaption(u8"第二章演示表");
    tableCh2.cell(0, 0).addParagraph("A");
    tableCh2.cell(0, 1).addParagraph("B");
    tableCh2.cell(1, 0).addParagraph("C");
    tableCh2.cell(1, 1).addParagraph("D");

    doc.addHeading("疯狂星期四", 2);
    doc.addHeading("买一送一", 3);
    // ---- Save ----
    std::string outputPath = "demo_output.docx";
    if (doc.save(outputPath)) {
        std::cout << "Document saved to: " << outputPath << std::endl;
    } else {
        std::cerr << "Failed to save document!" << std::endl;
        return 1;
    }

    return 0;
}
