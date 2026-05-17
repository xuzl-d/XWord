#include "xword/xword.hpp"
#include <iostream>
#include <cmath>

int main() {
    using namespace xword;

    // ---- Step 1: create a template.docx using the programmatic API ----
    {
        Document gen;

        // A header with a placeholder
        gen.setHeader("${reportTitle}");

        // A paragraph with mixed text and placeholders
        gen.addParagraph("报告日期：${date}");

        gen.addParagraph("尊敬的${name}：");
        gen.addParagraph();

        // Conditional block: show detail only when flag is set
        gen.addParagraph("{%if showDetail%}");
        gen.addParagraph("详细信息：总金额 ${amount} 万元。");
        gen.addParagraph("占比：${ratio}%。");
        gen.addParagraph("{%else%}");
        gen.addParagraph("（摘要模式，不含明细）");
        gen.addParagraph("{%endif%}");

        gen.addParagraph();
        gen.addParagraph("此致");
        gen.addParagraph("敬礼");

        // Footer with page numbers
        gen.setFooter()
            .addRun("第 ").addPageNumber()
            .addRun(" 页 / 共 ").addPageCount().addRun(" 页")
            .setAlignment(Alignment::Center);

        if (!gen.save("__template.docx")) {
            std::cerr << "FAILED to create template\n";
            return 1;
        }
        std::cout << "Template created.\n";
    }

    // ---- Step 2: open template, fill data, save rendered result ----
    {
        Document doc;

        if (!doc.open("__template.docx")) {
            std::cerr << "FAILED to open template\n";
            return 1;
        }

        doc.set("reportTitle", "季度报告");
        doc.set("date", "2026-05-17");
        doc.set("name", "张三");
        doc.set("amount", "128.50");
        doc.set("ratio", 45.67, 1);   // double with 1 decimal
        doc.set("showDetail", true);  // enable detail block

        if (!doc.save("__rendered_true.docx")) {
            std::cerr << "FAILED to render true branch\n";
            return 1;
        }
        std::cout << "Rendered with showDetail=true.\n";
    }

    // ---- Step 3: render with condition false ----
    {
        Document doc;
        if (!doc.open("__template.docx")) {
            std::cerr << "FAILED to open template (false branch)\n";
            return 1;
        }
        doc.set("reportTitle", "季度报告");
        doc.set("date", "2026-05-17");
        doc.set("name", "李四");
        doc.set("amount", "0.00");
        doc.set("ratio", 0.0, 1);
        doc.set("showDetail", false); // hide detail block

        if (!doc.save("__rendered_false.docx")) {
            std::cerr << "FAILED to render false branch\n";
            return 1;
        }
        std::cout << "Rendered with showDetail=false.\n";
    }

    std::cout << "All template rendering steps completed.\n";
    return 0;
}
