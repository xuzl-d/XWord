#include "xword/xword.hpp"
#include <cassert>
#include <iostream>
#include <filesystem>

int main() {
    using namespace xword;

    // Test 1: Basic document creation
    {
        Document doc;
        doc.addHeading("Test Heading", 1);
        doc.addParagraph("Hello World");
        assert(doc.save("test_basic.docx"));
        std::cout << "Test 1 passed: Basic document\n";
    }

    // Test 2: Rich text
    {
        Document doc;
        doc.addParagraph()
            .addRun("Bold text", RunStyle().bold())
            .addRun("Normal text");
        assert(doc.save("test_richtext.docx"));
        std::cout << "Test 2 passed: Rich text\n";
    }

    // Test 3: Table
    {
        Document doc;
        auto& table = doc.addTable(2, 2);
        table.setStyle(TableStyle::Grid);
        table.cell(0, 0).addParagraph("A");
        table.cell(0, 1).addParagraph("B");
        table.cell(1, 0).addParagraph("C");
        table.cell(1, 1).addParagraph("D");
        assert(doc.save("test_table.docx"));
        std::cout << "Test 3 passed: Table\n";
    }

    // Test 4: Page settings
    {
        Document doc;
        doc.setPage(Page().setSize(PageSize::Letter).setOrientation(Orientation::Landscape));
        doc.addParagraph("Landscape letter page");
        assert(doc.save("test_page.docx"));
        std::cout << "Test 4 passed: Page settings\n";
    }

    // Test 5: Lists
    {
        Document doc;
        doc.addBulletList().addItem("Item A").addItem("Item B");
        doc.addOrderedList().addItem("Step 1").addItem("Step 2");
        assert(doc.save("test_lists.docx"));
        std::cout << "Test 5 passed: Lists\n";
    }

    // Test 6: Equations
    {
        // Basic inline equation
        auto eq1 = Equation("x^2", EquationMode::Inline);
        std::string xml1 = eq1.toXml();
        assert(xml1.find("<m:oMath>") != std::string::npos);
        assert(xml1.find("<m:sSup>") != std::string::npos);
        std::cout << "Test 6a passed: Inline superscript\n";

        // Fraction
        auto eq2 = Equation("\\frac{a}{b}", EquationMode::Inline);
        std::string xml2 = eq2.toXml();
        assert(xml2.find("<m:f>") != std::string::npos);
        std::cout << "Test 6b passed: Fraction\n";

        // Sum with body
        auto eq3 = Equation("\\sum_{i=1}^{n} x_i", EquationMode::Display);
        std::string xml3 = eq3.toXml();
        assert(xml3.find("<m:nary>") != std::string::npos);
        assert(xml3.find("<m:sSub>") != std::string::npos);
        // Verify x_i appears in the output
        assert(xml3.find("x</m:t>") != std::string::npos);
        assert(xml3.find("i</m:t>") != std::string::npos);
        std::cout << "Test 6c passed: Sum with body x_i\n";

        // Display equation
        auto eq4 = Equation("E=mc^2", EquationMode::Display);
        std::string xml4 = eq4.toXml();
        assert(xml4.find("<m:oMathPara>") != std::string::npos);
        std::cout << "Test 6d passed: Display equation\n";

        // Matrix
        auto eq5 = Equation("\\begin{pmatrix} a & b \\\\ c & d \\end{pmatrix}", EquationMode::Display);
        std::string xml5 = eq5.toXml();
        assert(xml5.find("<m:m>") != std::string::npos);
        std::cout << "Test 6e passed: Matrix\n";

        // Greek symbols
        auto eq6 = Equation("\\alpha + \\beta = \\gamma", EquationMode::Inline);
        std::string xml6 = eq6.toXml();
        assert(!xml6.empty());
        std::cout << "Test 6f passed: Greek symbols\n";
    }

    // Cleanup
    std::filesystem::remove("test_basic.docx");
    std::filesystem::remove("test_richtext.docx");
    std::filesystem::remove("test_table.docx");
    std::filesystem::remove("test_page.docx");
    std::filesystem::remove("test_lists.docx");

    std::cout << "All tests passed!\n";
    return 0;
}
