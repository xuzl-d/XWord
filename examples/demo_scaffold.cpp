#include "xword/xword.hpp"
#include <iostream>
#include <string>

// Generates 扣件式落地脚手架_1.docx — a 落地式脚手架 (ground-supported scaffold)
// calculation book that mirrors the reference document of the same name:
// same chapter layout, body/heading styles, table and figure captions, and the
// same LaTeX source for every formula.
//
// Figure resources live in examples/res/scaffold/.
int main() {
    using namespace xword;

    Document doc;

    // ---- Page settings: A4 portrait, 2.54 cm margins all round ----
    doc.setPage(Page()
        .setSize(PageSize::A4)
        .setOrientation(Orientation::Portrait)
        .setMargins(2.54, 2.54, 2.54, 2.54));

    // East-Asian line grid, as in the reference document: Word then lays every
    // line out on a 15.6pt pitch instead of the natural line height.
    doc.currentSection().setDocumentGrid(DocGridType::Lines, 312);

    // The reference document opens with an empty leading section; the section
    // break carries the page setup, and the body follows in the next section.
    doc.addSectionBreak(SectionBreakType::Continuous);

    // ---- Body text: 宋体 / Times New Roman ----
    // The document default (Normal style, and therefore table cells, captions
    // and formulas) is 五号 10.5pt; body paragraphs override it to 12pt.
    // The reference document has no space after body paragraphs, so every
    // paragraph below calls setSpacingAfter(0) to cancel the 8pt that the
    // document defaults would otherwise apply.
    doc.setBodyFont(u8"宋体", "Times New Roman", u8"宋体");
    doc.setBodyFontSize(10.5);
    doc.setBodyLineSpacing(1.25);
    doc.setBodyRunStyle(RunStyle().fontSize(12));
    doc.setTableRunStyle(RunStyle().fontSize(10.5).font(u8"宋体").eastAsiaFont(u8"宋体"));
    doc.setDefaultParagraphIndent(2, 12);

    // ---- Heading styles: 15 / 14 / 12 pt, black bold ----
    // No explicit font: headings inherit 宋体 / Times New Roman from the
    // document defaults, so Latin text inside a heading stays Times New Roman.
    doc.setHeadingStyle(1, HeadingStyle()
        .setFontSize(15).setBold(true)
        .setSpaceBefore(17).setSpaceAfter(16.5)
        .setAlignment(Alignment::Center));
    doc.setHeadingStyle(2, HeadingStyle()
        .setFontSize(14).setBold(true)
        .setSpaceBefore(13).setSpaceAfter(13));
    doc.setHeadingStyle(3, HeadingStyle()
        .setFontSize(12).setBold(true)
        .setSpaceBefore(13).setSpaceAfter(13));

    auto body = [&]() -> Paragraph& { return doc.addParagraph().setSpacingAfter(0); };
    // Table cells in the reference document are centred with no extra spacing.
    // Rows carry a 396.9-twip minimum height in the reference document.
    auto cellPara = [](Cell& c) -> Paragraph& {
        // Single line spacing: the document default is 1.25, but the reference
        // document only applies that to body paragraphs, not to table cells.
        return c.addParagraph().setAlignment(Alignment::Center)
                .setSpacingAfter(0).setLineSpacing(1.0);
    };
    auto cellMath = [](Cell& c, const std::string& latex) -> Equation& {
        return c.addMath(latex, EquationMode::Display);
    };
    // Table and figure captions: bold, centred, unindented, 10.5pt.
    auto caption = [&](const std::string& text) -> Paragraph& {
        return doc.addParagraph().setSpacingAfter(0).setFirstLineIndent(0)
                  .setLineSpacing(1.0)
                  .addRun(text, RunStyle().bold().fontSize(10.5).font(u8"宋体").eastAsiaFont(u8"宋体"))
                  .setAlignment(Alignment::Center);
    };
    auto figure = [&](const char* path, double cx, double cy) -> Image& {
        return doc.addImage(path)
                  .setDimensions(Length::pt(cx), Length::pt(cy))
                  .setAlignment(Alignment::Center);
    };

    // ---- 第1章 落地式脚手架计算书-扣件式落地脚手架_1 ----
    doc.addHeading(u8"第1章 落地式脚手架计算书-扣件式落地脚手架_1", 1);
    // ---- 1.1 计算依据 ----
    doc.addHeading(u8"1.1 计算依据", 2);
    body().addRun(u8"《施工脚手架通用规范》GB55023-2022");
    body().addRun(u8"《建筑施工扣件式钢管脚手架安全技术规范》JGJ130-2011");
    body().addRun(u8"《建筑结构荷载规范》GB50009-2012");
    body().addRun(u8"《钢结构设计标准》GB50017-2017");
    body().addRun(u8"《建筑地基基础设计规范》GB50007-2011");
    body().addRun(u8"《建筑结构可靠性设计统一标准》GB50068-2018");
    // ---- 1.2 脚手架计算参数 ----
    doc.addHeading(u8"1.2 脚手架计算参数", 2);
    caption(u8"表1 脚手架布置参数");
    {
        auto& t = doc.addTable(5, 4);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"脚手架类型");
        cellPara(t.cell(0, 1)).addRun(u8"扣件式脚手架");
        cellPara(t.cell(0, 2)).addRun(u8"脚手架搭设排数");
        cellPara(t.cell(0, 3)).addRun(u8"双排");
        cellPara(t.cell(1, 0)).addRun(u8"钢管计算截面规格");
        cellPara(t.cell(1, 1)).addRun(u8"Φ48x3.5mm");
        cellPara(t.cell(1, 2)).addRun(u8"脚手架搭设高度H(m)");
        cellPara(t.cell(1, 3)).addRun(u8"12.00");
        cellPara(t.cell(2, 0)).addRun(u8"脚手架搭设长度L(m)");
        cellPara(t.cell(2, 1)).addRun(u8"9.00");
        cellPara(t.cell(2, 2)).addRun(u8"脚手架步距h(mm)");
        cellPara(t.cell(2, 3)).addRun(u8"1500");
        cellPara(t.cell(3, 0)).addRun(u8"立杆纵向间距La(mm)");
        cellPara(t.cell(3, 1)).addRun(u8"1500");
        cellPara(t.cell(3, 2)).addRun(u8"立杆横向间距Lb(mm)");
        cellPara(t.cell(3, 3)).addRun(u8"900");
        cellPara(t.cell(4, 0)).addRun(u8"内排立杆到墙距离(mm)");
        cellPara(t.cell(4, 1)).addRun(u8"300");
        cellPara(t.cell(4, 2)).addRun(u8"扣件抗滑移折减系数");
        cellPara(t.cell(4, 3)).addRun(u8"1.00");
    }
    {
        auto& t = doc.addTable(2, 4);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"纵横向水平杆布置方式");
        cellPara(t.cell(0, 1)).addRun(u8"横向水平杆在上");
        cellPara(t.cell(0, 2)).addRun(u8"纵向水平杆上横向水平杆根数n");
        cellPara(t.cell(0, 3)).addRun(u8"2");
        cellPara(t.cell(1, 0)).addRun(u8"横杆与立杆连接方式");
        cellPara(t.cell(1, 1)).addRun(u8"单扣件");
        cellPara(t.cell(1, 2)).addRun(u8"钢管强度折减系数");
        cellPara(t.cell(1, 3)).addRun(u8"1.00");
    }
    {
        auto& t = doc.addTable(5, 4);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"连墙件截面类型");
        cellPara(t.cell(0, 1)).addRun(u8"钢管");
        cellPara(t.cell(0, 2)).addRun(u8"连墙件计算截面规格");
        cellPara(t.cell(0, 3)).addRun(u8"Φ48.3x3.6mm");
        cellPara(t.cell(1, 0)).addRun(u8"连墙件布置方式");
        cellPara(t.cell(1, 1)).addRun(u8"两步三跨");
        cellPara(t.cell(1, 2)).addRun(u8"连墙件连接方式");
        cellPara(t.cell(1, 3)).addRun(u8"螺栓连接");
        cellPara(t.cell(2, 0)).addRun(u8"连墙件计算长度L0(mm)");
        cellPara(t.cell(2, 1)).addRun(u8"300");
        cellPara(t.cell(2, 2)).addRun(u8"约束脚手架平面外变形轴向力N0(kN)");
        cellPara(t.cell(2, 3)).addRun(u8"2.00");
        cellPara(t.cell(3, 0)).addRun(u8"螺栓个数N(个)");
        cellPara(t.cell(3, 1)).addRun(u8"3");
        cellPara(t.cell(3, 2)).addRun(u8"螺栓直径d(mm)");
        cellPara(t.cell(3, 3)).addRun(u8"18.0");
        cellPara(t.cell(4, 0)).addRun(u8"螺栓抗拉强度设计值(N/mm2)");
        cellPara(t.cell(4, 1)).addRun(u8"170.0");
        t.cell(4, 2).addParagraph("");
        t.cell(4, 3).addParagraph("");
    }
    caption(u8"表2 荷载参数");
    {
        auto& t = doc.addTable(3, 4);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"脚手板自重标准值g2k(kN/m2)");
        cellPara(t.cell(0, 1)).addRun(u8"0.35");
        cellPara(t.cell(0, 2)).addRun(u8"栏杆、挡脚板标准值g3k(kN/m2)");
        cellPara(t.cell(0, 3)).addRun(u8"0.17");
        cellPara(t.cell(1, 0)).addRun(u8"脚手板、栏杆及挡脚板铺设层数n1");
        cellPara(t.cell(1, 1)).addRun(u8"3");
        cellPara(t.cell(1, 2)).addRun(u8"同时施工层数n2");
        cellPara(t.cell(1, 3)).addRun(u8"2");
        cellPara(t.cell(2, 0)).addRun(u8"施工均布荷载标准值qk(kN/m2)");
        cellPara(t.cell(2, 1)).addRun(u8"2.00");
        cellPara(t.cell(2, 2)).addRun(u8"安全设施与安全网重量g4k(kN/m2)");
        cellPara(t.cell(2, 3)).addRun(u8"0.01");
    }
    {
        auto& t = doc.addTable(3, 4);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"省份");
        cellPara(t.cell(0, 1)).addRun(u8"北京");
        cellPara(t.cell(0, 2)).addRun(u8"地区");
        cellPara(t.cell(0, 3)).addRun(u8"北京市");
        cellPara(t.cell(1, 0)).addRun(u8"风荷载体型系数");
        cellPara(t.cell(1, 1)).addRun(u8"0.80");
        cellPara(t.cell(1, 2)).addRun(u8"风压高度变化系数");
        cellPara(t.cell(1, 3)).addRun(u8"0.65");
        cellPara(t.cell(2, 0)).addRun(u8"基本风压(kN/m2)");
        cellPara(t.cell(2, 1)).addRun(u8"0.30");
        t.cell(2, 2).addParagraph("");
        t.cell(2, 3).addParagraph("");
    }
    caption(u8"表3 荷载系数参数");
    {
        auto& t = doc.addTable(5, 3);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        t.cell(0, 0).addParagraph("");
        cellPara(t.cell(0, 1)).addRun(u8"正常使用极限状态");
        cellPara(t.cell(0, 2)).addRun(u8"承载能力极限状态");
        cellPara(t.cell(1, 0)).addRun(u8"可变荷载调整系数γL");
        cellPara(t.cell(1, 1)).addRun(u8"1");
        cellPara(t.cell(1, 2)).addRun(u8"0.90");
        cellPara(t.cell(2, 0)).addRun(u8"可变荷载分项系数γQ");
        cellPara(t.cell(2, 1)).addRun(u8"1");
        cellPara(t.cell(2, 2)).addRun(u8"1.5");
        cellPara(t.cell(3, 0)).addRun(u8"永久荷载分项系数γG");
        cellPara(t.cell(3, 1)).addRun(u8"1");
        cellPara(t.cell(3, 2)).addRun(u8"1.3");
        cellPara(t.cell(4, 0)).addRun(u8"结构重要性系数γ0");
        cellPara(t.cell(4, 1)).addRun(u8"1.00");
        cellPara(t.cell(4, 2)).addRun(u8"1.00");
    }
    caption(u8"表4 基础参数");
    {
        auto& t = doc.addTable(2, 4);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"脚手架基础类型");
        cellPara(t.cell(0, 1)).addRun(u8"地基");
        cellPara(t.cell(0, 2)).addRun(u8"地基承载力特征值fg(kN/m2)");
        cellPara(t.cell(0, 3)).addRun(u8"140.0");
        cellPara(t.cell(1, 0)).addRun(u8"垫板底面积A(m2)");
        cellPara(t.cell(1, 1)).addRun(u8"0.25");
        cellPara(t.cell(1, 2)).addRun(u8"地基承载力调整系数mf");
        cellPara(t.cell(1, 3)).addRun(u8"0.40");
    }
    figure("../examples/res/scaffold/scaffold_plan.emf", 415.5, 91.0);
    caption(u8"图1  平面图");
    figure("../examples/res/scaffold/scaffold_section1.emf", 143.3, 653.5);
    caption(u8"图2  剖面图1");
    figure("../examples/res/scaffold/scaffold_section2.emf", 415.5, 547.4);
    caption(u8"图3  剖面图2");
    // ---- 1.3 横向水平杆验算 ----
    doc.addHeading(u8"1.3 横向水平杆验算", 2);
    body().addRun(u8"横杆按照单跨简支梁进行强度、挠度及变形计算。");
    doc.addHeading(u8"1.3.1 荷载值计算", 3);
    body().addRun(u8"横杆的自重标准值：    ").addEquation(u8"G_{\\text{k}}=0.038").addRun(u8"kN/m");
    body().addRun(u8"脚手板的荷载标准值：").addEquation(u8"G_{\\text{2k}}=\\frac{g_{\\text{2k}}*l_{\\text{a}}}{3}=\\frac{0.35*1.5}{3}=0.175").addRun(u8"kN/m");
    body().addRun(u8"活荷载标准值：            ").addEquation(u8"Q_{\\text{k}}=\\frac{q_{\\text{k}}*l_{\\text{a}}}{3}=\\frac{2*1.5}{3}=1.00").addRun(u8"kN/m");
    body().addRun(u8"荷载的计算值：", RunStyle().bold());
    body().addRun(u8"承载能力极限状态：");
    body().addRun(u8"q ").addEquation(u8"=1.3*\\left(G_{\\text{k}}+G_{\\text{2k}}\\right)+1.5*\\gamma_{\\text{L}}Q_{\\text{k}}");
    body().addRun(u8"    ").addEquation(u8"=1.3*\\left(0.038+0.175\\right)+1.5*0.90*1.000=1.627").addRun(u8"kN/m");
    body().addRun(u8"正常使用极限状态：");
    body().addRun(u8"q' ").addEquation(u8"=G_{\\text{k}}+G_{\\text{2k}}+Q_{\\text{k}}");
    body().addRun(u8"    ").addEquation(u8"=\\left(0.038+0.175\\right)+1.000=1.213").addRun(u8"kN/m");
    body().addRun(u8"横杆计算简图如下：");
    figure("../examples/res/scaffold/boom_span_diagram.emf", 415.5, 63.6);
    caption(u8"图4  横杆计算简图(kN,kN/m)");
    doc.addHeading(u8"1.3.2 抗弯验算", 3);
    body().addRun(u8"横杆弯矩图如下：");
    figure("../examples/res/scaffold/boom_moment.emf", 415.5, 55.9);
    caption(u8"图5  横杆弯矩示意图（kN·m）");
    body().addRun(u8"按单跨简支梁计算得：").addEquation(u8"M=0.165").addRun(u8"kN·m");
    body().addRun(u8"横杆截面抵抗矩：        ").addEquation(u8"W={5.080cm}^{3}");
    body().addRun(u8"横杆材料强度：            ").addEquation(u8"f=205").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    body().addRun(u8"计算得，截面应力  ").addEquation(u8"\\sigma=\\frac{\\gamma_{\\text{0}}M}{W}=\\frac{1*164742}{5080}=32.430").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    body().addRun(u8"取横杆材料强度     ").addEquation(u8"f_{\\text{a}}=1.00*205=205").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    doc.addDisplayEquation(u8"\\sigma\\text{≤}f_{\\text{a}}\\text{，满足要求。}");
    doc.addHeading(u8"1.3.3 挠度验算", 3);
    figure("../examples/res/scaffold/boom_deflection.emf", 415.5, 55.9);
    caption(u8"图6  横杆挠度示意图（mm）");
    body().addRun(u8"按单跨简支梁计算得  ").addEquation(u8"\\nu_{\\text{max}}=0.41mm");
    body().addRun(u8"最大容许挠度  ").addEquation(u8"\\nu_{\\text{lim}}=min").addRun(u8"[").addEquation(u8"l_{\\text{b}}")
          .addRun(u8"/150").addRun(u8",").addEquation(u8"10").addRun(u8"]")
          .addEquation(u8"=6.00mm");
    doc.addDisplayEquation(u8"\\nu_{\\text{max}}\\text{≤}\\nu_{\\text{lim}}\\text{，满足要求。}");
    doc.addHeading(u8"1.3.4 支座反力计算", 3);
    body().addRun(u8"按单跨简支梁计算得：");
    body().addRun(u8"承载能力极限状态：").addEquation(u8"R_{\\text{max}}=0.732kN");
    body().addRun(u8"正常使用极限状态：").addEquation(u8"R_{\\text{max}}'=0.546kN");
    // ---- 1.4 纵向水平杆验算 ----
    doc.addHeading(u8"1.4 纵向水平杆验算", 2);
    body().addRun(u8"纵杆按照三跨连续梁进行强度、挠度及变形计算。");
    doc.addHeading(u8"1.4.1 荷载值计算", 3);
    body().addRun(u8"横杆在上，取纵杆上脚手板的荷载及活荷载标准值均为  ").addEquation(u8"0").addRun(u8"kN/m");
    body().addEquation(u8"G_{\\text{2k}}=0").addRun(u8"kN/m   ").addEquation(u8"Q_{\\text{k}}=0").addRun(u8"kN/m");
    body().addRun(u8"纵杆的自重标准值：").addEquation(u8"G_{\\text{k}}=0.038").addRun(u8"kN/m");
    body().addRun(u8"荷载的计算值：");
    body().addRun(u8"承载能力极限状态：");
    body().addRun(u8"q ").addEquation(u8"=1.3*\\left(G_{\\text{k}}+G_{\\text{2k}}\\right)+1.5*\\gamma_{\\text{L}}Q_{\\text{k}}=1.3*0.038=0.050").addRun(u8"kN/m");
    body().addRun(u8"正常使用极限状态：");
    body().addRun(u8"q' ").addEquation(u8"=G_{\\text{k}}+G_{\\text{2k}}+Q_{\\text{k}}=0.04").addRun(u8"kN/m");
    body().addRun(u8"横杆传来的集中荷载的计算值：");
    body().addRun(u8"承载能力极限状态：").addEquation(u8"F=0.732").addRun(u8"kN");
    body().addRun(u8"正常使用极限状态：").addEquation(u8"F'=0.546").addRun(u8"kN");
    body().addRun(u8"纵杆计算简图如下：");
    figure("../examples/res/scaffold/ledger_span_diagram.emf", 415.5, 95.0);
    caption(u8"图7  纵杆计算简图(kN,kN/m)");
    doc.addHeading(u8"1.4.2 抗弯验算", 3);
    body().addRun(u8"纵杆弯矩图如下：");
    figure("../examples/res/scaffold/ledger_moment.emf", 415.5, 86.7);
    caption(u8"图8  纵杆弯矩示意图（kN·m）");
    body().addRun(u8"按三跨连续梁计算得：").addEquation(u8"M=0.30").addRun(u8"kN·m");
    body().addRun(u8"纵杆截面抵抗矩：        ").addEquation(u8"W={5.080cm}^{3}");
    body().addRun(u8"纵杆材料强度：            ").addEquation(u8"f=205").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    body().addRun(u8"计算得，截面应力  ").addEquation(u8"\\sigma=\\frac{\\gamma_{\\text{0}}M}{W}=\\frac{1*303059}{5080}=59.657").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    body().addRun(u8"取纵杆材料强度     ").addEquation(u8"f_{\\text{a}}=1.00*205=205").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    doc.addDisplayEquation(u8"\\sigma\\text{≤}f_{\\text{a}}\\text{，满足要求。}");
    doc.addHeading(u8"1.4.3 挠度验算", 3);
    figure("../examples/res/scaffold/ledger_deflection.emf", 415.5, 57.0);
    caption(u8"图9  纵杆挠度示意图（mm）");
    body().addRun(u8"按三跨连续梁计算得  ").addEquation(u8"\\nu_{\\text{max}}=1.456mm");
    body().addRun(u8"最大容许挠度  ").addEquation(u8"\\nu_{\\text{lim}}=min").addRun(u8"[").addEquation(u8"l_{\\text{a}}")
          .addRun(u8"/150").addRun(u8",").addEquation(u8"10").addRun(u8"]")
          .addEquation(u8"=10.00mm");
    doc.addDisplayEquation(u8"\\nu_{\\text{max}}\\text{≤}\\nu_{\\text{lim}}\\text{，满足要求。}");
    doc.addHeading(u8"1.4.4 支座反力计算", 3);
    body().addRun(u8"计算得：");
    body().addRun(u8"承载能力极限状态：").addEquation(u8"R_{\\text{max}}=2.474kN");
    body().addRun(u8"正常使用极限状态：").addEquation(u8"R_{\\text{max}}'=1.846kN");
    // ---- 1.5 扣件抗滑承载力验算 ----
    doc.addHeading(u8"1.5 扣件抗滑承载力验算", 2);
    body().addRun(u8"取扣件的作用力：").addEquation(u8"R=1.00*2.47=2.47kN");
    body().addRun(u8"取扣件的抗滑承载力：").addEquation(u8"R_{\\text{C}}=1.00*8=8.00kN");
    body().addRun(u8"计算得，").addEquation(u8"\\gamma_{\\text{0}}*R\\leq{}R_{\\text{C}}");
    body().addRun(u8"满足要求。", RunStyle().bold().color("00B050"));
    // ---- 1.6 脚手架荷载标准值 ----
    doc.addHeading(u8"1.6 脚手架荷载标准值", 2);
    body().addRun(u8"1)立杆承受的结构自重标准值").addEquation(u8"N_{\\text{G1k}}");
    body().addRun(u8"结构自重作用于内外立杆。");
    body().addRun(u8"水平杆自重荷载标准值：").addEquation(u8"g_{\\text{1k1}}=0.038").addRun(u8"kN/").addEquation(u8"m");
    body().addRun(u8"立杆自重荷载标准值：    ").addEquation(u8"g_{\\text{1k2}}=0.038").addRun(u8"kN/").addEquation(u8"m");
    body().addRun(u8"外立杆：");
    body().addEquation(u8"N_{\\text{G1k}}=g_{\\text{1k2}}*H+g_{\\text{1k1}}*\\left(2*l_{\\text{a}}+l_{\\text{b}}\\right)*H").addRun(u8"/").addEquation(u8"2").addRun(u8"/")
          .addEquation(u8"h+g_{\\text{1k1}}*l_{\\text{b}}*n*n_{\\text{1}}").addRun(u8"/").addEquation(u8"2");
    body().addRun(u8"        ").addEquation(u8"=0.038*12+0.038*\\left(2*1.5+0.9\\right)*12").addRun(u8"/").addEquation(u8"2")
          .addRun(u8"/").addEquation(u8"1.5+0.038*0.9*2*3").addRun(u8"/").addEquation(u8"2=1.163kN");
    body().addRun(u8"2)脚手板的自重标准值").addEquation(u8"N_{\\text{G2k}}");
    body().addRun(u8"脚手板荷载作用于内外立杆。");
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{G2k}}=g_{\\text{2k}}*n_{\\text{1}}*l_{\\text{a}}*l_{\\text{b}}").addRun(u8"/2").addEquation(u8"=0.350*3*1.5*0.9")
          .addRun(u8"/2").addEquation(u8"=0.709kN");
    body().addRun(u8"3)栏杆与挡脚板自重标准值").addEquation(u8"N_{\\text{G3k}}");
    body().addRun(u8"栏杆与挡脚板荷载仅作用于外立杆。");
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{G3k}}=g_{\\text{3k}}*n_{\\text{1}}*l_{\\text{a}}=0.170*3*1.5=0.765kN");
    body().addRun(u8"4)围护材料的自重标准值").addEquation(u8"N_{\\text{G4k}}");
    body().addRun(u8"围护材料荷载仅作用于外立杆。");
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{G4k}}=g_{\\text{4k}}*l_{\\text{a}}*H=0.010*1.5*12=0.180kN");
    body().addRun(u8"5)架体及构配件自重标准值").addEquation(u8"N_{\\text{Gk}}");
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{Gk1}}=N_{\\text{G1k}}+N_{\\text{G2k}}+N_{\\text{G3k}}+N_{\\text{G4k}}");
    body().addRun(u8"                         ").addEquation(u8"=1.163+0.709+0.765+0.180=2.817kN");
    body().addRun(u8"内立杆：").addEquation(u8"N_{\\text{Gk2}}=N_{\\text{G1k}}+N_{\\text{G2k}}=1.163+0.709=1.872kN");
    doc.addHeading(u8"1.6.1 立杆施工活荷载计算", 3);
    body().addRun(u8"施工活荷载作用于内外立杆。");
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{Qk}}=q_{\\text{k}}*n_{\\text{2}}*l_{\\text{a}}*l_{\\text{b}}").addRun(u8"/2").addEquation(u8"=2.000*2*1.5*0.9")
          .addRun(u8"/2").addEquation(u8"=2.700kN");
    doc.addHeading(u8"1.6.2 荷载组合", 3);
    body().addRun(u8"不组合风荷载作用下立杆轴向力", RunStyle().bold());
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{1}}=1.3*N_{\\text{Gk1}}+1.5*\\gamma_{\\text{L}}*N_{\\text{Qk}}");
    body().addRun(u8"                      ").addEquation(u8"=1.3*2.817+1.5*0.9*2.700=7.307kN");
    body().addRun(u8"内立杆：").addEquation(u8"N_{\\text{2}}=1.3*N_{\\text{Gk2}}+1.5*\\gamma_{\\text{L}}*N_{\\text{Qk}}");
    body().addRun(u8"                      ").addEquation(u8"=1.3*1.872+1.5*0.9*2.700=6.078kN");
    body().addRun(u8"组合风荷载作用下立杆轴向力", RunStyle().bold());
    body().addRun(u8"外立杆：").addEquation(u8"N_{\\text{1}}=1.3*N_{\\text{Gk1}}+0.9*1.5*\\gamma_{\\text{L}}*N_{\\text{Qk}}");
    body().addRun(u8"                         ").addEquation(u8"=1.3*2.817+0.9*1.5*0.9*2.700=6.942kN");
    body().addRun(u8"内立杆：").addEquation(u8"N_{\\text{2}}=1.3*N_{\\text{Gk2}}+0.9*\\gamma_{\\text{L}}*1.5*N_{\\text{Qk}}");
    body().addRun(u8"                         ").addEquation(u8"=1.3*1.872+0.9*1.5*0.9*2.700=5.714kN");
    doc.addHeading(u8"1.6.3 风荷载引起的立杆段弯矩", 3);
    body().addRun(u8"风荷载标准值：").addEquation(u8"w_{\\text{k}}=\\mu_{\\text{s}}\\mu_{\\text{z}}w_{\\text{0}}=0.800*0.650*0.300=0.156kN").addRun(u8"/m^2");
    body().addRun(u8"风荷载引起的立杆段弯矩：");
    body().addEquation(u8"M_{\\text{w}}=0.9*1.5*w_{\\text{k}}l_{\\text{a}}h^{2}").addRun(u8"/10").addEquation(u8"=0.9*1.5*0.156*1.5*{1.5}^{2}").addRun(u8"/10")
          .addEquation(u8"=0.071kN").addRun(u8"·m");
    // ---- 1.7 立杆稳定性计算 ----
    doc.addHeading(u8"1.7 立杆稳定性计算", 2);
    doc.addHeading(u8"1.7.1 立杆长细比验算", 3);
    body().addRun(u8"长细比验算时取 ").addEquation(u8"K=1.0");
    body().addRun(u8"立杆计算长度   ").addEquation(u8"l_{\\text{0}}=K\\mu{}h=1.0*1.47*1.5=2205mm");
    body().addRun(u8"立杆回转半径    ").addEquation(u8"i=15.8mm");
    body().addRun(u8"长细比  ").addEquation(u8"\\lambda=l_{\\text{0}}").addRun(u8"/").addEquation(u8"i=2205")
          .addRun(u8"/").addEquation(u8"15.79=139.66");
    doc.addDisplayEquation(u8"\\text{}\\lambda\\text{ ≤ }210\\text{，满足要求。}");
    doc.addHeading(u8"1.7.2 轴心受压构件的稳定系数计算", 3);
    body().addRun(u8"稳定系数计算时取 ").addEquation(u8"K=1.155");
    body().addRun(u8"立杆计算长度   ").addEquation(u8"l_{\\text{0}}=K\\mu{}h=1.155*1.47*1.5=2546.8mm");
    body().addRun(u8"长细比  ").addEquation(u8"\\lambda=l_{\\text{0}}").addRun(u8"/").addEquation(u8"i=2547")
          .addRun(u8"/").addEquation(u8"15.79=161.30");
    body().addRun(u8"查《规范》表A得，").addEquation(u8"\\phi=0.271");
    doc.addHeading(u8"1.7.3 立杆稳定性验算", 3);
    body().addRun(u8"不组合风荷载作用", RunStyle().bold());
    body().addRun(u8"外立杆的轴心压力设计值 ").addEquation(u8"N=7.307kN");
    body().addRun(u8" ").addEquation(u8"\\sigma=\\gamma_{\\text{0}}N").addRun(u8"/").addEquation(u8"\\phi{}A=1.0*7307")
          .addRun(u8"/").addRun(u8"(").addEquation(u8"0.271*489").addRun(u8")")
          .addEquation(u8"=55.14").addRun(u8"N/m").addEquation(u8"m^{2}");
    doc.addDisplayEquation(u8"\\sigma\\text{≤}f_{\\text{a}}\\text{，满足要求。}");
    body().addRun(u8"组合风荷载作用", RunStyle().bold());
    body().addRun(u8"外立杆的轴心压力设计值 ").addEquation(u8"N=6.942kN");
    body().addRun(u8" ").addEquation(u8"\\sigma=\\gamma_{\\text{0}}\\left(N\\text{/}\\phi{}A+M_{\\text{w}}\\text{/}W\\right)");
    body().addRun(u8"     ").addEquation(u8"=1.0*\\left(6942\\text{/}\\text{(}0.271*489\\text{)}+71078\\text{/}5080\\right)=66.38").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    doc.addDisplayEquation(u8"\\sigma\\text{≤}f_{\\text{a}}\\text{，满足要求。}");
    // ---- 1.8 最大搭设高度计算 ----
    doc.addHeading(u8"1.8 最大搭设高度计算", 2);
    body().addRun(u8"不组合风荷载作用");
    body().addRun(u8"构配件产生的轴向力标准值 ").addEquation(u8"G_{\\text{2k}}=1.654kN");
    body().addRun(u8"施工荷载产生的轴向力标准值 ").addEquation(u8"N_{\\text{Qk}}=2.700kN");
    body().addRun(u8"最大搭设高度 ").addEquation(u8"H_{\\text{max}}=\\frac{\\phi{}Af-{1.3G}_{\\text{2k}}-{1.5N}_{\\text{Qk}}}{1.3g_{\\text{k}}}");
    doc.addDisplayEquation(u8"H_{\\text{max}}=\\frac{0.27*0.49*205-1.3*1.65-1.5*2.70}{1.3*0.0969}=166.396m");
    body().addRun(u8"组合风荷载作用");
    body().addRun(u8"风荷载产生的弯矩标准值 ").addEquation(u8"M_{\\text{wk}}=0.053kN·m");
    body().addRun(u8"最大搭设高度 ").addEquation(u8"H_{\\text{max}}=\\frac{\\phi{}Af-{1.3G}_{\\text{2k}}-0.9*{1.5N}_{\\text{Qk}}-\\frac{0.9*1.5\\phi{}AM_{\\text{wk}}}{W}}{1.3g_{\\text{k}}}");
    doc.addDisplayEquation(u8"H_{\\text{max}}=\\frac{0.27*0.49*205-1.3*1.65-0.9*1.5*2.70-\\frac{0.9*1.5*0.27*0.49*52650}{5080}}{1.3*0.0969}=154.895m");
    body().addRun(u8"取 ").addEquation(u8"H_{\\text{max}}=154.895m");
    body().addRun(u8"最大搭设高度满足要求。", RunStyle().bold().color("00B050"));
    // ---- 1.9 连墙件承载力验算 ----
    doc.addHeading(u8"1.9 连墙件承载力验算", 2);
    doc.addHeading(u8"1.9.1 连墙件轴力设计值计算", 3);
    body().addRun(u8"单个连墙件覆盖脚手架迎风面面积");
    body().addRun(u8" ").addEquation(u8"A_{\\text{w}}=2").addRun(u8"*").addEquation(u8"3")
          .addRun(u8"*").addEquation(u8"l_{\\text{a}}h=2*3*1.5*1.5={13.50m}^{2}");
    body().addRun(u8"风荷载产生的连墙件轴力");
    body().addRun(u8" ").addEquation(u8"N_{\\text{lw}}={1.5w}_{\\text{k}}A_{\\text{w}}=1.5*0.156*13.50=3.16kN");
    body().addRun(u8"连墙件的轴力设计值  ").addEquation(u8"N_{\\text{l}}=N_{\\text{lw}}+N_{\\text{0}}=3.16+2.00=5.16kN");
    doc.addHeading(u8"1.9.2 强度及稳定验算", 3);
    body().addRun(u8"截面控制应力                 ").addEquation(u8"0.85f=0.85*205=174.25").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    body().addRun(u8"连墙件的截面面积       ").addEquation(u8"A={506.00mm}^{2}");
    body().addRun(u8"连墙件的净截面面积  ").addEquation(u8"A_{\\text{c}}=0.85A=0.85*506.00={430.10mm}^{2}");
    body().addRun(u8"强度验算", RunStyle().bold());
    body().addEquation(u8"N_{\\text{l}}").addRun(u8"/").addEquation(u8"A_{\\text{c}}=5159").addRun(u8"/")
          .addEquation(u8"430.10=11.99").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    doc.addDisplayEquation(u8"N_{\\text{l}}\\text{/}A_{\\text{c}}\\text{≤}0.85f\\text{满足要求。}");
    body().addRun(u8"稳定性验算", RunStyle().bold());
    body().addRun(u8"连墙件的截面回转半径   ").addEquation(u8"i=15.85mm");
    body().addRun(u8"连墙件的截面长细比        ").addEquation(u8"\\lambda=L_{\\text{0}}").addRun(u8"/").addEquation(u8"i=18.93");
    body().addRun(u8"查表得 ").addEquation(u8"\\phi=0.949");
    body().addEquation(u8"N_{\\text{l}}").addRun(u8"/").addEquation(u8"\\phi{}A=5159").addRun(u8"/")
          .addEquation(u8"\\left(0.949*506.00\\right)=10.74").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    doc.addDisplayEquation(u8"N_{\\text{l}}\\text{/}\\phi{}A\\text{≤}0.85f\\text{满足要求。}");
    doc.addHeading(u8"1.9.3 连接强度验算", 3);
    body().addRun(u8"螺栓验算  ").addEquation(u8"f_{\\text{t}}=\\frac{N_{\\text{l}}}{\\frac{\\pi*d^{2}*N}{4}}=\\frac{5159}{\\frac{3.14*{18.0}^{2}*4}{4}}=5.068").addRun(u8"N/").addEquation(u8"{mm}^{2}");
    doc.addDisplayEquation(u8"f_{\\text{t}}\\text{≤}F_{\\text{t}}=170.0\\text{N/}mm^{2}\\text{满足要求。}");
    body().addRun(u8"连墙件扣件抗滑移承载力  ").addEquation(u8"R_{\\text{c}}=1.00*8.00=8.00kN");
    doc.addDisplayEquation(u8"N_{\\text{l}}\\text{≤}R_{\\text{c}}\\text{满足要求。}");
    // ---- 1.10 立杆地基承载力验算 ----
    doc.addHeading(u8"1.10 立杆地基承载力验算", 2);
    body().addRun(u8"根据立杆荷载标准组合");
    body().addRun(u8"立杆基础底面的压力          ").addEquation(u8"N=N_{\\text{Gk1}}+N_{\\text{Qk}}=2.82+2.70=5.52kN");
    body().addRun(u8"立杆基础底面的平均压力  ").addEquation(u8"p=N_{\\text{d}}").addRun(u8"/").addEquation(u8"A_{\\text{d}}=5.52")
          .addRun(u8"/").addEquation(u8"0.25=22.07kPa");
    body().addRun(u8"地基承载力设计值              ").addEquation(u8"f_{\\text{a}}=m_{\\text{f}}f_{\\text{g}}=0.40*140=56.00kPa");
    doc.addDisplayEquation(u8"\\text{计算得 }p\\leq{}f_{\\text{a}}\\text{，满足要求。}");
    // ---- 1.11 计算结果汇总 ----
    doc.addHeading(u8"1.11 计算结果汇总", 2);
    caption(u8"表5 计算结果总览表");
    {
        auto& t = doc.addTable(14, 5);
        t.setBorderStyle(TableStyle::Grid).setAutoFit(false);
        for (int r = 0; r < t.rows(); ++r) t.setRowHeight(r, Length::twips(397));
        cellPara(t.cell(0, 0)).addRun(u8"验算项目");
        cellPara(t.cell(0, 1)).addRun(u8"验算内容");
        cellPara(t.cell(0, 2)).addRun(u8"计算值");
        cellPara(t.cell(0, 3)).addRun(u8"允许值");
        cellPara(t.cell(0, 4)).addRun(u8"结论");
        cellPara(t.cell(1, 0)).addRun(u8"横向水平杆");
        cellPara(t.cell(1, 1)).addRun(u8"抗弯");
        cellMath(t.cell(1, 2), u8"\\sigma=32.43\\text{N/}mm^{2}");
        cellMath(t.cell(1, 3), u8"\\text{[}f\\text{]}=205\\text{N/}mm^{2}");
        cellPara(t.cell(1, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        t.cell(2, 0).addParagraph("");
        cellPara(t.cell(2, 1)).addRun(u8"挠度");
        cellMath(t.cell(2, 2), u8"\\nu=0.41mm");
        cellMath(t.cell(2, 3), u8"\\text{[}\\nu\\text{]}=6.00mm");
        cellPara(t.cell(2, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        cellPara(t.cell(3, 0)).addRun(u8"纵向水平杆");
        cellPara(t.cell(3, 1)).addRun(u8"抗弯");
        cellMath(t.cell(3, 2), u8"\\sigma=59.66\\text{N/}mm^{2}");
        cellMath(t.cell(3, 3), u8"\\text{[}f\\text{]}=205\\text{N/}mm^{2}");
        cellPara(t.cell(3, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        t.cell(4, 0).addParagraph("");
        cellPara(t.cell(4, 1)).addRun(u8"挠度");
        cellMath(t.cell(4, 2), u8"\\nu=1.46mm");
        cellMath(t.cell(4, 3), u8"\\text{[}\\nu\\text{]}=10.00mm");
        cellPara(t.cell(4, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        cellPara(t.cell(5, 0)).addRun(u8"扣件抗滑移");
        cellPara(t.cell(5, 1)).addRun(u8"扣件抗滑移");
        cellMath(t.cell(5, 2), u8"R_{\\max}=2.47kN");
        cellMath(t.cell(5, 3), u8"R_{C}=8.00kN");
        cellPara(t.cell(5, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        cellPara(t.cell(6, 0)).addRun(u8"立杆");
        cellPara(t.cell(6, 1)).addRun(u8"长细比");
        cellMath(t.cell(6, 2), u8"\\lambda=140");
        cellMath(t.cell(6, 3), u8"\\text{[}\\lambda\\text{]}=210");
        cellPara(t.cell(6, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        t.cell(7, 0).addParagraph("");
        cellPara(t.cell(7, 1)).addRun(u8"稳定性");
        cellMath(t.cell(7, 2), u8"\\sigma=66.38\\text{N/}mm^{2}");
        cellMath(t.cell(7, 3), u8"\\text{[}f\\text{]}=205\\text{N/}mm^{2}");
        cellPara(t.cell(7, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        cellPara(t.cell(8, 0)).addRun(u8"架体");
        cellPara(t.cell(8, 1)).addRun(u8"高度");
        cellMath(t.cell(8, 2), u8"H=12.00m");
        cellMath(t.cell(8, 3), u8"H_{\\max}=154.90m");
        cellPara(t.cell(8, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        cellPara(t.cell(9, 0)).addRun(u8"连墙件");
        cellPara(t.cell(9, 1)).addRun(u8"强度验算");
        cellMath(t.cell(9, 2), u8"N_{l}/A_{c}=11.99\\text{N/}mm^{2}");
        cellMath(t.cell(9, 3), u8"\\text{[}f\\text{]}=174.25\\text{N/}mm^{2}");
        cellPara(t.cell(9, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        t.cell(10, 0).addParagraph("");
        cellPara(t.cell(10, 1)).addRun(u8"稳定性验算");
        cellMath(t.cell(10, 2), u8"N_{l}/\\phi{}A=10.74\\text{N/}mm^{2}");
        cellMath(t.cell(10, 3), u8"\\text{[}f\\text{]}=174.25\\text{N/}mm^{2}");
        cellPara(t.cell(10, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        t.cell(11, 0).addParagraph("");
        cellPara(t.cell(11, 1)).addRun(u8"螺栓连接验算");
        cellMath(t.cell(11, 2), u8"f_{t}=5.068\\text{N/}mm^{2}");
        cellMath(t.cell(11, 3), u8"F_{t}=170.0\\text{N/}mm^{2}");
        cellPara(t.cell(11, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        t.cell(12, 0).addParagraph("");
        cellPara(t.cell(12, 1)).addRun(u8"扣件抗滑移");
        cellMath(t.cell(12, 2), u8"N_{l}=5.16kN");
        cellMath(t.cell(12, 3), u8"R_{c}=8.00kN");
        cellPara(t.cell(12, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
        cellPara(t.cell(13, 0)).addRun(u8"地基基础");
        cellPara(t.cell(13, 1)).addRun(u8"承载力");
        cellMath(t.cell(13, 2), u8"p=22.07kPa");
        cellMath(t.cell(13, 3), u8"\\gamma_{u}f_{a}=56.00kPa");
        cellPara(t.cell(13, 4)).addRun(u8"满足要求!", RunStyle().color("00B050"));
    }

    // ---- Save to the repository root (run from the build directory) ----
    const std::string outputPath = "../扣件式落地脚手架_1.docx";
    if (doc.save(outputPath)) {
        std::cout << "Document saved to: " << outputPath << std::endl;
    } else {
        std::cerr << "Failed to save document!" << std::endl;
        return 1;
    }
    return 0;
}
