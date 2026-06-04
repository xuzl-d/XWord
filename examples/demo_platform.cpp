#include "xword/xword.hpp"
#include <iostream>
#include <string>

int main() {
    using namespace xword;

    Document doc;

    // ---- Page settings ----
    doc.setPage(Page()
        .setSize(PageSize::A4)
        .setOrientation(Orientation::Portrait)
        .setMargins(2.54, 2.54, 3.0, 3.0));

    // ---- Body text defaults ----
    // Normal style: 宋体 五号(10.5pt) — applies to tables cells
    // Body run override: 12pt — applied to body paragraphs automatically
    doc.setBodyFont(u8"宋体", "Times New Roman", u8"宋体");
    doc.setBodyFontSize(10.5);       // 五号 = 10.5pt for Normal style (table cells)
    doc.setBodyRunStyle(RunStyle().fontSize(12)); // body paragraphs default to 12pt
    doc.setBodyLineSpacing(1.25);    // 1.25× line spacing
    doc.setDisplayEquationStyle(RunStyle().fontSize(12));

    // ---- Heading style customization (no color — match reference black headings) ----
    // H1: 宋体 15pt, space before 340 twips (17pt), after 330 twips (16.5pt)
    doc.setHeadingStyle(1, HeadingStyle()
        .setFont(u8"宋体").setFontSize(15).setBold(true)
        .setLineSpacing(1.5)
        .setSpaceBefore(17).setSpaceAfter(16.5)
        .setAlignment(Alignment::Center));
    // H2: 宋体 14pt, space before/after 260 twips (13pt)
    doc.setHeadingStyle(2, HeadingStyle()
        .setFont(u8"宋体").setFontSize(14).setBold(true)
        .setLineSpacing(1.3)
        .setSpaceBefore(13).setSpaceAfter(13));
    // H3: 宋体 12pt, space before/after 260 twips (13pt)
    doc.setHeadingStyle(3, HeadingStyle()
        .setFont(u8"宋体").setFontSize(12).setBold(true)
        .setSpaceBefore(13).setSpaceAfter(13));

    // ---- Enable heading auto-numbering with chapter format ----
    doc.enableHeadingNumbering();
    doc.setHeadingNumFormat(HeadingNumFormat::Chapter);

    // ---- Caption numbering: sequential (表1, 表2, …, 图1, 图2, …) ----
    doc.enableImageNumbering(u8"图", CaptionNumStyle::Sequential);
    doc.enableTableNumbering(u8"表", CaptionNumStyle::Sequential);

    // ---- Set default indent for body paragraphs (2 chars at 12pt = 480 twips) ----
    doc.setDefaultParagraphIndent(2, 12);

    // ============================================================
    // 第1章 悬挑钢平台计算书-悬挑钢平台_1
    // ============================================================
    doc.addHeading("悬挑钢平台计算书-悬挑钢平台_1", 1);

    // ---- 1.1 计算依据 ----
    doc.addHeading("计算依据", 2);

    doc.addParagraph("《工程结构通用规范》GB55001-2021").setSpacingAfter(0);
    doc.addParagraph("《施工脚手架通用规范》GB55023-2022").setSpacingAfter(0);
    doc.addParagraph("《建筑施工扣件式钢管脚手架安全技术规范》JGJ130-2011").setSpacingAfter(0);
    doc.addParagraph("《建筑结构荷载规范》GB50009-2012").setSpacingAfter(0);
    doc.addParagraph("《钢结构通用规范》GB55006-2021").setSpacingAfter(0);
    doc.addParagraph("《钢结构设计标准》GB50017-2017").setSpacingAfter(0);
    doc.addParagraph("《建筑结构可靠性设计统一标准》GB50068-2018");

    // ---- 表1 架体布置参数 ----
    {
        auto& t = doc.addTable(5, 4);
        t.setBorderStyle(TableStyle::Grid).setCaption("架体布置参数（单位：mm）");

        t.cell(0, 0).addParagraph("悬挑钢梁类型");   t.cell(0, 1).addParagraph("热轧普通工字钢");
        t.cell(0, 2).addParagraph("悬挑钢梁截面");   t.cell(0, 3).addParagraph("I18");

        t.cell(1, 0).addParagraph("悬挑钢梁材质");   t.cell(1, 1).addParagraph("Q235");
        t.cell(1, 2).addParagraph("悬挑长度");       t.cell(1, 3).addParagraph("2600");

        t.cell(2, 0).addParagraph("支撑类型");       t.cell(2, 1).addParagraph("上拉杆");
        t.cell(2, 2).addParagraph("支撑数量");       t.cell(2, 3).addParagraph("1");

        t.cell(3, 0).addParagraph("上拉杆直径");     t.cell(3, 1).addParagraph("18mm");
        t.cell(3, 2).addParagraph("上拉杆抗拉强度"); t.cell(3, 3).addParagraph("205MPa");

        t.cell(4, 0).addParagraph("花篮螺栓直径");   t.cell(4, 1).addParagraph("18mm");
        t.cell(4, 2).addParagraph("与建筑连接方式"); t.cell(4, 3).addParagraph("螺栓");
    }

    // ---- 表2 架体其他参数 ----
    {
        auto& t = doc.addTable(4, 4);
        t.setBorderStyle(TableStyle::Grid).setCaption("架体其他参数（单位：mm）");

        t.cell(0, 0).addParagraph("外锚固点距建筑边缘距离");
        t.cell(0, 1).addParagraph("1400");
        t.cell(0, 2).addParagraph("内外锚固点间距");
        t.cell(0, 3).addParagraph("150");

        t.cell(1, 0).addParagraph("锚固方式");       t.cell(1, 1).addParagraph("压环钢筋");
        t.cell(1, 2).addParagraph("锚固件个数");     t.cell(1, 3).addParagraph("2");

        t.cell(2, 0).addParagraph("锚固件抗拉强度"); t.cell(2, 1).addParagraph("65");
        t.cell(2, 2).addParagraph("锚固件直径");     t.cell(2, 3).addParagraph("16");

        t.cell(3, 0).addParagraph("混凝土强度等级"); t.cell(3, 1).addParagraph("C30");
        t.cell(3, 2).addParagraph("");
        t.cell(3, 3).addParagraph("");
    }

    // ---- 图1 悬挑支撑平面图 ----
    doc.addImage("../examples/res/image1.emf")
       .setCaption("悬挑支撑平面图")
       .setAlignment(Alignment::Center);

    // ---- 图2 悬挑支撑剖面图 ----
    doc.addImage("../examples/res/image2.emf")
        .setCaption("悬挑支撑剖面图")
        .setAlignment(Alignment::Center);

    // ---- 1.2 悬挑支撑梁验算 ----
    doc.addHeading("悬挑支撑梁验算", 2);

    doc.addParagraph()
        .addRun("经计算，各悬挑支撑梁弯矩、变形计算结果如下：", RunStyle().color("0000FF"));

    // ---- 表3 各主梁计算结果 ----
    {
        auto& t = doc.addTable(5, 3);
        t.setBorderStyle(TableStyle::Grid).setCaption("各主梁计算结果");

        t.cell(0, 0).addParagraph("主梁位置(mm)");
        t.cell(0, 1).addParagraph("最大弯矩(kN·m)");
        t.cell(0, 2).addParagraph("最大变形(mm)");

        t.cell(1, 0).addParagraph("x = 0");     t.cell(1, 1).addParagraph("9.89"); t.cell(1, 2).addParagraph("1.8793");
        t.cell(2, 0).addParagraph("x = 1200");  t.cell(2, 1).addParagraph("9.89"); t.cell(2, 2).addParagraph("1.8793");
        t.cell(3, 0).addParagraph("x = 2400");  t.cell(3, 1).addParagraph("9.89"); t.cell(3, 2).addParagraph("1.8793");
        t.cell(4, 0).addParagraph("x = 3600");  t.cell(4, 1).addParagraph("9.89"); t.cell(4, 2).addParagraph("1.8793");
    }

    // ---- 1.2.1 抗弯验算 ----
    doc.addHeading("抗弯验算", 3);

    doc.addParagraph().addRun("选择x=0mm弯矩最大主梁进行验算", RunStyle().bold());
    doc.addParagraph()
        .addRun("悬挑主梁荷载设计值计算简图如下：", RunStyle().color("0000FF"));

    // 图3
    doc.addImage("../examples/res/image3.emf")
       .setCaption("计算简图(kN,kN/m)")
       .setAlignment(Alignment::Center);

    doc.addParagraph()
        .addRun("悬挑主梁弯矩图如下：", RunStyle().color("0000FF"));

    // 图4
    doc.addImage("../examples/res/image4.emf")
        .setCaption("弯矩示意图（kN·m）")
        .setAlignment(Alignment::Center);

    doc.addParagraph()
        .addRun("最大正弯矩")
        .addEquation("M=1.258\\ \\text{kN}\\cdot\\text{m}");
    doc.addParagraph()
        .addRun("最大负弯矩")
        .addEquation("M_{\\max}^{-}=-9.891\\ \\text{kN}\\cdot\\text{m}");
    doc.addParagraph()
        .addRun("计算得，截面应力")
        .addEquation("\\sigma=\\frac{M_{\\max}}{W}=\\frac{9890887}{185000}=53.464\\ \\text{MPa}");
    doc.addEquation("\\sigma=53.46\\ \\text{MPa}\\le[f]=205\\ \\text{MPa}");
    doc.addEquation("\\text{满足要求!}");

    doc.addParagraph("当悬挑主梁支撑形式为拉杆时，主梁为压弯杆件");
    doc.addDisplayEquation("N=\\frac{R\\times b_1}{h_1}=\\frac{30.15\\times1500}{3000}=15.07\\ \\text{kN}");
    doc.addDisplayEquation("\\sigma_p=\\frac{M_{\\max}}{\\gamma W}+\\frac{N}{A}= \\frac{9890887}{1.05\\times185000}+\\frac{15075}{3074}=55.822\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma_p=55.82\\ \\text{MPa}\\le[f]=205\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.2.2 抗剪验算 ----
    doc.addHeading("抗剪验算", 3);

    doc.addParagraph()
        .addRun("悬挑主梁剪力图如下：", RunStyle().color("0000FF"));

    // 图5
    doc.addImage("../examples/res/image5.emf")
       .setCaption("剪力示意图（kN）")
       .setAlignment(Alignment::Center);

    doc.addDisplayEquation("V_{\\max}=15.96\\ \\text{kN}");
    doc.addDisplayEquation("\\tau=\\frac{VS}{It_w}= \\frac{15957\\times106515}{16600000\\times6.50}=15.752\\ \\text{MPa}");
    doc.addDisplayEquation("\\tau=15.75\\ \\text{MPa}\\le[\\tau]=120\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.2.3 整体稳定验算 ----
    doc.addHeading("整体稳定验算", 3);

    doc.addParagraph().addRun("按《钢标》GB50017-2017,C.0.1计算得")
    .addEquation("\\phi_b=0.9023");
    doc.addDisplayEquation("\\sigma_s=\\frac{M_{\\max}}{\\varphi_b W}= \\frac{9890887}{0.902\\times185000}=59.251\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma_s=59.25\\ \\text{MPa}\\le[f]=205\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.2.4 挠度验算 ----
    doc.addHeading("挠度验算", 3);

    doc.addParagraph()
        .addRun("悬挑主梁荷载标准值计算简图如下：", RunStyle().color("0000FF"));

    // 图6
    doc.addImage("../examples/res/image6.emf")
       .setCaption("计算简图(kN,kN/m)")
       .setAlignment(Alignment::Center);

    doc.addParagraph()
        .addRun("悬挑主梁挠度变形图如下：", RunStyle().color("0000FF"));

    // 图7
    doc.addImage("../examples/res/image7.emf")
       .setCaption("挠度示意图（mm）")
       .setAlignment(Alignment::Center);

    doc.addDisplayEquation("\\nu_{\\max}=1.8793\\ \\text{mm}");
    doc.addDisplayEquation("\\upsilon_{\\max}=1.8793\\ \\text{mm}");
    doc.addDisplayEquation("υ_{\\mathrm{max}}=1.8793\\mathrm{mm}≤\\mathrm{[} υ\\mathrm{]}=2500×1\\mathrm{/400}=6.25\\mathrm{mm}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.2.5 支座反力 ----
    doc.addHeading("支座反力", 3);

    doc.addParagraph()
        .addRun("各悬挑主梁的支座反力将作为荷载用于连接节点验算", RunStyle().color("0000FF"));

    // 表4
    {
        auto& t = doc.addTable(4, 5);
        t.setBorderStyle(TableStyle::Grid).setCaption("各悬挑主梁支座反力设计值");

        t.cell(0, 0).addParagraph("位置(mm)");
        t.cell(0, 1).addParagraph("x=0");    t.cell(0, 2).addParagraph("x=1200");
        t.cell(0, 3).addParagraph("x=2400"); t.cell(0, 4).addParagraph("x=3600");

        t.cell(1, 0).addParagraph("y=-1550");
        t.cell(1, 1).addParagraph("1.12kN"); t.cell(1, 2).addParagraph("1.12kN");
        t.cell(1, 3).addParagraph("1.12kN"); t.cell(1, 4).addParagraph("1.12kN");

        t.cell(2, 0).addParagraph("y=-150");
        t.cell(2, 1).addParagraph("-2.37kN"); t.cell(2, 2).addParagraph("-2.37kN");
        t.cell(2, 3).addParagraph("-2.37kN"); t.cell(2, 4).addParagraph("-2.37kN");

        t.cell(3, 0).addParagraph("y=1350");
        t.cell(3, 1).addParagraph("30.15kN"); t.cell(3, 2).addParagraph("30.15kN");
        t.cell(3, 3).addParagraph("30.15kN"); t.cell(3, 4).addParagraph("30.15kN");
    }

    // 表5
    {
        auto& t = doc.addTable(4, 5);
        t.setBorderStyle(TableStyle::Grid).setCaption("各悬挑主梁支座反力标准值");

        t.cell(0, 0).addParagraph("位置(mm)");
        t.cell(0, 1).addParagraph("x=0");    t.cell(0, 2).addParagraph("x=1200");
        t.cell(0, 3).addParagraph("x=2400"); t.cell(0, 4).addParagraph("x=3600");

        t.cell(1, 0).addParagraph("y=-1550");
        t.cell(1, 1).addParagraph("0.89kN"); t.cell(1, 2).addParagraph("0.89kN");
        t.cell(1, 3).addParagraph("0.89kN"); t.cell(1, 4).addParagraph("0.89kN");

        t.cell(2, 0).addParagraph("y=-150");
        t.cell(2, 1).addParagraph("-1.90kN"); t.cell(2, 2).addParagraph("-1.90kN");
        t.cell(2, 3).addParagraph("-1.90kN"); t.cell(2, 4).addParagraph("-1.90kN");

        t.cell(3, 0).addParagraph("y=1350");
        t.cell(3, 1).addParagraph("24.02kN"); t.cell(3, 2).addParagraph("24.02kN");
        t.cell(3, 3).addParagraph("24.02kN"); t.cell(3, 4).addParagraph("24.02kN");
    }

    // ---- 1.3 钢拉杆验算 ----
    doc.addHeading("钢拉杆验算", 2);

    // ---- 1.3.1 钢拉杆强度验算 ----
    doc.addHeading("钢拉杆强度验算", 3);

    doc.addDisplayEquation("R_{\\max}=30.15\\ \\text{kN}");
    doc.addDisplayEquation("\\gamma=\\arctan\\frac{h}{b}= \\arctan\\frac{3000}{1500}=63.43^\\circ");
    doc.addDisplayEquation("N_{\\max}=\\frac{R_{\\max}}{\\sin\\gamma}= \\frac{30.15}{\\sin63.43}=33.71\\ \\text{kN}");
    doc.addDisplayEquation("A=\\frac{\\pi\\times d^2}{4}= \\frac{3.14\\times18^2}{4}=254.47\\ \\text{mm}^2");
    doc.addDisplayEquation("\\sigma=\\frac{N_{\\max}}{A}= \\frac{33708}{254.5}=132.47\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma=132.47\\ \\text{MPa}\\le[f]=205\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.3.2 花篮螺栓强度验算 ----
    doc.addHeading("花篮螺栓强度验算", 3);

    doc.addDisplayEquation("A_e=\\frac{\\pi\\times d_e^2}{4}= \\frac{3.14\\times18^2}{4}=265.33\\ \\text{mm}^2");
    doc.addDisplayEquation("\\sigma_e=\\frac{N_{\\max}}{A_e}= \\frac{33708}{265.3}=127.04\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma_e=127.04\\ \\text{MPa}\\le[f]=170\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.4 压环钢筋强度验算 ----
    doc.addHeading("压环钢筋强度验算", 2);

    doc.addDisplayEquation("x=-1550\\ \\text{mm}\\ \\text{处支座最大拉力}\\ R_1=0.00\\ \\text{kN}");
    doc.addDisplayEquation("x=-150\\ \\text{mm}\\ \\text{处支座最大拉力}\\ R_2=2.37\\ \\text{kN}");
    doc.addDisplayEquation("T=\\max\\left(\\frac{R_1}{2},R_2\\right)\\times0.5=1.18\\ \\text{kN}");
    doc.addDisplayEquation("\\sigma=\\frac{T}{0.25\\times\\pi\\times d^2}= \\frac{1183}{0.25\\times3.14\\times16^2}=5.88\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma=5.88\\ \\text{MPa}\\le[f]\\times0.85=55.25\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}").setStyle(RunStyle().color("0000FF").bold().fontSize(12));

    // ---- 1.5 吊耳板验算 ----
    doc.addHeading("吊耳板验算", 2);

    doc.addDisplayEquation("N_d=\\frac{N}{2}=16.85\\ \\text{kN}");
    doc.addDisplayEquation("\\gamma=63.43^\\circ");

    // ---- 1.5.1 吊耳板构造验算 ----
    doc.addHeading("吊耳板构造验算", 3);

    // 图8
    doc.addImage("examples/res/image8.png")
       .setCaption("吊耳板示意")
       .setAlignment(Alignment::Center);

    doc.addDisplayEquation("B_e=2\\times t+16=2\\times12+16=40\\ \\text{mm}");
    doc.addDisplayEquation("B_e=40\\ \\text{mm}\\le b=50\\ \\text{mm}");
    doc.addDisplayEquation("\\text{满足要求!}");
    doc.addDisplayEquation("\\frac{4\\times B_e}{3}=\\frac{4\\times40}{3}=53.33\\ \\text{mm}");
    doc.addDisplayEquation("\\frac{4\\times B_e}{3}=53.33\\ \\text{mm}\\le a=65\\ \\text{mm}");
    doc.addParagraph().addRun("满足要求!", RunStyle().color("0000FF").bold());

    // ---- 1.5.2 耳板孔净截面处的抗拉强度验算 ----
    doc.addHeading("耳板孔净截面处的抗拉强度验算", 3);

    doc.addDisplayEquation("b_1=\\min\\left(B_e,b-\\frac{d_0}{3}\\right)= \\min\\left(40,50-\\frac{25}{3}\\right)=40.0\\ \\text{mm}");
    doc.addDisplayEquation("\\sigma_1=\\frac{N_d}{2\\times t\\times b_1}= \\frac{16854}{2\\times12\\times40.0}=17.56\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma_1=17.56\\ \\text{MPa}\\le[f_t]=205\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}").getStyle().color("0000FF");

    // ---- 1.5.3 耳板端部截面抗拉强度验算 ----
    doc.addHeading("耳板端部截面抗拉强度验算", 3);

    doc.addDisplayEquation("b_2=a-\\frac{2\\times d_0}{3}=65-\\frac{2\\times25}{3}=48.3\\ \\text{mm}");
    doc.addDisplayEquation("\\sigma_2=\\frac{N_d}{2\\times t\\times b_2}= \\frac{16854}{2\\times12\\times48.3}=14.53\\ \\text{MPa}");
    doc.addDisplayEquation("\\sigma_2=14.53\\ \\text{MPa}\\le[f_t]=205\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.5.4 耳板抗剪强度验算 ----
    doc.addHeading("耳板抗剪强度验算", 3);

    doc.addDisplayEquation("Z=\\sqrt{\\left(a+0.5\\times d_0\\right)^2-\\left(0.5\\times d_0\\right)^2}");
    doc.addDisplayEquation("Z=\\sqrt{\\left(65+0.5\\times25\\right)^2-\\left(0.5\\times25\\right)^2}=76.5\\ \\text{mm}");
    doc.addDisplayEquation("\\tau=\\frac{N_d}{2\\times t\\times Z}= \\frac{16854}{2\\times12\\times76.5}=9.18\\ \\text{MPa}");
    doc.addDisplayEquation("\\tau=9.18\\ \\text{MPa}\\le[f_v]=125\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}").getStyle().color("0000FF");

    // ---- 1.5.5 耳板与主梁焊缝强度验算 ----
    doc.addHeading("耳板与主梁焊缝强度验算", 3);

    doc.addDisplayEquation("N'=N\\times\\sin\\gamma=16.85\\times\\sin63.43=15.07\\ \\text{kN}");
    doc.addDisplayEquation("V'=N\\times\\cos\\gamma=16.85\\times\\cos63.43=7.54\\ \\text{kN}");
    doc.addDisplayEquation("\\sigma_f=\\frac{N'}{0.7\\times h_f\\times l_{w1}}= \\frac{15.07}{0.7\\times8.0\\times120}=22.43\\ \\text{MPa}");
    doc.addDisplayEquation("\\tau_{f1}=\\frac{V'}{0.7\\times h_f\\times l_{w1}}= \\frac{7.54}{0.7\\times8.0\\times120}=11.22\\ \\text{MPa}");
    doc.addDisplayEquation("\\sqrt{\\left(\\frac{\\sigma_f}{\\beta_f}\\right)^2+\\tau_{f1}^2}= \\sqrt{\\left(\\frac{22.43}{1.22}\\right)^2+11.22^2}=21.54\\ \\text{MPa}");
    doc.addDisplayEquation("21.54\\ \\text{MPa}\\le[f_f]=160\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}").getStyle().color("0000FF");

    // ---- 1.5.6 拉杆与端板焊缝强度验算 ----
    doc.addHeading("拉杆与端板焊缝强度验算", 3);

    doc.addDisplayEquation("\\tau_{f2}=\\frac{N}{0.7\\times h_f\\times l_{w2}}= \\frac{33708.37}{0.7\\times8.0\\times120}=50.16\\ \\text{MPa}");
    doc.addDisplayEquation("\\tau_{f2}=50.16\\ \\text{MPa}\\le[f_f]=160\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}").getStyle().color("0000FF");

    // ---- 1.5.7 拉杆与耳板销轴抗剪验算 ----
    doc.addHeading("拉杆与耳板销轴抗剪验算", 3);

    doc.addDisplayEquation("\\tau_b=\\frac{N}{n\\times0.25\\times\\pi\\times d^2}= \\frac{33708}{1\\times0.25\\times3.14\\times22^2}=44.34\\ \\text{MPa}");
    doc.addDisplayEquation("\\tau_b=44.34\\ \\text{MPa}\\le[f_{vb}]=125\\ \\text{MPa}");
    doc.addDisplayEquation("\\text{满足要求!}");

    // ---- 1.6 杆件与建筑螺栓连接验算 ----
    doc.addHeading("杆件与建筑螺栓连接验算", 2);

    doc.addDisplayEquation("V=\\frac{N_v}{n}=\\frac{30.15}{4}=7.54\\ \\text{kN}");
    doc.addDisplayEquation("T=\\frac{N_t}{n}=\\frac{15.07}{4}=3.77\\ \\text{kN}");
    doc.addDisplayEquation("N_{vb}=\\frac{\\pi\\times d_e^2\\times f_{vb}}{4}= \\frac{3.14\\times14.1^2\\times140}{4\\times1000}=21.92\\ \\text{kN}");
    doc.addDisplayEquation("N_{tb}=\\frac{\\pi\\times d_e^2\\times f_{tb}}{4}= \\frac{3.14\\times14.1^2\\times170}{4\\times1000}=26.62\\ \\text{kN}");
    doc.addDisplayEquation("\\sqrt{\\left(\\frac{V}{N_{vb}}\\right)^2+\\left(\\frac{T}{N_{tb}}\\right)^2}= \\sqrt{\\left(\\frac{7.54}{21.92}\\right)^2+\\left(\\frac{3.77}{26.62}\\right)^2}=0.372");
    doc.addDisplayEquation("0.372\\le1.0");
    doc.addDisplayEquation("\\text{满足要求!}").getStyle().color("0000FF");

    // ---- 1.7 计算结果汇总 ----
    doc.addHeading("计算结果汇总", 2);

    // 表6
    {
        auto& t = doc.addTable(18, 5);
        t.setBorderStyle(TableStyle::Grid).setCaption("计算结果总览表");

        t.cell(0, 0).addParagraph("验算项目");
        t.cell(0, 1).addParagraph("验算内容");
        t.cell(0, 2).addParagraph("计算值");
        t.cell(0, 3).addParagraph("允许值");
        t.cell(0, 4).addParagraph("结论");

        // Row 1
        t.cell(1, 0).addParagraph("悬挑主梁");
        t.cell(1, 1).addParagraph("抗弯");
        t.cell(1, 2).addEquation("\\sigma = 53.46\\ \\text{MPa}");
        t.cell(1, 3).addEquation("[f]=205\\ \\text{MPa}");
        t.cell(1, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 2
        t.cell(2, 0).addParagraph("");
        t.cell(2, 1).addParagraph("压弯");
        t.cell(2, 2).addEquation("\\sigma_p = 55.82\\ \\text{MPa}");
        t.cell(2, 3).addEquation("[f]=205\\ \\text{MPa}");
        t.cell(2, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 3
        t.cell(3, 0).addParagraph("");
        t.cell(3, 1).addParagraph("抗剪");
        t.cell(3, 2).addEquation("\\tau = 25.67\\ \\text{MPa}");
        t.cell(3, 3).addEquation("[f]=125\\ \\text{MPa}");
        t.cell(3, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 4
        t.cell(4, 0).addParagraph("");
        t.cell(4, 1).addParagraph("整体稳定");
        t.cell(4, 2).addEquation("\\sigma_s = 59.25\\ \\text{MPa}");
        t.cell(4, 3).addEquation("[f]=205\\ \\text{MPa}");
        t.cell(4, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 5
        t.cell(5, 0).addParagraph("");
        t.cell(5, 1).addParagraph("挠度");
        t.cell(5, 2).addEquation("\\delta = 2.5\\ \\text{mm}");
        t.cell(5, 3).addEquation("");
        t.cell(5, 4).addParagraph("满足要求!").getStyle().color("0000FF");

        // Row 6
        t.cell(6, 0).addParagraph("钢拉杆");
        t.cell(6, 1).addParagraph("强度");
        t.cell(6, 2).addEquation("");
        t.cell(6, 3).addEquation("");
        t.cell(6, 4).addParagraph("满足要求!").getStyle().color("0000FF");

        // Row 7
        t.cell(7, 0).addParagraph("花篮螺栓");
        t.cell(7, 1).addParagraph("强度");
        t.cell(7, 2).addEquation("");
        t.cell(7, 3).addEquation("");
        t.cell(7, 4).addParagraph("满足要求!").getStyle().color("0000FF");

        // Row 8
        t.cell(8, 0).addParagraph("压环钢筋");
        t.cell(8, 1).addParagraph("强度");
        t.cell(8, 2).addEquation("");
        t.cell(8, 3).addEquation("");
        t.cell(8, 4).addParagraph("满足要求!").getStyle().color("0000FF");

        // Row 9
        t.cell(9, 0).addParagraph("吊耳板");
        t.cell(9, 1).addParagraph("构造");
        t.cell(9, 2).addEquation("");
        t.cell(9, 3).addEquation("");
        t.cell(9, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 10
        t.cell(10, 0).addParagraph("");
        t.cell(10, 1).addParagraph("构造");
        t.cell(10, 2).addEquation("");
        t.cell(10, 3).addEquation("");
        t.cell(10, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 11
        t.cell(11, 0).addParagraph("");
        t.cell(11, 1).addParagraph("净截面抗拉");
        t.cell(11, 2).addEquation("");
        t.cell(11, 3).addEquation("");
        t.cell(11, 4).addParagraph("满足要求!").getStyle().color("0000FF");
            
        // Row 12
        t.cell(12, 0).addParagraph("");
        t.cell(12, 1).addParagraph("端部抗拉");
        t.cell(12, 2).addEquation("");
        t.cell(12, 3).addEquation("");
        t.cell(12, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 13
        t.cell(13, 0).addParagraph("");
        t.cell(13, 1).addParagraph("端部抗剪");
        t.cell(13, 2).addEquation("");
        t.cell(13, 3).addEquation("");
        t.cell(13, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 14
        t.cell(14, 0).addParagraph("");
        t.cell(14, 1).addParagraph("耳板主梁焊缝");
        t.cell(14, 2).addEquation("");
        t.cell(14, 3).addEquation("");
        t.cell(14, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 15
        t.cell(15, 0).addParagraph("");
        t.cell(15, 1).addParagraph("拉杆端板焊缝");
        t.cell(15, 2).addEquation("");
        t.cell(15, 3).addEquation("");
        t.cell(15, 4).addParagraph("满足要求!").getStyle().color("0000FF");
        // Row 16
        t.cell(16, 0).addParagraph("");
        t.cell(16, 1).addParagraph("销轴抗剪");
        t.cell(16, 2).addEquation("");
        t.cell(16, 3).addEquation("");
        t.cell(16, 4).addParagraph("满足要求!").getStyle().color("0000FF");

        // Row 17
        t.cell(17, 0).addParagraph("杆件建筑连接");
        t.cell(17, 1).addParagraph("螺栓");
        t.cell(17, 2).addEquation("0.372");
        t.cell(17, 3).addEquation("1.0");
        t.cell(17, 4).addParagraph("满足要求!").getStyle().color("0000FF");

        t.mergeCells(1, 0, 5, 0);
        t.mergeCells(9, 0, 16, 0);
    }

    // ---- Save ----
    std::string outputPath = "platform_output.docx";
    if (doc.save(outputPath)) {
        std::cout << "Document saved to: " << outputPath << std::endl;

        // Open the generated file
#ifdef _WIN32
        system(("start \"\" \"" + outputPath + "\"").c_str());
#elif __APPLE__
        system(("open \"" + outputPath + "\"").c_str());
#else
        system(("xdg-open \"" + outputPath + "\"").c_str());
#endif

    } else {
        std::cerr << "Failed to save document!" << std::endl;
        return 1;
    }

    return 0;
}
